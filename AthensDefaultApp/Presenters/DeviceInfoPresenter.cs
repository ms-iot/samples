// © Copyright(C) Microsoft. All rights reserved.

using System;
using System.Linq;
using Windows.Networking;
using Windows.Networking.Connectivity;

namespace AthensDefaultApp
{
    internal class DeviceInfoPresenter
    {
        private SYSTEM_INFO systemInfo;

        internal DeviceInfoPresenter()
        {
            systemInfo = SystemInfoFactory.GetNativeSystemInfo();
        }

        internal static string GetDeviceName()
        {
            var hostname = NetworkInformation.GetHostNames()
                .FirstOrDefault(x => x.Type == HostNameType.DomainName);
            if (hostname != null)
            {
                return hostname.CanonicalName;
            }
            return "<no device name>";
        }

        internal static string GetCurrentNetworkName()
        {
            var icp = NetworkInformation.GetInternetConnectionProfile();
            return icp.ProfileName;
        }

        internal static string GetCurrentIpv4Address()
        {
            var icp = NetworkInformation.GetInternetConnectionProfile();
            var name = icp.ProfileName;
            if (icp != null && icp.NetworkAdapter != null)
            {
                var hostnames = NetworkInformation.GetHostNames();

                foreach (var hn in hostnames)
                {
                    if (hn.IPInformation != null &&
                        hn.IPInformation.NetworkAdapter != null &&
                        hn.IPInformation.NetworkAdapter.NetworkAdapterId == icp.NetworkAdapter.NetworkAdapterId &&
                        hn.Type == HostNameType.Ipv4)
                    {
                        return hn.CanonicalName;
                    }
                }
            }

            return "<no Internet connection>";
        }        

        internal string GetBoardName()
        {
            var loader = new Windows.ApplicationModel.Resources.ResourceLoader();

            return loader.GetString("MBMName");
        }

        internal Uri GetBoardImageUri()
        {
            return new Uri("ms-appx:///Assets/MBMBoard.png");
        }
    }
}
