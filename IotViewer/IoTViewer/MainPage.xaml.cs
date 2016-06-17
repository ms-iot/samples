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
        private MapManager mapManager;
        public MainPage()
        {
            this.InitializeComponent();
            mapManager = new MapManager(myMap);
            myMap.Loaded += MyMap_Loaded;
            ReceiveLocation("0");
            ReceiveLocation("1");
        }

    private async void ReceiveLocation(string partition)
        {
            DateTime start = DateTime.UtcNow;
            while(true)
            {
                Message loc = await AzureIoT.ReceiveMessages(partition, DateTime.UtcNow - TimeSpan.FromMinutes(1), mapManager);
                //SetMapLocation(loc);
            }

        }
        private async void MyMap_Loaded(object sender, RoutedEventArgs e)
        {
            // center on Notre Dame Cathedral  
            Message currLoc = SimulatedAzureIoT.GetResults();
            mapManager.SetMapLocation(currLoc);
        }

    }
}


