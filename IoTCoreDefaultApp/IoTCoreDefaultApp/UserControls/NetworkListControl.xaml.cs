using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Devices.WiFi;
using Windows.Security.Credentials;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

namespace IoTCoreDefaultApp
{
    public sealed partial class NetworkListControl : UserControl
    {
        private NetworkPresenter networkPresenter = new NetworkPresenter();
        private bool ConnectAutomatically = true;
        private string CurrentPassword = string.Empty;

        public event EventHandler<EventArgs> NetworkConnected;

        public NetworkListControl()
        {
            this.InitializeComponent();

            this.DataContext = LanguageManager.GetInstance();
        }

        private void EnableView(bool enable)
        {
            RefreshButton.IsEnabled = enable;
            WifiListView.IsEnabled = enable;
        }

        private async void RefreshButton_Click(object sender, RoutedEventArgs e)
        {
            await RecreateWifiNetworkListAsync();
        }

        public async Task SetupNetworkAsync()
        {
            SetupEthernet();
            await RecreateWifiNetworkListAsync();
        }

        private void SetupEthernet()
        {
            var ethernetProfile = NetworkPresenter.GetDirectConnectionName();

            if (ethernetProfile == null)
            {
                NoneFoundText.Visibility = Visibility.Visible;
                DirectConnectionStackPanel.Visibility = Visibility.Collapsed;
            }
            else
            {
                NoneFoundText.Visibility = Visibility.Collapsed;
                DirectConnectionStackPanel.Visibility = Visibility.Visible;
            }
        }

        private async Task RecreateWifiNetworkListAsync()
        {
            if (await networkPresenter.WifiIsAvailable())
            {
                EnableView(false);

                ObservableCollection<WiFiAvailableNetwork> networks;
                try
                {
                    networks = new ObservableCollection<WiFiAvailableNetwork>(await networkPresenter.GetAvailableNetworks());
                }
                catch (Exception e)
                {
                    Debug.WriteLine(String.Format("Error scanning: 0x{0:X}: {1}", e.HResult, e.Message));
                    NoWifiFoundText.Text = e.Message;
                    NoWifiFoundText.Visibility = Visibility.Visible;
                    EnableView(true);
                    return;
                }

                if (networks.Count > 0)
                {

                    var connectedNetwork = networkPresenter.GetCurrentWifiNetwork();
                    if (connectedNetwork != null)
                    {
                        networks.Remove(connectedNetwork);
                        networks.Insert(0, connectedNetwork);
                        WifiListView.ItemsSource = networks;
                        SwitchToItemState(connectedNetwork, WifiConnectedState, true);
                    }
                    else
                    {
                        WifiListView.ItemsSource = networks;
                    }


                    NoWifiFoundText.Visibility = Visibility.Collapsed;
                    WifiListView.Visibility = Visibility.Visible;
                    EnableView(true);
                    return;
                }
            }

            NoWifiFoundText.Visibility = Visibility.Visible;
            WifiListView.Visibility = Visibility.Collapsed;
        }

        private void WifiListView_ItemClick(object sender, ItemClickEventArgs e)
        {
            var connectedNetwork = networkPresenter.GetCurrentWifiNetwork();
            var item = e.ClickedItem;
            if (connectedNetwork == item)
            {
                SwitchToItemState(item, WifiConnectedMoreOptions, true);
            }
        }

        private void WifiListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var listView = sender as ListView;
            foreach (var item in e.RemovedItems)
            {
                SwitchToItemState(item, WifiInitialState, true);
            }

            foreach (var item in e.AddedItems)
            {
                ConnectAutomatically = true;
                var connectedNetwork = networkPresenter.GetCurrentWifiNetwork();

                if (connectedNetwork == item)
                {
                    SwitchToItemState(connectedNetwork, WifiConnectedMoreOptions, true);
                }
                else
                {
                    SwitchToItemState(item, WifiConnectState, true);
                }
            }
        }

