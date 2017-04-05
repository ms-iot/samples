using Microsoft.Azure.Devices.Client;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace XamarinIoTViewer
{
    class AzureIoTHub
    {
        private const string deviceConnectionString = "...";
        private const string deviceId = "...";
        public async Task SendDeviceToCloudMessageAsync(string msg)
        {
            var deviceClient = DeviceClient.CreateFromConnectionString(deviceConnectionString, TransportType.Http1);
            Message message = new Message(Encoding.UTF8.GetBytes(msg));

            await deviceClient.SendEventAsync(message);
        }
        public string getDeviceId()
        {
            return deviceId;
        }
        public async Task ReceiveCloudToDeviceMessageAsync(MessageController msgs)
        {
            while (true)
            {

                var deviceClient = DeviceClient.CreateFromConnectionString(deviceConnectionString, TransportType.Http1);
                var receivedMessage = await deviceClient.ReceiveAsync();
                if (receivedMessage != null)
                {
                    byte[] msgBytes = receivedMessage.GetBytes();
                    string messageData = Encoding.UTF8.GetString(msgBytes, 0, msgBytes.Length);
                    await deviceClient.CompleteAsync(receivedMessage);
                    Debug.WriteLine(messageData);
                    msgs.ShowMessage(messageData);
                }

                await Task.Delay(TimeSpan.FromSeconds(1));
            }
        }
        
    }
}
