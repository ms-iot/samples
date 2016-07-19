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
using IoTHubBuddy.Models;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace IoTHubBuddy
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MapPage : Page
    {
        private MessageManager msgManager;
        private MapIcon deviceLoc;
        private IoTAccountData hubData;
        public MapPage()
        {
            this.InitializeComponent();
            
            deviceLoc = new MapIcon();
            myMap.MapElements.Add(deviceLoc);
            
            
        }
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            hubData= e.Parameter as IoTAccountData;
            if (hubData != null)
            {
                username.Text = hubData.EventHubInfo.HubName;
                msgManager = new MessageManager(this);
                foreach (string partition in hubData.EventHubInfo.PartitionIds)
                {
                    ReceiveLocation(partition);

                }
            }
            
        }
        private async void ReceiveLocation(string partition)
        {
            DateTime start = DateTime.UtcNow;
            await AzureIoT.ReceiveMessages(partition, DateTime.UtcNow - TimeSpan.FromMinutes(1), msgManager, hubData);

        }

        public async void SetMapLocation(double lat, double lng, string timestamp)
        {
            var center =
               new Geopoint(new BasicGeoposition()
               {
                   Latitude = lat,
                   Longitude = lng

               });

            // retrieve map
            await myMap.TrySetSceneAsync(MapScene.CreateFromLocationAndRadius(center, 250));
            deviceLoc.Location = center;
            deviceLoc.Title = timestamp;

        }
        public void AddMessageToLog(string msg)
        {
            this.myMessages.Items.Insert(0, msg);
        }
        private async Task SignOutAccountAsync(WebAccount account)
        {
            ApplicationData.Current.LocalSettings.Values.Remove("CurrentUserProviderId");
            ApplicationData.Current.LocalSettings.Values.Remove("CurrentUserId");
            account.SignOutAsync();
        }
        private async void SignOutButton_Click(object sender, RoutedEventArgs e)
        {
            await AccountManager.SignOut();
            this.Frame.Navigate(typeof(MainPage));
        }

        private void ClearLog_Click(object sender, RoutedEventArgs e)
        {
            this.myMessages.Items.Clear();
        }
    }
}
