// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AdapterLib
{
    class ZigBeeProfileLibrary
    {
        // Singleton class 
        private static readonly ZigBeeProfileLibrary instance = new ZigBeeProfileLibrary();

        private const string UNKNOWN_PROFILE_NAME = "Unknown profile id";
        private const string UNKNOWN_DEVICE_NAME = "Unknown device type";

        // profile IDs
        private const UInt16 ZIGBEE_HOME_AUTOMATION_ID = 0x0104;
        private const UInt16 ZIGBEE_LIGHT_LINK_ID = 0xC05E;
        private const UInt16 ZIGBEE_SMART_ENERGY_ID = 0x0109;

        public enum DeviceType
        {
            other = 0,
            onOffLight,
            dimmableLight,
        }
        private struct DeviceInfo
        {
            public string m_name;
            public DeviceType m_type;

            public DeviceInfo(string name, DeviceType type)
            {
                m_name = name;
                m_type = type;
            }
        }

        private Dictionary<UInt16, string> m_profiles = new Dictionary<UInt16, string>();
        private Dictionary<UInt16, Dictionary<UInt16, DeviceInfo>> m_devicesPerProfile = new Dictionary<UInt16, Dictionary<UInt16, DeviceInfo>>();

        private Dictionary<UInt16, DeviceInfo> m_deviceTypesZigBeeLightLink = new Dictionary<UInt16, DeviceInfo>();
        private Dictionary<UInt16, DeviceInfo> m_deviceTypesZigBeeHomeAutomation = new Dictionary<UInt16, DeviceInfo>();
        private Dictionary<UInt16, DeviceInfo> m_deviceTypesZigBeeSmartEnergy = new Dictionary<UInt16, DeviceInfo>();

        private ZigBeeProfileLibrary()
        {
            m_profiles.Add(ZIGBEE_LIGHT_LINK_ID, "ZigBee Light Link");
            m_profiles.Add(ZIGBEE_HOME_AUTOMATION_ID, "ZigBee Home Automation");
            m_profiles.Add(ZIGBEE_SMART_ENERGY_ID, "ZigBee Smart Energy");
            m_devicesPerProfile.Add(ZIGBEE_LIGHT_LINK_ID, m_deviceTypesZigBeeLightLink);
            m_devicesPerProfile.Add(ZIGBEE_HOME_AUTOMATION_ID, m_deviceTypesZigBeeHomeAutomation);
            m_devicesPerProfile.Add(ZIGBEE_SMART_ENERGY_ID, m_deviceTypesZigBeeSmartEnergy);

            // supported ZigBee Light Link devices
            m_deviceTypesZigBeeLightLink.Add(0x0000, new DeviceInfo("On-off light", DeviceType.onOffLight));
            m_deviceTypesZigBeeLightLink.Add(0x0100, new DeviceInfo("Dimmable light", DeviceType.dimmableLight));
            m_deviceTypesZigBeeLightLink.Add(0x0200, new DeviceInfo("Color light", DeviceType.dimmableLight));
            m_deviceTypesZigBeeLightLink.Add(0x0210, new DeviceInfo("Extended color light", DeviceType.dimmableLight));
            m_deviceTypesZigBeeLightLink.Add(0x0220, new DeviceInfo("Color temperature light", DeviceType.dimmableLight));

            // supported ZigBee Home Automation devices
            //Generic Group
            m_deviceTypesZigBeeHomeAutomation.Add(0x0000, new DeviceInfo("On/Off Switch", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0001, new DeviceInfo("Level Control Switch", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0002, new DeviceInfo("On/Off Output", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0003, new DeviceInfo("Level Controllable Output", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0004, new DeviceInfo("Scene Selector", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0005, new DeviceInfo("Configuration Tool", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0006, new DeviceInfo("Remote Control", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0007, new DeviceInfo("Combined Interface", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0008, new DeviceInfo("Range Extender", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0009, new DeviceInfo("Mains Power Outlet", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x000A, new DeviceInfo("Door Lock", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x000B, new DeviceInfo("Door Lock Controller", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x000C, new DeviceInfo("Simple Sensor", DeviceType.other));
            //Lighting Group
            m_deviceTypesZigBeeHomeAutomation.Add(0x0100, new DeviceInfo("On-Off Light", DeviceType.onOffLight));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0101, new DeviceInfo("Dimmable Light", DeviceType.dimmableLight));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0102, new DeviceInfo("Color Dimmable Light", DeviceType.dimmableLight));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0103, new DeviceInfo("On/Off Light Switch", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0104, new DeviceInfo("Dimmer Switch", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0105, new DeviceInfo("Color Dimmer Switch", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0106, new DeviceInfo("Light Sensor", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0107, new DeviceInfo("Occupancy Sensor", DeviceType.other));
            //Closures Group
            m_deviceTypesZigBeeHomeAutomation.Add(0x0200, new DeviceInfo("Shade", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0201, new DeviceInfo("Shade Controller", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0202, new DeviceInfo("Window Covering Device", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0203, new DeviceInfo("Window Covering Controller", DeviceType.other));
            //HVAC Group
            m_deviceTypesZigBeeHomeAutomation.Add(0x0300, new DeviceInfo("Heating / Cooling Unit", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0301, new DeviceInfo("Thermostat", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0302, new DeviceInfo("Temperature Sensor", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0303, new DeviceInfo("Pump", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0304, new DeviceInfo("Pump Controller", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0305, new DeviceInfo("Pressure Sensor", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0306, new DeviceInfo("Flow Sensor", DeviceType.other));
            //IAS Group
            m_deviceTypesZigBeeHomeAutomation.Add(0x0400, new DeviceInfo("IAS Control and Indicating Equipment", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0401, new DeviceInfo("IAS Ancillerary Control Equipment", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0402, new DeviceInfo("IAS Zone", DeviceType.other));
            m_deviceTypesZigBeeHomeAutomation.Add(0x0403, new DeviceInfo("IAS Warning Device", DeviceType.other));

            // supported ZigBee Smart Energy devices
            m_deviceTypesZigBeeSmartEnergy.Add(0x0008, new DeviceInfo("Range Extender", DeviceType.other));
            m_deviceTypesZigBeeSmartEnergy.Add(0x0500, new DeviceInfo("Energy Service Interface", DeviceType.other));
            m_deviceTypesZigBeeSmartEnergy.Add(0x0501, new DeviceInfo("Metering Device", DeviceType.other));
            m_deviceTypesZigBeeSmartEnergy.Add(0x0502, new DeviceInfo("In-Premises Display", DeviceType.other));
            m_deviceTypesZigBeeSmartEnergy.Add(0x0503, new DeviceInfo("Programmable Communicating Thermostat", DeviceType.other));
            m_deviceTypesZigBeeSmartEnergy.Add(0x0504, new DeviceInfo("Load Control Device", DeviceType.other));
            m_deviceTypesZigBeeSmartEnergy.Add(0x0505, new DeviceInfo("Smart Appliance", DeviceType.other));
            m_deviceTypesZigBeeSmartEnergy.Add(0x0506, new DeviceInfo("Prepayment Terminal", DeviceType.other));

        }

        public bool GetProfileAndDeviceNames(UInt16 originalProfileId, UInt16 deviceId, out string profileName, out string deviceName, out UInt16 commandProfileId)
        {
            profileName = UNKNOWN_PROFILE_NAME;
            deviceName = UNKNOWN_DEVICE_NAME;

            // profile ID that will be used to send command might be different than the profile ID of the ZigBee device. 
            // For example, command will be sent using HA profile ID instead of ZLL profile ID for ZLL devices (as required by ZLL standard)
            if (originalProfileId == ZIGBEE_LIGHT_LINK_ID)
            {
                commandProfileId = ZIGBEE_HOME_AUTOMATION_ID;
            }
            else
            {
                commandProfileId = originalProfileId;
            }

            if (m_profiles.TryGetValue(originalProfileId, out profileName))
            {
                Dictionary<UInt16, DeviceInfo> devicesDictionary = null;
                if (m_devicesPerProfile.TryGetValue(originalProfileId, out devicesDictionary))
                {
                    DeviceInfo deviceInfo;
                    if (devicesDictionary.TryGetValue(deviceId, out deviceInfo))
                    {
                        deviceName = deviceInfo.m_name;
                        return true;
                    }
                    else
                    {
                        // set to unknown (TryGetValue "nullify" its out param)
                        deviceName = UNKNOWN_DEVICE_NAME;
                    }
                }
            }
            else
            {
                // set to unknown (TryGetValue "nullify" its out param)
                profileName = UNKNOWN_PROFILE_NAME;
            }

            return false;
        } 
        public bool IsLight(UInt16 profileId, UInt16 deviceId, out DeviceType deviceType)
        {
            // default device type to other
            deviceType = DeviceType.other;

            Dictionary<UInt16, DeviceInfo> devicesDictionary = null;
            if (m_devicesPerProfile.TryGetValue(profileId, out devicesDictionary))
            {
                DeviceInfo deviceInfo;
                if (devicesDictionary.TryGetValue(deviceId, out deviceInfo) &&
                    (deviceInfo.m_type == DeviceType.onOffLight ||
                     deviceInfo.m_type == DeviceType.dimmableLight))
                {
                    deviceType = deviceInfo.m_type;
                    return true;
                }
            }

            return false;
        }
        public static ZigBeeProfileLibrary Instance
        {
            get { return instance; }
        }
    }
}
