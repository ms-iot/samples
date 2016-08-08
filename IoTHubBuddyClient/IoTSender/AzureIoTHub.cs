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
    private string devID = "";
    
    public AzureIoTHub()
    {
        
    }
    public bool Connect()
    {
        try
        {
            initializeWithProvisionedDevice();
            return true;
        }
        catch (System.Exception e)
        {
            return false;
        }
    }
    public bool Connect(string connectionstring)
    {
        try
        {
            deviceClient = DeviceClient.CreateFromConnectionString(connectionstring, TransportType.Amqp);
            devConnectString = connectionstring;
            return true;
        }
        catch (System.Exception e)
        {
            return false;
        }
    }
    public void initializeWithProvisionedDevice()
    {
        TpmDevice myDevice = new TpmDevice(0);
        string hubUri = myDevice.GetHostName();
        devID = myDevice.GetDeviceId();
        string sasToken = myDevice.GetSASToken();
        deviceClient = DeviceClient.Create(hubUri,
            AuthenticationMethodFactory.CreateAuthenticationWithToken(devID, sasToken),
            TransportType.Amqp);

    }
    public string GetDeviceId()
    {
        return devID;
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
