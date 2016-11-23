//#define AUTOMATE_FOR_TESTING

using System.Collections.ObjectModel;
using Windows.UI.Xaml.Data;
using Xamarin.Forms;


namespace CompanionAppClient
{
    public partial class MainPage : ContentPage
    {
#if AUTOMATE_FOR_TESTING
        private string _TestAutomation_AccessPointPartial = "";
        private string _TestAutomation_NetworkPartial = "";
        private string _TestAutomation_NetworkPassword = "";
#endif
        public static IAccessPointHelper AccessPointHelper { get; set; }
        private ObservableCollection<AccessPoint> _AvailableAccessPoints = new ObservableCollection<AccessPoint>();
        private ObservableCollection<Network> _AvailableNetworks = new ObservableCollection<Network>();

        private string _StatusAction = "";
        private string _StatusResult = "";

        private bool AccessPointsScanned { get; set; }
        private bool AccessPointConnected { get; set; }
        private bool ClientNetworksEnumerated { get; set; }
        private bool ClientNetworkConnected { get; set; }

        public MainPage()
        {
            InitializeComponent();

            _connectButton.IsEnabled = false;

            _availableAccessPointListView.ItemsSource = _AvailableAccessPoints;
            _availableNetworkListView.ItemsSource = _AvailableNetworks;

            AccessPointHelper.AccessPointConnectedEvent += AccessPointHelper_AccessPointConnectedEvent;
            AccessPointHelper.AccessPointsEnumeratedEvent += AccessPointHelper_AccessPointsEnumeratedEvent;
            AccessPointHelper.ClientNetworkConnectedEvent += AccessPointHelper_ClientNetworkConnectedEvent;
            AccessPointHelper.ClientNetworksEnumeratedEvent += AccessPointHelper_ClientNetworksEnumeratedEvent;

            AccessPointsScanned = false;
            AccessPointConnected = false;
            ClientNetworkConnected = false;
            ClientNetworksEnumerated = false;

            _clientNetworkPassword.IsEnabled = false;
            _connectClientNetwork.IsEnabled = false;
            _connectButton.IsEnabled = false;
            _getClientNetworks.IsEnabled = false;

            _ScanApsGrouping.OutlineColor = Color.Yellow;
            _ConnectApGrouping.OutlineColor = Color.Default;
            _NetworksGrouping.OutlineColor = Color.Default;
            _DisconnectGrouping.OutlineColor = Color.Default;
            _ConnectGrouping.OutlineColor = Color.Default;

            _StatusAction = "";
            _StatusResult = "";

#if AUTOMATE_FOR_TESTING
            AccessPointHelper.FindAccessPoints(_AvailableAccessPoints);
#endif
        }

        private void UpdateStatus(string action, string result)
        {
            if (action != null) _StatusAction = action;
            if (result != null) _StatusResult = result;
            _Status.Text = string.Format("{0} ... {1}", _StatusAction, _StatusResult);
        }

        private void AccessPointHelper_ClientNetworksEnumeratedEvent(string status)
        {
            Device.BeginInvokeOnMainThread(() => {
                ClientNetworksEnumerated = true;

                _getClientNetworks.IsEnabled = ClientNetworksEnumerated;

                _ScanApsGrouping.OutlineColor = Color.Default;
                _ConnectApGrouping.OutlineColor = Color.Default;
                _NetworksGrouping.OutlineColor = Color.Default;
                _ConnectGrouping.OutlineColor = ClientNetworksEnumerated ? Color.Yellow : Color.Default;
                _DisconnectGrouping.OutlineColor = Color.Default;

                UpdateStatus(null, status);
#if AUTOMATE_FOR_TESTING
                foreach (var n in _AvailableNetworks)
                {
                    if (n.Ssid.Contains(_TestAutomation_NetworkPartial))
                    {
                        AccessPointHelper.ConnectToClientNetwork(n.Ssid.ToString(), _TestAutomation_NetworkPassword);
                        break;
                    }
                }
#endif
            });
        }

