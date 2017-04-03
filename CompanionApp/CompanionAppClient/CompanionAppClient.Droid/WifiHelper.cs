// Copyright (c) Microsoft. All rights reserved.

using Android.App;
using Android.Content;
using Android.Net;
using Android.Net.Wifi;
using Foundation;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Xamarin.Forms;

namespace CompanionAppClient.Droid
{
    public class WifiHelper : IAccessPointHelper
    {
        public event Action<string> AccessPointsEnumeratedEvent;
        public event Action<string> AccessPointConnectedEvent;
        public event Action<string> ClientNetworkConnectedEvent;
        public event Action<string> ClientNetworksEnumeratedEvent;

        private SemaphoreSlim _SocketLock = new SemaphoreSlim(1, 1);
        private WifiReceiver _WifiReceiver = null;
        private TcpClient _ConnectedSocket = null;
        private NetworkStream _NetworkStream = null;

        class WifiReceiver : BroadcastReceiver
        {
            ObservableCollection<AccessPoint> _AvailableAccessPoints;
            WifiHelper _Helper;
            public WifiReceiver(WifiHelper helper, ObservableCollection<AccessPoint> availableAccessPoints)
            {
                _AvailableAccessPoints = availableAccessPoints;
                _Helper = helper;
            }
            public override void OnReceive(Context context, Intent intent)
            {
                Android.App.Application.Context.UnregisterReceiver(_Helper._WifiReceiver);

                var wifiManager = Android.App.Application.Context.GetSystemService(Context.WifiService) as WifiManager;
                var scanWifiNetworks = wifiManager.ScanResults;

                // Add distinct AP Ssids in sorted order
                scanWifiNetworks.Select(x => x.Ssid).
                                 Distinct().
                                 OrderBy(ssid => ssid).ToList().
                                 ForEach(ssid => {
                    _AvailableAccessPoints.Add(new AccessPoint() { Ssid = ssid });
                });

                if (_Helper.AccessPointsEnumeratedEvent != null)
                {
                    _Helper.AccessPointsEnumeratedEvent("Enumerated");
                }
            }
        }

        public async Task FindAccessPoints(ObservableCollection<AccessPoint> availableAccessPoints)
        {
            availableAccessPoints.Clear();

            var wifiManager = Android.App.Application.Context.GetSystemService(Context.WifiService) as WifiManager;
            if (wifiManager.IsWifiEnabled)
            {
                if (_WifiReceiver == null)
                {
                    _WifiReceiver = new WifiReceiver(this, availableAccessPoints);
                }
                Android.App.Application.Context.RegisterReceiver(_WifiReceiver, new IntentFilter(WifiManager.ScanResultsAvailableAction));
                wifiManager.StartScan();
            }
            else if (AccessPointsEnumeratedEvent != null)
            {
                AccessPointsEnumeratedEvent("Enumerated");
            }
        }

        public async Task RequestClientNetworks(ObservableCollection<Network> availableNetworks)
        {
            if (_ConnectedSocket == null) { return; }

            // Assumes thread ID of main thread is 1 for Android
            var isMainThread = (Thread.CurrentThread.ManagedThreadId == 1);
            var waitForClear = new AutoResetEvent(false);
            Action clearAction = () => {
                availableNetworks.Clear();
                waitForClear.Set();
            };
            if (isMainThread)
            {
                clearAction.Invoke();
            }
            else
            {
                Device.BeginInvokeOnMainThread(clearAction);
                waitForClear.WaitOne();
            }

            var networkRequest = new CompanionAppCommunication() { Verb = "GetAvailableNetworks" };
            // Send request for available networks
            WriteToSocket(networkRequest);

            // Read response with available networks                
            var networkResponse = await GetNextRequest();
            if (networkResponse != null && networkResponse.Verb == "AvailableNetworks")
            {
                var availableNetworkArray = JsonToStringList(networkResponse.Data);
                availableNetworkArray.ForEach(availableNetwork => {
                    var network = new Network() { Ssid = availableNetwork };
                    Action addAction = () => { availableNetworks.Add(network); };
                    if (isMainThread)
                    {
                        addAction.Invoke();
                    }
                    else
                    {
                        Device.BeginInvokeOnMainThread(addAction);
                    }
                });
            }

            if (ClientNetworksEnumeratedEvent != null)
            {
                ClientNetworksEnumeratedEvent("Enumerated");
            }
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
            catch (Exception)
            {
                // Failure to connect is handled in ConnectToAccessPoint
            }
        }


