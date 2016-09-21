using IoTConnector.Models;
using Microsoft.Azure.Devices;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Text;
using System.Threading.Tasks;
using Windows.Data.Json;

namespace IoTConnector
{
    static class IoTDataManager
    {
        private const string AzureResourceUri = "https://management.azure.com/";
        private const string AzureResourceApi = "2015-01-01";
        private const string IoTHubApi = "2016-02-03";
        private static string token;

        /// <summary>
        /// check validity for returned json object. If it doesn't contain a value, then there was an error in gathering the data
        /// Error parsing/handling will be implemented later
        /// </summary>
        /// <param name="jo"></param>
        /// <returns></returns>
        private static bool checkJsonObject(JsonObject jo)
        {
            return jo.ContainsKey("value");
        }
        /// <summary>
        /// sends a GET resquest to azure management REST API
        /// </summary>
        /// <param name="token"></param>
        /// <param name="relative"></param>
        /// <returns></returns>
        public static async Task<JsonObject> GetIoTData(string relative, string tenant="common")
        {
            try
            {
                if(token == null)
                {
                    token = await AccountManager.GetAzureAuthenticationToken(tenant);

                }
                using (var client = new HttpClient())
                {
                    client.DefaultRequestHeaders.Authorization = new System.Net.Http.Headers.AuthenticationHeaderValue("Bearer", token);
                    client.DefaultRequestHeaders.Accept.Add(new System.Net.Http.Headers.MediaTypeWithQualityHeaderValue("application/json"));
                    var restApi = new Uri(AzureResourceUri + relative);
                    var infoResult = await client.GetAsync(restApi);
                    string content = await infoResult.Content.ReadAsStringAsync();
                    var jsonObject = JsonObject.Parse(content);
                    if (checkJsonObject(jsonObject))
                    {
                        return jsonObject;
                    }
                    else
                    {
                        string message = "The response object was malformed.";
                        if(jsonObject.ContainsKey("error"))
                        {
                            message = jsonObject.GetObject().GetNamedValue("error").GetString();
                        }
                        throw new System.Exception(message);
                        
                    }
                }
            } catch (System.Exception e)
            {
                throw new System.Exception(e.Message);
            }
            

        }
        /// <summary>
        /// sends POST request to azure management REST API
        /// </summary>
        /// <param name="relative"></param>
        /// <returns></returns>
        public static async Task<JsonObject> PostIoTData(string relative)
        {
            try
            {
                using (var client = new HttpClient())
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
                    if (checkJsonObject(jsonObject))
                    {
                        return jsonObject;
                    }
                    throw new System.Exception(jsonObject.GetObject().GetNamedValue("error").GetString());
                    
                }
            } catch (System.Exception e)
            {
                throw e;
            }


        }
        /// <summary>
        /// Get a list of azure subscriptions
        /// </summary>
        /// <param name="token"></param>
        /// <returns></returns>
        public static async Task<ICollection<string>> GetSubscription(string tenant)
        {
            List<string> subscriptionIds = new List<string>();
            string relative = "subscriptions?api-version="+AzureResourceApi;
            try
            {
                token = null;
                JsonObject result = await GetIoTData(relative, tenant);
                if (result != null)
                {
                    var subscriptions = result["value"].GetArray();
                    subscriptionIds.AddRange(subscriptions.Select(_ => _.GetObject().GetNamedValue("subscriptionId").GetString()));
                }
            } catch(System.Exception e)
            {
                Debug.WriteLine(e.Message);
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
            var relative = "subscriptions/" + subId + "/resourcegroups?api-version="+AzureResourceApi;
            try
            {
                JsonObject result = await GetIoTData(relative);
                if (result != null)
                {
                    var groups = result["value"].GetArray();
                    groupNames.AddRange(groups.Select(_ => _.GetObject().GetNamedValue("name").GetString()));
                }
            } catch(System.Exception e)
            {

                Debug.WriteLine(e.Message);
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
            string relative = "subscriptions/" + subId + "/providers/Microsoft.Devices/IotHubs?api-version="+IoTHubApi;
            try
            {
                JsonObject result = await GetIoTData(relative);
                if (result != null)
                {
                    var hubs = result["value"].GetArray();
                    foreach (var hub in hubs)
                    {
                        var events = hub.GetObject().GetNamedValue("properties").GetObject().GetNamedValue("eventHubEndpoints").GetObject().GetNamedValue("events").GetObject();
                        string idContent = hub.GetObject().GetNamedValue("name").GetString();
                        var partIds = events.GetNamedValue("partitionIds").GetArray();
                        List<string> ids = partIds.Select(_=>_.GetString()).ToList();
                        string path = events.GetNamedValue("path").GetString();
                        string endpoint = events.GetNamedValue("endpoint").GetString();
                        iothubs.Add(new EventHubData(ids, path, endpoint, idContent));
                    }
                }
            
            } catch(System.Exception e)
            {
                Debug.WriteLine(e.Message);
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
            try
            {
                JsonObject result = await PostIoTData(relative);
                var keys = result["value"].GetArray().SkipWhile(k => k.GetObject().GetNamedValue("keyName").GetString() != policy);
                if(keys.Count() != 0)
                {
                    return keys.First().GetObject().GetNamedValue("primaryKey").GetString();
                }
            } catch (System.Exception e)
            {
                throw e;
            }
            throw new System.Exception("Accessing the primary key failed");
            
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
            List<string> devices = new List<string>();
            try
            {
                string key = await GetPrimaryKey(subId, group, hubname, policy);
                if (key != null)
                {
                    var connectionString = "HostName=" + hubname + ".azure-devices.net;SharedAccessKeyName=" + policy + ";SharedAccessKey=" + key;
                    var registryManager = RegistryManager.CreateFromConnectionString(connectionString);
                    var devs = await registryManager.GetDevicesAsync(1000);
                    devices.AddRange(devs.Select(_ => _.Id));
                }
            } catch (System.Exception e)
            {
                Debug.WriteLine(e);
            }
            
            return devices;
        }
        /// <summary>
        /// gets all the tenants that a user belongs to
        /// </summary>
        /// <param name="tkn"></param>
        /// <returns></returns>
        public static async Task<ICollection<string>> GetTenants(string tkn)
        {
            List<string> tenantIds = new List<string>();
            string relative = "tenants?api-version=" + AzureResourceApi;
            try
            {
                token = tkn;
                JsonObject result = await GetIoTData(relative);
                if (result != null)
                {
                    var tenants = result["value"].GetArray();
                    tenantIds.AddRange(tenants.Select(_ => _.GetObject().GetNamedValue("tenantId").GetString()));
                }
            }
            catch (System.Exception e)
            {
                Debug.WriteLine(e.Message);
            }


            return tenantIds;
        }
        /// <summary>
        /// Given a token from the logged in user, retrieve the list of all devices registered to that user under any IoT Hub
        /// </summary>
        /// <param name="tkn"></param>
        /// <returns></returns>
        public static async Task<ICollection<IoTAccountData>> GetAllDevices(string tkn)
        {
            ICollection<string> tenants = await GetTenants(tkn);
            List<IoTAccountData> devices = new List<IoTAccountData>();
            string name = AccountManager.GetUserName();
            string policy = "iothubowner";
            foreach (string ten in tenants)
            {
                ICollection<string> subscriptions = await GetSubscription(ten);
                foreach(string sub in subscriptions)
                {
                    ICollection<string> resources = await GetResourceGroups(sub);
                    ICollection<IoTConnector.Models.EventHubData> hubs = await GetIoTHubs(sub);
                    foreach(string group in resources)
                    {
                        foreach(EventHubData hub in hubs)
                        {
                            ICollection<string> devs = await GetIoTDevices(sub, group, hub.HubName, policy);
                            
                            foreach(string dev in devs)
                            {
                                string key = await GetPrimaryKey(sub, group, hub.HubName, policy);
                                IoTAccountData account = new IoTAccountData(ten, name, sub, group, hub.HubName, dev, policy, key, hub);
                                devices.Add(account);
                            }
                            
                            
                        }
                    }

                }

            }
            return devices;
        }
    }
}
