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
using Windows.Security.Credentials;
using Windows.Storage;
using Windows.UI.ApplicationSettings;
using Newtonsoft.Json;
using System.Diagnostics;

namespace IoTHubBuddy
{
    class AccountManager
    {
        const string AppSpecificProviderId = "myproviderid";
        const string AppSpecificProviderName = "App specific provider";

        const string AccountsContainer = "accountssettingscontainer";
        const string ProviderIdSubContainer = "providers";
        const string AuthoritySubContainer = "authorities";

        const string MicrosoftProviderId = "https://login.microsoft.com";
        const string MicrosoftAccountAuthority = "consumers";
        const string AzureActiveDirectoryAuthority = "organizations";

        const string MicrosoftAccountClientId = "none";
        const string MicrosoftAccountScopeRequested = "wl.basic";
        const string AzureActiveDirectoryClientId = "5cec90d8-14eb-41ac-9b2c-b3e8a7d95da5";
        const string AzureActiveDirectoryScopeRequested = "wl.basic";
        private static MainPage mp;
        /// <summary>
        /// build the account settings pane with the cached accounts
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        /// <param name="page"></param>
        public static async void OnAccountCommandsRequested(
           AccountsSettingsPane sender,
           AccountsSettingsPaneCommandsRequestedEventArgs e, MainPage page)
        {
            mp = page;
            // In order to make async calls within this callback, the deferral object is needed
            AccountsSettingsPaneEventDeferral deferral = e.GetDeferral();
            CreateLocalDataContainers();
            await AddWebAccountProvidersToPane(e);
            await AddWebAccountsToPane(e);

            deferral.Complete();
        }
        /// <summary>
        /// find all providers and add to pane
        /// </summary>
        /// <param name="e"></param>
        /// <returns></returns>
        private static async Task AddWebAccountProvidersToPane(AccountsSettingsPaneCommandsRequestedEventArgs e)
        {
            List<WebAccountProvider> providers = await GetAllProviders();

            foreach (WebAccountProvider provider in providers)
            {
                WebAccountProviderCommand providerCommand = new WebAccountProviderCommand(provider, WebAccountProviderCommandInvoked);
                e.WebAccountProviderCommands.Add(providerCommand);
            }
        }
        /// <summary>
        /// find all cached accounts and add to pane
        /// </summary>
        /// <param name="e"></param>
        /// <returns></returns>
        private static async Task AddWebAccountsToPane(AccountsSettingsPaneCommandsRequestedEventArgs e)
        {
            List<WebAccount> accounts = await GetAllAccounts();

            foreach (WebAccount account in accounts)
            {
                WebAccountCommand command = new WebAccountCommand(account, WebAccountInvoked, SupportedWebAccountActions.Remove);
                e.WebAccountCommands.Add(command);
            }

        }
        /// <summary>
        /// Get a list of stored accounts
        /// NOTE: the storage model is old, and will be changed soon
        /// </summary>
        /// <returns></returns>
        private static async Task<List<WebAccount>> GetAllAccounts()
        {
            List<WebAccount> accounts = new List<WebAccount>();

            ApplicationDataContainer AccountListContainer = ApplicationData.Current.LocalSettings.Containers[AccountsContainer];

            foreach (Object value in AccountListContainer.Containers[ProviderIdSubContainer].Values.Keys)
            {
                String accountID = value as String;
                String providerID = AccountListContainer.Containers[ProviderIdSubContainer].Values[accountID] as String;
                String authority = AccountListContainer.Containers[AuthoritySubContainer].Values[accountID] as String;
                //KNOWN BUG: storing the authority for organization accounts for some reason doesn't work. Will look into once auth has been finalized
                if (authority == null)
                {
                    authority = "organizations";
                }
                WebAccountProvider provider = await GetProvider(providerID, authority);

                    WebAccount loadedAccount = await WebAuthenticationCoreManager.FindAccountAsync(provider, accountID);
                    if (loadedAccount != null)
                    {
                        accounts.Add(loadedAccount);
                    }
                    else
                    {
                        // The account has been deleted
                        ApplicationDataContainer accountsContainer = ApplicationData.Current.LocalSettings.Containers[AccountsContainer];

                        accountsContainer.Containers[ProviderIdSubContainer].Values.Remove(accountID);
                        accountsContainer.Containers[AuthoritySubContainer].Values.Remove(accountID);
                    }
                }

            return accounts;
        }
        /// <summary>
        /// Get the two different account types- consumers (live ID) and organizations (AAD)
        /// </summary>
        /// <returns></returns>
        private static async Task<List<WebAccountProvider>> GetAllProviders()
        {
            List<WebAccountProvider> providers = new List<WebAccountProvider>();
            providers.Add(await GetProvider(MicrosoftProviderId, MicrosoftAccountAuthority));
            providers.Add(await GetProvider(MicrosoftProviderId, AzureActiveDirectoryAuthority));

            return providers;
        }

