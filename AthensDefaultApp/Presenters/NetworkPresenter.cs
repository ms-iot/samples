using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Windows.Devices.WiFi;
using Windows.Networking;
using Windows.Networking.Connectivity;
using Windows.Security.Credentials;

namespace AthensDefaultApp
{
    internal class NetworkPresenter
    {
        private static uint EthernetIanaType = 6;

        internal static string GetDirectConnectionName()
        {
            var icp = NetworkInformation.GetInternetConnectionProfile();

            var interfaceType = icp == null ? 0 : icp.NetworkAdapter.IanaInterfaceType;

            return (interfaceType == EthernetIanaType ? icp.ProfileName : "None found");
        }

        internal static string GetCurrentNetworkName()
        {
            var icp = NetworkInformation.GetInternetConnectionProfile();
            return icp != null ? icp.ProfileName : null;
        }

        internal static string GetCurrentIpv4Address()
        {
            var icp = NetworkInformation.GetInternetConnectionProfile();
            if (icp != null && icp.NetworkAdapter != null && icp.NetworkAdapter.NetworkAdapterId != null)
            {
                var name = icp.ProfileName;

                var hostnames = NetworkInformation.GetHostNames();

                foreach (var hn in hostnames)
                {
                    if (hn.IPInformation != null &&
                        hn.IPInformation.NetworkAdapter != null &&
                        hn.IPInformation.NetworkAdapter.NetworkAdapterId != null &&
                        hn.IPInformation.NetworkAdapter.NetworkAdapterId == icp.NetworkAdapter.NetworkAdapterId &&
                        hn.Type == HostNameType.Ipv4)
                    {
                        return hn.CanonicalName;
                    }
                }
            }

            return "<no Internet connection>";
        }

        private Dictionary<WiFiAvailableNetwork, WiFiAdapter> networkNameToInfo;

        private WiFiAccessStatus? accessStatus;

        private async Task<bool> UpdateInfo()
        {
            if ((await TestAccess()) == false)
            {
                return false;
            }

            networkNameToInfo = new Dictionary<WiFiAvailableNetwork, WiFiAdapter>();

            var adapters = WiFiAdapter.FindAllAdaptersAsync();

            foreach (var adapter in await adapters)
            {
                await adapter.ScanAsync();

                if (adapter.NetworkReport == null)
                {
                    continue;
                }

                foreach(var network in adapter.NetworkReport.AvailableNetworks)
                {
                    if (!string.IsNullOrEmpty(network.Ssid))
                    {
                        networkNameToInfo[network] = adapter;
                    }
                }
            }

            return true;
        }

        internal async Task<IList<WiFiAvailableNetwork>> GetAvailableNetworks()
        {
            await UpdateInfo();

            return networkNameToInfo.Keys.ToList();
        }

        internal async Task<bool> ConnectToNetwork(WiFiAvailableNetwork network, bool autoConnect)
        {
            if (network == null)
            {
                return false;
            }

            var result = await networkNameToInfo[network].ConnectAsync(network, autoConnect ? WiFiReconnectionKind.Automatic : WiFiReconnectionKind.Manual);

            return (result.ConnectionStatus == WiFiConnectionStatus.Success);
        }

        internal void DisconnectNetwork(WiFiAvailableNetwork network)
        {
            networkNameToInfo[network].Disconnect();
        }

        internal static bool IsNetworkOpen(WiFiAvailableNetwork network)
        {
            return network.SecuritySettings.NetworkEncryptionType == NetworkEncryptionType.None;
        }

        internal async Task<bool> ConnectToNetworkWithPassword(WiFiAvailableNetwork network, bool autoConnect, PasswordCredential password)
        {
            if (network == null)
            {
                return false;
            }

            var result = await networkNameToInfo[network].ConnectAsync(
                network,
                autoConnect ? WiFiReconnectionKind.Automatic : WiFiReconnectionKind.Manual,
                password);

            return (result.ConnectionStatus == WiFiConnectionStatus.Success);
        }

        private async Task<bool> TestAccess()
        {
            if (!accessStatus.HasValue)
            {
                accessStatus = await WiFiAdapter.RequestAccessAsync();
            }

            return (accessStatus == WiFiAccessStatus.Allowed);
        }
    }
}
