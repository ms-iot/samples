using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IoTConnector
{
    class Message
    {
        public string lat { get; set; }
        public string lng { get; set; }
        public string timestamp { get; set; }


        public Message(string latitude, string longitude, string time)
        {
            lat = latitude;
            lng = longitude;
            timestamp = time;
        }
    }
}
