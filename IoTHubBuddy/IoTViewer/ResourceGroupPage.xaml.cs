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
using IoTHubBuddy.Models;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace IoTHubBuddy
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class ResourceGroupPage : Page
    {
        private IoTAccountData data;
        public ResourceGroupPage()
        {
            this.InitializeComponent();
        }
        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            IoTAccountData account = e.Parameter as IoTAccountData;
            if(!checkSubscriptionValidity(account))
            {
                showError("An error occurred. Please try logging in again.", true);
            } else
            {
                data = account;
                ICollection<string> groups = await IoTDataManager.GetResourceGroups(data.Subscription);
                if (groups.Count == 0)
                {
                    showError("You do not have any resource groups under this subscription");
                }
                else
                {
                    hideErrors();
                    foreach (var group in groups)
                    {
                        ResourceList.Items.Add(group.ToString());
                    }
                }
            }
            
            
        }
        private void showError(string error, bool showLoginBtn = false)
        {
            ResourceList.Visibility = Visibility.Collapsed;
            ErrorMessage.Text = error;
            ErrorMessage.Visibility = Visibility.Visible;
            if(showLoginBtn)
            {
                Login.Visibility = Visibility.Visible;
                BackButton.Visibility = Visibility.Collapsed;
            }
            
        }
        private void hideErrors()
        {
            ResourceList.Visibility = Visibility.Visible;
            ErrorMessage.Visibility = Visibility.Collapsed;
            Login.Visibility = Visibility.Collapsed;
            BackButton.Visibility = Visibility.Visible;
        }
        private void ItemSelected(object sender, ItemClickEventArgs e)
        {
            string group = e.ClickedItem.ToString();
            IoTAccountData account = new IoTAccountData();
            account.Subscription = data.Subscription;
            account.ResourceGroup = group;
            this.Frame.Navigate(typeof(HubPage), account);
        }
        private void Back_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(SubscriptionPage));
        }
        private void Login_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(MainPage));
        }
        private bool checkSubscriptionValidity(IoTAccountData account)
        {
            if (account == null)
            {
                return false;
            }
            if (account.Subscription == null)
            {
                return false;
            }
            return true;
        }
    }
}
