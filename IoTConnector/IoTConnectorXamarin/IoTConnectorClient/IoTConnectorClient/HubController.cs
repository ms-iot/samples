using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IoTConnectorClient
{
    class HubController
    {
        private LocationProvider lp;
        private AzureIoTHub msgHub;
        private HttpInterfaceManager view;
        public HubController(LocationProvider loc, HttpInterfaceManager server)
        {
            view = server;
            lp = loc;
            msgHub = new AzureIoTHub(this);
        }
        public async Task<bool> Connect()
        {
            try
            {
                msgHub.Connect();
                msgHub.ReceiveCloudToDeviceMessageAsync(DateTime.UtcNow);
                return true;
            } catch(System.Exception e)
            {
                return false;
            }
        }
        private async Task<string> GenerateCoordinateMessage(string device)
        {
            string coords = await lp.GetLocation();
            Debug.WriteLine(coords);
            string[] parsedmsg = coords.Split(',');
            var update = new
            {
                type = "pinUpdate",
                latitude = parsedmsg[0],
                longitude = parsedmsg[1],
                pin = BlinkyController.PinNumber(),
                pinStatus = BlinkyController.PinStatus()
            };

            var str = new
            {
                message = update,
                time = DateTime.Now.ToString(),
                receiver = device,
                sender = msgHub.GetDeviceId(),
                version = Constants.VERSION

            };
            var fullMsg = JsonConvert.SerializeObject(str);
            Debug.WriteLine(fullMsg);
            return fullMsg;
        }

        public async Task<string> sendCoordinateMessage(string device)
        {
            string msg = await GenerateCoordinateMessage(device);
            try
            {
                await msgHub.SendDeviceToCloudMessageAsync(msg);
                return msg;
            } catch (System.Exception e )
            {
                throw new Exception(e.Message);
            }
        }
        
        private void ExecuteLightCommand(JToken msg)
        {
            var pin = msg["pin"];
            if (pin != null)
            {
                BlinkyController.Run(Int32.Parse(pin.ToString()));
                
            }
        }
        public async Task ParseReceivedMessage(string message)
        {
            view.addListElement(message);
            JObject jsonMsg = JObject.Parse(message);
            Debug.WriteLine(jsonMsg);
            var msg = jsonMsg["message"];
            if (msg != null)
            {
                var version = jsonMsg["version"].ToString();
                if (version == Constants.VERSION)
                {
                    var type = msg["type"];
                    if (type != null)
                    {
                        switch (type.ToString().ToUpper())
                        {
                            case "LIGHT":
                                ExecuteLightCommand(msg);
                                try
                                {
                                    await sendCoordinateMessage(jsonMsg["sender"].ToString());
                                }
                                catch (System.Exception e)
                                {
                                    Debug.WriteLine("sending response failed");
                                }
                                break;
                        }
                    }
                }

            }
        }
    }
}
