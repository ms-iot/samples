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
using Windows.ApplicationModel.Core;
using Windows.Globalization;
using System.Threading.Tasks;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

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

        private bool isListening=false;
        private StringBuilder dictatedTextBuilder = new StringBuilder();

        //Steam Socket Parameters
        private StreamSocketListener listener=new StreamSocketListener();
        public StreamSocket clientSocket;
        private bool connected = false;

        public static Language en_lang = new Language("en-US");
        public static Language ch_lang = new Language("zh-CN");

        public static Language InputLang=en_lang;
        public static Language OutputLang = ch_lang;
        public static string synLangStr = "zh";

        private SpeechRecognizer speechRecognizer;
        private SpeechSynthesizer synthesizer;

        private static uint HResultPrivacyStatementDeclined = 0x80045509;

        public MainPage()
        {
            this.InitializeComponent();
            startListener();
            InitRecogAndSyn();

        }
        private async void startListener()
        {
            if (String.IsNullOrEmpty(ConstantParam.SelfPort))
            {
                StatusText.Text = "Please provide a service port for listening.";
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

                    string InLang ="en";
                    string outLang = "zh-CHS";
                    Translator Trans = new Translator(text, InLang, outLang);
                    string translatedS = Trans.GetTranslatedString();

                    SpeechSynthesisStream stream = await synthesizer.SynthesizeTextToStreamAsync(translatedS);
                    var ignored = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                    {
                        media.SetSource(stream, stream.ContentType);
                        media.Play();
                        originalmsg.Text = text;
                        ReceivedText.Text = translatedS;
                        ReceivedText.FontSize = 20;
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
                var ignored = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    StatusText.Text = "Error in Socket reading data from Remote host: " + exception.Message;
                    connected = false;
                    CoreApplication.Properties.Remove("clientSocket");
                    CoreApplication.Properties.Remove("clientDataWriter");
                });
            }
        }
        public async void InitRecogAndSyn()
        {
            await InitializeRecognizer(en_lang);
            await Initializesynthesizer();
        }
        private async Task InitializeRecognizer(Language recognizerLanguage)
        {
            if (speechRecognizer != null)
            {
                speechRecognizer.ContinuousRecognitionSession.Completed -= ContinuousRecognitionSession_Completed;
                speechRecognizer.ContinuousRecognitionSession.ResultGenerated -= ContinuousRecognitionSession_ResultGenerated;
                speechRecognizer.HypothesisGenerated -= SpeechRecognizer_HypothesisGenerated;
                speechRecognizer.Dispose();
                speechRecognizer = null;

            }
            speechRecognizer = new SpeechRecognizer(recognizerLanguage);
            var dictationConstraint = new SpeechRecognitionTopicConstraint(SpeechRecognitionScenario.Dictation, "dictation");
            speechRecognizer.Constraints.Add(dictationConstraint);
            SpeechRecognitionCompilationResult result = await speechRecognizer.CompileConstraintsAsync();
            if (result.Status != SpeechRecognitionResultStatus.Success)
            {
                checkError.Visibility = Visibility.Visible;
                errorCheck.Visibility = Visibility.Visible;
                errorCheck.Text = "Recognition Failed!";
            }

            // Handle continuous recognition events. Completed fires when various error states occur. ResultGenerated fires when
            // some recognized phrases occur, or the garbage rule is hit. HypothesisGenerated fires during recognition, and
            // allows us to provide incremental feedback based on what the user's currently saying.
            speechRecognizer.ContinuousRecognitionSession.Completed += ContinuousRecognitionSession_Completed;
            speechRecognizer.ContinuousRecognitionSession.ResultGenerated += ContinuousRecognitionSession_ResultGenerated;
            speechRecognizer.HypothesisGenerated += SpeechRecognizer_HypothesisGenerated;

        }
        public async Task Initializesynthesizer()
        {

           synthesizer = new SpeechSynthesizer();

            isListening = false;
            dispatcher = this.Dispatcher;

            // select the language display
            var voices = SpeechSynthesizer.AllVoices;
            foreach (VoiceInformation voice in voices)
            {
                if (voice.Language.Contains(synLangStr))
                {
                    synthesizer.Voice = voice;
                    break;
                }
            }

            // Check Microphone Plugged in
            bool permissionGained = await AudioCapturePermissions.RequestMicrophoneCapture();
            if (!permissionGained)
            {
                this.dictationTextBox.Text = "Requesting Microphone Capture Fails; Make sure Microphone is plugged in";
            }
            btnStartTalk.IsEnabled = true;
        }

        private async void SendDataToHost(string dataToBeSent)
        {
            await ConnectHost();
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
                await writer.FlushAsync();
            }
            catch (Exception exception)
            {
            }
        }

        private async Task ConnectHost()
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

                        await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                        {
                            connStatus.Text = "Connected Successfully!";
                        });
                        connected = true;
                    }
                    catch (Exception exception)
                    {
                        // If this is an unknown status it means that the error is fatal and retry will likely fail.
                        if (SocketError.GetStatus(exception.HResult) == SocketErrorStatus.Unknown)
                        {
                            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                            {
                                connStatus.Text += "Reconnection failed, Double check Host is on and try again";
                            });
                            return;
                        }
                    }                    

                }

            }
        }

        private async void ContinuousRecognitionSession_Completed(SpeechContinuousRecognitionSession sender, SpeechContinuousRecognitionCompletedEventArgs args)
        {
            if(args.Status!=SpeechRecognitionResultStatus.Success)
            {
                if (args.Status == SpeechRecognitionResultStatus.TimeoutExceeded)
                {
                    await dispatcher.RunAsync(CoreDispatcherPriority.Normal,()=>
                    {
                        checkError.Visibility = Visibility.Visible;
                        errorCheck.Visibility = Visibility.Visible;
                        errorCheck.Text = "Automatic Time out of Dictation";
                        StartTalkButtonText.Text = "Start Talk";
                        dictationTextBox.Text = dictatedTextBuilder.ToString();
                        isListening = false;
                    });
                }
                else
                {
                    await dispatcher.RunAsync(CoreDispatcherPriority.Normal,()=> {
                        checkError.Visibility = Visibility.Visible;
                        errorCheck.Visibility = Visibility.Visible;
                        errorCheck.Text = "Continuous Recognition Completed:"+args.Status.ToString();
                        StartTalkButtonText.Text = "Start Talk";
                        isListening = false;
                    });
                }
            }
   
        }
        /// <summary>
        /// Handle events fired when a result is generated. Check for high to medium confidence, and then append the
        /// string to the end of the stringbuffer, and replace the content of the textbox with the string buffer, to
        /// remove any hypothesis text that may be present.
        /// </summary>
        /// <param name="sender">The Recognition session that generated this result</param>
        /// <param name="args">Details about the recognized speech</param>
      
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
        private async void ContinuousRecognitionSession_ResultGenerated(SpeechContinuousRecognitionSession sender, SpeechContinuousRecognitionResultGeneratedEventArgs args)
        {
            string s = args.Result.Text;
            //Send the Data
            SendDataToHost(s);

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

        private async void btnStartTalk_Click(object sender, RoutedEventArgs e)
        {
            if (isListening == false)
            {
                if (speechRecognizer.State == SpeechRecognizerState.Idle)
                {
                    StartTalkButtonText.Text = "Stop Talk";
                    try
                    {
                        isListening = true;
                        await speechRecognizer.ContinuousRecognitionSession.StartAsync();
                    }
                    catch (Exception ex)
                    {
                        if ((uint)ex.HResult == HResultPrivacyStatementDeclined)
                        {
                            hlOpenPrivacySettings.Visibility = Visibility.Visible;
                        }
                        else
                        {
                            var messageDialog = new Windows.UI.Popups.MessageDialog(ex.Message, "Exception");
                            await messageDialog.ShowAsync();
                        }
                        isListening = false;
                        StartTalkButtonText.Text = "Start Talk";

                    }
                }
            }
            else
            {
                isListening = false;
                StartTalkButtonText.Text = "Start Talk";
                if (speechRecognizer.State != SpeechRecognizerState.Idle)
                {
                    await speechRecognizer.ContinuousRecognitionSession.StopAsync();
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

        private async void openPrivacySettings_Click(Windows.UI.Xaml.Documents.Hyperlink sender, Windows.UI.Xaml.Documents.HyperlinkClickEventArgs args)
        {
            await Windows.System.Launcher.LaunchUriAsync(new Uri("ms-settings:privacy-speechtyping"));
        }

        private async void btnReConnect_Click(object sender, RoutedEventArgs e)
        {
            if (connected)
            {

                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    connStatus.Text = "Reconnected Successfully!";
                });
               
            }
            else
            {
                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    connStatus.Text = "Reconnection failed, Double check Host is on and try again ";
                });
               
                return;
            }
         
        }

        private void ReceivedMsgTextBox_TextChanged(object sender, TextChangedEventArgs e)
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
