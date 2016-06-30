using System;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Client;
using Newtonsoft.Json;
using Microsoft.Devices.Tpm;

class AzureIoTHub
{

    private string devConnectString = "";
    private static DeviceClient deviceClient;
    
    public AzureIoTHub()
    {
        try
        {
            initializeWithProvisionedDevice();
        } catch (System.Exception e)
        {
            throw e;
        }
    }
    public AzureIoTHub(string connectionstring)
    {
        devConnectString = connectionstring;
        try
        {
            deviceClient = DeviceClient.CreateFromConnectionString(devConnectString, TransportType.Amqp);
        }
        catch (System.FormatException e)
        {
            throw e;
        }

    }
    public void initializeWithProvisionedDevice()
    {
        TpmDevice myDevice = new TpmDevice(0);
        string hubUri = myDevice.GetHostName();
        string deviceId = myDevice.GetDeviceId();
        string sasToken = myDevice.GetSASToken();
        deviceClient = DeviceClient.Create(hubUri,
            AuthenticationMethodFactory.CreateAuthenticationWithToken(deviceId, sasToken),
            TransportType.Amqp);

    }
    public async Task SendDeviceToCloudMessageAsync(string msg)
    {
        var message = new Message(Encoding.ASCII.GetBytes(msg));

        await deviceClient.SendEventAsync(message);
    }
    public async Task<string> ReceiveCloudToDeviceMessageAsync()
    {
        var deviceClient = DeviceClient.CreateFromConnectionString(devConnectString, TransportType.Amqp);

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
