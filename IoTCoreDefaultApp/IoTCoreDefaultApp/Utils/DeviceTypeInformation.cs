// Copyright (c) Microsoft. All rights reserved.

using Windows.Security.ExchangeActiveSyncProvisioning;

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
                    switch (deviceInfo.SystemProductName)
                    {
                        case "Raspberry Pi 2 Model B":
                            _type = DeviceTypes.RPI2;
                            break;

                        case "MinnowBoard MAX B3 PLATFORM":
                            _type = DeviceTypes.MBM;
                            break;

                        default:
                            _type = DeviceTypes.GenericBoard;
                            break;
                    }
                }
                return _type;
            }
        }
    }
}
