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
using IoTConnector.Models;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace IoTConnector
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
        /// <summary>
        /// Use token to generate a list of devices, show errors if there are no devices
        /// </summary>
        /// <param name="e"></param>
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
                ShowError("Missing token during navigation. Please login again.");
            }
            

        }
        /// <summary>
        /// Navigate to map page with selected device
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ItemSelected(object sender, ItemClickEventArgs e)
        {
            IoTAccountData device = e.ClickedItem as IoTAccountData;
            this.Frame.Navigate(typeof(MapPage), device);
        }
        /// <summary>
        /// hide device grid and show error
        /// </summary>
        /// <param name="error"></param>
        private void ShowError(string error)
        {
            DeviceTable.Visibility = Visibility.Collapsed;
            ErrorMessage.Text = error;
            ErrorMessage.Visibility = Visibility.Visible;
            Login.Visibility = Visibility.Visible;
            AccountManager.SignOut();
        }
        /// <summary>
        /// hide error message and show device grid
        /// </summary>
        private void HideErrors()
        {
            DeviceTable.Visibility = Visibility.Visible;
            ErrorMessage.Visibility = Visibility.Collapsed;
            Login.Visibility = Visibility.Collapsed;
        }
        /// <summary>
        /// Navigate back to main page in order to login again
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Login_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(MainPage));
        }
    }
}
