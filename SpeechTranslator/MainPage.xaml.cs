// Copyright (c) Microsoft. All rights reserved.

using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

using Windows.Media.SpeechRecognition;
using Windows.Media.SpeechSynthesis;
using Windows.UI.Core;

using Windows.Networking;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using System.Text;



// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409
using Windows.ApplicationModel.Core;
using Windows.Devices.Gpio;
using Windows.Globalization;
using System.Threading.Tasks;

namespace SpeechTranslator
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        // Speech events may come in on a thread other than the UI thread, keep track of the UI thread's
        // dispatcher, so we can update the UI in a thread-safe manner.
        private CoreDispatcher dispatcher;

        // Keep track of whether the continuous recognizer is currently running, so it can be cleaned up appropriately.
        private bool isListening;
        private StringBuilder dictatedTextBuilder = new StringBuilder();

        //Steam Socket Parameters
        private StreamSocketListener listener;
        public StreamSocket clientSocket;
        private bool connected = false;


        /// <summary>
        /// GPIO Section
        /// </summary>
        // MBM Pin Parameter
        const int switchPinNum1 = 2;
        const int switchPinNum2 = 9;

        ////// Rpi2 Pin Parameter
        //const int switchPinNum1 = 5;
        //const int switchPinNum2 = 13;

        public static GpioController gpioController = null;
        public static GpioPin switchPin1 = null;
        public static GpioPin switchPin2 = null;

        public static Windows.Globalization.Language en_lang = new Windows.Globalization.Language("en-US");
        public static Windows.Globalization.Language ch_lang = new Windows.Globalization.Language("zh-CN");

        public static Language selectedLang;
        public static SpeechRecognizer speechRecognizer;
        public static SpeechSynthesizer synthesizer;
        public MainPage()
        {
            this.InitializeComponent();
            listener = new StreamSocketListener();
            startListener();
            InitGpio();

        }
        public async void InitGpio()
        {
            gpioController = GpioController.GetDefault();
            try
            {
                switchPin1 = gpioController.OpenPin(switchPinNum1);
                switchPin2 = gpioController.OpenPin(switchPinNum2);

                switchPin1.SetDriveMode(GpioPinDriveMode.Input);
                switchPin2.SetDriveMode(GpioPinDriveMode.Input);

                GpioPinValue v1 = switchPin1.Read();
                GpioPinValue v2 = switchPin2.Read();

                await startSRProcess();

                switchPin1.DebounceTimeout = new TimeSpan(0, 0, 0, 1, 0);

                // value change
                switchPin1.ValueChanged += async (s, e) =>
                {
                    if (isListening)
                    {
                        try
                        {
                            await speechRecognizer.ContinuousRecognitionSession.StopAsync();
                        }
                        catch
                        {

                        }

                    }

                    await startSRProcess();

                };

            }
            catch (Exception)
            {
            }
        }
        public async Task selectLang()
        {
            if (switchPin1.Read() == GpioPinValue.Low)
            {
                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    LangText.Text = "en";
                    selectedLang = en_lang;
                });
            }
            else
            {
                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    LangText.Text = "zh-cn";
                    selectedLang = ch_lang;
                });
            }
        }



        public async Task startSRProcess()
        {

            //Set Initial
            if (switchPin1.Read() == GpioPinValue.Low)
            {
                selectedLang = en_lang;
            }
            else
            {
                selectedLang = ch_lang;
            }

            // The speech recognizer used throughout this sample.
            if (selectedLang.LanguageTag.Contains("zh"))
            {
                try
                {
                    speechRecognizer = new SpeechRecognizer(ch_lang);
                }
                catch
                {
                }

            }
            else
            {
                try
                {
                    speechRecognizer = new SpeechRecognizer(en_lang);
                }
                catch (Exception ex)
                {
                    string mg = ex.Message;
                }

            }

            synthesizer = new SpeechSynthesizer();

            isListening = false;
            dispatcher = this.Dispatcher;

            // select the language display
            var voices = Windows.Media.SpeechSynthesis.SpeechSynthesizer.AllVoices;
            foreach (VoiceInformation voice in voices)
            {
                if ((voice.Language.Contains("zh") && selectedLang.LanguageTag.Contains("zh")) || voice.Language.Contains("en") && selectedLang.LanguageTag.Contains("en"))
                {
                    synthesizer.Voice = voice;
                    break;
                }
            }
            await GotoNavigation();
        }

        /// <summary>
        /// Function Section
        /// 
        /// </summary>
        private async void startListener()
        {
            if (String.IsNullOrEmpty(ConstantParam.SelfPort))
            {
                StatusText.Text = "Please provide a service port.";
                return;
            }
            listener.ConnectionReceived += OnConnection;

            try
            {
                await listener.BindServiceNameAsync(ConstantParam.SelfPort);
                StatusText.Text = "Listening on port: " + ConstantParam.SelfPort;
                ReceivedText.Text = "";
            }
            catch (Exception exception)
            {
                if (SocketError.GetStatus(exception.HResult) == SocketErrorStatus.Unknown)
                {
                    throw;
                }
                StatusText.Text = "Start listening failed with error: " + exception.Message;
            }
        }
        private async void OnConnection(StreamSocketListener sender, StreamSocketListenerConnectionReceivedEventArgs args)
        {

            DataReader reader = new DataReader(args.Socket.InputStream);
            try
            {
                while (true)
                {
                    // Read first 4 bytes (length of the subsequent string).
                    uint sizeFieldCount = await reader.LoadAsync(sizeof(uint));
                    if (sizeFieldCount != sizeof(uint))
                    {
                        // The underlying socket was closed before we were able to read the whole data.
                        return;
                    }

                    // Read the string.
                    uint stringLength = reader.ReadUInt32();
                    uint actualStringLength = await reader.LoadAsync(stringLength);
                    if (stringLength != actualStringLength)
                    {
                        // The underlying socket was closed before we were able to read the whole data.
                        return;
                    }

                    // Display the string.
                    string text = reader.ReadString(actualStringLength);

                    // Get the language and the content received
                    char delimiter = ':';
                    string[] res;
                    res = text.Split(delimiter);
                    string InLang = "en";
                    if (res[0].Contains("zh"))
                    {
                        InLang = "zh-CHS";
                    }
                    string outLang = "en";
                    if (selectedLang.LanguageTag.Contains("zh"))
                    {
                        outLang = "zh-CHS";
                    }
                    string content = res[1];

                    Translator Trans = new Translator(content, InLang, outLang);
                    string translatedS = Trans.GetTranslatedString();

                    SpeechSynthesisStream stream = await synthesizer.SynthesizeTextToStreamAsync(translatedS);
                    var ignored = Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                    {
                        media.SetSource(stream, stream.ContentType);
                        media.Play();
                        ReceivedText.Text = text;
                    });
                }
            }
            catch (Exception exception)
            {
                // If this is an unknown status it means that the error is fatal and retry will likely fail.
                if (SocketError.GetStatus(exception.HResult) == SocketErrorStatus.Unknown)
                {
                    throw;
                }
                var ignored = Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                {
                    StatusText.Text = "Read stream failed with error: " + exception.Message;
                    StatusText.Text += "Reconnecting Later!";
                    connected = false;
                    CoreApplication.Properties.Remove("clientSocket");
                    CoreApplication.Properties.Remove("clientDataWriter");
                });
            }
        }
        private async void SendDataToHost(string dataToBeSent)
        {
            if (!connected)
            {
                if (!CoreApplication.Properties.ContainsKey("clientSocket"))
                {
                    HostName hostName;
                    try
                    {
                        hostName = new HostName(ConstantParam.ServerHostname);
                    }
                    catch (ArgumentException)
                    {
                        return;
                    }

                    StreamSocket locsocket = new StreamSocket();
                    // save the soket so subsequence can use it
                    CoreApplication.Properties.Add("clientSocket", locsocket);
                    try
                    {
                        await locsocket.ConnectAsync(hostName, ConstantParam.ClientPort);
                        await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                        {
                            StatusText.Text += "Connected!";
                        });
                        connected = true;
                    }
                    catch (Exception exception)
                    {
                        // If this is an unknown status it means that the error is fatal and retry will likely fail.
                        if (SocketError.GetStatus(exception.HResult) == SocketErrorStatus.Unknown)
                        {
                            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                            {
                                StatusText.Text += "Connect to host failed";
                            });

                            return;
                        }

                    }
                }
            }
            // If the connection was not setup still, we return instead of continuing to send data which will break the code.
            if (!connected)
            {
                return;
            }
            object outValue;
            StreamSocket socket;
            if (!CoreApplication.Properties.TryGetValue("clientSocket", out outValue))
            {
                return;
            }
            socket = (StreamSocket)outValue;
            // Create a DataWriter if we did not create one yet. Otherwise use one that is already cached.
            DataWriter writer;
            if (!CoreApplication.Properties.TryGetValue("clientDataWriter", out outValue))
            {
                writer = new DataWriter(socket.OutputStream);
                CoreApplication.Properties.Add("clientDataWriter", writer);
            }
            else
            {
                writer = (DataWriter)outValue;
            }

            // Write first the length of the string as UINT32 value followed up by the string. 
            // Writing data to the writer will just store data in memory.
            string stringToSend = dataToBeSent;
            writer.WriteUInt32(writer.MeasureString(stringToSend));
            writer.WriteString(stringToSend);

            // Write the locally buffered data to the network.
            try
            {
                await writer.StoreAsync();
            }
            catch (Exception)
            {
            }
        }
        private async Task startDictate()
        {
            if (speechRecognizer.State == SpeechRecognizerState.Idle)
            {
                try
                {
                    isListening = true;
                    await speechRecognizer.ContinuousRecognitionSession.StartAsync();
                }
                catch (Exception ex)
                {
                    string mg = ex.Message;
                }

            }
        }
        private async Task GotoNavigation()
        {
            // Prompt the user for permission to access the microphone. This request will only happen
            // once, it will not re-prompt if the user rejects the permission.
            bool permissionGained = await AudioCapturePermissions.RequestMicrophonePermission();
            if (!permissionGained)
            {
                this.dictationTextBox.Text = "Permission to access capture resources was not given by the user, reset the application setting in Settings->Privacy->Microphone.";
            }

            // Apply the dictation topic constraint to optimize for dictated freeform speech.
            var dictationConstraint = new SpeechRecognitionTopicConstraint(SpeechRecognitionScenario.Dictation, "dictation");
            speechRecognizer.Constraints.Add(dictationConstraint);

            try
            {
                await speechRecognizer.CompileConstraintsAsync();
            }
            catch (Exception ex)
            {
                string mg = ex.Message;
            }

            // handle contiuous recognition events. Completed filres when various error states occur. ResultGenerated fires when
            // some recognized phrases occur, or hte garbate rule is hit. HypothesisGenerated fires during recognition, and 
            // allow us to rovide incremental feedback based on what the users' currently saying
            speechRecognizer.ContinuousRecognitionSession.Completed += ContinuousRecognitionSession_Completed;
            speechRecognizer.ContinuousRecognitionSession.ResultGenerated += ContinuousRecognitionSession_ResultGenerated;
            speechRecognizer.HypothesisGenerated += SpeechRecognizer_HypothesisGenerated;
            await startDictate();
        }
        private async void ContinuousRecognitionSession_Completed(SpeechContinuousRecognitionSession sender, SpeechContinuousRecognitionCompletedEventArgs args)
        {

            if (args.Status != SpeechRecognitionResultStatus.Success)
            {
                await startSRProcess();
            }
        }

        /// <summary>
        /// Handle events fired when a result is generated. Check for high to medium confidence, and then append the
        /// string to the end of the stringbuffer, and replace the content of the textbox with the string buffer, to
        /// remove any hypothesis text that may be present.
        /// </summary>
        /// <param name="sender">The Recognition session that generated this result</param>
        /// <param name="args">Details about the recognized speech</param>
        private async void ContinuousRecognitionSession_ResultGenerated(SpeechContinuousRecognitionSession sender, SpeechContinuousRecognitionResultGeneratedEventArgs args)
        {
            // We may choose to discard content that has low confidence, as that could indicate that we're picking up
            // noise via the microphone, or someone could be talking out of earshot.

            // Speak it out

            string s = args.Result.Text;
            string lang = selectedLang.LanguageTag + ":";

            //Send the Data
            SendDataToHost(lang + s);

            if (args.Result.Status == SpeechRecognitionResultStatus.Success)
            {
                dictatedTextBuilder.Append(s + " ");
                await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    dictationTextBox.Text = dictatedTextBuilder.ToString();
                    btnClearText.IsEnabled = true;
                });
            }
            else
            {
                // In some scenarios, a developer may choose to ignore giving the user feedback in this case, if speech
                // is not the primary input mechanism for the application.
                // Here, just remove any hypothesis text by resetting it to the last known good.
                await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    dictationTextBox.Text = dictatedTextBuilder.ToString();
                });
            }

        }

        /// <summary>
        /// While the user is speaking, update the textbox with the partial sentence of what's being said for user feedback.
        /// </summary>
        /// <param name="sender">The recognizer that has generated the hypothesis</param>
        /// <param name="args">The hypothesis formed</param>
        private async void SpeechRecognizer_HypothesisGenerated(SpeechRecognizer sender, SpeechRecognitionHypothesisGeneratedEventArgs args)
        {
            string hypothesis = args.Hypothesis.Text;

            // Update the textbox with the currently confirmed text, and the hypothesis combined.
            string textboxContent = dictatedTextBuilder.ToString() + " " + hypothesis + " ...";

            await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                dictationTextBox.Text = textboxContent;
                btnClearText.IsEnabled = true;
            });
        }
        private void SpeechRecognizer_StateChanged(SpeechRecognizer sender, SpeechRecognizerStateChangedEventArgs args)
        {
            //await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            //{
            //    MainPage.Current.NotifyUser("Speech recognizer state: " + args.State.ToString(), NotifyType.StatusMessage);
            //});
        }
        private async void ContinuousRecognize_Click(object sender, RoutedEventArgs e)
        {
            if (isListening == false)
            {

                if (speechRecognizer.State == SpeechRecognizerState.Idle)
                {
                    try
                    {
                        isListening = true;
                        await speechRecognizer.ContinuousRecognitionSession.StartAsync();
                    }
                    catch (Exception ex)
                    {
                        string mg = ex.Message;
                    }
                }
            }
            else
            {
                isListening = false;
                if (speechRecognizer.State != SpeechRecognizerState.Idle)
                {
                    // Cancelling recognition prevents any currently recognized speech from
                    // generating a ResultGenerated event. StopAsync() will allow the final session to 
                    // complete.
                    await speechRecognizer.ContinuousRecognitionSession.StopAsync();

                    // Ensure we don't leave any hypothesis text behind
                    dictationTextBox.Text = dictatedTextBuilder.ToString();
                }
            }
        }
        private void btnClearText_Click(object sender, RoutedEventArgs e)
        {
            btnClearText.IsEnabled = false;
            dictatedTextBuilder.Clear();
            dictationTextBox.Text = "";
        }
        private void dictationTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            var grid = (Grid)VisualTreeHelper.GetChild(dictationTextBox, 0);
            for (var i = 0; i <= VisualTreeHelper.GetChildrenCount(grid) - 1; i++)
            {
                object obj = VisualTreeHelper.GetChild(grid, i);
                if (!(obj is ScrollViewer))
                {
                    continue;
                }

                ((ScrollViewer)obj).ChangeView(0.0f, ((ScrollViewer)obj).ExtentHeight, 1.0f);
                break;
            }
        }
    }
}
