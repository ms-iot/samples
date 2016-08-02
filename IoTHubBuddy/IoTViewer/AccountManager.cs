using Microsoft.Azure.Devices;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Text;
using System.Threading.Tasks;
using Windows.Data.Json;
using Microsoft.IdentityModel.Clients.ActiveDirectory;
using Windows.Security.Authentication.Web.Core;
using Windows.Storage;
using Windows.UI.ApplicationSettings;
using Newtonsoft.Json;
using System.Diagnostics;

namespace IoTHubBuddy
{
    class AccountManager
    {
        const string AzureActiveDirectoryClientId = "1950a258-227b-4e31-a9cf-717495945fc2";
        const string AzureActiveDirectoryScopeRequested = "wl.basic";
        static Uri publicRedirectUri = new Uri("urn:ietf:wg:oauth:2.0:oob");
        const string authUri = "https://login.microsoftonline.com/";
        const string azureAuthUri = "https://management.core.windows.net/";
        const string azureUri = "https://management.azure.com/";
        const string graphUri = "https://graph.windows.net/";
        static string userName = "";

        private static async Task<AuthenticationResult> GetAuthenticationResult(string currAuthUri, string resourceUri, string tenant)
        {
            var authority = "{0}{1}".FormatInvariant(currAuthUri, tenant);
            var authContext = new AuthenticationContext(authority, true);
            try
            {
                return await authContext.AcquireTokenAsync(resourceUri, AzureActiveDirectoryClientId, publicRedirectUri);
            }
            catch (Exception e)
            {
                throw e;
            }
        }
        private static async Task<AuthenticationResult> GetAuthenticationResult(string tenant, string authUri, string resourceUri, string user)
        {
            var authority = "{0}{1}".FormatInvariant(authUri, tenant);
            var authContext = new AuthenticationContext(authority, true);
            var userId = new UserIdentifier(user, UserIdentifierType.OptionalDisplayableId);
            try
            {
                return await authContext.AcquireTokenAsync(resourceUri, AzureActiveDirectoryClientId, publicRedirectUri, PromptBehavior.Auto, userId);
            }
            catch (Exception)
            {
                return null;
            }
        }
        private static async Task<JsonObject> GetGraphResource(string token, string relative)
        {
            try
            {

                using (var client = new HttpClient())
                {
                    client.DefaultRequestHeaders.Authorization = new System.Net.Http.Headers.AuthenticationHeaderValue("Bearer", token);
                    //client.DefaultRequestHeaders.Accept.Add(new System.Net.Http.Headers.MediaTypeWithQualityHeaderValue("application/json"));
                    var restApi = new Uri(graphUri + relative);
                    var infoResult = await client.GetAsync(restApi);
                    string content = await infoResult.Content.ReadAsStringAsync();
                    var jsonObject = JsonObject.Parse(content);
                    if (jsonObject.GetObject().ContainsKey("value"))
                    {
                        return jsonObject;
                    }
                    else
                    {
                        string message = "The response object was malformed.";
                        if (jsonObject.ContainsKey("error"))
                        {
                            message = jsonObject.GetObject().GetNamedValue("error").GetString();
                        }
                        //throw new System.Exception(message);
                        return null;

                    }
                }
            }
            catch (System.Exception e)
            {
                throw new System.Exception(e.Message);
            }
        }
        public static async Task<string> GetPhoto()
        {
            string userid = null;
            if (ApplicationData.Current.LocalSettings.Values.ContainsKey("CurrentUserId"))
            {
                userid = ApplicationData.Current.LocalSettings.Values["CurrentUserId"] as string;
            }
            AuthenticationResult result = null;
            string tenant = "common";
            if (!string.IsNullOrEmpty(userid))
            {
                string[] user = userid.Split('@');
                tenant = user[1];
                result = await GetAuthenticationResult(user[1], authUri, graphUri, userid);
            }
            if(result != null)
            {
                string rel = "v1.0/users/"+result.UserInfo.UniqueId+"/photo?api-version=1.0";
                var jobj = await GetGraphResource(result.AccessToken, rel); 
            }
            return null;
            


        }
        public static async Task<string> GetAzureAuthenticationToken(string tenant = "common")
        {
            string userid = null;
            if (ApplicationData.Current.LocalSettings.Values.ContainsKey("CurrentUserId"))
            {
                userid = ApplicationData.Current.LocalSettings.Values["CurrentUserId"] as string;
            }
            AuthenticationResult result = null;
            if (string.IsNullOrEmpty(userid) || tenant != "common")
            {
                result = await GetAuthenticationResult(authUri, azureAuthUri, tenant);
            } else
            {
                string[] user = userid.Split('@');
                result = await GetAuthenticationResult(user[1], authUri, azureAuthUri, userid);
            }

            if (result != null)
            {
                ApplicationData.Current.LocalSettings.Values["CurrentUserId"] = result.UserInfo.DisplayableId;
                //var s = await GetPhoto(); <-- DOESN'T WORK YET
                userName = result.UserInfo.GivenName + " " + result.UserInfo.FamilyName;
                return result.AccessToken;
            } else
            {
                throw new System.Exception(result.ErrorDescription);
            }
        }
        public static void SignOut()
        {
            if (ApplicationData.Current.LocalSettings.Values.ContainsKey("CurrentUserId"))
            {
                ApplicationData.Current.LocalSettings.Values.Remove("CurrentUserId");
            }
        }
        public static string GetUserName()
        {
            return userName;
        }

    }
}
