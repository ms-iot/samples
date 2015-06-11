// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Linq;
using Windows.Networking;
using Windows.Networking.Connectivity;

namespace IoTCoreDefaultApp
{
    public static class DeviceInfoPresenter
    {
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

        public static string GetBoardName()
        {
            var loader = new Windows.ApplicationModel.Resources.ResourceLoader();
#if MBM
            return loader.GetString("MBMName");
#elif RPI
            return loader.GetString("Rpi2Name");
#else
            return loader.GetString("GenericBoardName");
#endif
        }

        public static Uri GetBoardImageUri()
        {
#if MBM
            return new Uri("ms-appx:///Assets/MBMBoard.png");
#elif RPI
            return new Uri("ms-appx:///Assets/RaspberryPiBoard.png");
#else
            return new Uri("ms-appx:///Assets/GenericBoard.png");
#endif
        }
    }
}
