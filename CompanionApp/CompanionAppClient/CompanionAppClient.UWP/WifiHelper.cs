// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel.Core;
using Windows.Devices.WiFi;
using Windows.Networking;
using Windows.Networking.Connectivity;
using Windows.Networking.Sockets;
using Windows.Security.Credentials;
using Windows.Storage.Streams;
using Windows.System.Threading;

namespace CompanionAppClient.UWP
{
    public class WifiHelper : IAccessPointHelper
    {
        public event Action<string> AccessPointsEnumeratedEvent;
        public event Action<string> AccessPointConnectedEvent;
        public event Action<string> ClientNetworkConnectedEvent;
        public event Action<string> ClientNetworksEnumeratedEvent;

        private SemaphoreSlim _SocketLock = new SemaphoreSlim(1, 1);
        private StreamSocket _ConnectedSocket = null;
        private DataWriter _DataWriter = null;
        private DataReader _DataReader = null;
        private WiFiAdapter _connectedWifiAdapter = null;

        public async Task FindAccessPoints(ObservableCollection<AccessPoint> availableAccessPoints)
        {
            await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () => {
                availableAccessPoints.Clear();
            });

            // Add distinct AP Ssids in sorted order
            var wifiAdapterList = await WiFiAdapter.FindAllAdaptersAsync();
            wifiAdapterList.SelectMany(adapter => adapter.NetworkReport.AvailableNetworks).
                            Select(network => network.Ssid).
                            Distinct().
                            OrderBy(ssid => ssid).ToList().
                            ForEach(async ssid => {
                var ap = new AccessPoint() { Ssid = ssid };
                await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () => {
                    availableAccessPoints.Add(ap);
                });
            });

