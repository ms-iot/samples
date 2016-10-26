using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace IoTConnector.Models
{
    /// <summary>
    /// Data object that holds necessary iot hub information
    /// </summary>
    public class IoTAccountData
    {
        public string Tenant { get; set; }
        public string Name { get; set; }

        public string Subscription { get; set; }
        
        public string ResourceGroup { get; set; }

        public string HubName { get; set; }

        public string DeviceName { get; set; }

        public string SharedAccessPolicy { get; set; }

        public string PrimaryKey { get; set; }

        public EventHubData EventHubInfo { get; set; }

        public IoTAccountData()
        {

        }
        public IoTAccountData(string tenant, string name, string subscription, string rg, string hub, string device, string policy, string key, EventHubData data)
        {
            this.Tenant = tenant;
            this.Name = name;
            this.Subscription = subscription;
            this.ResourceGroup = rg;
            this.HubName = hub;
            this.DeviceName = device;
            this.SharedAccessPolicy = policy;
            this.PrimaryKey = key;
            this.EventHubInfo = data;
        }
        public static IoTAccountData Clone(IoTAccountData data)
        {
            return new IoTAccountData(data.Tenant, data.Name, data.Subscription, data.ResourceGroup, data.HubName, data.DeviceName, data.SharedAccessPolicy, data.PrimaryKey, data.EventHubInfo);
        }
        
    }
}
