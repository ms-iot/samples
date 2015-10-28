// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
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

        // defaults before device language scanning occurs
        public static Language host_language = new Language("en-US");
        public static string voiceMatchLanguageCode = "en";
        private string inLanguageSpecificCode = "en";
        private string outLanguageSpecificCode = "en"; 

        private SpeechRecognizer speechRecognizer;
        private SpeechSynthesizer synthesizer;

        private static uint HResultPrivacyStatementDeclined = 0x80045509;
        private TranslatorManager translator;
        private string toSpeak;

        public bool IsReadyToRecognize => speechRecognizer != null;

        public MainPage()
        {
            this.InitializeComponent();
            translator = new TranslatorManager();

            PopulateLanguageDropdown();
            InitRecogAndSyn();
        }

        private async Task Translate(string text)
        {
            var translatedS = string.Empty;
            try
            {
                translatedS = await translator.Translate( text, inLanguageSpecificCode, outLanguageSpecificCode );
            }
            catch (Exception e)
            {
                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    checkError.Visibility = Visibility.Visible;
                    errorCheck.Visibility = Visibility.Visible;
                    errorCheck.Text = "Translation: " + e.Message;
                } );

                return;
            }

            SpeechSynthesisStream stream = await synthesizer.SynthesizeTextToStreamAsync(translatedS);
            var ignored2 = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                media.SetSource(stream, stream.ContentType);
                media.Play();
                originalmsg.Text = text;
                ReceivedText.Text = translatedS;
                ReceivedText.FontSize = 20;
            });
        }

        public async void InitRecogAndSyn()
        {
            await InitializeRecognizer(host_language);
            await InitializeSynthesizer();
        }

        private string Abbreviated(string languageTag)
        {
            var index = languageTag.IndexOf('-');
            return index > -1 ? languageTag.Substring(0, index) : languageTag;
        }

        private async Task InitializeRecognizer(Language recognizerLanguage, Language speechLanguage = null)
        {
            //Default spoken language to first non-recognizer language
            speechLanguage = speechLanguage ??
                             SpeechRecognizer.SupportedGrammarLanguages.FirstOrDefault(
                                 l => l.LanguageTag != recognizerLanguage.LanguageTag);

            if (speechLanguage == null)
            {
                checkError.Visibility = Visibility.Visible;
                errorCheck.Visibility = Visibility.Visible;
                errorCheck.Text = "No alternate languages installed";
                return;
            }

            //Set recognition and spoken languages based on choice and alternates
            voiceMatchLanguageCode = Abbreviated(speechLanguage.LanguageTag);
            inLanguageSpecificCode = recognizerLanguage.LanguageTag;
            outLanguageSpecificCode = speechLanguage.LanguageTag;

            if (speechRecognizer != null)
            {
                speechRecognizer.ContinuousRecognitionSession.Completed -= ContinuousRecognitionSession_Completed;
                speechRecognizer.ContinuousRecognitionSession.ResultGenerated -= ContinuousRecognitionSession_ResultGenerated;
                speechRecognizer.HypothesisGenerated -= SpeechRecognizer_HypothesisGenerated;
                speechRecognizer.Dispose();
                speechRecognizer = null;
            }

            speechRecognizer = new SpeechRecognizer(recognizerLanguage);

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
        public async Task InitializeSynthesizer()
        {
            if (synthesizer == null)
            {
                synthesizer = new SpeechSynthesizer();
            }

            isListening = false;
            dispatcher = this.Dispatcher;

            // select the language display
            var voices = SpeechSynthesizer.AllVoices;
            foreach (VoiceInformation voice in voices)
            {
                if (voice.Language.Contains(voiceMatchLanguageCode)) 
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
        }

        private async void ContinuousRecognitionSession_Completed(SpeechContinuousRecognitionSession sender, SpeechContinuousRecognitionCompletedEventArgs args)
        {
            if(args.Status!=SpeechRecognitionResultStatus.Success)
            {
                if (args.Status == SpeechRecognitionResultStatus.TimeoutExceeded)
                {
                    await dispatcher.RunAsync(CoreDispatcherPriority.Normal,async ()=>
                    {
                        await StopRecognition();
                        checkError.Visibility = Visibility.Visible;
                        errorCheck.Visibility = Visibility.Visible;
                        errorCheck.Text = "Automatic Time out of Dictation";
                        dictationTextBox.Text = dictatedTextBuilder.ToString();
                    });
                }
                else
                {
                    await dispatcher.RunAsync(CoreDispatcherPriority.Normal,async ()=> {
                        await StopRecognition();

                        checkError.Visibility = Visibility.Visible;
                        errorCheck.Visibility = Visibility.Visible;
                        errorCheck.Text = "Continuous Recognition Completed:"+args.Status.ToString();
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

            if( args.Result.Status == SpeechRecognitionResultStatus.Success)
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

            this.toSpeak = s;
        }

        private async void btnStartTalk_Click(object sender, RoutedEventArgs e)
        {
            if (!isListening)
            {
                await StartRecognition();
            }
            else
            {
                await StopRecognition();
            }
        }

        private async Task StartRecognition()
        {
            if( speechRecognizer.State == SpeechRecognizerState.Idle )
            {
                errorCheck.Text = string.Empty;

                StartTalkButtonText.Text = "Stop Talk";
                try
                {
                    isListening = true;
                    await speechRecognizer.ContinuousRecognitionSession.StartAsync();
                }
                catch( Exception ex )
                {
                    if( (uint)ex.HResult == HResultPrivacyStatementDeclined )
                    {
                        hlOpenPrivacySettings.Visibility = Visibility.Visible;
                    }
                    else
                    {
                        var messageDialog = new Windows.UI.Popups.MessageDialog( ex.Message, "Exception" );
                        await messageDialog.ShowAsync();
                    }

                    isListening = false;
                    StartTalkButtonText.Text = "Start Talk";
                }
            }
        }

        private async Task StopRecognition()
        {
            isListening = false;
            StartTalkButtonText.Text = "Start Talk";
            if( speechRecognizer.State != SpeechRecognizerState.Idle )
            {
                await speechRecognizer.ContinuousRecognitionSession.StopAsync();
                dictationTextBox.Text = dictatedTextBuilder.ToString();
            }

            if (!string.IsNullOrWhiteSpace(this.toSpeak))
            {
                await Translate( this.toSpeak );
                this.toSpeak = string.Empty;
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

        /// <summary>
        /// Look up the supported languages for this speech recognition scenario, 
        /// that are installed on this machine, and populate a dropdown with a list.
        /// </summary>
        private void PopulateLanguageDropdown()
        {
            var defaultLanguage = SpeechRecognizer.SystemSpeechLanguage;
            var supportedLanguages = SpeechRecognizer.SupportedGrammarLanguages;

            foreach (Language lang in supportedLanguages)
            {
                var item = new ComboBoxItem
                {
                    Tag = lang,
                    Content = lang.DisplayName
                };

                cbLanguageSelection.Items.Add(item);
                if (lang.LanguageTag == defaultLanguage.LanguageTag)
                {
                    item.IsSelected = true;
                    cbLanguageSelection.SelectedItem = item;
                }
            }
        }

        private async void cbLanguageSelection_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if(speechRecognizer != null)
            {
                ComboBoxItem item = (ComboBoxItem)(cbLanguageSelection.SelectedItem);
                Language newLanguage = (Language)item.Tag;
                if(speechRecognizer.CurrentLanguage != newLanguage)
                {
                    try
                    {
                        await StopRecognition();
                        await InitializeRecognizer(newLanguage);
                        await InitializeSynthesizer();
                    }
                    catch(Exception exception)
                    {
                        var messageDialog = new Windows.UI.Popups.MessageDialog(exception.Message, "Exception");
                        await messageDialog.ShowAsync();
                    }
                }
            }
        }
    }
}
