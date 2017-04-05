using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace IoTConnectorClient
{
    class DeviceConnectionStringBuilder
    {
        private const string HOSTNAME = "HostName";
        private const string DEVICE_ID = "DeviceId";
        private const string SHARED_ACCESS_KEY = "SharedAccessKey";

        public string SharedAccessKey { get; set; }
        public string DeviceId { get; set; }
        public string HostName { get; set; }

        public DeviceConnectionStringBuilder() { }
        public DeviceConnectionStringBuilder(string connectionString)
        {
            Regex reg = new Regex("(^=;]+)=([^;]+)");
            IDictionary<string, string> deviceParams = new Dictionary<string, string>();
            MatchCollection matches = reg.Matches(connectionString);
            foreach(Match m in matches)
            {
                deviceParams[m.Groups[1].Value] = m.Groups[2].Value;
            }
            this.HostName = deviceParams[HostName];
            this.DeviceId = deviceParams[DeviceId];
            this.SharedAccessKey = deviceParams[SharedAccessKey];
        }

        public static string CreateWithDeviceInfo(string host, string device, string sharedaccesskey)
        {
            DeviceConnectionStringBuilder devbuild = new DeviceConnectionStringBuilder
            {
                SharedAccessKey = sharedaccesskey,
                DeviceId = device,
                HostName = host
            };
            return devbuild.ToString();

        }
        public override string ToString()
        {
            StringBuilder builder = new StringBuilder();
            string separator = ";";
            if(!string.IsNullOrEmpty(this.HostName))
            {
                builder.Append(HOSTNAME+"="+this.HostName);
            }
            if(!string.IsNullOrEmpty(this.DeviceId))
            {
                builder.Append(separator);
                builder.Append(DEVICE_ID + "=" + this.DeviceId);
            }
            if(!string.IsNullOrEmpty(this.SharedAccessKey))
            {
                builder.Append(separator);
                builder.Append(SHARED_ACCESS_KEY + "=" + this.SharedAccessKey);
            }
            return builder.ToString();
        }
    }
}
