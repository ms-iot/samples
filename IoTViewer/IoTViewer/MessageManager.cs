using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Geolocation;
using Windows.UI.Xaml.Controls.Maps;

namespace IoTViewer
{
    class MessageManager
    {
        MainPage m_view;
        public MessageManager(MainPage view)
        {
            m_view = view;
        }
        public void parseMessage(string msg)
        {
            JObject jsonMsg = JObject.Parse(msg);
            Debug.WriteLine(jsonMsg);
            if (validateMessage(jsonMsg))
            {
                string type = jsonMsg["message"]["type"].ToString();
                if (type == "coordinates")
                {
                    this.SetMapLocation(new Message(jsonMsg["message"]["latitude"].ToString(), jsonMsg["message"]["longitude"].ToString(), jsonMsg["time"].ToString()));
                }
                this.AddToLog(msg);
            }
            else
            {
                this.AddToLog("An invalid message was sent. Please check your IoT device");
            }
        }
        public void SetMapLocation(Message locMsg)
        {
            m_view.SetMapLocation(Double.Parse(locMsg.lat), Double.Parse(locMsg.lng), locMsg.timestamp.ToString());
        }
        public void AddToLog(string locMsg)
        {
            m_view.AddMessageToLog(locMsg);
        }
        private static bool validateMessage(JObject msg)
        {
            if (msg["message"] == null)
            {
                return false;
            }
            if (msg["message"]["type"] == null)
            {
                return false;
            }
            if (msg["time"] == null)
            {
                return false;
            }
            return true;
        }
    }
}
