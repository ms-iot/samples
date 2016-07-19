using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Geolocation;
using Windows.UI.Xaml.Controls.Maps;

namespace IoTHubBuddy
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
        /// <summary>
        /// ensure validity of message format by checking for the necessary fields
        /// </summary>
        /// <param name="msg"></param>
        /// <returns></returns>
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
