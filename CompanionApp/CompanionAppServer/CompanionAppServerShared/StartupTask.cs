using System;
using Windows.ApplicationModel.Background;
using Windows.System.Threading;
using Windows.Networking.Sockets;
using System.Diagnostics;
using Windows.Storage.Streams;
using System.Runtime.Serialization.Json;
using System.IO;
using System.Text;
using System.Collections.Generic;
using Windows.Devices.WiFi;
using System.Linq;
using System.Reflection;
using Windows.Security.Credentials;
using System.Threading.Tasks;
using Windows.Networking;
using System.Threading;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace CompanionAppServerShared
{
    public sealed class CompanionAppCommunication
    {
        public CompanionAppCommunication() { }
        public string Verb { get; set; }
        public string Data { get; set; }
    }

    public sealed class Util
    {
        public event Action<string> DebugInfo;

        private void HandleDebugInfo(string msg)
        {
            if (DebugInfo != null)
            {
                DebugInfo(msg);
            }
            Debug.WriteLine(msg);
        }

        public void ServerAsync()
        {
            HandleDebugInfo(string.Format("Server starting"));
            //WorkItemHandler advertisementWorker = async (context) =>
            //{
            //    var advertisementPort = "50080";
            //    using (var Client = new DatagramSocket())
            //    using (var stream = await Client.GetOutputStreamAsync(new HostName("255.255.255.255"), advertisementPort))
            //    {
            //        using (var writer = new DataWriter(stream))
            //        {
            //            var msg = "hello";
            //            var data = Encoding.UTF8.GetBytes(msg);
            //            while (true)
            //            {
            //                writer.WriteBytes(data);
            //                await writer.StoreAsync();
            //                HandleDebugInfo(string.Format("Sent UDP packet: 255.255.255.255:{0} = {1}", advertisementPort, msg));

            //                new ManualResetEvent(false).WaitOne(1000);
            //            }
            //        }
            //    }
            //};
            //ThreadPool.RunAsync(advertisementWorker);

            //WorkItemHandler worker = async (context) =>
            //{
            //    var Server = new DatagramSocket();
            //    Server.MessageReceived += Server_MessageReceived;
            //    string connectionString = "50074";
            //    await Server.BindServiceNameAsync(connectionString);
            //    HandleDebugInfo(string.Format("Listening for UDP on {0}", connectionString));
            //};

            //ThreadPool.RunAsync(worker);

            WorkItemHandler streamSocketWorker = async (context) =>
            {
                var Listener = new StreamSocketListener();
                Listener.ConnectionReceived += Listener_ConnectionReceived;
                string connectionString = "50074";
                await Listener.BindServiceNameAsync(connectionString);
                HandleDebugInfo(string.Format("Listening for StreamSocket connection on {0}", connectionString));
            };

            ThreadPool.RunAsync(streamSocketWorker);
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

        private class WifiSet
        {
            public WiFiAdapter Adapter;
            public WiFiAvailableNetwork Network;
        }

        private async Task<WifiSet> FindWifi(string ssid)
        {
            var wiFiAdapterList = await WiFiAdapter.FindAllAdaptersAsync();
            foreach (var adapter in wiFiAdapterList)
            {
                var availableNetworks = adapter.NetworkReport.AvailableNetworks;
                foreach (var availableNetwork in availableNetworks)
                {
                    if (ssid == null || availableNetwork.Ssid.Equals(ssid))
                    {
                        return new WifiSet() { Adapter = adapter, Network = availableNetwork };
                    }
                }
            }
            return null;
        }

        private async void ConnectToNetworkHandler(string networkInfo, DataWriter writer)
        {
            string[] ssidAndPassword = networkInfo.Split('=');

            var wifiSet = await FindWifi(ssidAndPassword[0]);
            if (wifiSet != null)
            {
                PasswordCredential credential = new PasswordCredential();
                credential.Password = ssidAndPassword[1];

                HandleDebugInfo(string.Format("Connecting to network using credentials: {0} [{1}]", wifiSet.Network.Ssid, credential.Password));
                var result = await wifiSet.Adapter.ConnectAsync(wifiSet.Network, WiFiReconnectionKind.Manual, credential);

                var connectionResultResponse = new CompanionAppCommunication() { Verb = "ConnectResult", Data = result.ConnectionStatus.ToString() };
                await SendResponse(connectionResultResponse, writer);
            }
        }

        private async void DisconnectFromNetworkHandler(string networkInfo, DataWriter writer)
        {
            var wifiSet = await FindWifi(networkInfo);
            if (wifiSet != null)
            {
                HandleDebugInfo(string.Format("Disconnecting from network: {0}", wifiSet.Network.Ssid));
                wifiSet.Adapter.Disconnect();

                var connectionResultResponse = new CompanionAppCommunication() { Verb = "DisconnectResult", Data = "Disconnected" };
                await SendResponse(connectionResultResponse, writer);
            }
        }

        private async void GetAvailableNetworkListHandler(DataWriter writer)
        {
            List<string> networks = new List<string>();

            var m_wiFiAdapterList = await WiFiAdapter.FindAllAdaptersAsync();
            var m_wiFiAdapter = m_wiFiAdapterList.FirstOrDefault();
            foreach (var adapter in m_wiFiAdapterList)
            {
                var availableNetworks = adapter.NetworkReport.AvailableNetworks;
                foreach (var availablenetwork in availableNetworks)
                {
                    networks.Add(availablenetwork.Ssid);
                }
            }
            var networkResponse = new CompanionAppCommunication() { Verb = "AvailableNetworks" };
            networkResponse.Data = Jsonify(typeof(string[]), networks.ToArray());
            await SendResponse(networkResponse, writer);
        }

        private async Task<CompanionAppCommunication> GetNextRequest(DataReader reader)
        {
            await reader.LoadAsync(1024);

            string data = string.Empty;
            while (reader.UnconsumedBufferLength > 0)
            {
                data += reader.ReadString(reader.UnconsumedBufferLength);
            }

            if (data.Length != 0)
            {
                HandleDebugInfo(string.Format("incoming request {0}", data));

                using (var stream = new MemoryStream(Encoding.Unicode.GetBytes(data)))
                {
                    var serializer = new DataContractJsonSerializer(typeof(CompanionAppCommunication));
                    return serializer.ReadObject(stream) as CompanionAppCommunication;
                }
            }

            return null;
        }

        private async Task SendResponse(CompanionAppCommunication response, DataWriter writer)
        {
            using (var stream = new MemoryStream())
            {
                // Send request for available networks
                string networkResponseString = Jsonify(typeof(CompanionAppCommunication), response);
                writer.WriteString(networkResponseString);
                await writer.StoreAsync();
                HandleDebugInfo(string.Format("Sent: {0}", networkResponseString));
            }

        }

        private async Task CreateStreamSocketServer(string connectionPortString)
        {
            try
            {
                var wifiInfo = await FindWifi(null);

                // Listen for connection over connection string that was sent
                var listener = new StreamSocketListener();
                listener.ConnectionReceived += Listener_ConnectionReceived;
                listener.BindServiceNameAsync(connectionPortString, SocketProtectionLevel.PlainSocket, wifiInfo.Adapter.NetworkAdapter);
                HandleDebugInfo(string.Format("Started listening for StreamSocket connections: {0}", connectionPortString));

            }
            catch (Exception e)
            {

            }
        }

        private async Task CreateStreamSocketClient(HostName hostName, string connectionPortString)
        {
            try
            {
                var wifiInfo = await FindWifi(null);

                // Conect to port string that was sent
                var Client = new StreamSocket();
                HandleDebugInfo(string.Format("Opening socket: {0}:{1}", hostName, connectionPortString));
                await Client.ConnectAsync(hostName, connectionPortString, SocketProtectionLevel.PlainSocket, wifiInfo.Adapter.NetworkAdapter);
                HandleDebugInfo(string.Format("Socket connected: {0}:{1}", hostName, connectionPortString));

                await HandleSocket(Client.InputStream, Client.OutputStream);
            }
            catch (Exception e)
            {

            }
        }

        private async void Server_MessageReceived(DatagramSocket sender, DatagramSocketMessageReceivedEventArgs args)
        {
            // Introduction from CompainionAppClient
            HandleDebugInfo(string.Format("Message received from {0}:{1}", args.RemoteAddress.DisplayName, args.RemotePort));
            var connectionPortString = args.GetDataReader().ReadString(args.GetDataReader().UnconsumedBufferLength);
            HandleDebugInfo(string.Format("Message ==> {0}", connectionPortString));

            // Response to CompanionAppClient
            using (var Client = new DatagramSocket())
            using (var stream = await Client.GetOutputStreamAsync(new HostName("255.255.255.255"), connectionPortString))
            {
                using (var writer = new DataWriter(stream))
                {
                    string streamSocketPort = "50076";
                    await CreateStreamSocketServer(streamSocketPort);
                    //await CreateStreamSocketClient(args.RemoteAddress, streamSocketPort);

                    var data = Encoding.UTF8.GetBytes(streamSocketPort);
                    writer.WriteBytes(data);
                    await writer.StoreAsync();
                    HandleDebugInfo(string.Format("Sent UDP packet: 255.255.255.255:{0} = {1}", connectionPortString, streamSocketPort));

                }
            }


        }

        private async Task HandleSocket(IInputStream istream, IOutputStream ostream)
        {
            using (var reader = new DataReader(istream))
            using (var writer = new DataWriter(ostream))
            {
                reader.InputStreamOptions = InputStreamOptions.Partial;
                while (true)
                {
                    var networkRequest = await GetNextRequest(reader);
                    if (networkRequest == null) break;

                    if (networkRequest.Verb.Equals("GetAvailableNetworks"))
                    {
                        GetAvailableNetworkListHandler(writer);
                    }
                    else if (networkRequest.Verb.Equals("ConnectToNetwork"))
                    {
                        ConnectToNetworkHandler(networkRequest.Data, writer);
                    }
                    else if (networkRequest.Verb.Equals("DisconnectFromNetwork"))
                    {
                        DisconnectFromNetworkHandler(networkRequest.Data, writer);
                    }
                }
            }
        }

        private async void Listener_ConnectionReceived(StreamSocketListener sender, StreamSocketListenerConnectionReceivedEventArgs args)
        {
            HandleDebugInfo(string.Format("Connection received from {0}:{1}", args.Socket.Information.RemoteAddress.DisplayName, args.Socket.Information.RemotePort));
            await HandleSocket(args.Socket.InputStream, args.Socket.OutputStream);
        }
    }
}
