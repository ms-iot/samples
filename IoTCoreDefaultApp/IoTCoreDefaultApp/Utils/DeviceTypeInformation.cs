// Copyright (c) Microsoft. All rights reserved.

using Windows.Security.ExchangeActiveSyncProvisioning;
using System;

namespace IoTCoreDefaultApp.Utils
{
    public enum DeviceTypes { RPI2, MBM, GenericBoard, Unknown };
    public static class DeviceTypeInformation
    {
        static DeviceTypes _type = DeviceTypes.Unknown;
        public static DeviceTypes Type
        {
            get
            {
                if (_type == DeviceTypes.Unknown)
                {
                    var deviceInfo = new EasClientDeviceInformation();
                    if (deviceInfo.SystemProductName.IndexOf("MinnowBoard", StringComparison.OrdinalIgnoreCase) >= 0)
                    {
                        _type = DeviceTypes.MBM; 
                    }
                    else if (deviceInfo.SystemProductName.IndexOf("Raspberry", StringComparison.OrdinalIgnoreCase) >= 0)
                    {
                        _type = DeviceTypes.RPI2;
                    }
                    else
                    {
                        _type = DeviceTypes.GenericBoard;
                    }
                }
                return _type;
            }
        }
    }
}
