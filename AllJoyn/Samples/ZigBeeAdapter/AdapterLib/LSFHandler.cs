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
using BridgeRT;

namespace AdapterLib
{
    class LSFHandler : ILSFHandler
    {
        private const int OEM_LS_COLOR_TEMPERATURE_MIN = 2700;
        private const int OEM_LS_COLOR_TEMPERATURE_MAX = 9000;

        private const uint ERROR_SUCCESS = 0;

        // ILSFHandler
        //---------------
        public bool LampDetails_Color { get; }
        public uint LampDetails_ColorRenderingIndex { get; }
        public bool LampDetails_Dimmable { get; }
        public bool LampDetails_HasEffects { get; }
        public uint LampDetails_IncandescentEquivalent { get; }
        public uint LampDetails_LampBaseType { get; }
        public uint LampDetails_LampBeamAngle { get; }
        public string LampDetails_LampID { get; }
        public uint LampDetails_LampType { get; }
        public uint LampDetails_Make { get; }
        public uint LampDetails_MaxLumens { get; }
        public uint LampDetails_MaxTemperature { get; }
        public uint LampDetails_MaxVoltage { get; }
        public uint LampDetails_MinTemperature { get; }
        public uint LampDetails_MinVoltage { get; }
        public uint LampDetails_Model { get; }
        public uint LampDetails_Type { get; }
        public bool LampDetails_VariableColorTemp { get; }
        public uint LampDetails_Version { get; }
        public uint LampDetails_Wattage { get; }
        public uint LampParameters_BrightnessLumens { get; }
        public uint LampParameters_EnergyUsageMilliwatts { get; }
        public uint LampParameters_Version { get; }
        public uint[] LampService_LampFaults { get; } = { 0 };
        public uint LampService_LampServiceVersion { get; }
        public uint LampService_Version { get; }
        public uint LampState_ColorTemp { get; set; }
        public uint LampState_Hue { get; set; }
        public IAdapterSignal LampState_LampStateChanged { get; }
        public uint LampState_Saturation { get; set; }
        public uint LampState_Version { get; }

        // default on/off to off, will be updated later
        private bool m_onOffState = false;
        public bool LampState_OnOff
        {
            get
            {
                return GetOnOffStateFromDevice();
            }
            set
            {
                SetOnOffStateOnDevice(value);
            }
        }

        byte m_level = 0;
        public uint LampState_Brightness
        {
            get
            {
                return GetLevelFromDevice();
            }
            set
            {
                SetLevelOnDevice(value);
            }
        }
        public uint ClearLampFault(uint InLampFaultCode, out uint LampResponseCode, out uint OutLampFaultCode)
        {
            LampResponseCode = 0;
            OutLampFaultCode = InLampFaultCode;
            return ERROR_SUCCESS;
        }
        public uint LampState_ApplyPulseEffect(State FromState, State ToState, uint Period, uint Duration, uint NumPulses, ulong Timestamp, out uint LampResponseCode)
        {
            LampResponseCode = 0;
            return ERROR_SUCCESS;
        }
        public uint TransitionLampState(ulong Timestamp, State NewState, uint TransitionPeriod, out uint LampResponseCode)
        {
            LampResponseCode = 0;
            return ERROR_SUCCESS;
        }

        // ZigBee adapter related
        //------------------------

