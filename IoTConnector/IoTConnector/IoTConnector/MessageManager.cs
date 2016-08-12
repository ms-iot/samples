using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Geolocation;
using Windows.UI.Xaml.Controls.Maps;

namespace IoTConnector
{
    class MessageManager
    {
        MapPage m_view;
        public MessageManager(MapPage view)
        {
            m_view = view;
        }
        /// <summary>
        /// parse message, check validity of message format, display coordinates or error message accordingly
        /// </summary>
        /// <param name="msg"></param>
        public bool parseMessage(string msg)
        {
            JObject jsonMsg = JObject.Parse(msg);
            Debug.WriteLine(jsonMsg);
            string status = "An invalid message was sent. Please check your IoT device";
            var message = jsonMsg["message"];
            bool isValid = false;
            if(message != null)
            {
                var version = message["version"].ToString();
                if(version == Constants.MESSAGE_VERSION)
                {
                    var type = message["type"];
                    if (type != null)
                    {
                        var time = jsonMsg["time"];
                        if (time != null)
                        {
                            if (type.ToString() == "coordinates")
                            {
                                this.SetMapLocation(new Message(jsonMsg["message"]["latitude"].ToString(), jsonMsg["message"]["longitude"].ToString(), jsonMsg["time"].ToString()));
                                status = msg;
                                isValid = true;
                            }
                        }
                    }
                }
                
            }
            this.AddToLog(status);
            return isValid;
        }
        /// <summary>
        /// set the pin location on the ui map
        /// </summary>
        /// <param name="locMsg"></param>
        public void SetMapLocation(Message locMsg)
        {
            m_view.SetMapLocation(Double.Parse(locMsg.lat), Double.Parse(locMsg.lng), locMsg.timestamp.ToString());
        }
        /// <summary>
        /// add the desired message to the message log
        /// </summary>
        /// <param name="locMsg"></param>
        public void AddToLog(string locMsg)
        {
            m_view.AddMessageToLog(locMsg);
        }
       
    }
}
