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
    public sealed partial class HubPage : Page
    {
        private IoTAccountData data;
        private ICollection<EventHubData> hubs;
        public HubPage()
        {
            this.InitializeComponent();
            
        }
        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            IoTAccountData account = e.Parameter as IoTAccountData;
            if(CheckValidity(account))
            {
                
                data = account;
                hubs = await IoTDataManager.GetIoTHubs(data.Subscription);
                if(hubs.Count == 0)
                {
                    ShowError("You have no IoTHubs registered under this subscription.");
                } else
                {
                    HideErrors();
                    foreach (EventHubData h in hubs)
                    {
                        HubList.Items.Add(h.HubName);
                    }
                }
                
            } else
            {
                ShowError("An error occurred. Please try logging in again", true);
            }
            
            
        }
        private void ItemSelected(object sender, ItemClickEventArgs e)
        {
            string hub = e.ClickedItem.ToString();
            
            data.HubName = hub;
            EventHubData eventhub = EventSelected(hub);
            eventhub.HubName += ".azure-devices.net";
            data.EventHubInfo = eventhub;
            this.Frame.Navigate(typeof(DevicePage), data);
        }
        private EventHubData EventSelected(string hubName)
        {
            IEnumerable<EventHubData> selected = hubs.SkipWhile(hub => hub.HubName != hubName);
            return selected.First<EventHubData>();
        }
        private void Back_Click(object sender, RoutedEventArgs e)
        {
            IoTAccountData account = IoTAccountData.Clone(data);
            this.Frame.Navigate(typeof(ResourceGroupPage), account);
        }
        private void ShowError(string error, bool showLoginBtn = false)
        {
            HubList.Visibility = Visibility.Collapsed;
            ErrorMessage.Text = error;
            ErrorMessage.Visibility = Visibility.Visible;
            if (showLoginBtn)
            {
                Login.Visibility = Visibility.Visible;
            }

        }
        private void HideErrors()
        {
            HubList.Visibility = Visibility.Visible;
            ErrorMessage.Visibility = Visibility.Collapsed;
            Login.Visibility = Visibility.Collapsed;
        }
        private void Login_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(MainPage));
        }
        private bool CheckValidity(IoTAccountData account)
        {
            if (account == null)
            {
                return false;
            }
            if (account.Subscription == null)
            {
                return false;
            }
            if(account.ResourceGroup == null)
            {
                return false;
            }
            return true;
        }
    }
}