        public async Task ConnectToAccessPoint(AccessPoint accessPoint)
        {
            var androidContext = Android.App.Application.Context;
            var wifiManager = androidContext.GetSystemService(Context.WifiService) as WifiManager;

            var wc = new WifiConfiguration();
            wc.Ssid = string.Format("\"{0}\"", accessPoint.Ssid);
            wc.PreSharedKey = string.Format("\"{0}\"", "p@ssw0rd");
            wc.StatusField = WifiStatus.Enabled;

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

                Debug.WriteLine(string.Format("Connected successfully to: {0}", wc.Ssid));

                await Task.Delay(TimeSpan.FromSeconds(5));

                Android.Net.Network wifiNetwork = null;
                if (connectivityManager.ActiveNetworkInfo.TypeName.Equals("WIFI"))
                {
                    wifiNetwork = connectivityManager.ActiveNetwork;
                }
                else
                {
                    var allWifiNetworks = connectivityManager.GetAllNetworks();
                    wifiNetwork = allWifiNetworks.Where(n => connectivityManager.GetNetworkInfo(n).TypeName.Equals("WIFI")).First();
                }

                if (wifiNetwork != null)
                {
                    var boundToNetwork = connectivityManager.BindProcessToNetwork(wifiNetwork);
                    await CreateStreamSocketClient(SharedConstants.SOFT_AP_IP, SharedConstants.CONNECTION_PORT);
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
        }

        public async Task ConnectToClientNetwork(string networkSsid, string password)
        {
            if (_ConnectedSocket == null) { return; }

            var connectRequest = new CompanionAppCommunication() { Verb = "ConnectToNetwork", Data = string.Format("{0}={1}", networkSsid, password) };
            // Send request to connect to network
            WriteToSocket(connectRequest);

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
            // Send request to disconnect to network
            WriteToSocket(disconnectRequest);

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
            }
            finally
            {
                _SocketLock.Release();
            }

            var activity = (Activity)Forms.Context;
            activity.FinishAffinity();
        }

        private void HandleSocket(TcpClient socket)
        {
            _SocketLock.Wait();

            try
            {
                _ConnectedSocket = socket;
                _NetworkStream = _ConnectedSocket.GetStream();
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
                byte[] resp = new byte[2048];
                var memStream = new MemoryStream();
                var bytes = 0;

                do
                {
                    bytes = _NetworkStream.Read(resp, 0, resp.Length);
                    memStream.Write(resp, 0, bytes);
                } while (_NetworkStream.DataAvailable);

                ASCIIEncoding asen = new ASCIIEncoding();
                var data = asen.GetString(memStream.ToArray());

                //
                // In this sample, protected information is sent over the channel
                // as plain text.  This data needs to be protcted with encryption
                // based on a trust relationship between the Companion App client
                // and server.
                //

                if (data.Length != 0)
                {
                    Debug.WriteLine(string.Format("incoming request {0}", data));

                    var jsonData = Newtonsoft.Json.Linq.JObject.Parse(data);
                    msg = new CompanionAppCommunication() {
                        Verb = (string)jsonData["Verb"],
                        Data = (string)jsonData["Data"]
                    };
                }
            }
            finally
            {
                _SocketLock.Release();
            }
            return msg;
        }

        private void WriteToSocket(CompanionAppCommunication communication)
        {
            //
            // In this sample, protected information is sent over the channel
            // as plain text.  This data needs to be protcted with encryption
            // based on a trust relationship between the Companion App client
            // and server.
            //

            _SocketLock.Wait();

            try
            {
                var data = Jsonify(communication);
                ASCIIEncoding asen = new ASCIIEncoding();
                byte[] ba = asen.GetBytes(data);
                _NetworkStream.Write(ba, 0, ba.Length);
                Debug.WriteLine(string.Format("Sent: {0}", data));
            }
            finally
            {
                _SocketLock.Release();
            }
        }

        private string Jsonify(object data)
        {
            return Newtonsoft.Json.JsonConvert.SerializeObject(data);
        }

        private List<string> JsonToStringList(string data)
        {
            if (!string.IsNullOrEmpty(data))
            {
                var jsonData = Newtonsoft.Json.Linq.JArray.Parse(data);
                return jsonData.Children().Select(x => (string)x).OrderBy(x => x).ToList();
            }
            return new List<string>(0);
        }
    }
}
