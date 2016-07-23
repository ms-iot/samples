using IoTHubBuddy.Models;
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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace IoTHubBuddy
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class TenantPage : Page
    {
        public TenantPage()
        {
            this.InitializeComponent();
        }
        private void ItemSelected(object sender, ItemClickEventArgs e)
        {
            string tenant = e.ClickedItem.ToString();
            IoTAccountData data = new IoTAccountData();
            data.Tenant = tenant;
            this.Frame.Navigate(typeof(SubscriptionPage), data);
        }
        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            string token = e.Parameter as string;
            if(token != null)
            {
                ICollection<string> tenants = await IoTDataManager.GetTenants(token);
                if (tenants.Count == 0)
                {
                    showError("You do not have any tenants under this account");
                }
                else
                {
                    hideErrors();
                    foreach (var tenant in tenants)
                    {
                        TenantList.Items.Add(tenant.ToString());
                    }
                }
            } else
            {
                showError("Token was null");
            }
            

        }
        private void showError(string error, bool showLoginBtn = false)
        {
            TenantList.Visibility = Visibility.Collapsed;
            ErrorMessage.Text = error;
            ErrorMessage.Visibility = Visibility.Visible;
            if (showLoginBtn)
            {
                Login.Visibility = Visibility.Visible;
                BackButton.Visibility = Visibility.Collapsed;
            }

        }
        private void hideErrors()
        {
            TenantList.Visibility = Visibility.Visible;
            ErrorMessage.Visibility = Visibility.Collapsed;
            Login.Visibility = Visibility.Collapsed;
            BackButton.Visibility = Visibility.Visible;
        }
        private void Login_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(MainPage));
        }
        private void Back_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(MainPage));
        }
    }
}
