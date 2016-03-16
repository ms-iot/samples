// Copyright (c) Microsoft. All rights reserved.

using Windows.Security.ExchangeActiveSyncProvisioning;
using System;

namespace IoTCoreDefaultApp.Utils
{
    public enum DeviceTypes { RPI2, RPI3, MBM, DB410, GenericBoard, Unknown };
    public static class DeviceTypeInformation
    {
        static DeviceTypes _type = DeviceTypes.Unknown;
        static string _productName = null;

        static void Init()
        {
            if (_type == DeviceTypes.Unknown)
            {
                var deviceInfo = new EasClientDeviceInformation();
                _productName = deviceInfo.SystemProductName;
                if (deviceInfo.SystemProductName.IndexOf("MinnowBoard", StringComparison.OrdinalIgnoreCase) >= 0)
                {
                    _type = DeviceTypes.MBM;
                }
                else if (deviceInfo.SystemProductName.IndexOf("Raspberry", StringComparison.OrdinalIgnoreCase) >= 0)
                {
                    if (deviceInfo.SystemProductName.IndexOf("Pi 3", StringComparison.OrdinalIgnoreCase) >= 0)
                    {
                        _type = DeviceTypes.RPI3;
                    }
                    else
                    {
                        _type = DeviceTypes.RPI2;
                    }
                }
                else if (deviceInfo.SystemProductName == "SBC")
                {
                    _type = DeviceTypes.DB410;
                }
                else
                {
                    _type = DeviceTypes.GenericBoard;
                }
            }
        }

        public static bool IsRaspberryPi
        {
            get
            {
                return Type == DeviceTypes.RPI2 || Type == DeviceTypes.RPI3;
            }
        }

        public static DeviceTypes Type
        {
            get
            {
                Init();
                return _type;
            }
        }

        // this might return null
        public static string ProductName
        {
            get
            {
                Init();
                return _productName;
            }
        }
    }
}
