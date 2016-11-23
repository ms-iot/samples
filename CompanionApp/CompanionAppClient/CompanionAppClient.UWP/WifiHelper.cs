using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.Core;
using Windows.Devices.WiFi;
using Windows.Networking;
using Windows.Networking.Connectivity;
using Windows.Networking.Sockets;
using Windows.Security.Credentials;
using Windows.Security.Cryptography;
using Windows.Security.Cryptography.Core;
using Windows.Storage.Streams;
using Windows.System.Threading;

namespace CompanionAppClient.UWP
{
    public class CompanionAppCommunication
    {
        public CompanionAppCommunication() { }
        public string Verb { get; set; }
        public string Data { get; set; }
    }

    public class WifiHelper : IAccessPointHelper
    {
        public event Action<string> AccessPointsEnumeratedEvent;
        public event Action<string> AccessPointConnectedEvent;
        public event Action<string> ClientNetworkConnectedEvent;
        public event Action<string> ClientNetworksEnumeratedEvent;

        private StreamSocket _ConnectedSocket = null;
        private DataWriter _DataWriter = null;
        private DataReader _DataReader = null;
        private WiFiAdapter _connectedWifiAdapter = null;

        public void FindAccessPoints(ObservableCollection<AccessPoint> availableAccessPoints)
        {
            WorkItemHandler worker = async (context) =>
            {
                await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () => {
                    availableAccessPoints.Clear();
                });

                var m_wiFiAdapterList = await WiFiAdapter.FindAllAdaptersAsync();
                var m_wiFiAdapter = m_wiFiAdapterList.FirstOrDefault();
                foreach (var adapter in m_wiFiAdapterList)
                {
                    var availableNetworks = adapter.NetworkReport.AvailableNetworks;
                    foreach (var availablenetwork in availableNetworks)
                    {
                        var ap = new AccessPoint()
                        {
                            Ssid = availablenetwork.Ssid,
                            Details = adapter.NetworkAdapter.NetworkAdapterId.ToString(),
                        };

                        await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () => {
                            availableAccessPoints.Add(ap);
                        });
                    }
                }