        private async void ConnectButton_Clicked(object sender, RoutedEventArgs e)
        {
            try
            {
                EnableView(false);

                var button = sender as Button;
                var network = button.DataContext as WiFiAvailableNetwork;
                if (NetworkPresenter.IsNetworkOpen(network))
                {
                    await ConnectToWifiAsync(network, null, Window.Current.Dispatcher);
                }
                else
                {
                    SwitchToItemState(network, WifiPasswordState, false);
                }
            }
            finally
            {
                EnableView(true);
            }
        }

        private async Task ConnectToWifiAsync(WiFiAvailableNetwork network, PasswordCredential credential, CoreDispatcher dispatcher)
        {
            await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                SwitchToItemState(network, WifiConnectingState, false);
            });

            var didConnect = credential == null ?
                networkPresenter.ConnectToNetwork(network, ConnectAutomatically) :
                networkPresenter.ConnectToNetworkWithPassword(network, ConnectAutomatically, credential);
            DataTemplate nextState = (await didConnect) ? WifiConnectedState : WifiInitialState;
            bool isConnected = (nextState == WifiConnectedState);

            await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                var list = WifiListView.ItemsSource as ObservableCollection<WiFiAvailableNetwork>;
                var itemLocation = list.IndexOf(network);
                if (0 != itemLocation && isConnected)
                {
                    list.Move(itemLocation, 0);
                }
                var item = SwitchToItemState(network, nextState, true);
                if (item != null)
                {
                    item.IsSelected = true;
                }
            });

            if (isConnected)
            {
                NetworkConnected?.Invoke(this, new EventArgs());
            }
        }

        private void DisconnectButton_Clicked(object sender, RoutedEventArgs e)
        {
            var button = sender as Button;
            var network = button.DataContext as WiFiAvailableNetwork;
            var connectedNetwork = networkPresenter.GetCurrentWifiNetwork();

            if (network == connectedNetwork)
            {
                networkPresenter.DisconnectNetwork(network);
                var item = SwitchToItemState(network, WifiInitialState, true);
                item.IsSelected = false;
            }
        }

        private async void NextButton_Clicked(object sender, RoutedEventArgs e)
        {
            var button = sender as Button;
            PasswordCredential credential;

            if (string.IsNullOrEmpty(CurrentPassword))
            {
                credential = null;
            }
            else
            {
                credential = new PasswordCredential()
                {
                    Password = CurrentPassword
                };
            }

            var network = button.DataContext as WiFiAvailableNetwork;
            await ConnectToWifiAsync(network, credential, Window.Current.Dispatcher);
        }

        private void CancelButton_Clicked(object sender, RoutedEventArgs e)
        {
            var button = sender as Button;
            var item = SwitchToItemState(button.DataContext, WifiInitialState, false);
            item.IsSelected = false;
        }

        private ListViewItem SwitchToItemState(object dataContext, DataTemplate template, bool forceUpdate)
        {
            if (forceUpdate)
            {
                WifiListView.UpdateLayout();
            }
            var item = WifiListView.ContainerFromItem(dataContext) as ListViewItem;
            if (item != null)
            {
                item.ContentTemplate = template;
            }
            return item;
        }

        private void ConnectAutomaticallyCheckBox_Changed(object sender, RoutedEventArgs e)
        {
            var checkbox = sender as CheckBox;

            ConnectAutomatically = checkbox.IsChecked ?? false;
        }

        private void WifiPasswordBox_PasswordChanged(object sender, RoutedEventArgs e)
        {
            var passwordBox = sender as PasswordBox;
            CurrentPassword = passwordBox.Password;
        }

        private void WifiPasswordBox_Loaded(object sender, RoutedEventArgs e)
        {
            var passwordBox = sender as PasswordBox;
            if (passwordBox != null)
            {
                passwordBox.Focus(FocusState.Programmatic);
            }
        }

    }
}
