// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources;
using Windows.Devices.Enumeration;
using Windows.Devices.WiFi;
using Windows.Networking;
using Windows.Networking.Connectivity;
using Windows.Security.Credentials;

namespace IoTCoreDefaultApp
{
    public class NetworkPresenter
    {
        private readonly static uint EthernetIanaType = 6;
        private readonly static uint WirelessInterfaceIanaType = 71;
        private Dictionary<String, WiFiAdapter> WiFiAdapters = new Dictionary<string, WiFiAdapter>();
        private DeviceWatcher WiFiAdaptersWatcher;
        ManualResetEvent EnumAdaptersCompleted = new ManualResetEvent(false);
        
        public NetworkPresenter()
        {
            WiFiAdaptersWatcher = DeviceInformation.CreateWatcher(WiFiAdapter.GetDeviceSelector());
            WiFiAdaptersWatcher.EnumerationCompleted += AdaptersEnumCompleted;
            WiFiAdaptersWatcher.Added += AdaptersAdded;
            WiFiAdaptersWatcher.Removed += AdaptersRemoved;
            WiFiAdaptersWatcher.Start();
        }

        private void AdaptersRemoved(DeviceWatcher sender, DeviceInformationUpdate args)
        {
            WiFiAdapters.Remove(args.Id);
        }

        private void AdaptersAdded(DeviceWatcher sender, DeviceInformation args)
        {
            WiFiAdapters.Add(args.Id, null);
        }

        private async void AdaptersEnumCompleted(DeviceWatcher sender, object args)
        {
            List<String> WiFiAdaptersID = new List<string>(WiFiAdapters.Keys);
            for(int i = 0; i < WiFiAdaptersID.Count; i++)
            {
                string id = WiFiAdaptersID[i];
                try
                {
                    WiFiAdapters[id] = await WiFiAdapter.FromIdAsync(id);
                }
                catch (Exception)
                {
                    WiFiAdapters.Remove(id);
                }
            }
            EnumAdaptersCompleted.Set();
        }

        public static string GetDirectConnectionName()
        {
            var icp = NetworkInformation.GetInternetConnectionProfile();
            if (icp != null)
            {
                if(icp.NetworkAdapter.IanaInterfaceType == EthernetIanaType)
                {
                    return icp.ProfileName;
                }
            }

            return null;
        }

        public static string GetCurrentNetworkName()
        {
            var icp = NetworkInformation.GetInternetConnectionProfile();
            if (icp != null)
            {
                return icp.ProfileName;
            }

            var resourceLoader = ResourceLoader.GetForCurrentView();
            var msg = resourceLoader.GetString("NoInternetConnection");
            return msg;
        }

