/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

    The MIT License(MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Windows.Devices.WiFi;
using Windows.Networking;
using Windows.Networking.Connectivity;
using Windows.Security.Credentials;

namespace IoTCoreDefaultApp
{
    public class NetworkPresenter
    {
        private readonly static uint EthernetIanaType = 6;
        private readonly static uint WifiIanaType = 71;

        public static string GetDirectConnectionName()
        {
            var icp = NetworkInformation.GetInternetConnectionProfile();

            var interfaceType = icp == null ? 0 : icp.NetworkAdapter.IanaInterfaceType;

            return (interfaceType == EthernetIanaType ? icp.ProfileName : "None found");
        }

        public static string GetCurrentNetworkName()
        {
            var icp = NetworkInformation.GetInternetConnectionProfile();
            return icp != null ? icp.ProfileName : null;
        }

        public static string GetCurrentIpv4Address()
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

        private static WiFiAccessStatus? accessStatus;

        public static async Task<bool> WifiIsAvailable()
        {
            if ((await TestAccess()) == false)
            {
                return false;
            }

            var adapters = await WiFiAdapter.FindAllAdaptersAsync();

            return adapters.Count > 0;
        }

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

        public async Task<IList<WiFiAvailableNetwork>> GetAvailableNetworks()
        {
            await UpdateInfo();

            return networkNameToInfo.Keys.ToList();
        }

        public WiFiAvailableNetwork GetCurrentWifiNetwork()
        {
            var connectionProfiles = NetworkInformation.GetConnectionProfiles();

            if (connectionProfiles.Count < 1)
            {
                return null;
            }

            var validProfiles = connectionProfiles.Where(profile =>
            {
                return (profile.NetworkAdapter != null && profile.NetworkAdapter.IanaInterfaceType == WifiIanaType);
            });

            if (validProfiles.Count() < 1)
            {
                return null;
            }

            var firstProfile = validProfiles.First() as ConnectionProfile;

            return networkNameToInfo.Keys.First(wifiNetwork => wifiNetwork.Ssid.Equals(firstProfile.ProfileName));
        }

        public async Task<bool> ConnectToNetwork(WiFiAvailableNetwork network, bool autoConnect)
        {
            if (network == null)
            {
                return false;
            }

            var result = await networkNameToInfo[network].ConnectAsync(network, autoConnect ? WiFiReconnectionKind.Automatic : WiFiReconnectionKind.Manual);

            return (result.ConnectionStatus == WiFiConnectionStatus.Success);
        }

        public void DisconnectNetwork(WiFiAvailableNetwork network)
        {
            networkNameToInfo[network].Disconnect();
        }

        public static bool IsNetworkOpen(WiFiAvailableNetwork network)
        {
            return network.SecuritySettings.NetworkEncryptionType == NetworkEncryptionType.None;
        }

        public async Task<bool> ConnectToNetworkWithPassword(WiFiAvailableNetwork network, bool autoConnect, PasswordCredential password)
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

        private static async Task<bool> TestAccess()
        {
            if (!accessStatus.HasValue)
            {
                accessStatus = await WiFiAdapter.RequestAccessAsync();
            }

            return (accessStatus == WiFiAccessStatus.Allowed);
        }
    }
}