        private void AccessPointHelper_ClientNetworkConnectedEvent(string status)
        {
            Device.BeginInvokeOnMainThread(() => {
                ClientNetworkConnected = true;
                _disconnectClientNetwork.IsEnabled = ClientNetworkConnected;

                _ScanApsGrouping.OutlineColor = Color.Default;
                _ConnectApGrouping.OutlineColor = Color.Default;
                _NetworksGrouping.OutlineColor = Color.Default;
                _ConnectGrouping.OutlineColor = Color.Default;
                _DisconnectGrouping.OutlineColor = ClientNetworkConnected ? Color.Yellow : Color.Default;

                UpdateStatus(null, status);
            });
        }

        private void AccessPointHelper_AccessPointsEnumeratedEvent(string status)
        {
            Device.BeginInvokeOnMainThread(() => {
                AccessPointsScanned = true;
                _clientNetworkPassword.IsEnabled = AccessPointsScanned;

                _ScanApsGrouping.OutlineColor = Color.Default;
                _ConnectApGrouping.OutlineColor = AccessPointsScanned ? Color.Yellow : Color.Default;
                _NetworksGrouping.OutlineColor = Color.Default;
                _DisconnectGrouping.OutlineColor = Color.Default;
                _ConnectGrouping.OutlineColor = Color.Default;

                UpdateStatus(null, status);

#if AUTOMATE_FOR_TESTING
                foreach (var ap in _AvailableAccessPoints)
                {
                    if (ap.Ssid.Contains(_TestAutomation_AccessPointPartial))
                    {
                        _availableAccessPointListView.SelectedItem = ap;
                        AccessPointHelper.ConnectToAccessPoint(ap);
                        break;
                    }
                }
#endif
            });
        }

        private void AccessPointHelper_AccessPointConnectedEvent(string status)
        {
            Device.BeginInvokeOnMainThread(() => {
                AccessPointConnected = true;
                _getClientNetworks.IsEnabled = AccessPointConnected;

                _ScanApsGrouping.OutlineColor = Color.Default;
                _ConnectApGrouping.OutlineColor = Color.Default;
                _NetworksGrouping.OutlineColor = AccessPointConnected ? Color.Yellow : Color.Default;
                _DisconnectGrouping.OutlineColor = Color.Default;
                _ConnectGrouping.OutlineColor = Color.Default;

                UpdateStatus(null, status);

#if AUTOMATE_FOR_TESTING
                AccessPointHelper.GetClientNetworks(_AvailableNetworks);
#endif
            });
        }

        ~MainPage()
        {
            AccessPointHelper.Disconnect();
        }

        void SelectAccessPoint(object sender, SelectedItemChangedEventArgs e)
        {
            _connectButton.IsEnabled = AccessPointsScanned;
        }

        void SelectClientNetwork(object sender, SelectedItemChangedEventArgs e)
        {
            _connectClientNetwork.IsEnabled = ClientNetworksEnumerated;
        }

        void ConnectClientNetwork(object sender, System.EventArgs e)
        {
            UpdateStatus("Connecting to client network", "");
            var network = _availableNetworkListView.SelectedItem as Network;
            AccessPointHelper.ConnectToClientNetwork(network.Ssid, _clientNetworkPassword.Text);
        }

        void DisconnectClientNetwork(object sender, System.EventArgs e)
        {
            UpdateStatus("Disconnecting from client network", "");
            var network = _availableNetworkListView.SelectedItem as Network;
            AccessPointHelper.DisconnectFromClientNetwork(network.Ssid);
        }

        void ConnectToAccessPoint(object sender, System.EventArgs e)
        {
            UpdateStatus("Connecting to access point", "");
            var accessPoint = _availableAccessPointListView.SelectedItem as AccessPoint;
            AccessPointHelper.ConnectToAccessPoint(accessPoint);
        }

        public void GetClientNetworks(object sender, System.EventArgs e)
        {
            UpdateStatus("Getting available client networks", "");
            AccessPointHelper.GetClientNetworks(_AvailableNetworks);
        }

        public void FindAccessPoints(object sender, System.EventArgs e)
        {
            UpdateStatus("Getting available access points", "");
            AccessPointHelper.FindAccessPoints(_AvailableAccessPoints);
        }

        public void Exit(object sender, System.EventArgs e)
        {
            AccessPointHelper.Disconnect();
        }
    }
}