            if (AccessPointsEnumeratedEvent != null)
            {
                AccessPointsEnumeratedEvent("Enumerated");
            }
        }

        public async Task RequestClientNetworks(ObservableCollection<Network> availableNetworks)
        {
            if (_ConnectedSocket == null) { return; }

            await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () => {
                availableNetworks.Clear();
            });

            var networkRequest = new CompanionAppCommunication() { Verb = "GetAvailableNetworks" };
            // Send request for available networks
            await SendRequest(networkRequest);

            // Read response with available networks                
            var networkResponse = await GetNextRequest();
            if (networkResponse != null && networkResponse.Verb == "AvailableNetworks")
            {
                var availableNetworkArray = Dejsonify(typeof(string[]), networkResponse.Data) as string[];
                availableNetworkArray.OrderBy(x => x).Distinct().ToList().ForEach(async availableNetwork => {
                    var network = new Network() { Ssid = availableNetwork };
                    await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                    {
                        availableNetworks.Add(network);
                    });
                });
            }

            if (ClientNetworksEnumeratedEvent != null)
            {
                ClientNetworksEnumeratedEvent("Enumerated");
            }
        }

        private async Task CreateStreamSocketClient(HostName hostName, string connectionPortString)
        {
            try
            {
                // Conect to port string that was sent
                var Client = new StreamSocket();
                Debug.WriteLine(string.Format("Opening socket: {0}:{1}", hostName, connectionPortString));
                await Client.ConnectAsync(hostName, connectionPortString, SocketProtectionLevel.PlainSocket);
                Debug.WriteLine(string.Format("Socket connected: {0}:{1}", hostName, connectionPortString));
                HandleSocket(Client);
            }
            catch (Exception)
            {
                // Handle failure here as desired.  In this sample, 
                // the failure will be handled by ConnectToAccessPoint
            }
        }


        public async Task ConnectToAccessPoint(AccessPoint accessPoint)
        {
            var wifiAdapterList = await WiFiAdapter.FindAllAdaptersAsync();

            var wifiList = from adapter in wifiAdapterList from network in adapter.NetworkReport.AvailableNetworks select Tuple.Create(adapter, network);
            var apInfo = wifiList.Where(wifiInfo => wifiInfo.Item2.Ssid.Equals(accessPoint.Ssid)).First();

            WiFiConnectionResult result = null;
            if (apInfo != null)
            {
                var wifiNetwork = apInfo.Item2;
                var wiFiAdapter = apInfo.Item1;

                if (wifiNetwork.SecuritySettings.NetworkAuthenticationType == NetworkAuthenticationType.Open80211)
                {
                    Debug.WriteLine(string.Format("Opening connection to: {0}", wifiNetwork.Ssid));
                    result = await wiFiAdapter.ConnectAsync(wifiNetwork, WiFiReconnectionKind.Manual);
                }
                else
                {
                    PasswordCredential credential = new PasswordCredential();
                    credential.Password = "p@ssw0rd";

                    Debug.WriteLine(string.Format("Opening connection to using credentials: {0} [{1}]", wifiNetwork.Ssid, credential.Password));
                    result = await wiFiAdapter.ConnectAsync(wifiNetwork, WiFiReconnectionKind.Manual, credential);
                }

                if (result.ConnectionStatus == WiFiConnectionStatus.Success)
                {
                    Debug.WriteLine(string.Format("Connected successfully to: {0}.{1}", wiFiAdapter.NetworkAdapter.NetworkAdapterId, wifiNetwork.Ssid));
                    _connectedWifiAdapter = wiFiAdapter;

                    await CreateStreamSocketClient(new HostName(SharedConstants.SOFT_AP_IP), SharedConstants.CONNECTION_PORT);
                }
            }

            string connectionEventString = "Connected";
            if (_ConnectedSocket == null)
            {
                Debug.WriteLine(string.Format("Connection failed: {0}", result != null ? result.ConnectionStatus.ToString() : "access point not found"));
                connectionEventString = "FailedConnected";
            }
            if (AccessPointConnectedEvent != null)
            {
                AccessPointConnectedEvent(connectionEventString);
            }
        }

        public async Task ConnectToClientNetwork(string networkSsid, string password)
        {
            if (_ConnectedSocket == null) { return; }

            var connectRequest = new CompanionAppCommunication() { Verb = "ConnectToNetwork", Data = string.Format("{0}={1}", networkSsid, password) };
            // Send request to connect to network
            await SendRequest(connectRequest);

            // Read response with available networks
            var networkResponse = await GetNextRequest();
            if (networkResponse != null && networkResponse.Verb == "ConnectResult")
            {
                if (ClientNetworkConnectedEvent != null)
                {
                    ClientNetworkConnectedEvent(networkResponse.Data);
                }
            }
        }

        public async Task DisconnectFromClientNetwork(string networkSsid)
        {
            if (_ConnectedSocket == null) { return; }

            var disconnectRequest = new CompanionAppCommunication() { Verb = "DisconnectFromNetwork", Data = networkSsid };
            // Send request to connect to network
            await SendRequest(disconnectRequest);

            // Read response with available networks
            var networkResponse = await GetNextRequest();
            if (networkResponse != null && networkResponse.Verb == "DisconnectResult")
            {
                if (ClientNetworkConnectedEvent != null)
                {
                    ClientNetworkConnectedEvent(networkResponse.Data);
                }
            }
        }

        public async Task Disconnect()
        {
            _SocketLock.Wait();

            try
            {
                if (_DataReader != null)
                {
                    _DataReader.Dispose();
                    _DataReader = null;
                }
                if (_DataWriter != null)
                {
                    _DataWriter.Dispose();
                    _DataWriter = null;
                }
                if (_ConnectedSocket != null)
                {
                    _ConnectedSocket.Dispose();
                    _ConnectedSocket = null;
                }
            }
            finally
            {
                _SocketLock.Release();
            }

            if (_connectedWifiAdapter != null)
            {
                var wifiAdapter = _connectedWifiAdapter;
                _connectedWifiAdapter = null;
                wifiAdapter.Disconnect();
            }

            Windows.UI.Xaml.Application.Current.Exit();
        }

        private void HandleSocket(StreamSocket socket)
        {
            _SocketLock.Wait();

            try
            {
                Debug.WriteLine(string.Format("Connection established from: {0}:{1}", socket.Information.RemoteAddress.DisplayName, socket.Information.RemotePort));
                _ConnectedSocket = socket;
                _DataWriter = new DataWriter(_ConnectedSocket.OutputStream);
                _DataReader = new DataReader(_ConnectedSocket.InputStream);
                _DataReader.InputStreamOptions = InputStreamOptions.Partial;
            }
            finally
            {
                _SocketLock.Release();
            }
        }

        private async Task SendRequest(CompanionAppCommunication communication)
        {
            //
            // In this sample, protected information is sent over the channel
            // as plain text.  This data needs to be protcted with encryption
            // based on a trust relationship between the Companion App client
            // and server.
            //
            await _SocketLock.WaitAsync();
            try
            {
                string requestData = Jsonify(typeof(CompanionAppCommunication), communication);
                _DataWriter.WriteString(requestData);
                await _DataWriter.StoreAsync();
                Debug.WriteLine(string.Format("Sent: {0}", requestData));
            }
            finally
            {
                _SocketLock.Release();
            }
        }

        private async Task<CompanionAppCommunication> GetNextRequest()
        {
            CompanionAppCommunication msg = null;

            await _SocketLock.WaitAsync();
            try
            {
                await _DataReader.LoadAsync(1024);

                string data = string.Empty;
                while (_DataReader.UnconsumedBufferLength > 0)
                {
                    data += _DataReader.ReadString(_DataReader.UnconsumedBufferLength);
                }

                //
                // In this sample, protected information is sent over the channel
                // as plain text.  This data needs to be protcted with encryption
                // based on a trust relationship between the Companion App client
                // and server.
                //

                if (data.Length != 0)
                {
                    Debug.WriteLine(string.Format("incoming request {0}", data));
    
                    using (var stream = new MemoryStream(Encoding.Unicode.GetBytes(data)))
                    {
                        var serializer = new DataContractJsonSerializer(typeof(CompanionAppCommunication));
                        msg = serializer.ReadObject(stream) as CompanionAppCommunication;
                    }
                }
            }
            finally
            {
                _SocketLock.Release();
            }

            return msg;
        }

        private string Jsonify(Type typeInfo, object data)
        {
            string stringData = string.Empty;
            using (var stream = new MemoryStream())
            {
                var serializer = new DataContractJsonSerializer(typeInfo);
                serializer.WriteObject(stream, data);
                stream.Seek(0, SeekOrigin.Begin);

                using (var jsonStream = new StreamReader(stream))
                {
                    stringData = jsonStream.ReadToEnd();
                }
            }
            return stringData;
        }

        private object Dejsonify(Type typeInfo, string data)
        {
            using (var stream = new MemoryStream(Encoding.Unicode.GetBytes(data)))
            {
                var serializer = new DataContractJsonSerializer(typeInfo);
                return serializer.ReadObject(stream);
            }
        }
    }
}
