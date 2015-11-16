using System;
using System.Linq;
using System.Threading.Tasks;
using Windows.UI.Core;
using Windows.UI.Xaml.Controls;

namespace AllJoynVoice
{
    class Setup
    {
        private static readonly int MSEC_WAIT_FOR_SPEECH_OUTPUT = 5000;
        static private bool ledAvailable = false;

        public async static Task Start(CoreDispatcher uiDispatcher, MediaElement audioMedia)
        {
            try
            {
                LEDController.Init();
                LEDController.TurnOn(StatusLED.Idle);
                ledAvailable = true;
            }
            catch
            {
                // Ignore if LED control is not availale
                ledAvailable = false;
            }

            Task configReady = Config.Load();

            // -----

            Logger.LogInfo("Setting up speech synthesizer.");

            EasySpeechSynthesizer synthesizer = new EasySpeechSynthesizer(uiDispatcher, audioMedia);

            if (ledAvailable)
            {
                synthesizer.StartedSpeaking += () => LEDController.TurnOn(StatusLED.Speaking);
                synthesizer.StoppedSpeaking += () => LEDController.TurnOff(StatusLED.Speaking);
            }

            synthesizer.StartSpeaking();

            // -----

            Logger.LogInfo("Waiting for the configuration to be loaded.");

            try
            {
                await configReady;
            }
            catch (Exception ex)
            {
                await synthesizer.ClearQueue();
                await synthesizer.Speak("Something went wrong... Couldn't load the config file.");
                await Task.Delay(MSEC_WAIT_FOR_SPEECH_OUTPUT);
                Logger.LogException("Setup", ex);
                throw;
            }

            // -----

            Config.Notifier.OnNotification += async s => await synthesizer.Speak(s);

            // -----

            Logger.LogInfo("Setting up speech recognition.");

            CodewordSpeechRecognizer speechRecognizer = null;

            try
            {
                speechRecognizer = new CodewordSpeechRecognizer(Config.Codeword, Config.Actions);

                await speechRecognizer.CompileConstraintsAsync();

                if (ledAvailable)
                {
                    speechRecognizer.StateChanged += state =>
                    {
                        if (state == CodewordSpeechRecognizerState.Idle)
                        {
                            LEDController.TurnOn(StatusLED.Idle);
                            LEDController.TurnOff(StatusLED.Listening);
                        }
                        else if (state == CodewordSpeechRecognizerState.Listening)
                        {
                            LEDController.TurnOff(StatusLED.Idle);
                            LEDController.TurnOn(StatusLED.Listening);
                        }
                    };
                }

                speechRecognizer.WordRecognized += async word =>
                {
                    try
                    {
                        Logger.LogInfo("Executing action: " + word);

                        Task actionTask = Config.Actions.First(x => x.Id == word).RunAsync();

                        // Stop speaking and clear the queue when something is recognized.
                        await synthesizer.ClearQueue();

                        await actionTask;
                        await synthesizer.Speak("Done!");
                    }
                    catch (InvalidOperationException ex)
                    {
                        await synthesizer.Speak("Something went wrong...");
                        await synthesizer.Speak(ex.Message);
                        Logger.LogException(word, ex);
                    };
                };

                await speechRecognizer.StartAsync();
            }
            catch (Exception ex)
            {
                await synthesizer.StopSpeaking();
                synthesizer.StartSpeaking();
                await synthesizer.Speak("Something went wrong... Couldn't start speech recognition.");
                await Task.Delay(MSEC_WAIT_FOR_SPEECH_OUTPUT);
                Logger.LogException("Setup: SpeechRecognizer", ex);
                throw;
            }
        }
    }
}
