using System;
using System.Diagnostics;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Client;
using Newtonsoft.Json;
using Windows.UI.Xaml.Controls;

namespace WeatherDataReporter
{
    public sealed partial class MainPage : Page
    {
        static DeviceClient deviceClient;

        public MainPage()
        {
            this.InitializeComponent();

            deviceClient = DeviceClient.Create(iotHubUri, AuthenticationMethodFactory.CreateAuthenticationWithRegistrySymmetricKey(deviceId, deviceKey), TransportType.Http1);

            SendDeviceToCloudMessagesAsync();
        }

        private async void SendDeviceToCloudMessagesAsync()
        {
            //var weatherDataprovider = await WeatherDataProvider.Create();

            // Use this if you don't have a real sensor:
            var weatherDataprovider = await SimulatedWeatherDataProvider.Create();

            while (true)
            {
                double currentHumidity = weatherDataprovider.GetHumidity();
                double currentTemperature = weatherDataprovider.GetTemperature();

                var telemetryDataPoint = new
                {
                    time = DateTime.Now.ToString(),
                    deviceId = deviceId,
                    currentHumidity = currentHumidity,
                    currentTemperature = currentTemperature
                };
                var messageString = JsonConvert.SerializeObject(telemetryDataPoint);
                var message = new Message(Encoding.ASCII.GetBytes(messageString));

                await deviceClient.SendEventAsync(message);
                Debug.WriteLine("{0} > Sending message: {1}", DateTime.Now, messageString);

                this.listView.Items.Insert(0, messageString);

                await Task.Delay(1000);
            }
        }
    }
}
