using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Threading;
using Android.Content;
using Android.App;
using Android.Net.Wifi;
using System.Collections.Generic;
using Xamarin.Forms;
using Android.Net;

namespace CompanionAppClient.Droid
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

        private TcpClient _ConnectedSocket = null;
        private NetworkStream _NetworkStream = null;

        class WifiReceiver : BroadcastReceiver
        {
            HashSet<string> _FoundAccessPoints = new HashSet<string>();
            ObservableCollection<AccessPoint> _AvailableAccessPoints;
            WifiHelper _Helper;
            public WifiReceiver(WifiHelper helper, ObservableCollection<AccessPoint> availableAccessPoints)
            {
                _AvailableAccessPoints = availableAccessPoints;
                _Helper = helper;
            }
            public override void OnReceive(Context context, Intent intent)
            {
                var wifiManager = Android.App.Application.Context.GetSystemService(Context.WifiService) as WifiManager;
                var scanWifiNetworks = wifiManager.ScanResults;
                foreach (ScanResult wifiNetwork in scanWifiNetworks)
                {
                    string ssid = wifiNetwork.Ssid;
                    if (!_FoundAccessPoints.Contains(ssid))
                    {
                        _AvailableAccessPoints.Add(new CompanionAppClient.AccessPoint() { Ssid = ssid, Details = "none" });
                        _FoundAccessPoints.Add(ssid);
                    }
                }

                if (_Helper.AccessPointsEnumeratedEvent != null)
                {
                    _Helper.AccessPointsEnumeratedEvent("Enumerated");
                }
            }
        }

        public void FindAccessPoints(ObservableCollection<AccessPoint> availableAccessPoints)
        {
            availableAccessPoints.Clear();

            var wifiManager = Android.App.Application.Context.GetSystemService(Context.WifiService) as WifiManager;
            if (wifiManager.IsWifiEnabled)
            {
                Android.App.Application.Context.RegisterReceiver(new WifiReceiver(this, availableAccessPoints), new IntentFilter(WifiManager.ScanResultsAvailableAction));
                wifiManager.StartScan();
            }
            else if (AccessPointsEnumeratedEvent != null)
            {
                AccessPointsEnumeratedEvent("Enumerated");
            }
        }

        public void GetClientNetworks(ObservableCollection<Network> availableNetworks)
        {
            if (_ConnectedSocket == null) { return; }

            WaitCallback worker = async (context) =>
            {
                Device.BeginInvokeOnMainThread(() => {
                    availableNetworks.Clear();
                });

                var networkRequest = new CompanionAppCommunication() { Verb = "GetAvailableNetworks" };
                // Send request for available networks
                string requestData = Jsonify(networkRequest);
                WriteToSocket(requestData);

                // Read response with available networks                
                var networkResponse = await GetNextRequest(_NetworkStream);
                if (networkResponse != null && networkResponse.Verb == "AvailableNetworks")
                {
                    var availableNetworkArray = JsonToStringArray(networkResponse.Data);
                    foreach (var availableNetwork in availableNetworkArray)
                    {
                        var network = new Network() { Ssid = availableNetwork };
                        Device.BeginInvokeOnMainThread(() => {
                            availableNetworks.Add(network);
                        });
                    }
                }

                if (ClientNetworksEnumeratedEvent != null)
                {
                    ClientNetworksEnumeratedEvent("Enumerated");
                }
            };
            ThreadPool.QueueUserWorkItem(worker);
        }

        private async Task CreateStreamSocketClient(string hostName, string connectionPortString)
        {
            try
            {
                // Conect to port string that was sent
                Debug.WriteLine(string.Format("Opening socket: {0}:{1}", hostName, connectionPortString));
                var Client = new TcpClient();
                await Client.ConnectAsync(hostName, int.Parse(connectionPortString));
                Debug.WriteLine(string.Format("Socket connected: {0}:{1}", hostName, connectionPortString));
                HandleSocket(Client);
            }
            catch (Exception e)
            {

            }
        }


        public void ConnectToAccessPoint(AccessPoint accessPoint)
        {
            WaitCallback worker = async (context) =>
            {
                var androidContext = Android.App.Application.Context;
                var wifiManager = androidContext.GetSystemService(Context.WifiService) as WifiManager;
                
                var wc = new WifiConfiguration();
                wc.Ssid = string.Format("\"{0}\"", accessPoint.Ssid);
                wc.PreSharedKey = string.Format("\"{0}\"", "p@ssw0rd");
                wc.StatusField = WifiStatus.Enabled;
                //
                // what to set here????  http://stackoverflow.com/questions/8818290/how-do-i-connect-to-a-specific-wi-fi-network-in-android-programmatically
                //
                wc.AllowedGroupCiphers.Set((int)GroupCipherType.Tkip);
                wc.AllowedGroupCiphers.Set((int)GroupCipherType.Ccmp);
                wc.AllowedKeyManagement.Set((int)KeyManagementType.WpaPsk);
                wc.AllowedPairwiseCiphers.Set((int)PairwiseCipherType.Tkip);
                wc.AllowedPairwiseCiphers.Set((int)PairwiseCipherType.Ccmp);
                wc.AllowedProtocols.Set((int)Android.Net.Wifi.ProtocolType.Rsn);

                // connect to and enable the connection
                string connectionEventString = "Connected";
                int netId = wifiManager.AddNetwork(wc);
                if (netId != -1)
                {
                    var disconnected = wifiManager.Disconnect();
                    var enabled = wifiManager.EnableNetwork(netId, true);
                    var reconnected = wifiManager.Reconnect();

                    var connectivityManager = (ConnectivityManager)androidContext.GetSystemService(Context.ConnectivityService);
                    var wifiInfo = connectivityManager.GetNetworkInfo(ConnectivityType.Wifi);

                    Debug.WriteLine("Connected successfully to: {0}", wc.Ssid);

                    new ManualResetEvent(false).WaitOne(5 * 1000);

                    Android.Net.Network wifiNetwork = null;
                    if (connectivityManager.ActiveNetworkInfo.TypeName.Equals("WIFI"))
                    {
                        wifiNetwork = connectivityManager.ActiveNetwork;
                    }
                    else
                    {
                        foreach (var n in connectivityManager.GetAllNetworks())
                        {
                            if (connectivityManager.GetNetworkInfo(n).TypeName.Equals("WIFI"))
                            {
                                wifiNetwork = n;
                                break;
                            }
                        }
                    }

                    if (wifiNetwork != null)
                    {
                        var boundToNetwork = connectivityManager.BindProcessToNetwork(wifiNetwork);

                        string streamSocketPort = "50074";
                        string defaultSoftApIp = "192.168.137.1";
                        await CreateStreamSocketClient(defaultSoftApIp, streamSocketPort);
                    }
                }

                if (_ConnectedSocket == null)
                {
                    connectionEventString = "FailedConnected";
                }

                if (AccessPointConnectedEvent != null)
                {
                    AccessPointConnectedEvent(connectionEventString);
                }
            };
            ThreadPool.QueueUserWorkItem(worker);
        }

        public void ConnectToClientNetwork(string networkSsid, string password)
        {
            if (_ConnectedSocket == null) { return; }

            WaitCallback worker = async (context) =>
            {
                var connectRequest = new CompanionAppCommunication() { Verb = "ConnectToNetwork", Data = string.Format("{0}={1}", networkSsid, password) };
                // Send request to connect to network
                string requestData = Jsonify(connectRequest);
                WriteToSocket(requestData);

                // Read response with available networks
                var networkResponse = await GetNextRequest(_NetworkStream);
                if (networkResponse != null && networkResponse.Verb == "ConnectResult")
                {
                    if (ClientNetworkConnectedEvent != null)
                    {
                        ClientNetworkConnectedEvent(networkResponse.Data);
                    }
                }
            };
            ThreadPool.QueueUserWorkItem(worker);
        }

        public void DisconnectFromClientNetwork(string networkSsid)
        {
            if (_ConnectedSocket == null) { return; }

            WaitCallback worker = async (context) =>
            {
                var connectRequest = new CompanionAppCommunication() { Verb = "DisconnectFromNetwork", Data = networkSsid };
                // Send request to connect to network
                string requestData = Jsonify(connectRequest);
                WriteToSocket(requestData);

                // Read response with available networks
                var networkResponse = await GetNextRequest(_NetworkStream);
                if (networkResponse != null && networkResponse.Verb == "DisconnectResult")
                {
                    if (ClientNetworkConnectedEvent != null)
                    {
                        ClientNetworkConnectedEvent(networkResponse.Data);
                    }
                }
            };
            ThreadPool.QueueUserWorkItem(worker);
        }

        public void Disconnect()
        {
            if (_NetworkStream != null)
            {
                _NetworkStream.Dispose();
                _NetworkStream = null;
            }
            if (_ConnectedSocket != null)
            {
                _ConnectedSocket.Dispose();
                _ConnectedSocket = null;
            }

            var activity = (Activity)Forms.Context;
            activity.FinishAffinity();
        }

        private void HandleSocket(TcpClient socket)
        {
            _ConnectedSocket = socket;
            _NetworkStream = _ConnectedSocket.GetStream();
        }

        private async Task<CompanionAppCommunication> GetNextRequest(NetworkStream stream)
        {
            byte[] resp = new byte[2048];
            var memStream = new MemoryStream();
            var bytes = 0;

            do
            {
                bytes = stream.Read(resp, 0, resp.Length);
                memStream.Write(resp, 0, bytes);
            } while (stream.DataAvailable);

            ASCIIEncoding asen = new ASCIIEncoding();
            var data = asen.GetString(memStream.ToArray());

            if (data.Length != 0)
            {
                Debug.WriteLine(string.Format("incoming request {0}", data));

                var jsonData = Newtonsoft.Json.Linq.JObject.Parse(data);
                return new CompanionAppCommunication() { Verb = (string)jsonData["Verb"], Data = (string)jsonData["Data"] };
            }

            return null;
        }

        private void WriteToSocket(string data)
        {
            ASCIIEncoding asen = new ASCIIEncoding();
            byte[] ba = asen.GetBytes(data);
            _NetworkStream.Write(ba, 0, ba.Length);
            Debug.WriteLine("Sent: {0}", data);
        }

        private string Jsonify(object data)
        {
            return Newtonsoft.Json.JsonConvert.SerializeObject(data);
        }

        private string[] JsonToStringArray(string data)
        {
            List<string> strings = new List<string>();
            if (!string.IsNullOrEmpty(data))
            {
                var jsonData = Newtonsoft.Json.Linq.JArray.Parse(data);
                foreach (var str in jsonData.Children())
                {
                    strings.Add((string)str);
                }
            }
            return strings.ToArray();
        }
    }
}
