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

        private static MainPage mp;

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
        public static async Task<string> GetAzureAuthenticationToken(string tenant = "common")
        {
            var result = await GetAuthenticationResult(authUri, azureAuthUri, tenant);
            if(result != null)
            {
                return result.AccessToken;
            } else
            {
                throw new System.Exception(result.ErrorDescription);
            }
        }
        public static async Task<bool> LoginWithCredentials(string tenant="common")
        {
            try
            {
                var result = await GetAuthenticationResult(authUri, azureAuthUri, tenant);
                return true;
            } catch (Exception e )
            {
                return false;
            }


        }

    }
}
