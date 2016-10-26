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
using Windows.Security.Authentication.Web;
using Windows.UI.Popups;
using IoTConnector.Models;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=
//NOTE: Authentication doesn't really work right now, so if you don't have a corporate/organization azure account
//please fill in the necessary strings and uncomment NavigateToMap() in On_Loaded()

namespace IoTConnector
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        
        
        public MainPage()
        {
            this.InitializeComponent();
            string URI = string.Format("ms-appx-web://Microsoft.AAD.BrokerPlugIn/{0}", WebAuthenticationBroker.GetCurrentApplicationCallbackUri().Host.ToUpper());
            

        }
        /// <summary>
        /// event handler for login in click, will bring up login window
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void LoginButton_Click(object sender, RoutedEventArgs e)
        {
            // AccountsSettingsPane.Show();
            //bool login = await AccountManager.LoginWithCredentials();
            try
            {
                string token = await AccountManager.GetAzureAuthenticationToken();
                this.Frame.Navigate(typeof(DevicePage), token);

            } catch(System.Exception ex)
            {
                DisplayErrorMessage(ex.Message);
            }
        }
        
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            //AccountsSettingsPane.GetForCurrentView().AccountCommandsRequested += BuildPaneAsync;

        }
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            //AccountsSettingsPane.GetForCurrentView().AccountCommandsRequested -= BuildPaneAsync;
        }
        /// <summary>
        /// populate the account window pane with cached accounts and the add account button
        /// </summary>
        /// <param name="s"></param>
        /// <param name="e"></param>
        private async void BuildPaneAsync(AccountsSettingsPane s,
    AccountsSettingsPaneCommandsRequestedEventArgs e)
        {
            //AccountManager.OnAccountCommandsRequested(s, e, this);
            
        }
        /// <summary>
        /// load an instance of MapPage.xaml
        /// </summary>
        /// <param name="name"></param>
        public void NavigateToMap(IoTAccountData test)
        {
            this.Frame.Navigate(typeof(MapPage), test);
        }
        /// <summary>
        /// load an instance of Subscription.xaml
        /// </summary>
        /// <param name="token"></param>
        public void NavigateToDevices(string token)
        {
            this.Frame.Navigate(typeof(DevicePage), token);
        }
        public void DisplayErrorMessage(string error)
        {
            this.ErrorMessage.Text = error;
        }
        /// <summary>
        /// sign in silently first. If that fails, then bring up gui
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private async void On_Loaded(object sender, RoutedEventArgs e)
        {
            List<string> partitionIds = new List<string>();
            partitionIds.Add("0");
            partitionIds.Add("1");
            string entity = "..."; //should start with "iothub-ehub-<hub name>-<random numbers>"
            string port = "sb://.../"; //should start with "sb://" and end with "sevicebus.windows.net/"
            string name = "..."; //iothub name
            string deviceName = "..."; //name of iot device you want to listen to
            string primaryKey = "...";

            EventHubData eventhub = new EventHubData(partitionIds, entity, port, name);
            IoTAccountData a = new IoTAccountData(null, null, null, null, null, deviceName, "iothubowner", primaryKey, eventhub);
            //NavigateToMap(a);
        }

        private async void SignOutButton_Click(object sender, RoutedEventArgs e)
        {
            //await AccountManager.SignOut();
            LoginButton.Visibility = Windows.UI.Xaml.Visibility.Visible;
        }

        private void HyperlinkButton_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(AboutPage), this);
        }
    }
}


