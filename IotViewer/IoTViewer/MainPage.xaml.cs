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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace IoTViewer
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        private MessageManager msgManager;
        private MapIcon deviceLoc;
        public MainPage()
        {
            this.InitializeComponent();
            msgManager = new MessageManager(this);
            deviceLoc = new MapIcon();
            myMap.MapElements.Add(deviceLoc);
            myMap.Loaded += MyMap_Loaded;
            ReceiveLocation("0");
            ReceiveLocation("1");
        }

    private async void ReceiveLocation(string partition)
        {
            DateTime start = DateTime.UtcNow;
            await AzureIoT.ReceiveMessages(partition, DateTime.UtcNow - TimeSpan.FromMinutes(1), msgManager);

        }
        private async void MyMap_Loaded(object sender, RoutedEventArgs e)
        {
            Message currLoc = SimulatedAzureIoT.GetResults();
            msgManager.SetMapLocation(currLoc);
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
    }
}


