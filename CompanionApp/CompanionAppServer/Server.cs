using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.WiFi;
using Windows.Networking.Sockets;
using Windows.Security.Credentials;
using Windows.Storage.Streams;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace CompanionAppServer
{
    public sealed class CompanionAppCommunication
    {
        public CompanionAppCommunication() { }
        public string Verb { get; set; }
        public string Data { get; set; }
    }

    class Server
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

        public async Task Start()
        {
            HandleDebugInfo(string.Format("Server starting"));

            var Listener = new StreamSocketListener();
            Listener.ConnectionReceived += Listener_ConnectionReceived;
            string connectionString = "50074";
            await Listener.BindServiceNameAsync(connectionString);
            HandleDebugInfo(string.Format("Listening for StreamSocket connection on {0}", connectionString));
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
            var wifiAdapterList = await WiFiAdapter.FindAllAdaptersAsync();
            var wifiList = from adapter in wifiAdapterList
                           from network in adapter.NetworkReport.AvailableNetworks
                           where network.Ssid.Equals(ssid)
                           select new WifiSet() { Adapter = adapter, Network = network };
            return wifiList.First();
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
            var wifiAdapterList = await WiFiAdapter.FindAllAdaptersAsync();
            var wifiList = from adapter in wifiAdapterList
                           from network in adapter.NetworkReport.AvailableNetworks
                           select network.Ssid;
            var networks = wifiList.OrderBy(x => x).Distinct();

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

            //
            // In this sample, protected information is sent over the channel
            // as plain text.  This data needs to be protcted with encryption
            // based on a trust relationship between the Companion App client
            // and server.
            //

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
            //
            // In this sample, protected information is sent over the channel
            // as plain text.  This data needs to be protcted with encryption
            // based on a trust relationship between the Companion App client
            // and server.
            //

            using (var stream = new MemoryStream())
            {
                // Send request
                string requestData = Jsonify(typeof(CompanionAppCommunication), response);
                writer.WriteString(requestData);
                await writer.StoreAsync();
                HandleDebugInfo(string.Format("Sent: {0} [{1}]", requestData, requestData));
            }

        }

        private async Task HandleSocket(IInputStream istream, IOutputStream ostream)
        {
            try
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
            catch (Exception)
            {
                // Connection lost.
            }
        }

        private async void Listener_ConnectionReceived(StreamSocketListener sender, StreamSocketListenerConnectionReceivedEventArgs args)
        {
            HandleDebugInfo(string.Format("Connection received from {0}:{1}", args.Socket.Information.RemoteAddress.DisplayName, args.Socket.Information.RemotePort));
            await HandleSocket(args.Socket.InputStream, args.Socket.OutputStream);
        }
    }
}
