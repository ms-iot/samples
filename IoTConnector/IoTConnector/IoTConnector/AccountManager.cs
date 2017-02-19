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
using Windows.Security.Authentication.Web.Core;
using Windows.Storage;
using Windows.UI.ApplicationSettings;
using Newtonsoft.Json;
using System.Diagnostics;
using Microsoft.IdentityModel.Clients.ActiveDirectory;

namespace IoTConnector
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

        /// <summary>
        /// Requests a token from the given resource with the user's tenant
        /// </summary>
        /// <param name="currAuthUri"></param>
        /// <param name="resourceUri"></param>
        /// <param name="tenant"></param>
        /// <returns></returns>
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
        /// <summary>
        /// Requests a token from the given resource with the user's tenant and username. This allows for silent tokens if the user is already logged in.
        /// </summary>
        /// <param name="tenant"></param>
        /// <param name="authUri"></param>
        /// <param name="resourceUri"></param>
        /// <param name="user"></param>
        /// <returns></returns>
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
        /// <summary>
        /// Gets an azure aad token with the given tenant, or the user's stored settings
        /// </summary>
        /// <param name="tenant"></param>
        /// <returns></returns>
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
                userName = result.UserInfo.GivenName + " " + result.UserInfo.FamilyName;
                return result.AccessToken;
            } else
            {
                throw new System.Exception(result.ErrorDescription);
            }
        }
        /// <summary>
        /// remove user login from storage
        /// </summary>
        public static void SignOut()
        {
            if (ApplicationData.Current.LocalSettings.Values.ContainsKey("CurrentUserId"))
            {
                ApplicationData.Current.LocalSettings.Values.Remove("CurrentUserId");
            }
        }
        /// <summary>
        /// Gets the current user's username
        /// </summary>
        /// <returns></returns>
        public static string GetUserName()
        {
            return userName;
        }

    }
}
