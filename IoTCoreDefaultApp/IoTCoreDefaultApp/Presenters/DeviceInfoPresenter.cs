// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Linq;
using Windows.Networking;
using Windows.Networking.Connectivity;
using IoTCoreDefaultApp.Utils;

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

            switch (DeviceTypeInformation.Type)
            {
                case DeviceTypes.RPI2:
                    return loader.GetString("Rpi2Name");

                case DeviceTypes.MBM:
                    return loader.GetString("MBMName");

                default:
                    return loader.GetString("GenericBoardName");
            }
        }

        public static Uri GetBoardImageUri()
        {
            switch (DeviceTypeInformation.Type)
            {
                case DeviceTypes.RPI2:
                    return new Uri("ms-appx:///Assets/RaspberryPiBoard.png");

                case DeviceTypes.MBM:
                    return new Uri("ms-appx:///Assets/MBMBoard.png");

                default:
                    return new Uri("ms-appx:///Assets/GenericBoard.png");
            }
        }
    }
}
