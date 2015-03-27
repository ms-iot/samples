// © Copyright(C) Microsoft. All rights reserved.

using System;
using System.Linq;
using Windows.Networking;
using Windows.Networking.Connectivity;

namespace AthensDefaultApp
{
    public class DeviceInfoPresenter
    {
        private SYSTEM_INFO systemInfo;

        public DeviceInfoPresenter()
        {
            systemInfo = SystemInfoFactory.GetNativeSystemInfo();
        }

        public static string GetDeviceName()
        {
            var hostname = NetworkInformation.GetHostNames()
                .FirstOrDefault(x => x.Type == HostNameType.DomainName);
            if (hostname != null)
            {
                return hostname.CanonicalName;
            }
            return "<no device name>";
        }

        public string GetBoardName()
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

        public Uri GetBoardImageUri()
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
