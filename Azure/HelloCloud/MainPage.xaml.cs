using System;
using System.Text;
using Microsoft.Azure.Devices.Client;
using Windows.UI.Xaml.Controls;

namespace HelloCloud
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
            SendDeviceToCloudMessagesAsync();
        }
        static async void SendDeviceToCloudMessagesAsync()
        {
            var deviceClient = DeviceClient.Create(iotHubUri,
                    AuthenticationMethodFactory.
                        CreateAuthenticationWithRegistrySymmetricKey(deviceId, deviceKey),
                    TransportType.Http1);

            var str = "Hello, Cloud!";
            var message = new Message(Encoding.ASCII.GetBytes(str));

            await deviceClient.SendEventAsync(message);
        }


    }
}
