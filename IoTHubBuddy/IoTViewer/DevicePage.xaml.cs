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
    public sealed partial class DevicePage : Page
    {
        private IoTAccountData data;
        public DevicePage()
        {
            this.InitializeComponent();
        }
        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            IoTAccountData account = e.Parameter as IoTAccountData;
            if (CheckValidity(account))
            {
                data = account;
                data.SharedAccessPolicy = "iothubowner";
                ICollection<string> devices = await IoTDataManager.GetIoTDevices(data.Subscription, data.ResourceGroup, data.HubName, data.SharedAccessPolicy);
                data.PrimaryKey = await IoTDataManager.GetPrimaryKey(data.Subscription, data.ResourceGroup, data.HubName, data.SharedAccessPolicy);
                if (devices.Count == 0)
                {
                    ShowError("You have no devices registered in this hub");
                } else
                {
                    HideErrors();
                    foreach (string dev in devices)
                    {
                        DeviceList.Items.Add(dev);
                    }
                }
                
            } else
            {
                ShowError("An error occurred. Please try logging in again.", true);
            }
            

        }
        private void ItemSelected(object sender, ItemClickEventArgs e)
        {
            string device = e.ClickedItem.ToString();
            data.DeviceName = device;;
            this.Frame.Navigate(typeof(MapPage), data);
        }
        private bool CheckValidity(IoTAccountData account)
        {
            if (account == null)
            {
                return false;
            }
            if (account.Subscription == null || account.HubName == null || account.ResourceGroup == null || account.EventHubInfo == null)
            {
                return false;
            }
            return true;
        }
        private void ShowError(string error, bool showLoginBtn = false)
        {
            DeviceList.Visibility = Visibility.Collapsed;
            ErrorMessage.Text = error;
            ErrorMessage.Visibility = Visibility.Visible;
            if (showLoginBtn)
            {
                Login.Visibility = Visibility.Visible;
                BackButton.Visibility = Visibility.Collapsed;
            }

        }
        private void HideErrors()
        {
            DeviceList.Visibility = Visibility.Visible;
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
            IoTAccountData account = IoTAccountData.Clone(data);
            this.Frame.Navigate(typeof(HubPage), account);
        }
    }
}