        /// <summary>
        /// return the specified provider
        /// </summary>
        /// <param name="ProviderID"></param>
        /// <param name="AuthorityId"></param>
        /// <returns></returns>
        private static async Task<WebAccountProvider> GetProvider(string ProviderID, string AuthorityId = "")
        {
            return await WebAuthenticationCoreManager.FindAccountProviderAsync(ProviderID, AuthorityId);
        }
        /// <summary>
        /// gives the user an option to sign out of their account
        /// </summary>
        /// <param name="command"></param>
        /// <param name="args"></param>
        private static async void WebAccountInvoked(WebAccountCommand command, WebAccountInvokedArgs args)
        {
            if (args.Action == WebAccountAction.Remove)
            {
                await SignOutAccountAsync(command.WebAccount);
            }
        }
        /// <summary>
        /// once the user has logged in, try to get a token for azure management
        /// </summary>
        /// <param name="command"></param>
        private static async void WebAccountProviderCommandInvoked(WebAccountProviderCommand command)
        {
            if ((command.WebAccountProvider.Id == MicrosoftProviderId) && (command.WebAccountProvider.Authority == MicrosoftAccountAuthority))
            {
                // ClientID is ignored by MSA
                await AuthenticateWithRequestToken(command.WebAccountProvider, MicrosoftAccountScopeRequested, MicrosoftAccountClientId);
            }
            else if ((command.WebAccountProvider.Id == MicrosoftProviderId) && (command.WebAccountProvider.Authority == AzureActiveDirectoryAuthority))
            {
                await AuthenticateWithRequestToken(command.WebAccountProvider, AzureActiveDirectoryScopeRequested, AzureActiveDirectoryClientId);
            }

        }
        /// <summary>
        /// attempt to get a token from azure management, if successful then navigate to subscription page
        /// </summary>
        /// <param name="provider"></param>
        /// <param name="scope"></param>
        /// <param name="client"></param>
        /// <returns></returns>
        public static async Task AuthenticateWithRequestToken(WebAccountProvider provider, string scope, string client)
        {
            WebTokenRequest request = new WebTokenRequest(provider, scope, client);
            Debug.WriteLine("provider: " + provider.Id + " scope: " + scope + " client: " + client);
            request.Properties.Add("resource", "https://management.azure.com/");
            WebTokenRequestResult result = await WebAuthenticationCoreManager.RequestTokenAsync(request);
            if (result.ResponseStatus == WebTokenRequestStatus.Success)
            {
                WebAccount account = result.ResponseData[0].WebAccount;
                StoreWebAccount(account);
                string token = result.ResponseData[0].Token;

                Debug.WriteLine("token: " + token);
                mp.NavigateToSubscription(token);

            }
            

        }

