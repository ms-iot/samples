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
        static string port = "ihsuprodbyres039dednamespace.servicebus.windows.net";
        static string eventHubEntity = "iothub-ehub-hub-dimaha-42362-31f5229b77";
        static string keyname = "iothubowner";
        static string key = "JSqXsxpMIi/VFmts3/GRFXUgjhzLGerci4XVkhH+yHA=";
        private static readonly long StartOfEpoch = (new DateTime(1970, 1, 1, 0, 0, 0, 0)).Ticks;
        
        public static async Task ReceiveMessages(string partition, DateTime offset, MessageManager msgman)
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
                        msgman.parseMessage(msg);
                        
                    }
                   
                }
            }
        }
        


    }

}
