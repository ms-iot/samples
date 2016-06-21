using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Microsoft.Azure.Devices;
using Windows.Devices.Geolocation;
using Windows.UI.Xaml.Controls.Maps;
using System.Threading.Tasks;
using System.Text;
using System.Diagnostics;
using Windows.Security.Authentication.Web.Core;
using Windows.System;
using Windows.UI.ApplicationSettings;
using Windows.Data.Json;
using Windows.Web.Http;
using Windows.Security.Credentials;
using Windows.Storage;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace IoTViewer
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private string m_name;
        
        public MainPage()
        {
            this.InitializeComponent();
            
        }

    
        private void LoginButton_Click(object sender, RoutedEventArgs e)
        {
            AccountsSettingsPane.Show();
        }
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            AccountsSettingsPane.GetForCurrentView().AccountCommandsRequested += BuildPaneAsync;
        }
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            AccountsSettingsPane.GetForCurrentView().AccountCommandsRequested -= BuildPaneAsync;
        }
        private async void BuildPaneAsync(AccountsSettingsPane s,
    AccountsSettingsPaneCommandsRequestedEventArgs e)
        {
            var deferral = e.GetDeferral();
            var msaProvider = await WebAuthenticationCoreManager.FindAccountProviderAsync(
        "https://login.microsoft.com", "consumers");
            var command = new WebAccountProviderCommand(msaProvider, GetMsaTokenAsync);

            e.WebAccountProviderCommands.Add(command);


            deferral.Complete();
            
        }
        private async void GetMsaTokenAsync(WebAccountProviderCommand command)
        {
            string token = await GetTokenSilentlyAsync();
            if (token == null) {
                WebTokenRequest request = new WebTokenRequest(command.WebAccountProvider, "wl.basic");
                WebTokenRequestResult result = await WebAuthenticationCoreManager.RequestTokenAsync(request);
                if (result.ResponseStatus == WebTokenRequestStatus.Success)
                {
                    WebAccount account = result.ResponseData[0].WebAccount;
                    StoreWebAccount(account);
                    token = result.ResponseData[0].Token;
                }
            }
            LoginWithToken(token);

        }
        private async void LoginWithToken(string token)
        {
            var restApi = new Uri(@"https://apis.live.net/v5.0/me?access_token=" + token);
            string name = "";
            using (var client = new HttpClient())
            {
                var infoResult = await client.GetAsync(restApi);
                string content = await infoResult.Content.ReadAsStringAsync();

                var jsonObject = JsonObject.Parse(content);
                string id = jsonObject["id"].GetString();
                name = jsonObject["name"].GetString();
                m_name = name;
                UserIdTextBlock.Text = "Id: " + id;
                UserNameTextBlock.Text = "Name: " + name;
                LoginButton.Visibility = Visibility.Collapsed;
                SignOutButton.Visibility = Visibility.Visible;
                this.Frame.Navigate(typeof(MapPage), name);
            }
        }
        private async void StoreWebAccount(WebAccount account)
        {
            ApplicationData.Current.LocalSettings.Values["CurrentUserProviderId"] = account.WebAccountProvider.Id;
            ApplicationData.Current.LocalSettings.Values["CurrentUserId"] = account.Id;
        }
        private async Task<string> GetTokenSilentlyAsync()
        {
            string providerId = ApplicationData.Current.LocalSettings.Values["CurrentUserProviderId"]?.ToString();
            string accountId = ApplicationData.Current.LocalSettings.Values["CurrentUserId"]?.ToString();

            if (null == providerId || null == accountId)
            {
                return null;
            }

            WebAccountProvider provider = await WebAuthenticationCoreManager.FindAccountProviderAsync(providerId);
            WebAccount account = await WebAuthenticationCoreManager.FindAccountAsync(provider, accountId);

            WebTokenRequest request = new WebTokenRequest(provider, "wl.basic");

            WebTokenRequestResult result = await WebAuthenticationCoreManager.GetTokenSilentlyAsync(request, account);
            if (result.ResponseStatus == WebTokenRequestStatus.UserInteractionRequired)
            {
                // Unable to get a token silently - you'll need to show the UI
                return null;
            }
            else if (result.ResponseStatus == WebTokenRequestStatus.Success)
            {
                // Success
                return result.ResponseData[0].Token;
            }
            else
            {
                // Other error 
                return null;
            }
        }
        private async Task SignOutAccountAsync(WebAccount account)
        {
            ApplicationData.Current.LocalSettings.Values.Remove("CurrentUserProviderId");
            ApplicationData.Current.LocalSettings.Values.Remove("CurrentUserId");
            account.SignOutAsync();
        }

        private async void On_Loaded(object sender, RoutedEventArgs e)
        {
            string token = await GetTokenSilentlyAsync();
            if(token != null)
            {
                LoginWithToken(token);
                SignOutButton.Visibility = Windows.UI.Xaml.Visibility.Visible;
                //this.Frame.Navigate(typeof(MapPage), name);
            } else
            {
                LoginButton.Visibility = Windows.UI.Xaml.Visibility.Visible;
            }
        }

        private async void SignOutButton_Click(object sender, RoutedEventArgs e)
        {
            string providerId = ApplicationData.Current.LocalSettings.Values["CurrentUserProviderId"]?.ToString();
            string accountId = ApplicationData.Current.LocalSettings.Values["CurrentUserId"]?.ToString();

            if (null == providerId || null == accountId)
            {
                return;
            }

            WebAccountProvider provider = await WebAuthenticationCoreManager.FindAccountProviderAsync(providerId);
            WebAccount account = await WebAuthenticationCoreManager.FindAccountAsync(provider, accountId);
            await this.SignOutAccountAsync(account);
            LoginButton.Visibility = Windows.UI.Xaml.Visibility.Visible;
            SignOutButton.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            UserIdTextBlock.Text = "Id: ";
            UserNameTextBlock.Text = "Name: ";
        }
    }
}


