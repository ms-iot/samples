// Copyright (c) Microsoft. All rights reserved.


using SDKTemplate;
using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading.Tasks;
using Windows.Devices.WiFi;
using Windows.Foundation.Metadata;
using Windows.Networking.Connectivity;
using Windows.Security.Credentials;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace WiFiConnect
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class WiFiConnect_Scenario : Page
    {
        MainPage rootPage;
        private WiFiAdapter firstAdapter;

        public ObservableCollection<WiFiNetworkDisplay> ResultCollection
        {
            get;
            private set;
        }

        public WiFiConnect_Scenario()
        {
            this.InitializeComponent();
        }

        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            ResultCollection = new ObservableCollection<WiFiNetworkDisplay>();
            rootPage = MainPage.Current;

            // RequestAccessAsync must have been called at least once by the app before using the API
            // Calling it multiple times is fine but not necessary
            // RequestAccessAsync must be called from the UI thread
            var access = await WiFiAdapter.RequestAccessAsync();
            if (access != WiFiAccessStatus.Allowed)
            {
                rootPage.NotifyUser("Access denied", NotifyType.ErrorMessage);
            }
            else
            {
                DataContext = this;

                var result = await Windows.Devices.Enumeration.DeviceInformation.FindAllAsync(WiFiAdapter.GetDeviceSelector());
                if (result.Count >= 1)
                {
                    firstAdapter = await WiFiAdapter.FromIdAsync(result[0].Id);

                    var button = new Button();
                    button.Content = string.Format("Scan Available Wifi Networks");
                    button.Click += Button_Click;
                    Buttons.Children.Add(button);
                }
                else
                {
                    rootPage.NotifyUser("No WiFi Adapters detected on this machine.", NotifyType.ErrorMessage);
                }
            }
        }

        private async void Button_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                await firstAdapter.ScanAsync();
            }
            catch (Exception err)
            {
                rootPage.NotifyUser(String.Format("Error scanning WiFi adapter: 0x{0:X}: {1}", err.HResult, err.Message), NotifyType.ErrorMessage);
                return;
            }
            await DisplayNetworkReportAsync(firstAdapter.NetworkReport);
        }

        public string GetCurrentWifiNetwork()
        {
            var connectionProfiles = NetworkInformation.GetConnectionProfiles();

            if (connectionProfiles.Count < 1)
            {
                return null;
            }

            var validProfiles = connectionProfiles.Where(profile =>
            {
                return (profile.IsWlanConnectionProfile && profile.GetNetworkConnectivityLevel() != NetworkConnectivityLevel.None);
            });

            if (validProfiles.Count() < 1)
            {
                return null;
            }

            ConnectionProfile firstProfile = validProfiles.First();

            return firstProfile.ProfileName;
        }

        private bool IsConnected(WiFiAvailableNetwork network)
        {
            if (network == null)
                return false;

            string profileName = GetCurrentWifiNetwork();
            if (!String.IsNullOrEmpty(network.Ssid) && 
                !String.IsNullOrEmpty(profileName) &&
                (network.Ssid == profileName))
            {
                return true;
            }

            return false;
        }

        private async Task DisplayNetworkReportAsync(WiFiNetworkReport report)
        {
            rootPage.NotifyUser(string.Format("Network Report Timestamp: {0}", report.Timestamp), NotifyType.StatusMessage);

            ResultCollection.Clear();

            foreach (var network in report.AvailableNetworks)
            {
                var item = new WiFiNetworkDisplay(network, firstAdapter);
                /*await*/ item.UpdateAsync();
                if (IsConnected(network))
                {
                    ResultCollection.Insert(0, item);
                    ResultsListView.SelectedItem = ResultsListView.Items[0];
                    ResultsListView.ScrollIntoView(ResultsListView.SelectedItem);
                    SwitchToItemState(network, WifiConnectedState, false);
                }
                else
                {
                    ResultCollection.Add(item);
                }
            }
            ResultsListView.Focus(FocusState.Pointer);
        }

        private ListViewItem SwitchToItemState(object dataContext, DataTemplate template, bool forceUpdate)
        {
            if (forceUpdate)
            {
                ResultsListView.UpdateLayout();
            }
            var item = ResultsListView.ContainerFromItem(dataContext) as ListViewItem;
            if (item != null)
            {
                item.ContentTemplate = template;
            }
            return item;
        }

        private void ResultsListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var selectedNetwork = ResultsListView.SelectedItem as WiFiNetworkDisplay;
            if (selectedNetwork == null)
            {
                return;
            }

            foreach(var item in e.RemovedItems)
            {
                SwitchToItemState(item, WifiInitialState, true);
            }

            foreach (var item in e.AddedItems)
            {
                var network = item as WiFiNetworkDisplay;
                SetSelectedItemState(network);
            }
        }

        private void SetSelectedItemState(WiFiNetworkDisplay network)
        {
            if (network == null)
                return;

            if (IsConnected(network.AvailableNetwork))
            {
                SwitchToItemState(network, WifiConnectedState, true);
            }
            else
            {
                SwitchToItemState(network, WifiConnectState, true);
            }
        }

        private void PushButtonConnect_Click(object sender, RoutedEventArgs e)
        {
            DoWifiConnect(sender, e, true);
        }

        private void ConnectButton_Click(object sender, RoutedEventArgs e)
        {
            DoWifiConnect(sender, e, false);
        }

        private async void DoWifiConnect(object sender, RoutedEventArgs e, bool pushButtonConnect)
        { 
            var selectedNetwork = ResultsListView.SelectedItem as WiFiNetworkDisplay;
            if (selectedNetwork == null || firstAdapter == null)
            {
                rootPage.NotifyUser("Network not selected", NotifyType.ErrorMessage);
                return;
            }
            WiFiReconnectionKind reconnectionKind = WiFiReconnectionKind.Manual;
            if(selectedNetwork.ConnectAutomatically)
            {
                reconnectionKind = WiFiReconnectionKind.Automatic;
            }

            Task<WiFiConnectionResult> didConnect = null;
            WiFiConnectionResult result = null;
            if (pushButtonConnect)
            {
                if (ApiInformation.IsApiContractPresent("Windows.Foundation.UniversalApiContract", 5, 0))
                {
                    didConnect = firstAdapter.ConnectAsync(selectedNetwork.AvailableNetwork, reconnectionKind, null, String.Empty, WiFiConnectionMethod.WpsPushButton).AsTask<WiFiConnectionResult>();
                }
            }
            else if (selectedNetwork.IsEapAvailable)
            {
                if (selectedNetwork.UsePassword)
                {
                    var credential = new PasswordCredential();
                    if (!String.IsNullOrEmpty(selectedNetwork.Domain))
                    {
                        credential.Resource = selectedNetwork.Domain;
                    }
                    credential.UserName = selectedNetwork.UserName ?? "";
                    credential.Password = selectedNetwork.Password ?? "";

                    didConnect = firstAdapter.ConnectAsync(selectedNetwork.AvailableNetwork, reconnectionKind, credential).AsTask<WiFiConnectionResult>();
                }
                else
                {
                    didConnect = firstAdapter.ConnectAsync(selectedNetwork.AvailableNetwork, reconnectionKind).AsTask<WiFiConnectionResult>();
                }
            }
            else if (selectedNetwork.AvailableNetwork.SecuritySettings.NetworkAuthenticationType == Windows.Networking.Connectivity.NetworkAuthenticationType.Open80211 &&
                    selectedNetwork.AvailableNetwork.SecuritySettings.NetworkEncryptionType == NetworkEncryptionType.None)
            {
                didConnect = firstAdapter.ConnectAsync(selectedNetwork.AvailableNetwork, reconnectionKind).AsTask<WiFiConnectionResult>();
            }
            else
            {
                // Only the password potion of the credential need to be supplied
                if (String.IsNullOrEmpty(selectedNetwork.Password))
                {
                    didConnect = firstAdapter.ConnectAsync(selectedNetwork.AvailableNetwork, reconnectionKind).AsTask<WiFiConnectionResult>();
                }
                else
                {
                    var credential = new PasswordCredential();
                    credential.Password = selectedNetwork.Password ?? "";

                    didConnect = firstAdapter.ConnectAsync(selectedNetwork.AvailableNetwork, reconnectionKind, credential).AsTask<WiFiConnectionResult>();
                }
            }

            SwitchToItemState(selectedNetwork, WifiConnectingState, false);

            if (didConnect != null)
            {
                result = await didConnect;
            }

            if (result != null && result.ConnectionStatus == WiFiConnectionStatus.Success)
            {
                rootPage.NotifyUser(string.Format("Successfully connected to {0}.", selectedNetwork.Ssid), NotifyType.StatusMessage);

                // refresh the webpage
                webViewGrid.Visibility = Visibility.Visible;
                toggleBrowserButton.Content = "Hide Browser Control";
                refreshBrowserButton.Visibility = Visibility.Visible;

                ResultCollection.Remove(selectedNetwork);
                ResultCollection.Insert(0, selectedNetwork);
                ResultsListView.SelectedItem = ResultsListView.Items[0];
                ResultsListView.ScrollIntoView(ResultsListView.SelectedItem);

                SwitchToItemState(selectedNetwork, WifiConnectedState, false);
            }
            else
            {
                rootPage.NotifyUser(string.Format("Could not connect to {0}. Error: {1}", selectedNetwork.Ssid, result.ConnectionStatus), NotifyType.ErrorMessage);
                SwitchToItemState(selectedNetwork, WifiConnectState, false);
            }

            // Since a connection attempt was made, update the connectivity level displayed for each
            foreach (var network in ResultCollection)
            {
                network.UpdateConnectivityLevel();
            }
        }

        private void Browser_Toggle_Click(object sender, RoutedEventArgs e)
        {
            if (webViewGrid.Visibility == Visibility.Visible)
            {
                webViewGrid.Visibility = Visibility.Collapsed;
                refreshBrowserButton.Visibility = Visibility.Collapsed;
                toggleBrowserButton.Content = "Show Browser Control";
            }
            else
            {
                webViewGrid.Visibility = Visibility.Visible;
                refreshBrowserButton.Visibility = Visibility.Visible;
                toggleBrowserButton.Content = "Hide Browser Control";
            }
        }
        private void Browser_Refresh(object sender, RoutedEventArgs e)
        {
            webView.Refresh();
        }

        private void Disconnect_Click(object sender, RoutedEventArgs e)
        {
            var selectedNetwork = ResultsListView.SelectedItem as WiFiNetworkDisplay;
            if (selectedNetwork == null || firstAdapter == null)
            {
                rootPage.NotifyUser("Network not selected", NotifyType.ErrorMessage);
                return;
            }

            selectedNetwork.Disconnect();
            SetSelectedItemState(selectedNetwork);
        }
    }
}

