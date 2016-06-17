using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Azure.Devices;
using Microsoft.Azure.Devices.Client;
using Microsoft.WindowsAzure.Storage.Blob;
using Amqp;
using Amqp.Framing;
using System.Text.RegularExpressions;
using System.Collections;
using System.Diagnostics;
using Amqp.Types;
using Newtonsoft.Json.Linq;

namespace IoTViewer
{
    class AzureIoT
    {
        private const string ENDPOINT = "Endpoint";
        private const string SHARED_ACCESS_KEY_NAME = "SharedAccessKeyName";
        private const string SHARED_ACCESS_KEY = "SharedAccessKey";
        private const string SHARED_ACCESS_SIGNATURE = "SharedAccessSignature";
        private const string ENTITY_PATH = "EntityPath";
        private const string PUBLISHER = "Publisher";
        private const string TRANSPORT_TYPE = "TransportType";
        const string deviceConnectionString = "HostName=hub-dimaha.azure-devices.net;DeviceId=dimaha01;SharedAccessKey=Pp+rGghQKmhx4Rdl4OaUGF9dcwHZ+n+Zt3jQwO3xFic=";
        static string port = "ihsuprodbyres039dednamespace.servicebus.windows.net";
        static string eventHubEntity = "iothub-ehub-hub-dimaha-42362-31f5229b77";
        static string keyname = "iothubowner";
        static string key = "JSqXsxpMIi/VFmts3/GRFXUgjhzLGerci4XVkhH+yHA=";
        private static readonly long StartOfEpoch = (new DateTime(1970, 1, 1, 0, 0, 0, 0)).Ticks;
        
        public static async Task<Message> ReceiveMessages(string partition, DateTime offset, MapManager mappy)
        {
            Address address = new Address(port, 5671, keyname, key, "/", "amqps");
            Connection connection = await Connection.Factory.CreateAsync(address);
            Session session = new Session(connection);
            string totalMilliseconds = ((long)(offset- new DateTime(StartOfEpoch, DateTimeKind.Utc)).TotalMilliseconds).ToString();
            Map filters = new Map();
            filters.Add(new Amqp.Types.Symbol("apache.org:selector-filter:string"),
                                        new DescribedValue(
                                            new Amqp.Types.Symbol("apache.org:selector-filter:string"),
                                            "amqp.annotation.x-opt-enqueuedtimeutc > " + totalMilliseconds + ""));
            ReceiverLink receiver = new ReceiverLink(session,
                "my-receiver",
                new global::Amqp.Framing.Source()
                {
                    Address =
                eventHubEntity + "/ConsumerGroups/$Default/Partitions/"+partition,
                    FilterSet = filters
                }, null);
            Amqp.Types.Symbol deviceIdKey = new Amqp.Types.Symbol("iothub-connection-device-id");
            string deviceId = "dimaha01";
            while (true)
            {
                Amqp.Message m = await receiver.ReceiveAsync(10000);
                if (m != null)
                {
                    var id = m.MessageAnnotations.Map[deviceIdKey].ToString();
                    if (id == deviceId)
                    {
                        receiver.Accept(m);
                        Data data = (Data)m.BodySection;
                        string msg = System.Text.Encoding.UTF8.GetString(data.Binary, 0, data.Binary.Length);
                        JObject jsonMsg = JObject.Parse(msg);
                        Debug.WriteLine(jsonMsg);
                        if (validateMessage(jsonMsg))
                        {
                            string type = jsonMsg["message"]["type"].ToString();
                            if (type == "coordinates")
                            {
                                mappy.SetMapLocation(new Message(jsonMsg["message"]["latitude"].ToString(), jsonMsg["message"]["longitude"].ToString(), jsonMsg["time"].ToString()));
                            }
                        }
                    }
                   
                }
            }
        }
        private static  bool validateMessage(JObject msg)
        {
            if(msg["message"] == null)
            {
                return false;
            }
            if(msg["message"]["type"] == null)
            {
                return false;
            }
            if(msg["time"] == null)
            {
                return false;
            }
            return true;
        }


    }

}
