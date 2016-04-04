using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Diagnostics;
using System.Net.Http;
using System.Threading.Tasks;
using Microsoft.IdentityModel.Clients.ActiveDirectory;
using Newtonsoft.Json;
using Windows.UI.Xaml.Controls;
using System.Text;
using System.Text.RegularExpressions;
using System.Runtime.Serialization;

namespace WeatherStation
{
    // Power BI Datasets
    [DataContract]
    public class PBIDatasets
    {
        [DataMember(Name = "value")]
        public Dataset[] Datasets { get; set; }
    }

    public class Dataset
    {
        [DataMember(Name = "id")]
        public string Id { get; set; }

        [DataMember(Name = "name")]
        public string Name { get; set; }
    }

    class PBIClient : IPBIClient
    {
        // WeatherPBIApp: replace with the actual Client ID from the Azure Application:
        private const string clientID = "<replace>";

        // RedirectUri you used when you registered your app.
        // For a client app, a redirect uri gives AAD more details on the specific application that it will authenticate.
        private const string redirectUri = "https://login.live.com/oauth20_desktop.srf";

        // Resource Uri for Power BI API
        private const string resourceUri = "https://analysis.windows.net/powerbi/api";

        // OAuth2 authority Uri
        private const string authority = "https://login.windows.net/common/oauth2/authorize";

        // Uri for Power BI datasets
        private string datasetsUri = "https://api.powerbi.com/v1.0/myorg/datasets";

        public PBIClient()
        {
        }

        public async Task ConnectAsync()
        {
            // Check the connection for redirects
            HttpWebRequest request = System.Net.WebRequest.Create(datasetsUri) as System.Net.HttpWebRequest;
            request.Method = "GET";
            request.ContentType = "application/json";
            request.Headers["Authorization"] = String.Format("Bearer {0}", await AccessTokenAsync());

            try
            {
                HttpWebResponse response = (HttpWebResponse)await request.GetResponseAsync();
                if (response.StatusCode == HttpStatusCode.TemporaryRedirect)
                {
                    datasetsUri = response.Headers["Location"];
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
        }

        public async Task CreateDatasetAsync(string datasetName)
        {
            try
            {
                // Create a POST web request to list all datasets
                HttpWebRequest request = await CreateDatasetRequest("POST");

                var datasets = await GetDatasetsWithNameAsync(datasetName);

                if (!datasets.Any())
                {
                    // POST request using the json schema from WeatherData
                    await PostRequestAsync(request, JSONBuilder.ToJsonSchema(new WeatherData(), datasetName));
                }
                else
                {
                    System.Diagnostics.Debug.WriteLine("Dataset exists");
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
        }

        public async Task<string> PushDataToTableAsync<T>(Dataset dataset, string tableName, T data)
        {
            HttpWebRequest request = await CreateDatasetRequest(dataset, tableName, "POST");
            var str = JSONBuilder.ToJson(data);
            return await PostRequestAsync(request, str);
        }

        public async Task<PBIDatasets> GetAllDatasetsAsync()
        {
            // Create a GET web request to list all datasets
            HttpWebRequest request = await CreateDatasetRequest("GET");

            // Get HttpWebResponse from GET request
            string responseContent = await GetResponseAsync(request);

            // Deserialize JSON string from response
            var PBIDatasets = JsonConvert.DeserializeObject<PBIDatasets>(responseContent);

            return PBIDatasets;
        }

        public async Task<IEnumerable<Dataset>> GetDatasetsWithNameAsync(string name)
        {
            var datasets = (await GetAllDatasetsAsync()).Datasets;
            if (datasets == null)
            {
                return new List<Dataset>(); // empty
            }
            var q = from d in datasets where d.Name == name select d;
            return q;
        }

        public async Task ClearRowsAsync(string datasetName, string tableName)
        {
            // Get dataset id from a table name
            var all_datasets = await GetDatasetsWithNameAsync(datasetName);
            if (all_datasets.Any())
            {
                var dataset = all_datasets.First();

                // Create a DELETE web request
                HttpWebRequest request = await CreateDatasetRequest(dataset, tableName, "DELETE");
                await GetResponseAsync(request);
            }
        }

        private Task<HttpWebRequest> CreateDatasetRequest(Dataset dataset, string tableName, string method)
        {
            var query = String.Format("{0}/{1}/tables/{2}/rows", datasetsUri, dataset.Id, tableName);
            return CreateDatasetrequestWorker(query, method);
        }

        private Task<HttpWebRequest> CreateDatasetRequest(string method)
        {
            return CreateDatasetrequestWorker(datasetsUri, method);
        }

        private async Task<HttpWebRequest> CreateDatasetrequestWorker(string query, string method)
        {
            HttpWebRequest request = System.Net.WebRequest.Create(query) as System.Net.HttpWebRequest;
            request.Method = method;
            request.ContentType = "application/json";

            var token = await AccessTokenAsync();

            Debug.Assert(token != null); // must be resolved already
            request.Headers["Authorization"] = String.Format("Bearer {0}", token);
            request.Headers["Keep-Alive"] = "true";
            return request;
        }

        private async Task<string> PostRequestAsync(HttpWebRequest request, string json)
        {
            byte[] byteArray = System.Text.Encoding.UTF8.GetBytes(json);

            // Write JSON byte[] into a Stream
            using (Stream writer = await request.GetRequestStreamAsync())
            {
                writer.Write(byteArray, 0, byteArray.Length);
            }
            return await GetResponseAsync(request);
        }

        private async Task<string> GetResponseAsync(HttpWebRequest request)
        {
            string response = string.Empty;

            using (HttpWebResponse httpResponse = await request.GetResponseAsync() as System.Net.HttpWebResponse)
            {
                // Get StreamReader that holds the response stream
                using (StreamReader reader = new System.IO.StreamReader(httpResponse.GetResponseStream()))
                {
                    response = reader.ReadToEnd();
                }
            }
            return response;
        }

        string token = String.Empty;
        private AuthenticationContext authContext = null;
        private DeviceCodeResult deviceCode = null;

        public async Task<AuthData> AcquireDeviceCodeAsync()
        {
            TokenCache TC = new TokenCache();
            // Create an instance of AuthenticationContext to acquire an Azure access token
            authContext = new AuthenticationContext(authority, TC);
            deviceCode = await authContext.AcquireDeviceCodeAsync(resourceUri, clientID);
            return new AuthData
            {
                VerificationUrl = deviceCode.VerificationUrl,
                UserCode = deviceCode.UserCode
            };
        }

        private async Task<string> AccessTokenAsync()
        {
            if (token == String.Empty)
            {
                var result = await authContext.AcquireTokenByDeviceCodeAsync(deviceCode);
                token = result.AccessToken;
            }
            else
            {
                token = (await authContext.AcquireTokenSilentAsync(resourceUri, clientID)).AccessToken;
            }
            return token;
        }

        public async Task CompleteAuthentication()
        {
            await AccessTokenAsync();
        }
    }
}