        private ZigBeeEndPoint m_endPoint = null;
        public LSFHandler(ZigBeeEndPoint endPoint, ZigBeeProfileLibrary.DeviceType type)
        {
            m_endPoint = endPoint;

            LampDetails_Version = 1;
            LampService_LampServiceVersion = 1;
            LampState_Version = 1;
            LampParameters_Version = 1;

            if(type == ZigBeeProfileLibrary.DeviceType.dimmableLight)
            {
                LampDetails_Dimmable = true;
            }
            else
            {
                LampDetails_Dimmable = false;
            }

            // only on/off and dimmable for now
            LampDetails_Color = false;
            LampDetails_VariableColorTemp = false;
            LampDetails_HasEffects = false;

            LampDetails_IncandescentEquivalent = 60;
            LampDetails_LampBaseType = (uint)BridgeRT.LSFLampBaseType.BASETYPE_E26;
            LampDetails_LampBeamAngle = 160;
            LampDetails_LampType = (uint)BridgeRT.LSFLampType.LAMPTYPE_A19;
            LampDetails_Make = (uint)BridgeRT.LSFLampMake.MAKE_OEM1;
            LampDetails_MaxLumens = 620;
            LampDetails_MaxTemperature = OEM_LS_COLOR_TEMPERATURE_MAX;
            LampDetails_MaxVoltage = 120;
            LampDetails_MinTemperature = OEM_LS_COLOR_TEMPERATURE_MIN;
            LampDetails_MinVoltage = 100;
            LampDetails_Model = 1;
            LampDetails_Type = (uint)BridgeRT.LSFDeviceType.TYPE_LAMP;
            LampDetails_VariableColorTemp = false;
            LampDetails_Wattage = 9;

            LampDetails_LampID = m_endPoint.SerialNumber;

            LampState_ColorTemp = 0;
            LampState_Hue = 0;
            LampState_Saturation = 0;

            // ZigBee light (on/off cluster) has no state change 
            LampState_LampStateChanged = null;
        }

        private bool GetOnOffStateFromDevice()
        {
            ZclAttribute attributeOnOff = null;
            System.Object value;

            // look for OnOff cluster of this end point and get the value of the on/off attribute
            var onOffCluster = m_endPoint.GetCluster(OnOffCluster.CLUSTER_ID);
            if (onOffCluster != null &&
                onOffCluster.InternalAttributeList.TryGetValue(OnOffCluster.ATTRIBUTE_ONOFF, out attributeOnOff) &&
                attributeOnOff.Read(out value) &&
                value is bool)
            {
                m_onOffState = (bool) value;
            }

            return m_onOffState;
        }

        private void SetOnOffStateOnDevice(bool newOnOffState)
        {
            ZclCommand command = null;

            // look for OnOff cluster of this end point
            var onOffCluster = m_endPoint.GetCluster(OnOffCluster.CLUSTER_ID);
            if (onOffCluster == null)
            {
                return;
            }

            // get on or off command depending on the on/off state to set
            if(newOnOffState)
            {
                onOffCluster.CommandList.TryGetValue(OnOffCluster.COMMAND_ON, out command);
            }
            else
            {
                onOffCluster.CommandList.TryGetValue(OnOffCluster.COMMAND_OFF, out command);
            }

            if (command == null)
            {
                return;
            }

            // send the command
            command.Send();
            m_onOffState = newOnOffState;
        }

        uint GetLevelFromDevice()
        {
            if(LampDetails_Dimmable)
            {
                ZclAttribute attributeLevel = null;
                System.Object value;

                // look for level control cluster of this end point and get the value of the current level attribute
                var levelControlCluster = m_endPoint.GetCluster(LevelControlCluster.CLUSTER_ID);
                if (levelControlCluster != null &&
                    levelControlCluster.InternalAttributeList.TryGetValue(LevelControlCluster.ATTRIBUTE_CURRENTLEVEL, out attributeLevel) &&
                    attributeLevel.Read(out value) &&
                    value is byte)
                {
                    m_level = (byte)value;
                }
            }

            return (uint)m_level;
        }

        void SetLevelOnDevice(uint newLevel)
        {
            ZclCommand command = null;

            // look for level control cluster and move to level command
            var levelControlCluster = m_endPoint.GetCluster(LevelControlCluster.CLUSTER_ID);
            if (levelControlCluster == null ||
                !levelControlCluster.CommandList.TryGetValue(LevelControlCluster.COMMAND_MOVETOLEVEL, out command))
            {
                return;
            }

            // get level parameter
            var parameter = command.GetInputParamByName(LevelControlCluster.LEVEL_PARAM);
            if(parameter == null)
            {
                return;
            }

            // level is a byte in ZCL and a uint32 in LSF 
            if(newLevel > LevelControlCluster.MAX_LEVEL)
            {
                newLevel = LevelControlCluster.MAX_LEVEL;
            }
            parameter.Data = (byte) newLevel;
            command.Send();
            m_level = (byte) newLevel;
        }
    }
}
