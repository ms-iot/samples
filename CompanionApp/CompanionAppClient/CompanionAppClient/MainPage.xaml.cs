// Copyright (c) Microsoft. All rights reserved.

//#define AUTOMATE_FOR_TESTING
using System;
using System.Collections.ObjectModel;
using System.Threading.Tasks;
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
        enum State
        {
            Started,
            AccessPointsEnmerated,
            AccessPointSelected,
            AccessPointConnected,
            NetworksRequested,
            NetworksEnumerated,
            NetworkSelected,
            NetworkInfoSent,
            NetworkConnected,
        }

        private State CurrentState { get; set; }

        private void HandleState(State nextState)
        {
            _connectButton.IsEnabled = (nextState != State.Started) && (_availableAccessPointListView.SelectedItem != null);
            _requestClientNetworks.IsEnabled = (nextState >= State.AccessPointConnected);
            _clientNetworkPassword.IsEnabled = (nextState >= State.NetworksEnumerated) && (_availableNetworkListView.SelectedItem != null);
            _connectClientNetwork.IsEnabled = (nextState >= State.NetworksEnumerated) && (_availableNetworkListView.SelectedItem != null);

            _ScanApsGrouping.OutlineColor = (nextState == State.Started) ? Color.Yellow : Color.Default;
            _ConnectApGrouping.OutlineColor = (nextState == State.AccessPointsEnmerated || nextState == State.AccessPointSelected) ? Color.Yellow : Color.Default;
            _NetworksGrouping.OutlineColor = (nextState == State.AccessPointConnected) ? Color.Yellow : Color.Default;
            _ConnectGrouping.OutlineColor = (nextState == State.NetworksEnumerated || nextState == State.NetworkSelected) ? Color.Yellow : Color.Default;
            _DisconnectGrouping.OutlineColor = (nextState == State.NetworkConnected) ? Color.Yellow : Color.Default;

            CurrentState = nextState;
        }

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
            _requestClientNetworks.IsEnabled = false;

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

                HandleState(State.NetworksEnumerated);
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
                HandleState(State.NetworkConnected);
                UpdateStatus(null, status);
            });
        }

        private void AccessPointHelper_AccessPointsEnumeratedEvent(string status)
        {
            Device.BeginInvokeOnMainThread(() => {
                HandleState(State.AccessPointsEnmerated);
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
                HandleState(State.AccessPointConnected);
                UpdateStatus(null, status);

#if AUTOMATE_FOR_TESTING
                AccessPointHelper.RequestClientNetworks(_AvailableNetworks);
#endif
            });
        }

        ~MainPage()
        {
            AccessPointHelper.Disconnect();
        }

        void SelectAccessPoint(object sender, SelectedItemChangedEventArgs e)
        {
            HandleState(State.AccessPointSelected);
        }

        void SelectClientNetwork(object sender, SelectedItemChangedEventArgs e)
        {
            HandleState(State.NetworkSelected);
        }

        void HandleException(AggregateException exception)
        {
            Device.BeginInvokeOnMainThread(() =>
            {
                HandleState(State.Started);
                UpdateStatus(null, string.Format("Encountered problem: {0}", exception.Message));
            });
        }

        void ConnectClientNetwork(object sender, System.EventArgs e)
        {
            UpdateStatus("Connecting to client network", "");
            var network = _availableNetworkListView.SelectedItem as Network;
            var task = AccessPointHelper.ConnectToClientNetwork(network.Ssid, _clientNetworkPassword.Text);
            task.ContinueWith(t => { HandleException(t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
        }

        void DisconnectClientNetwork(object sender, System.EventArgs e)
        {
            UpdateStatus("Disconnecting from client network", "");
            var network = _availableNetworkListView.SelectedItem as Network;
            var task = AccessPointHelper.DisconnectFromClientNetwork(network.Ssid);
            task.ContinueWith(t => { HandleException(t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
        }

        void ConnectToAccessPoint(object sender, System.EventArgs e)
        {
            UpdateStatus("Connecting to access point", "");
            var accessPoint = _availableAccessPointListView.SelectedItem as AccessPoint;
            var task = AccessPointHelper.ConnectToAccessPoint(accessPoint);
            task.ContinueWith(t => { HandleException(t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
        }

        public void RequestClientNetworks(object sender, System.EventArgs e)
        {
            UpdateStatus("Getting available client networks", "");
            var task = AccessPointHelper.RequestClientNetworks(_AvailableNetworks);
            task.ContinueWith(t => { HandleException(t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
        }

        public void FindAccessPoints(object sender, System.EventArgs e)
        {
            UpdateStatus("Getting available access points", "");
            var task = AccessPointHelper.FindAccessPoints(_AvailableAccessPoints);
            task.ContinueWith(t => { HandleException(t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
        }

        public void Exit(object sender, System.EventArgs e)
        {
            AccessPointHelper.Disconnect();
        }
    }
}