                if (AccessPointsEnumeratedEvent != null)
                {
                    AccessPointsEnumeratedEvent("Enumerated");
                }
            };
            ThreadPool.RunAsync(worker);
        }

        public void GetClientNetworks(ObservableCollection<Network> availableNetworks)
        {
            if (_ConnectedSocket == null) { return; }

            WorkItemHandler worker = async (context) =>
            {
                await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () => {
                    availableNetworks.Clear();
                });

                var networkRequest = new CompanionAppCommunication() { Verb = "GetAvailableNetworks" };
                // Send request for available networks
                await SendRequest(networkRequest, _DataWriter);

                // Read response with available networks                
                var networkResponse = await GetNextRequest(_DataReader);
                if (networkResponse != null && networkResponse.Verb == "AvailableNetworks")
                {
                    var availableNetworkArray = Dejsonify(typeof(string[]), networkResponse.Data) as string[];
                    foreach (var availableNetwork in availableNetworkArray)
                    {
                        var network = new Network() { Ssid = availableNetwork };
                        await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () => {
                            availableNetworks.Add(network);
                        });
                    }
                }

                if (ClientNetworksEnumeratedEvent != null)
                {
                    ClientNetworksEnumeratedEvent("Enumerated");
                }
            };
            ThreadPool.RunAsync(worker);
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
            catch (Exception e)
            {

            }
        }


        public void ConnectToAccessPoint(AccessPoint accessPoint)
        {
            WorkItemHandler worker = async (context) =>
            {
                var wiFiAdapterList = await WiFiAdapter.FindAllAdaptersAsync();
                WiFiAvailableNetwork wifiNetwork = null;
                WiFiAdapter wiFiAdapter = null;

                foreach (var adapter in wiFiAdapterList)
                {
                    var availableNetworks = adapter.NetworkReport.AvailableNetworks;
                    foreach (var availablenetwork in availableNetworks)
                    {
                        Debug.WriteLine(string.Format("{0}.{1}", adapter.NetworkAdapter.NetworkAdapterId, availablenetwork.Ssid));
                        if (availablenetwork.Ssid == accessPoint.Ssid)
                        {
                            wifiNetwork = availablenetwork;
                            wiFiAdapter = adapter;
                            break;

                        }
                    }
                }

                WiFiConnectionResult result = null;
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

                    string streamSocketPort = "50074";
                    string defaultSoftApIp = "192.168.137.1";
                    await CreateStreamSocketClient(new HostName(defaultSoftApIp), streamSocketPort);
                }

                string connectionEventString = "Connected";
                if (_ConnectedSocket == null)
                {
                    Debug.WriteLine(string.Format("Connection failed: {0}", result.ConnectionStatus.ToString()));
                    connectionEventString = "FailedConnected";
                }
                if (AccessPointConnectedEvent != null)
                {
                    AccessPointConnectedEvent(connectionEventString);
                }
            };
            ThreadPool.RunAsync(worker);
        }

        public void ConnectToClientNetwork(string networkSsid, string password)
        {
            if (_ConnectedSocket == null) { return; }

            WorkItemHandler worker = async (context) =>
            {
                var connectRequest = new CompanionAppCommunication() { Verb = "ConnectToNetwork", Data = string.Format("{0}={1}", networkSsid, password) };
                // Send request to connect to network
                await SendRequest(connectRequest, _DataWriter);

                // Read response with available networks
                var networkResponse = await GetNextRequest(_DataReader);
                if (networkResponse != null && networkResponse.Verb == "ConnectResult")
                {
                    if (ClientNetworkConnectedEvent != null)
                    {
                        ClientNetworkConnectedEvent(networkResponse.Data);
                    }
                }
            };
            ThreadPool.RunAsync(worker);
        }

        public void DisconnectFromClientNetwork(string networkSsid)
        {
            if (_ConnectedSocket == null) { return; }

            WorkItemHandler worker = async (context) =>
            {
                var disconnectRequest = new CompanionAppCommunication() { Verb = "DisconnectFromNetwork", Data = networkSsid };
                // Send request to connect to network
                await SendRequest(disconnectRequest, _DataWriter);

                // Read response with available networks
                var networkResponse = await GetNextRequest(_DataReader);
                if (networkResponse != null && networkResponse.Verb == "DisconnectResult")
                {
                    if (ClientNetworkConnectedEvent != null)
                    {
                        ClientNetworkConnectedEvent(networkResponse.Data);
                    }
                }
            };
            ThreadPool.RunAsync(worker);
        }

        public void Disconnect()
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
            Debug.WriteLine(string.Format("Connection established from: {0}:{1}", socket.Information.RemoteAddress.DisplayName, socket.Information.RemotePort));
            _ConnectedSocket = socket;
            _DataWriter = new DataWriter(_ConnectedSocket.OutputStream);
            _DataReader = new DataReader(_ConnectedSocket.InputStream);
            _DataReader.InputStreamOptions = InputStreamOptions.Partial;
        }

        private async Task SendRequest(CompanionAppCommunication communication, DataWriter writer)
        {
            //
            // In this sample, protected information is sent over the channel
            // as plain text.  This data needs to be protcted with encryption
            // based on a trust relationship between the Companion App client
            // and server.
            //

            string requestData = Jsonify(typeof(CompanionAppCommunication), communication);
            writer.WriteString(requestData);
            await writer.StoreAsync();
            Debug.WriteLine(string.Format("Sent: {0}", requestData));
        }

        private async Task<CompanionAppCommunication> GetNextRequest(DataReader reader)
        {
            await reader.LoadAsync(1024);

            string data = string.Empty;
            while (reader.UnconsumedBufferLength > 0)
            {
                data += reader.ReadString(reader.UnconsumedBufferLength);
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
                    return serializer.ReadObject(stream) as CompanionAppCommunication;
                }
            }

            return null;
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