        /// <summary>
        /// store web account in local storage
        /// NOTE: this will likely change soon
        /// </summary>
        /// <param name="account"></param>
        private static void StoreWebAccount(WebAccount account)
        {
            ApplicationDataContainer accountsContainer = ApplicationData.Current.LocalSettings.Containers[AccountsContainer];

            if (account.UserName != "")
            {
                accountsContainer.Containers[ProviderIdSubContainer].Values[account.UserName] = account.WebAccountProvider.Id;
                accountsContainer.Containers[AuthoritySubContainer].Values[account.Id] = account.WebAccountProvider.Authority;
                
            }
            else
            {
                accountsContainer.Containers[ProviderIdSubContainer].Values[account.Id] = account.WebAccountProvider.Id;
                accountsContainer.Containers[AuthoritySubContainer].Values[account.Id] = account.WebAccountProvider.Authority;
            }
            ApplicationData.Current.LocalSettings.Values["CurrentUserProviderId"] = account.WebAccountProvider.Id;
            ApplicationData.Current.LocalSettings.Values["CurrentUserId"] = account.Id;
            ApplicationData.Current.LocalSettings.Values["CurrentAuthority"] = account.WebAccountProvider.Authority;

        }
        /// <summary>
        /// Attempt to get token silently (without prompting user for credentials) from stored account
        /// </summary>
        /// <returns></returns>
        public static async Task<string> GetTokenSilentlyAsync()
        {
            string providerId = ApplicationData.Current.LocalSettings.Values["CurrentUserProviderId"]?.ToString();
            string accountId = ApplicationData.Current.LocalSettings.Values["CurrentUserId"]?.ToString();
            string authority = ApplicationData.Current.LocalSettings.Values["CurrentAuthority"]?.ToString();

            if (null == providerId || null == accountId)
            {
                return "";
            }

            WebAccountProvider provider = await WebAuthenticationCoreManager.FindAccountProviderAsync(providerId, authority);
            WebAccount account = await WebAuthenticationCoreManager.FindAccountAsync(provider, accountId);
            string clientId = "none";
            if (authority == "organizations")
            {
                clientId = AzureActiveDirectoryClientId;
                
            }

            WebTokenRequest request = new WebTokenRequest(provider, "wl.basic", clientId);
            // Azure Active Directory requires a resource to return a token
            request.Properties.Add("resource", "https://management.azure.com/");

            WebTokenRequestResult result = await WebAuthenticationCoreManager.GetTokenSilentlyAsync(request, account);
            if (result.ResponseStatus == WebTokenRequestStatus.UserInteractionRequired)
            {
                // Unable to get a token silently - you'll need to show the UI
                throw new System.Exception("Unable to get token silently");
            }
            else if (result.ResponseStatus == WebTokenRequestStatus.Success)
            {
                // Success
                return result.ResponseData[0].Token;
            }
            else
            {
                // Other error 
                throw new System.Exception("Unknown error when requesting token silently");
            }
        }
        /// <summary>
        /// sign out of current account
        /// </summary>
        /// <returns></returns>
        public static async Task SignOut()
        {

            string providerId = ApplicationData.Current.LocalSettings.Values["CurrentUserProviderId"]?.ToString();
            string accountId = ApplicationData.Current.LocalSettings.Values["CurrentUserId"]?.ToString();

            string authority = ApplicationData.Current.LocalSettings.Values["CurrentAuthority"]?.ToString();

            if (null == providerId || null == accountId)
            {
                return;
            }

            WebAccountProvider provider = await WebAuthenticationCoreManager.FindAccountProviderAsync(providerId, authority);
            WebAccount account = await WebAuthenticationCoreManager.FindAccountAsync(provider, accountId);
            await SignOutAccountAsync(account);
        }
        /// <summary>
        /// remove account from storage and sign out
        /// </summary>
        /// <param name="account"></param>
        /// <returns></returns>
        public static async Task SignOutAccountAsync(WebAccount account)
        {
            ApplicationData.Current.LocalSettings.Values.Remove("CurrentUserProviderId");
            ApplicationData.Current.LocalSettings.Values.Remove("CurrentUserId");
            ApplicationData.Current.LocalSettings.Values.Remove("CurrentAuthority");
            ApplicationDataContainer accountsContainer = ApplicationData.Current.LocalSettings.Containers[AccountsContainer];
            if(account.UserName=="")
            {
                accountsContainer.Containers[ProviderIdSubContainer].Values.Remove(account.Id);
            } else
            {
                accountsContainer.Containers[ProviderIdSubContainer].Values.Remove(account.UserName);
            }
            account.SignOutAsync();
            
        }
        /// <summary>
        /// attempt to login silently without prompting user for credentials
        /// </summary>
        /// <returns></returns>
        public static async Task<string> LoginSilently()
        {
            return await GetTokenSilentlyAsync();
            
        }
        /// <summary>
        /// build containers for storage
        /// </summary>
        private static void CreateLocalDataContainers()
        {
            ApplicationData.Current.LocalSettings.CreateContainer(AccountsContainer, ApplicationDataCreateDisposition.Always);
            ApplicationData.Current.LocalSettings.Containers[AccountsContainer].CreateContainer(ProviderIdSubContainer, ApplicationDataCreateDisposition.Always);
            ApplicationData.Current.LocalSettings.Containers[AccountsContainer].CreateContainer(AuthoritySubContainer, ApplicationDataCreateDisposition.Always);
        }

        /// <summary>
        /// remove all accounts from cached storage
        /// </summary>
        /// <returns></returns>
        public static async Task LogoffAndRemoveAllAccounts()
        {
            List<WebAccount> accounts = await GetAllAccounts();

            foreach (WebAccount account in accounts)
            {
                await SignOutAccountAsync(account);
            }
            
        }
    }
}
