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
using Microsoft.IdentityModel.Clients.ActiveDirectory;
using Windows.UI.Popups;

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
            string URI = string.Format("ms-appx-web://Microsoft.AAD.BrokerPlugIn/{0}", WebAuthenticationBroker.GetCurrentApplicationCallbackUri().Host.ToUpper());


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
            AccountManager.OnAccountCommandsRequested(s, e, this);
            
        }
        public void NavigateToMap(string name)
        {
            this.Frame.Navigate(typeof(MapPage), name);
        }
        private async void On_Loaded(object sender, RoutedEventArgs e)
        {
            string name = await AccountManager.LoginSilently();
            if(name != null)
            {
                SignOutButton.Visibility = Windows.UI.Xaml.Visibility.Visible;
                this.Frame.Navigate(typeof(MapPage), name);
            } else
            {
                LoginButton.Visibility = Windows.UI.Xaml.Visibility.Visible;
            }
        }

        private async void SignOutButton_Click(object sender, RoutedEventArgs e)
        {
            await AccountManager.SignOut();
            LoginButton.Visibility = Windows.UI.Xaml.Visibility.Visible;
            SignOutButton.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            UserIdTextBlock.Text = "Id: ";
            UserNameTextBlock.Text = "Name: ";
        }
    }
}


