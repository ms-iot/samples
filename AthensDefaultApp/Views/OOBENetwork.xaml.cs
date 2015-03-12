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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace AthensDefaultApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class OOBENetwork : Page
    {
        public OOBENetwork()
        {
            this.InitializeComponent();
            SetupEthernet();
            SetupWifi();
        }

        private void SetupEthernet()
        {
            var ethernetProfile = NetworkPresenter.GetDirectConnectionName();

            if (ethernetProfile.Equals("None found"))
            {
                NoneFoundText.Visibility = Visibility.Visible;
                DirectConnectionsListView.Visibility = Visibility.Collapsed;
            }
            else
            {
                DirectConnectionsListView.ItemsSource = new List<string> { ethernetProfile };
            }
        }

        private void SetupWifi()
        {
            var network = new WifiNetwork();
            network.NetworkName = "Test Network Name";

            WifiListView.ItemsSource = new List<WifiNetwork>() { network };
        }

        private void WifiListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var listView = sender as ListView;

            foreach(var item in e.RemovedItems)
            {
                var listViewItem = listView.ContainerFromItem(item) as ListViewItem;
                listViewItem.ContentTemplate = WifiInitialState;
            }

            foreach(var item in e.AddedItems)
            {
                var listViewItem = listView.ContainerFromItem(item) as ListViewItem;
                listViewItem.ContentTemplate = WifiConnectState;
            }
        }

        private void ConnectButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            var button = sender as Button;

            var item = WifiListView.ContainerFromItem(button.DataContext) as ListViewItem;

            item.ContentTemplate = WifiPasswordState;
        }

        private void NextButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(MainPage));
        }

        private void CancelButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            var button = sender as Button;

            var item = WifiListView.ContainerFromItem(button.DataContext) as ListViewItem;

            item.ContentTemplate = WifiInitialState;
            item.IsSelected = false;
        }

        private void BackButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }

        private void SkipButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(MainPage));
        }
    }
}
