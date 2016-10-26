using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Client;
using System.Threading;

namespace HelloCloud
{
    public sealed partial class StartupTask : IBackgroundTask
    {
        public void Run(IBackgroundTaskInstance taskInstance)
        {
            SendDeviceToCloudMessagesAsync(taskInstance.GetDeferral());
        }

        static async void SendDeviceToCloudMessagesAsync(BackgroundTaskDeferral deferral)
        {
            var deviceClient = DeviceClient.Create(iotHubUri,
                    AuthenticationMethodFactory.
                        CreateAuthenticationWithRegistrySymmetricKey(deviceId, deviceKey),
                    TransportType.Http1);

            var str = "Hello, Cloud!";
            var message = new Message(Encoding.ASCII.GetBytes(str));

            await deviceClient.SendEventAsync(message);

            deferral.Complete();
        }
    }
}
