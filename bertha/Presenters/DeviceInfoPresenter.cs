// © Copyright(C) Microsoft. All rights reserved.

using System;
using System.Linq;
using System.Net.NetworkInformation;
using System.Text;
using Windows.Networking;
using Windows.Networking.Connectivity;

namespace bertha
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

        internal static string GetCurrentMACAddress()
        {
			//var networkInterfaces = NetworkInterface.GetAllNetworkInterfaces();
			//foreach (var nic in networkInterfaces)
			//{
			//    if (nic.OperationalStatus == OperationalStatus.Up)
			//    {
			//        return string.Join(":", nic.GetPhysicalAddress().GetAddressBytes().Select(b => b.ToString("X2")));
			//    }
			//}

			//return string.Empty;


			// Temporary until the API is found/fixed
			return "00:00:00:00";
        }

        internal string GetProcessor()
        {
            StringBuilder sb = new StringBuilder();

            sb.Append(GetProcessorArch(systemInfo.wProcessorArchitecture));
            sb.Append(' ');
            sb.Append(systemInfo.dwNumberOfProcessors);
            sb.Append((systemInfo.dwNumberOfProcessors > 1) ? " Cores" : " Core");

            return sb.ToString();
        }

        private static string GetProcessorArch(ushort arch)
        {
            switch (arch)
            {
                case SystemInfoFactory.PROCESSOR_ARCHITECTURE_INTEL:
                    return "x86";
                case SystemInfoFactory.PROCESSOR_ARCHITECTURE_AMD64:
                    return "x64";
                case SystemInfoFactory.PROCESSOR_ARCHITECTURE_IA64:
                    return "Intel Itanium";
                case SystemInfoFactory.PROCESSOR_ARCHITECTURE_ARM:
                    return "ARM";
                default:
                    return "Unknown";
            }
        }

        internal static string GetRAM()
        {
            // Temporary until WinRT API is available
            return "512 MB";
        }

        internal static string GetWindowsEdition()
        {
            // Temporary until WinRT API is available
            return "Windows 10 Athens";
        }

        internal static string GetActivationString()
        {
            // Temporary until WinRT API is available
            return "Windows is activated";
        }

        internal string GetBoardName()
        {
			var loader = new Windows.ApplicationModel.Resources.ResourceLoader();
			// Hacky for now
			switch (systemInfo.wProcessorArchitecture)
            {
                case SystemInfoFactory.PROCESSOR_ARCHITECTURE_INTEL:
                    return loader.GetString("MBMName");
                case SystemInfoFactory.PROCESSOR_ARCHITECTURE_ARM:
                    return loader.GetString("Rpi2Name");
                default:
                    return loader.GetString("GenericBoardName");
            }
        }

        internal Uri GetBoardImageUri()
        {
            // Hacky for now
            switch (systemInfo.wProcessorArchitecture)
            {
                case SystemInfoFactory.PROCESSOR_ARCHITECTURE_INTEL:
                    return new Uri("ms-appx:///Assets/MBMBoard.png");
                case SystemInfoFactory.PROCESSOR_ARCHITECTURE_ARM:
                    return new Uri("ms-appx:///Assets/RaspberryPiBoard.png");
                default:
                    return new Uri("ms-appx:///Assets/GenericBoard.png");
            }
        }
    }
}
