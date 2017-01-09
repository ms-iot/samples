using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Amqp;
using Amqp.Framing;
using System.Diagnostics;
using Amqp.Types;
using Newtonsoft.Json.Linq;
using IoTConnector.Models;


namespace IoTConnector
{
    class AzureIoT
    {
        private static readonly long StartOfEpoch = (new DateTime(1970, 1, 1, 0, 0, 0, 0)).Ticks;
        /// <summary>
        /// Receive messages from specified azure iothub on specified partition. The MessageManager parses the received message and displays it accordingly
        /// </summary>
        /// <param name="partition"></param>
        /// <param name="offset"></param>
        /// <param name="msgman"></param>
        /// <param name="hubData"></param>
        /// <returns></returns>
        public static async Task ReceiveMessages(string partition, DateTime offset, MessageManager msgman, IoTAccountData hubData)
        {
            string port = hubData.EventHubInfo.EventHubPort.Replace("sb://", "");
            port = port.Replace("/", "");
            Address address = new Address(port, 5671, hubData.SharedAccessPolicy, hubData.PrimaryKey, "/", "amqps");
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
                hubData.EventHubInfo.EventHubEntity + "/ConsumerGroups/$Default/Partitions/"+partition,
                    FilterSet = filters
                }, null);
            Amqp.Types.Symbol deviceIdKey = new Amqp.Types.Symbol("iothub-connection-device-id");
            string deviceId = hubData.DeviceName;
            while (true)
            {
                Amqp.Message m = await receiver.ReceiveAsync(10000);
                if (m != null)
                {
                    var id = m.MessageAnnotations.Map[deviceIdKey].ToString();
                    if (id == deviceId)
                    {
                        Data data = (Data)m.BodySection;
                        string msg = System.Text.Encoding.UTF8.GetString(data.Binary, 0, data.Binary.Length);
                        bool isValid = msgman.parseMessage(msg);
                        if(isValid)
                        {
                            receiver.Accept(m);
                        } else
                        {
                            receiver.Release(m);
                        }
                        
                        
                        
                    }
                   
                }
            }
        }
        


    }

}
