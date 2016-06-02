using Newtonsoft.Json;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.Gaming.Input;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Core;
using System;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace XboxControllerClient
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        // Change the IP address if running remotely
        readonly private string WebServiceUrl = "http://localhost:5000/gamepad";

        private Task readInputTask;
        private CancellationTokenSource cancelTokenSource;

        public MainPage()
        {
            this.InitializeComponent();
        }

        private async Task ReadInputLoop(CancellationToken token)
        {
            while (!token.IsCancellationRequested)
            {
                var gamepad = Gamepad.Gamepads.FirstOrDefault();
                if (gamepad != null)
                {
                    // Read gamepad input
                    var reading = gamepad.GetCurrentReading();

                    // Post the gamepad reading to WebService
                    var response = await this.PostGamePadReading(reading);
                    await this.UpdateText(this.GamePadReadingToString(reading) + response);
                }
                else
                {
                    await this.UpdateText("No gamepad connected.");
                }
            }
        }

        private async Task UpdateText(string message)
        {
            // Update Textbox using the UI thread
            await this.textBox.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => { this.textBox.Text = message; });
        }

        private async Task<string> PostGamePadReading(GamepadReading reading)
        {
            // Post the gamepad reading to WebService
            try
            {
                var httpClient = new HttpClient();
                var json = JsonConvert.SerializeObject(reading);
                var content = new StringContent(json, Encoding.ASCII, "application/json");
                var response = await httpClient.PostAsync(WebServiceUrl, content);

                if (response.IsSuccessStatusCode)
                {
                    return "Server: ok";
                }
                else
                {
                    return string.Format("Server: error code={0}", response.StatusCode);
                }
            }
            catch (HttpRequestException)
            {
                return "Server: not found\n";
            }
        }

        private string GamePadReadingToString(GamepadReading reading)
        {
            return string.Format("LeftStickX={0}\n" +
                                 "LeftstickY={1}\n" +
                                 "LeftTrigger={2}\n" +
                                 "RightStickX={3}\n" +
                                 "RightStickY={4}\n" +
                                 "RightTrigger={5}\n" +
                                 "Buttons={6}\n\n",
                                 reading.LeftThumbstickX, reading.LeftThumbstickY, reading.LeftTrigger,
                                 reading.RightThumbstickX, reading.RightThumbstickY, reading.RightTrigger,
                                 reading.Buttons);
        }

        private void OnUnloaded(object sender, RoutedEventArgs e)
        {
            this.cancelTokenSource.Cancel();
            this.readInputTask.Wait();
        }

        private void OnLoaded(object sender, RoutedEventArgs e)
        {
            this.cancelTokenSource = new CancellationTokenSource();

            // Execute the ReadInputLoop() on a background thread
            this.readInputTask = Task.Run(async () => { await this.ReadInputLoop(this.cancelTokenSource.Token); });
        }
    }
}
