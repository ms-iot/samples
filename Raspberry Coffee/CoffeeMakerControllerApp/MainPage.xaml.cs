using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using Windows.ApplicationModel.AppService;
using Windows.ApplicationModel.Background;
using Windows.ApplicationModel.Core;
using Windows.Devices.Gpio;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Media.SpeechRecognition;
using Windows.Networking.Sockets;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace CortonaCoffee
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private const int TIME_UNIT_SECOND = 1000;
        private const int TIME_UNIT_MINUTE = 60 * TIME_UNIT_SECOND;
        private const int TIME_UNIT_HOUR = 60 * TIME_UNIT_MINUTE;

        private const int COFFEE_MAKER_RELAY_PIN = 5;
        private const int COFFEE_MAKER_LED_PIN = 6;

        private bool coffeeRelayStatus = false;

        private GpioController gpio;

        private GpioPin coffeeMakerRelay;
        private GpioPin coffeeMakerLED;

        private Timer activeTimer = null;

        private int runningTime = 0;
        private int maxRunningTime = 30 * TIME_UNIT_MINUTE;

        public MainPage()
        {
            this.InitializeComponent();
            App.Current.UnhandledException += Current_UnhandledException;

            InitGpio();
            InitVoiceCommands();

            object httpServer = null;
            if(CoreApplication.Properties.TryGetValue("httpserver", out httpServer))
            {
                (httpServer as WebServer.HttpServer).OnRequestReceived += HttpServer_OnRequestReceived;
            }
            activeTimer = new Timer(ActiveTimerTick, coffeeRelayStatus, Timeout.Infinite, Timeout.Infinite);
        }

        private void Current_UnhandledException(object sender, UnhandledExceptionEventArgs e)
        {
            if(coffeeMakerRelay != null)
            {
                coffeeMakerRelay.Write(GpioPinValue.Low);
                coffeeMakerRelay.SetDriveMode(GpioPinDriveMode.Output);
            }
        }

        private void HttpServer_OnRequestReceived(WebServer.HTTPRequest request)
        {
            if(request.URL == "/api/CFMakerRelay/enable")
            {
                EnableCoffeeMakerRelay();
            }
            else if(request.URL == "/api/CFMakerRelay/disable")
            {
                DisableCoffeeMakerRelay();
            }
        }

        private void AppServiceConnection_ServiceClosed(AppServiceConnection sender, AppServiceClosedEventArgs args)
        {
            Debug.WriteLine("HttpWebApiService has closed. Status is " + args.Status);
        }

        private async void AppServiceConnection_RequestReceived(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
        {
            var message = args.Request.Message;
            string cmd = message["Cmd"] as string;
            switch (cmd)
            {
                case "Enable":
                    {
                        await Dispatcher.RunAsync(CoreDispatcherPriority.High, ()=>
                        {
                            EnableCoffeeMakerRelay();
                        });
                        break;
                    }
                case "Disable":
                    {
                        await Dispatcher.RunAsync(CoreDispatcherPriority.High, ()=>
                        {
                            DisableCoffeeMakerRelay();
                        });
                        break;
                    }
                default:
                    break;
            }
        }

        private async void InitVoiceCommands()
        {
            var enableVoiceCommandsFileUri = new Uri("ms-appx:///Grammars/EnableGrammar.xml");
            var enableVoiceCommandsFile = await Windows.Storage.StorageFile.GetFileFromApplicationUriAsync(enableVoiceCommandsFileUri);

            var disableVoiceCommandsFileUri = new Uri("ms-appx:///Grammars/DisableGrammar.xml");
            var disableVoiceCommandsFile = await Windows.Storage.StorageFile.GetFileFromApplicationUriAsync(disableVoiceCommandsFileUri);

            var speechRecognizer = new Windows.Media.SpeechRecognition.SpeechRecognizer();
            speechRecognizer.StateChanged += SpeechRecognizer_StateChanged;

            Windows.Media.SpeechRecognition.SpeechRecognitionGrammarFileConstraint enableGrammarContraints = new Windows.Media.SpeechRecognition.SpeechRecognitionGrammarFileConstraint(enableVoiceCommandsFile);
            Windows.Media.SpeechRecognition.SpeechRecognitionGrammarFileConstraint disableGrammarContraints = new Windows.Media.SpeechRecognition.SpeechRecognitionGrammarFileConstraint(disableVoiceCommandsFile);
            speechRecognizer.Constraints.Add(enableGrammarContraints);
            speechRecognizer.Constraints.Add(disableGrammarContraints);

            Debug.WriteLine("Compiling grammar...");
            var compilationResults = await speechRecognizer.CompileConstraintsAsync();

            if(compilationResults.Status == Windows.Media.SpeechRecognition.SpeechRecognitionResultStatus.Success)
            {
                speechRecognizer.Timeouts.EndSilenceTimeout = TimeSpan.FromSeconds(1.5);
                speechRecognizer.ContinuousRecognitionSession.Completed += ContinuousRecognitionSession_Completed;
                speechRecognizer.ContinuousRecognitionSession.ResultGenerated += ContinuousRecognitionSession_ResultGenerated;
                Debug.WriteLine("Grammar compiled.");
                Debug.WriteLine("Starting continuous recognition");
                await speechRecognizer.ContinuousRecognitionSession.StartAsync();
            }
            else
            {
                Debug.WriteLine("Could not complie grammar. " + compilationResults.Status);
            }
        }

        private async void ContinuousRecognitionSession_ResultGenerated(Windows.Media.SpeechRecognition.SpeechContinuousRecognitionSession sender, Windows.Media.SpeechRecognition.SpeechContinuousRecognitionResultGeneratedEventArgs args)
        {
            Debug.WriteLine(JsonConvert.SerializeObject(args.Result));

            if (args.Result.Confidence == Windows.Media.SpeechRecognition.SpeechRecognitionConfidence.High ||
                args.Result.Confidence == Windows.Media.SpeechRecognition.SpeechRecognitionConfidence.Medium)
            {
                await Dispatcher.RunAsync(CoreDispatcherPriority.High, () =>
                {
                    HandleRecognitionResult(args.Result);
                });
            }
            else
            {
                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    Debug.WriteLine("I'm sorry I did not understand your command. Please say again");
                });
            }
        }

        private void HandleRecognitionResult(SpeechRecognitionResult result)
        {
            if(result.Text.Equals("make coffee", StringComparison.CurrentCultureIgnoreCase) || 
                result.Text.Equals("start coffee", StringComparison.CurrentCultureIgnoreCase) ||
                result.Text.Equals("i want coffee", StringComparison.CurrentCultureIgnoreCase) ||
                result.Text.Equals("coffee please", StringComparison.CurrentCultureIgnoreCase))
            {
                Debug.WriteLine("Starting your coffee.");
                EnableCoffeeMakerRelay();
            }
            else if(result.Text.Equals("stop", StringComparison.CurrentCultureIgnoreCase) ||
                    result.Text.Equals("turn off", StringComparison.CurrentCultureIgnoreCase) ||
                    result.Text.Equals("that's enough", StringComparison.CurrentCultureIgnoreCase))
            {
                Debug.WriteLine("Stopping your coffee.");
                DisableCoffeeMakerRelay();
            }
        }

        private void ContinuousRecognitionSession_Completed(Windows.Media.SpeechRecognition.SpeechContinuousRecognitionSession sender, Windows.Media.SpeechRecognition.SpeechContinuousRecognitionCompletedEventArgs args)
        {
            Debug.WriteLine("Continous Recognition Session Completed.");
        }

        private void SpeechRecognizer_StateChanged(Windows.Media.SpeechRecognition.SpeechRecognizer sender, Windows.Media.SpeechRecognition.SpeechRecognizerStateChangedEventArgs args)
        {
            var speechRecognizerState = args.State;
            Debug.WriteLine("Speech Recognizer State change to " + speechRecognizerState);
        }

        private void InitGpio()
        {
            gpio = GpioController.GetDefault();

            coffeeMakerRelay = gpio.OpenPin(COFFEE_MAKER_RELAY_PIN);
            coffeeMakerRelay.Write(GpioPinValue.Low);
            coffeeMakerRelay.SetDriveMode(GpioPinDriveMode.Output);

            coffeeMakerLED = gpio.OpenPin(COFFEE_MAKER_LED_PIN);
            coffeeMakerLED.Write(GpioPinValue.Low);
            coffeeMakerLED.SetDriveMode(GpioPinDriveMode.Output);
        }

        private async void UpdateUI()
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                BtnOn.IsEnabled = !coffeeRelayStatus;
                BtnOff.IsEnabled = coffeeRelayStatus;
            });
        }

        private async void UpdateTimerUI()
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                if (coffeeRelayStatus)
                {
                    TimeSpan span = TimeSpan.FromMilliseconds(maxRunningTime - runningTime);
                    TextCountdown.Text = span.ToString(@"m\:ss");
                }
                else
                {
                    TextCountdown.Text = "--:--";
                }
            });
        }

        private void DisableCoffeeMakerRelay()
        {
            coffeeMakerRelay.Write(GpioPinValue.Low);
            coffeeMakerLED.Write(GpioPinValue.Low);

            activeTimer.Change(Timeout.Infinite, Timeout.Infinite);
            runningTime = 0;
            UpdateTimerUI();

            coffeeRelayStatus = false;
            UpdateUI();
        }

        private void EnableCoffeeMakerRelay()
        {
            coffeeMakerRelay.Write(GpioPinValue.High);
            coffeeMakerLED.Write(GpioPinValue.High);

            activeTimer.Change(0, TIME_UNIT_SECOND);

            coffeeRelayStatus = true;
            UpdateUI();
        }

        private void ToggleCoffeeMakerRelay()
        {
            var coffeeMakerRelayValue = coffeeMakerRelay.Read();
            if(coffeeMakerRelayValue == GpioPinValue.Low)
            {
                EnableCoffeeMakerRelay();
            }
            else
            {
                DisableCoffeeMakerRelay();
            }
        }

        private void BtnOn_Click(object sender, RoutedEventArgs e)
        {
            ToggleCoffeeMakerRelay();
        }

        private void BtnOff_Click(object sender, RoutedEventArgs e)
        {
            ToggleCoffeeMakerRelay();
        }

        private void ActiveTimerTick(object state)
        {
            runningTime += TIME_UNIT_SECOND;

            if(runningTime >= maxRunningTime)
            {
                activeTimer.Change(Timeout.Infinite, Timeout.Infinite);
                DisableCoffeeMakerRelay();
            }

            UpdateTimerUI();
        }
    }
}
