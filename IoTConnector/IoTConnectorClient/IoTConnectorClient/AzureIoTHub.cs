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
    /// <summary>
    /// Attempt to connect with a TPM provisioned device
    /// </summary>
    /// <returns> true if connection is successful, false otherwise</returns>
    public bool Connect()
    {
        try
        {
            initializeWithProvisionedDevice();
            return true;
        }
        catch (Exception)
        {
            return false;
        }
    }
    /// <summary>
    /// Attempt to connect with a valid connection string
    /// </summary>
    /// <param name="connectionstring"></param>
    /// <returns> true if connection is successful, false otherwise</returns>
    public bool Connect(string connectionstring)
    {
        try
        {
            deviceClient = DeviceClient.CreateFromConnectionString(connectionstring, TransportType.Amqp);
            devConnectString = connectionstring;
            return true;
        }
        catch (Exception)
        {
            return false;
        }
    }
    /// <summary>
    /// attempt to create a device client with the user credentials stored in the tpm
    /// </summary>
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
    /// <summary>
    /// return device id
    /// </summary>
    /// <returns></returns>
    public string GetDeviceId()
    {
        return devID;
    }
    /// <summary>
    /// send a message to the cloud
    /// </summary>
    /// <param name="msg"></param>
    /// <returns></returns>
    public async Task SendDeviceToCloudMessageAsync(string msg)
    {
        var message = new Message(Encoding.ASCII.GetBytes(msg));

        await deviceClient.SendEventAsync(message);
    }

    /// <summary>
    /// receive messages from the cloud
    /// </summary>
    /// <returns></returns>
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