        public static string GetCurrentIpv4Address()
        {
            var icp = NetworkInformation.GetInternetConnectionProfile();
            if (icp != null && icp.NetworkAdapter != null && icp.NetworkAdapter.NetworkAdapterId != null)
            {
                var name = icp.ProfileName;

                try
                {
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
                catch (Exception)
                {
                    // do nothing
                    // in some (strange) cases NetworkInformation.GetHostNames() fails... maybe a bug in the API...
                }
            }

            var resourceLoader = ResourceLoader.GetForCurrentView();
            var msg = resourceLoader.GetString("NoInternetConnection");
            return msg;
        }

        private Dictionary<WiFiAvailableNetwork, WiFiAdapter> networkNameToInfo;

        private static WiFiAccessStatus? accessStatus;

        // Call this method before accessing WiFiAdapters Dictionary
        private async Task UpdateAdapters()
        {
            bool fInit = false;
            foreach (var adapter in WiFiAdapters)
            {
                if (adapter.Value == null)
                {
                    // New Adapter plugged-in which requires Initialization
                    fInit = true;
                }
            }

            if (fInit)
            {
                List<String> WiFiAdaptersID = new List<string>(WiFiAdapters.Keys);
                for (int i = 0; i < WiFiAdaptersID.Count; i++)
                {
                    string id = WiFiAdaptersID[i];
                    try
                    {
                        WiFiAdapters[id] = await WiFiAdapter.FromIdAsync(id);
                    }
                    catch (Exception)
                    {
                        WiFiAdapters.Remove(id);
                    }
                }
            }
        }
        public async Task<bool> WifiIsAvailable()
        {
            if ((await TestAccess()) == false)
            {
                return false;
            }

            try
            {
                EnumAdaptersCompleted.WaitOne();
                await UpdateAdapters();
                return (WiFiAdapters.Count > 0);
            }
            catch (Exception)
            {
                return false;
            }
        }

        private async Task<bool> UpdateInfo()
        {
            if ((await TestAccess()) == false)
            {
                return false;
            }

            networkNameToInfo = new Dictionary<WiFiAvailableNetwork, WiFiAdapter>();
            List<WiFiAdapter> WiFiAdaptersList = new List<WiFiAdapter>(WiFiAdapters.Values);
            foreach (var adapter in WiFiAdaptersList)
            {
                if (adapter == null)
                {
                    return false;
                }

                await adapter.ScanAsync();

                if (adapter.NetworkReport == null)
                {
                    continue;
                }

                foreach (var network in adapter.NetworkReport.AvailableNetworks)
                {
                    if (!HasSsid(networkNameToInfo, network.Ssid))
                    {
                        networkNameToInfo[network] = adapter;
                    }
                }
            }

            return true;
        }

        private bool HasSsid(Dictionary<WiFiAvailableNetwork, WiFiAdapter> resultCollection, string ssid)
        {
            foreach (var network in resultCollection)
            {
                if (!string.IsNullOrEmpty(network.Key.Ssid) && network.Key.Ssid == ssid)
                {
                    return true;
                }
            }
            return false;
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
                return (profile.IsWlanConnectionProfile && profile.GetNetworkConnectivityLevel() != NetworkConnectivityLevel.None);
            });

            if (validProfiles.Count() < 1)
            {
                return null;
            }

            var firstProfile = validProfiles.First() as ConnectionProfile;

            return networkNameToInfo.Keys.FirstOrDefault(wifiNetwork => wifiNetwork.Ssid.Equals(firstProfile.ProfileName));
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


        public class NetworkInfo
        {
            public string NetworkName { get; set; }
            public string NetworkIpv6 { get; set; }
            public string NetworkIpv4 { get; set; }
            public string NetworkStatus { get; set; }
        }

        public static async Task<IList<NetworkInfo>> GetNetworkInformation()
        {
            var networkList = new Dictionary<Guid, NetworkInfo>();

            try
            {
                var hostNamesList = NetworkInformation.GetHostNames();
                var resourceLoader = ResourceLoader.GetForCurrentView();

                foreach (var hostName in hostNamesList)
                {
                    if ((hostName.Type == HostNameType.Ipv4 || hostName.Type == HostNameType.Ipv6) &&
                        (hostName != null && hostName.IPInformation != null && hostName.IPInformation.NetworkAdapter != null))
                    {
                        var profile = await hostName.IPInformation.NetworkAdapter.GetConnectedProfileAsync();
                        if (profile != null)
                        {
                            NetworkInfo info;
                            var found = networkList.TryGetValue(hostName.IPInformation.NetworkAdapter.NetworkAdapterId, out info);
                            if (!found)
                            {
                                info = new NetworkInfo();
                                networkList[hostName.IPInformation.NetworkAdapter.NetworkAdapterId] = info;
                                if (hostName.IPInformation.NetworkAdapter.IanaInterfaceType == WirelessInterfaceIanaType &&
                                    profile.ProfileName.Equals("Ethernet"))
                                {
                                    info.NetworkName = "Wireless LAN Adapter";
                                }
                                else
                                {
                                    info.NetworkName = profile.ProfileName;
                                }
                                var statusTag = profile.GetNetworkConnectivityLevel().ToString();
                                info.NetworkStatus = resourceLoader.GetString("NetworkConnectivityLevel_" + statusTag);
                            }
                            if (hostName.Type == HostNameType.Ipv4)
                            {
                                info.NetworkIpv4 = hostName.CanonicalName;
                            }
                            else
                            {
                                info.NetworkIpv6 = hostName.CanonicalName;
                            }
                        }
                    }
                }
            }
            catch (Exception)
            {
                // do nothing
                // in some (strange) cases NetworkInformation.GetHostNames() fails... maybe a bug in the API...
            }

            var res = new List<NetworkInfo>();
            res.AddRange(networkList.Values);
            return res;
        }
    }
}
