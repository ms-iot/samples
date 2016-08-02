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

        public DevicePage()
        {
            this.InitializeComponent();
        }
        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            string token = e.Parameter as string;
            if (!string.IsNullOrEmpty(token))
            {
                ICollection<IoTAccountData> devices = await IoTDataManager.GetAllDevices(token);
                if (devices.Count == 0)
                {
                    ShowError("You have no devices registered in this hub");
                } else
                {
                    HideErrors();
                    DeviceList.ItemsSource = devices;
                }
                
            } else
            {
                ShowError("An error occurred. Please try logging in again.");
            }
            

        }
        private void ItemSelected(object sender, ItemClickEventArgs e)
        {
            IoTAccountData device = e.ClickedItem as IoTAccountData;
            this.Frame.Navigate(typeof(MapPage), device);
        }
        
        private void ShowError(string error)
        {
            DeviceTable.Visibility = Visibility.Collapsed;
            ErrorMessage.Text = error;
            ErrorMessage.Visibility = Visibility.Visible;
            Login.Visibility = Visibility.Visible;
            AccountManager.SignOut();
        }
        private void HideErrors()
        {
            DeviceTable.Visibility = Visibility.Visible;
            ErrorMessage.Visibility = Visibility.Collapsed;
            Login.Visibility = Visibility.Collapsed;
        }
        private void Login_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(MainPage));
        }
    }
}
