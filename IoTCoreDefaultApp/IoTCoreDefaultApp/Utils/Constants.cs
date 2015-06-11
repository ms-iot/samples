// Copyright (c) Microsoft. All rights reserved.


namespace IoTCoreDefaultApp
{
    public static class Constants
    {
        public static string HasDoneOOBEKey = "DefaultAppHasDoneOOBE";
        public static string HasDoneOOBEValue = "YES";
        public const string GUID_DEVINTERFACE_USB_DEVICE = "A5DCBF10-6530-11D2-901F-00C04FB951ED";
        public static string[] TutorialDocNames = {
            "GetStarted",
#if RPI || ALWAYS_SHOW_BLINKY
            "HelloBlinky",
#endif
            "GetConnected",
            "GetCoding"
        };
    }
}
