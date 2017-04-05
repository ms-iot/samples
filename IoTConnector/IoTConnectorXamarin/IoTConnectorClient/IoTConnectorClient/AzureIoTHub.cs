using System;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Azure.Devices.Client;
using Newtonsoft.Json;
using Microsoft.Devices.Tpm;

namespace IoTConnectorClient
{
    class AzureIoTHub
    {

        private string devConnectString = "";
        private DeviceClient deviceClient;
        private string devID = "";
        private HubController hc;

        private static readonly long StartOfEpoch = (new DateTime(1970, 1, 1, 0, 0, 0, 0)).Ticks;
        public AzureIoTHub(HubController hub)
        {
            hc = hub;
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
            catch (System.Exception e)
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
            catch (System.Exception e)
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
        public async Task ReceiveCloudToDeviceMessageAsync(DateTime start)
        {

            while (true)
            {
                Message receivedMessage = null;

                try
                {
                    this.Connect();
                    receivedMessage = await deviceClient.ReceiveAsync();
                } catch (System.Exception e)
                {
                    try
                    {
                        this.Connect();
                        receivedMessage = await deviceClient.ReceiveAsync();
                    } catch (System.Exception ex)
                    {
                        await hc.ParseReceivedMessage("there was an issue receiving messages");
                    }
                    
                }


                if (receivedMessage != null)
                {
                    string totalMilliseconds = ((long)(start - new DateTime(StartOfEpoch, DateTimeKind.Utc)).TotalMilliseconds).ToString();
                    long delta = (long)(receivedMessage.EnqueuedTimeUtc.UtcDateTime - new DateTime(StartOfEpoch, DateTimeKind.Utc)).TotalMilliseconds - long.Parse(totalMilliseconds);
                    if (delta > 0)
                    {
                        var messageData = Encoding.ASCII.GetString(receivedMessage.GetBytes());
                        await deviceClient.CompleteAsync(receivedMessage);
                        await hc.ParseReceivedMessage(messageData);
                    } else
                    {
                        await deviceClient.RejectAsync(receivedMessage);
                    }
                }

                await Task.Delay(TimeSpan.FromSeconds(1));
            }

        }

    }

}
