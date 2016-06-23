using System;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Client;
using Newtonsoft.Json;

static class AzureIoTHub
{
    //
    // Note: this connection string is specific to the device "dimaha01". To configure other devices,
    // see information on iothub-explorer at http://aka.ms/iothubgetstartedVSCS
    //
    const string deviceConnectionString = "...";

    //
    // To monitor messages sent to device "dimaha01" use iothub-explorer as follows:
    //    iothub-explorer HostName=hub-dimaha.azure-devices.net;SharedAccessKeyName=service;SharedAccessKey= monitor-events "dimaha01"
    //

    // Refer to http://aka.ms/azure-iot-hub-vs-cs-wiki for more information on Connected Service for Azure IoT Hub

    public static async Task SendDeviceToCloudMessageAsync(string msg)
    {
        var deviceClient = DeviceClient.CreateFromConnectionString(deviceConnectionString, TransportType.Amqp);
        var message = new Message(Encoding.ASCII.GetBytes(msg));

        await deviceClient.SendEventAsync(message);
    }
    public static async Task<string> ReceiveCloudToDeviceMessageAsync()
    {
        var deviceClient = DeviceClient.CreateFromConnectionString(deviceConnectionString, TransportType.Amqp);

        while (true)
        {
            var receivedMessage = await deviceClient.ReceiveAsync();

            if (receivedMessage != null)
            {
                var messageData = Encoding.ASCII.GetString(receivedMessage.GetBytes());
                await deviceClient.CompleteAsync(receivedMessage);
                return messageData;
            }

            await Task.Delay(TimeSpan.FromSeconds(1));
        }
    }
}
