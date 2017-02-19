using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Devices.Geolocation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Security.Authentication.Web.Core;
using Windows.Security.Credentials;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Maps;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using IoTConnector.Models;
using System.Diagnostics;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace IoTConnector
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MapPage : Page
    {
        private MessageManager msgManager;
        private MapIcon deviceLoc;
        private IoTAccountData hubData;
        private bool toggle = false;
        public MapPage()
        {
            this.InitializeComponent();
            
            deviceLoc = new MapIcon();
            myMap.MapElements.Add(deviceLoc);
            
            
        }
        /// <summary>
        /// parse data to populate fields and start listening to device
        /// </summary>
        /// <param name="e"></param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            hubData= e.Parameter as IoTAccountData;
            if (hubData != null)
            {
                username.Content = hubData.Name;
                msgManager = new MessageManager(this);
                foreach (string partition in hubData.EventHubInfo.PartitionIds)
                {
                    ReceiveLocation(partition);

                }
            }
            
        }
        /// <summary>
        /// Begin recieving data on a given partition
        /// </summary>
        /// <param name="partition"></param>
        private async void ReceiveLocation(string partition)
        {
            DateTime start = DateTime.UtcNow;
            await AzureIoT.ReceiveMessages(partition, DateTime.UtcNow - TimeSpan.FromMinutes(1), msgManager, hubData);

        }

        /// <summary>
        /// update map with pin and given location from device
        /// </summary>
        /// <param name="lat"></param>
        /// <param name="lng"></param>
        /// <param name="timestamp"></param>
        public async void SetMapLocation(double lat, double lng, string timestamp)
        {
            var center =
               new Geopoint(new BasicGeoposition()
               {
                   Latitude = lat,
                   Longitude = lng

               });

            // retrieve map
            try
            {
                await myMap.TrySetSceneAsync(MapScene.CreateFromLocationAndRadius(center, 250));
                deviceLoc.Location = center;
                deviceLoc.Title = timestamp;
            } catch (System.Exception e)
            {
                Debug.WriteLine(e.Message);
                AddMessageToLog(e.Message);
            }
            
        }
        /// <summary>
        /// add a message to the UI message log
        /// </summary>
        /// <param name="msg"></param>
        public void AddMessageToLog(string msg)
        {
            this.myMessages.Items.Insert(0, msg);
        }
        /// <summary>
        /// sign out of account and show login page
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void SignOutButton_Click(object sender, RoutedEventArgs e)
        {
            AccountManager.SignOut();
            this.Frame.Navigate(typeof(MainPage));
        }
        /// <summary>
        /// clear UI message log
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ClearLog_Click(object sender, RoutedEventArgs e)
        {
            this.myMessages.Items.Clear();
        }
        /// <summary>
        /// navigate to about page
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void HyperlinkButton_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(AboutPage));
        }
        /// <summary>
        /// Toggle sign out button visibility
        /// </summary>
        private void ToggleUserVisibility()
        {
            if(toggle)
            {
                this.UserSettings.Visibility = Visibility.Collapsed;
            } else
            {
                this.UserSettings.Visibility = Visibility.Visible;
            }
            toggle = !toggle;
        }
        /// <summary>
        /// If the username is clicked, toggle sign out visibility
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void username_Click(object sender, RoutedEventArgs e)
        {
            ToggleUserVisibility();
        }
    }
}
