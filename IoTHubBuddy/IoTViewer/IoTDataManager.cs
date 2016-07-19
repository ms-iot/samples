using IoTHubBuddy.Models;
using Microsoft.Azure.Devices;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Text;
using System.Threading.Tasks;
using Windows.Data.Json;

namespace IoTHubBuddy
{
    static class IoTDataManager
    {
        private const string AzureResourceUri = "https://management.azure.com/";

        /// <summary>
        /// check validity for returned json object. If it doesn't contain a value, then there was an error in gathering the data
        /// Error parsing/handling will be implemented later
        /// </summary>
        /// <param name="jo"></param>
        /// <returns></returns>
        private static bool checkJsonObject(JsonObject jo)
        {
            if (!jo.ContainsKey("value"))
            {
                return false;
            }
            return true;
        }
        /// <summary>
        /// sends a GET resquest to azure management REST API
        /// </summary>
        /// <param name="token"></param>
        /// <param name="relative"></param>
        /// <returns></returns>
        public static async Task<JsonObject> GetIoTData(string token, string relative)
        {
            if(token == null)
            {
                return null;
            }
            using(var client = new HttpClient())
            {
                client.DefaultRequestHeaders.Authorization = new System.Net.Http.Headers.AuthenticationHeaderValue("Bearer", token);
                client.DefaultRequestHeaders.Accept.Add(new System.Net.Http.Headers.MediaTypeWithQualityHeaderValue("application/json"));
                var restApi = new Uri(AzureResourceUri + relative);
                var infoResult = await client.GetAsync(restApi);
                string content = await infoResult.Content.ReadAsStringAsync();
                var jsonObject = JsonObject.Parse(content);
                if(checkJsonObject(jsonObject))
                {
                   return jsonObject;
                } else
                {
                    return null;
                }
            }

        }
        /// <summary>
        /// sends POST request to azure management REST API
        /// </summary>
        /// <param name="relative"></param>
        /// <returns></returns>
        public static async Task<JsonObject> PostIoTData(string relative)
        {
            string token = await AccountManager.GetTokenSilentlyAsync();
            if(token == null)
            {
                return null;
            }
            using(var client = new HttpClient())
            {
                client.BaseAddress = new Uri("https://management.azure.com/");
                var message = new HttpRequestMessage(HttpMethod.Post, relative);

                message.Headers.Authorization = new AuthenticationHeaderValue("Bearer", token);

                message.Headers.AcceptLanguage.TryParseAdd("en-US");
                message.Headers.Add("x-ms-version", "2013-11-01");
                message.Headers.Accept.Add(new MediaTypeWithQualityHeaderValue("application/json"));

                var infoResult = await client.SendAsync(message);
                string content = await infoResult.Content.ReadAsStringAsync();
                var jsonObject = JsonObject.Parse(content);
                if (!checkJsonObject(jsonObject))
                {
                    return null;
                }
                return jsonObject;
            }
            
        }
        /// <summary>
        /// Get a list of azure subscriptions
        /// </summary>
        /// <param name="token"></param>
        /// <returns></returns>
        public static async Task<ICollection<string>> GetSubscription(string token)
        {
            List<string> subscriptionIds = new List<string>();
            string relative = "subscriptions?api-version=2015-01-01";
            JsonObject result = await GetIoTData(token, relative);
            if(result != null)
            {
                var subscriptions = result["value"].GetArray();
                foreach (var subscription in subscriptions)
                {
                    string idContent = subscription.GetObject().GetNamedValue("subscriptionId").GetString();
                    subscriptionIds.Add(idContent);
                }
            }
            
            return subscriptionIds;
        }
        /// <summary>
        /// get a list of resource group for a specific azure subscription
        /// </summary>
        /// <param name="subId"></param>
        /// <returns></returns>
        public static async Task<ICollection<string>> GetResourceGroups(string subId)
        {
            List<string> groupNames = new List<string>();
            var relative = "subscriptions/" + subId + "/resourcegroups?api-version=2015-01-01";
            string token = await AccountManager.GetTokenSilentlyAsync();
            JsonObject result = await GetIoTData(token, relative);
            if(result != null)
            {
                var groups = result["value"].GetArray();
                foreach (var group in groups)
                {
                    string idContent = group.GetObject().GetNamedValue("name").GetString();
                    groupNames.Add(idContent);
                }
            }
            
            return groupNames;
        }
        /// <summary>
        /// get a list of iot hubs for a specific subscription
        /// </summary>
        /// <param name="subId"></param>
        /// <returns></returns>
        public static async Task<ICollection<EventHubData>> GetIoTHubs(string subId)
        {
            List<EventHubData> iothubs = new List<EventHubData>();
            string relative = "subscriptions/" + subId + "/providers/Microsoft.Devices/IotHubs?api-version=2016-02-03";
            string token = await AccountManager.GetTokenSilentlyAsync();
            JsonObject result = await GetIoTData(token,relative);
            if(result != null)
            {
                var hubs = result["value"].GetArray();
                foreach (var hub in hubs)
                {
                    var events = hub.GetObject().GetNamedValue("properties").GetObject().GetNamedValue("eventHubEndpoints").GetObject().GetNamedValue("events").GetObject();
                    string idContent = hub.GetObject().GetNamedValue("name").GetString();
                    var partIds = events.GetNamedValue("partitionIds").GetArray();
                    List<string> ids = new List<string>();
                    foreach (var id in partIds)
                    {
                        ids.Add(id.GetString());
                    }
                    string path = events.GetNamedValue("path").GetString();
                    string endpoint = events.GetNamedValue("endpoint").GetString();
                    iothubs.Add(new EventHubData(ids, path, endpoint, idContent));
                }
            }
            
            return iothubs;
        }
        /// <summary>
        /// get the primary key for the iot device
        /// </summary>
        /// <param name="subId"></param>
        /// <param name="group"></param>
        /// <param name="hubName"></param>
        /// <param name="policy"></param>
        /// <returns></returns>
        public static async Task<string> GetPrimaryKey(string subId, string group, string hubName, string policy)
        {
           
            string relative = "subscriptions/" + subId + "/resourceGroups/" + group + "/providers/Microsoft.Devices/IotHubs/" + hubName + "/IoTHubKeys/listKeys?api-version=2015-08-15-preview";
            JsonObject result = await PostIoTData(relative);
            string key = null;
            if(result != null)
            {
                var keys = result["value"].GetArray();
                foreach (var k in keys)
                {
                    string keyName = k.GetObject().GetNamedValue("keyName").GetString();
                    if (keyName == policy)
                    {
                        key = k.GetObject().GetNamedValue("primaryKey").GetString();
                    }

                }
            }
            return key;
        }
        /// <summary>
        /// get a list of iot devices
        /// </summary>
        /// <param name="subId"></param>
        /// <param name="group"></param>
        /// <param name="hubname"></param>
        /// <param name="policy"></param>
        /// <returns></returns>
        public static async Task<ICollection<string>> GetIoTDevices(string subId, string group, string hubname, string policy)
        {
            string key = await GetPrimaryKey(subId, group, hubname, policy);
            List<string> devices = new List<string>();
            if(key != null)
            {
                var connectionString = "HostName=" + hubname + ".azure-devices.net;SharedAccessKeyName=" + policy + ";SharedAccessKey=" + key;
                var registryManager = RegistryManager.CreateFromConnectionString(connectionString);
                var devs = await registryManager.GetDevicesAsync(1000);
                foreach (var dev in devs)
                {
                    devices.Add(dev.Id);
                }
            }
            return devices;
        }
    }
}
