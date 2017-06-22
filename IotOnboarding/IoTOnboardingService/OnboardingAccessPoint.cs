// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Text;
using Windows.Devices.WiFiDirect;
using Windows.Security.Credentials;
using Windows.Storage.Streams;
using System.Linq;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace IoTOnboardingService
{
    struct TLVHeader
    {
        public byte m_type;
        public byte m_length;

        public byte[] GetBytes()
        {
            int size = Marshal.SizeOf(this);
            byte[] result = new byte[size];

            IntPtr ptr = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr(this, ptr, true);
            Marshal.Copy(ptr, result, 0, size);
            Marshal.FreeHGlobal(ptr);
            return result;
        }
    }


    class OnboardingAccessPoint
    {
        private WiFiDirectAdvertisementPublisher _publisher;
        private enum ETLVType : byte
        {
            FRIENDLY_NAME = 1,
            DEVICE_TYPE = 2,
            DEVICE_MANUFACTURER = 3,
            LANGUAGE_TAG = 4,
            DEVICE_ID = 5,
            PASSWORD = 6,
            UNCLASSIFIED_DEVICE_TYPE = 101,
        }

        private const int MAX_OUI_DATA_LENGTH = 253;
        private const int MAX_WPA2_PASSWORD_LENGTH = 64;
        private const int MIN_WPA2_PASSWORD_LENGTH = 8;
        private const byte OUI_TYPE = 0;
        private readonly byte[] MICROSOFT_OUI = new byte[]{ 0x84, 0x63, 0xd6 };
        private readonly string DEVICE_FRIENDLY_NAME = "IoT Device";
        
        private readonly byte[] DEVICE_TYPE = new byte[] { 0xFF, 0x00 };
        private readonly string DEVICE_MANUFACTURER_NAME = "Microsoft";
        private readonly string LANGUAGE_TAG = "en-US";
        private readonly string UNCLASSIFIED_DEVICE_TYPE = "Generic IoT Device";

        public OnboardingAccessPoint(string ssid, string password, Guid deviceID)
        {
            // Assert password length meets requirements if password is specified.
            // If password is null or empty, treat this as Open network.
            bool WPA2 = !String.IsNullOrEmpty(password);
            if (WPA2)
            {
                Debug.Assert(password.Length <= MAX_WPA2_PASSWORD_LENGTH);
                Debug.Assert(password.Length >= MIN_WPA2_PASSWORD_LENGTH);
            }

            // Begin advertising for legacy clients
            _publisher = new WiFiDirectAdvertisementPublisher();

            // Note: If this flag is not set, the legacy parameters are ignored
            _publisher.Advertisement.IsAutonomousGroupOwnerEnabled = true;

            // Setup Advertisement to use a custom SSID and WPA2 passphrase (null or empty implies Open)
            _publisher.Advertisement.LegacySettings.IsEnabled = true;
            _publisher.Advertisement.LegacySettings.Ssid = ssid;
            _publisher.Advertisement.LegacySettings.Passphrase = WPA2 ? new PasswordCredential { Password = password } : null;

            if (WPA2)
            { 
                // If using WPA2, configure this device's WiFi Access Point with the password needed to connect to it
                // and all other information needed by the Microsoft Onboaring Specification
                // After the device is onboarded to an end-user's network this will not be broadcast anymore.

                // The following code generates a Wifi Direct Information Element with the following format:
                //
                //
                //  1Byte   1Byte    3Bytes        1Byte      ... Several Bytes...
                // ---------------------------------------------------------------------------------------
                // | Type | Length |     OUI     | OUITYPE | Data [ETLV1],[ETLV2]...[ETLVn]              |
                // ---------------------------------------------------------------------------------------
                //   DD      num      84 63 d6       00      [01 0A 49 6f 54 20 44 65 76 69 63 65][02....]
                //          bytes                                  | I  o  T     D  e  v  i  c  e|
                //                                                 |   "Device Friendly Name"    |
                //
                // Where each "ETLV" is
                //
                //   1Byte       1Byte     ... Several Bytes...
                // ---------------------------------------------------------------------------------------
                // | ETLV Type | Length |   Data                                                         |
                // ---------------------------------------------------------------------------------------
                //
                IEnumerable< byte> ouiData =  MakeStringElement(ETLVType.FRIENDLY_NAME, DEVICE_FRIENDLY_NAME);
                ouiData = ouiData.Concat(MakeTLVElement(ETLVType.DEVICE_TYPE, DEVICE_TYPE));
                ouiData = ouiData.Concat(MakeStringElement(ETLVType.DEVICE_MANUFACTURER, DEVICE_MANUFACTURER_NAME));
                ouiData = ouiData.Concat(MakeStringElement(ETLVType.LANGUAGE_TAG, LANGUAGE_TAG));
                ouiData = ouiData.Concat(MakeGUIDElement(ETLVType.DEVICE_ID, deviceID));
                ouiData = ouiData.Concat(MakeStringElement(ETLVType.PASSWORD, password));
                ouiData = ouiData.Concat(MakeStringElement(ETLVType.UNCLASSIFIED_DEVICE_TYPE, UNCLASSIFIED_DEVICE_TYPE));

                // Create the IoT Device Wi-Fi Beacons Partner Information Element
                var infoElement = new WiFiDirectInformationElement();
                infoElement.OuiType = OUI_TYPE;
                infoElement.Oui = ByteArrayToBuffer(MICROSOFT_OUI);
                infoElement.Value = ByteArrayToBuffer(ouiData.ToArray());

                // The Maximum OUI Data Length was exceeded.  Trim ouiData String lengths 
                Debug.Assert(infoElement.Value.Length < MAX_OUI_DATA_LENGTH);

                // Add the custom Information Elements for publication
                var wifiInfoElements = new List<WiFiDirectInformationElement>();
                wifiInfoElements.Add(infoElement);
                _publisher.Advertisement.InformationElements = wifiInfoElements;
            }
        }       

        private byte[] LongToByteArrayNetworkOrder(Int32 inValue)
        {
            long networkOrderValue = System.Net.IPAddress.HostToNetworkOrder((long)inValue);
            networkOrderValue >>= 32;
            return BitConverter.GetBytes((Int32)networkOrderValue);
        }

        private byte[] ShortToByteArrayNetworkOrder(Int16 inValue)
        {
            Int16 networkOrderValue = System.Net.IPAddress.HostToNetworkOrder(inValue);
            return BitConverter.GetBytes(networkOrderValue);
        }
        private byte[] MakeGUIDElement(ETLVType type, Guid value)
        {
            var guidStringValue = value.ToString();
            var guidComponents = guidStringValue.Split('-');
            var part1 = LongToByteArrayNetworkOrder(Convert.ToInt32(guidComponents[0], 16));
            var part2 = ShortToByteArrayNetworkOrder(Convert.ToInt16(guidComponents[1], 16));
            var part3 = ShortToByteArrayNetworkOrder(Convert.ToInt16(guidComponents[2], 16));
            var part4 = ShortToByteArrayNetworkOrder(Convert.ToInt16(guidComponents[3], 16));

            IEnumerable<byte> result = part1.Concat(part2).Concat(part3).Concat(part4);
            var guidArray = value.ToByteArray().Skip(result.Count());
            result = result.Concat(guidArray);

            return MakeTLVElement(type, result.ToArray());
        }

        private byte[] MakeStringElement(ETLVType type, string value)
        {
            var asciiStringValue = Encoding.ASCII.GetBytes(value);
            return MakeTLVElement(type, asciiStringValue);
        }       

        private byte[] MakeTLVElement(ETLVType type, byte[] value)
        {
            TLVHeader header;
            header.m_type = Convert.ToByte(type);
            header.m_length = Convert.ToByte(value.Length);
            IEnumerable<byte> result = header.GetBytes().Concat(value);
            return result.ToArray();
        }     

        private IBuffer ByteArrayToBuffer(byte[] byteArray)
        {
            var stream = new InMemoryRandomAccessStream();
            var writer = new DataWriter(stream);
            writer.WriteBytes(byteArray);
            return writer.DetachBuffer();
        }

        public void Start()
        {
            if (_publisher.Status != WiFiDirectAdvertisementPublisherStatus.Started)
            {
                _publisher.Start();
            }
        }

        public void Stop()
        {
            if (_publisher.Status == WiFiDirectAdvertisementPublisherStatus.Started)
            {
                _publisher.Stop();
            }
        }
    }
}
