using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace XamarinIoTViewer
{
    class MessageController
    {
        private IViewer view;
        private AzureIoTHub hub;
        public MessageController(IViewer currview)
        {
            view = currview;
            hub = new AzureIoTHub();
            hub.ReceiveCloudToDeviceMessageAsync(this);
        }
        public async Task SendLightMessage(string pinNum, string device)
        {

            var pinmsg = new
            {
                type = "light",
                pin = pinNum
            };
            var str = new
            {
                message = pinmsg,
                time = DateTime.UtcNow.ToString(),
                receiver = device,
                sender = hub.getDeviceId(),
                version = "1.1"

            };
            var fullMsg = JsonConvert.SerializeObject(str);
            await hub.SendDeviceToCloudMessageAsync(fullMsg);

        }
        public void ShowMessage(string msg)
        {
            
            JObject parsed = parseMessage(msg);
            if(parsed != null)
            {
                view.showMessage(parsed);
            }
        }
        public JObject parseMessage(string message)
        {
            JObject jsonMsg = JObject.Parse(message);
            var msg = jsonMsg["message"];
            if (msg != null)
            {
                var version = jsonMsg["version"].ToString();
                if (version == "1.1")
                {
                    var type = msg["type"];
                    if (type != null)
                    {
                        if(type.ToString().ToUpper() == "PINUPDATE")
                        {
                            var lat = msg["latitude"];
                            var lng = msg["longitude"];
                            if (lat != null && lng != null)
                            {
                                return jsonMsg;
                            }
                        }
                        }
                    }
                }
            return null;
        }
    }
}

