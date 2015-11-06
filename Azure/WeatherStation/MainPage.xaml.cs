using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Client;
using Newtonsoft.Json;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace WeatherDataReporter
{
    public sealed partial class MainPage : Page
    {
        static DeviceClient deviceClient;
        static string iotHubUri = "{replace}";
        static string deviceKey = "{replace}";
        static string deviceId = "{replace}";

        public MainPage()
        {
            this.InitializeComponent();

            deviceClient = DeviceClient.Create(iotHubUri, AuthenticationMethodFactory.CreateAuthenticationWithRegistrySymmetricKey(deviceId, deviceKey), TransportType.Http1);

            SendDeviceToCloudMessagesAsync();
        }

        private static async void SendDeviceToCloudMessagesAsync()
        {
            var weatherDataprovider = await WeatherDataProvider.Create();

            // Use this if you don't have a real sensor:
            // var weatherDataprovider = await SimulatedWeatherDataProvider.Create();

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

                await Task.Delay(1000);
            }
        }
    }
}
