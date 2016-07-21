using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace IoTHubBuddy.Models
{
    public class IoTAccountData
    {

        public string Name { get; set; }

        public string Subscription { get; set; }
        
        public string ResourceGroup { get; set; }

        public string HubName { get; set; }

        public string DeviceName { get; set; }

        public string SharedAccessPolicy { get; set; }

        public string PrimaryKey { get; set; }

        public EventHubData EventHubInfo { get; set; }


        
    }
}
