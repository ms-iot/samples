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
    public sealed partial class SubscriptionPage : Page
    {
        private string token;
        public SubscriptionPage()
        {
            this.InitializeComponent();
        }
        private void ItemSelected(object sender, ItemClickEventArgs e)
        {
            string subscription = e.ClickedItem.ToString();
            IoTAccountData data = new IoTAccountData();
            data.Subscription = subscription;
            this.Frame.Navigate(typeof(ResourceGroupPage), data);
        }
        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            token = e.Parameter as string;
            ICollection<string> subscriptions = await IoTDataManager.GetSubscription(token);
            if(subscriptions.Count == 0)
            {
                showError("You do not have any Azure subscriptions under this account");
            } else
            {
                hideErrors();
                foreach (var sub in subscriptions)
                {
                    SubscriptionList.Items.Add(sub.ToString());
                }
            }
            
        }
        private void showError(string error, bool showLoginBtn = false)
        {
            SubscriptionList.Visibility = Visibility.Collapsed;
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
            SubscriptionList.Visibility = Visibility.Visible;
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
