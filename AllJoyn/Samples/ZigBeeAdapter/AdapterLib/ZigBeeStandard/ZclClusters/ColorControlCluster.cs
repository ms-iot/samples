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
    class ColorControlCluster : ZclCluster
    {
        internal const UInt16 CLUSTER_ID = 0x0300;
        internal const string CLUSTER_NAME = "ColorControl";

        internal const UInt16 ATTRIBUTE_CURRENTHUE = 0x0000;
        internal const UInt16 ATTRIBUTE_CURRENTSATURATION = 0x0001;
        internal const UInt16 ATTRIBUTE_REMAININGTIME = 0x0002;
        internal const UInt16 ATTRIBUTE_CURRENTX = 0x0003;
        internal const UInt16 ATTRIBUTE_CURRENTY = 0x0004;
        internal const UInt16 ATTRIBUTE_DRIFTCOMPENSATION = 0x0005;
        internal const UInt16 ATTRIBUTE_COMPENSATIONTEXT = 0x0006;
        internal const UInt16 ATTRIBUTE_COLORTEMPERATURE = 0x0007;
        internal const UInt16 ATTRIBUTE_COLORMODE = 0x0008;

        internal const UInt16 ATTRIBUTE_NUMBEROFPRIMARIES = 0x0010;
        internal const UInt16 ATTRIBUTE_PRIMARY1X = 0x0011;
        internal const UInt16 ATTRIBUTE_PRIMARY1Y = 0x0012;
        internal const UInt16 ATTRIBUTE_PRIMARY1INTENSITY = 0x0013;
        internal const UInt16 ATTRIBUTE_PRIMARY2X = 0x0015;
        internal const UInt16 ATTRIBUTE_PRIMARY2Y = 0x0016;
        internal const UInt16 ATTRIBUTE_PRIMARY2INTENSITY = 0x0017;
        internal const UInt16 ATTRIBUTE_PRIMARY3X = 0x0019;
        internal const UInt16 ATTRIBUTE_PRIMARY3Y = 0x001A;
        internal const UInt16 ATTRIBUTE_PRIMARY3INTENSITY = 0x001B;
        internal const UInt16 ATTRIBUTE_PRIMARY4X = 0x0020;
        internal const UInt16 ATTRIBUTE_PRIMARY4Y = 0x0021;
        internal const UInt16 ATTRIBUTE_PRIMARY4INTENSITY = 0x0022;
        internal const UInt16 ATTRIBUTE_PRIMARY5X = 0x0024;
        internal const UInt16 ATTRIBUTE_PRIMARY5Y = 0x0025;
        internal const UInt16 ATTRIBUTE_PRIMARY5INTENSITY = 0x0026;
        internal const UInt16 ATTRIBUTE_PRIMARY6X = 0x0028;
        internal const UInt16 ATTRIBUTE_PRIMARY6Y = 0x0029;
        internal const UInt16 ATTRIBUTE_PRIMARY6INTENSITY = 0x002A;

        internal const UInt16 ATTRIBUTE_WHITEPOINTX = 0x0030;
        internal const UInt16 ATTRIBUTE_WHITEPOINTY = 0x0031;
        internal const UInt16 ATTRIBUTE_COLORPOINTRX = 0x0032;
        internal const UInt16 ATTRIBUTE_COLORPOINTRY = 0x0033;
        internal const UInt16 ATTRIBUTE_COLORPOINTRINTENSITY = 0x0034;
        internal const UInt16 ATTRIBUTE_COLORPOINTGX = 0x0036;
        internal const UInt16 ATTRIBUTE_COLORPOINTGY = 0x0037;
        internal const UInt16 ATTRIBUTE_COLORPOINTGINTENSITY = 0x0038;
        internal const UInt16 ATTRIBUTE_COLORPOINTBX = 0x003A;
        internal const UInt16 ATTRIBUTE_COLORPOINTBY = 0x003B;
        internal const UInt16 ATTRIBUTE_COLORPOINTBINTENSITY = 0x003C;

        internal const byte COMMAND_MOVETOHUE = 0x00;
        internal const byte COMMAND_MOVEHUE = 0x01;
        internal const byte COMMAND_STEPHUE = 0x02;
        internal const byte COMMAND_MOVETOSATURATION = 0x03;
        internal const byte COMMAND_MOVESATURATION = 0x04;
        internal const byte COMMAND_STEPSATURATION = 0x05;
        internal const byte COMMAND_MOVETOHUEANDSATURATION = 0x06;
        internal const byte COMMAND_MOVETOCOLOR = 0x07;
        internal const byte COMMAND_MOVECOLOR = 0x08;
        internal const byte COMMAND_STEPCOLOR = 0x09;
        internal const byte COMMAND_MOVETOCOLORTEMPERATURE = 0x0A;

        internal ColorControlCluster(ZigBeeEndPoint endPoint, List<UInt16> supportedAttributes)
             : base(endPoint)
        {
            // set cluster name and cluster Id
            Name = CLUSTER_NAME;
            m_Id = CLUSTER_ID;

            // if supportedAttributes is null assume all attributes supported
            bool supportAllAttributesBydefault = false;
            if (supportedAttributes == null)
            {
                supportAllAttributesBydefault = true;
            }

            // current hue attribute (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_CURRENTHUE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_CURRENTHUE, "CurrentHue", ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // current saturation (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_CURRENTSATURATION))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_CURRENTSATURATION, "CurrentSaturation", ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // remaining time (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_REMAININGTIME))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_REMAININGTIME, "RemainingTime", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // current X (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_CURRENTX))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_CURRENTX, "CurrentX", ZclHelper.UINT16_TYPE, true, false);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // current Y (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_CURRENTY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_CURRENTY, "CurrentY", ZclHelper.UINT16_TYPE, true, false);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // drift compensation (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_DRIFTCOMPENSATION))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_DRIFTCOMPENSATION, "DriftCompensation", ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // compensation text (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COMPENSATIONTEXT))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COMPENSATIONTEXT, "CompensationText", ZclHelper.CHAR_STRING_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // color temperature (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COLORTEMPERATURE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COLORTEMPERATURE, "ColorTemperature", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // color mode (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COLORMODE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COLORMODE, "ColorMode", ZclHelper.ENUMERATION_8_BIT_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // number of primaries (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_NUMBEROFPRIMARIES))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_NUMBEROFPRIMARIES, "NumberOfPrimaries", ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 1 X (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY1X))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY1X, "Primary1X", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 1 Y (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY1Y))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY1Y, "Primary1Y", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 1 intensity (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY1INTENSITY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY1INTENSITY, "Primary1Intensity", ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 2 X (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY2X))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY2X, "Primary2X", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 2 Y (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY2Y))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY2Y, "Primary2Y", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 2 intensity (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY2INTENSITY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY2INTENSITY, "Primary2Intensity", ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 3 X (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY3X))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY3X, "Primary3X", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 3 Y (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY3Y))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY3Y, "Primary3Y", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 3 intensity (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY3INTENSITY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY3INTENSITY, "Primary3Intensity", ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 4 X (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY4X))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY4X, "Primary4X", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 4 Y (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY4Y))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY4Y, "Primary4Y", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 4 intensity (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY4INTENSITY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY4INTENSITY, "Primary4Intensity", ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 5 X (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY5X))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY5X, "Primary5X", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 5 Y (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY5Y))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY5Y, "Primary5Y", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 5 intensity (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY5INTENSITY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY5INTENSITY, "Primary5Intensity", ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 6 X (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY6X))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY6X, "Primary6X", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 6 Y (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY6Y))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY6Y, "Primary6Y", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // primary 6 intensity (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_PRIMARY6INTENSITY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_PRIMARY6INTENSITY, "Primary6Intensity", ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // white point X 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_WHITEPOINTX))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_WHITEPOINTX, "WhitePointX", ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // white point Y 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_WHITEPOINTY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_WHITEPOINTY, "WhitePointY", ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // color point R X 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COLORPOINTRX))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COLORPOINTRX, "ColorPointRX", ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // color point R Y 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COLORPOINTRY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COLORPOINTRY, "ColorPointRY", ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // color point R intensity 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COLORPOINTRINTENSITY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COLORPOINTRINTENSITY, "ColorPointRIntensity", ZclHelper.UINT8_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // color point G X 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COLORPOINTGX))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COLORPOINTGX, "ColorPointGX", ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // color point G Y 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COLORPOINTGY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COLORPOINTGY, "ColorPointGY", ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // color point G intensity 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COLORPOINTGINTENSITY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COLORPOINTGINTENSITY, "ColorPointGIntensity", ZclHelper.UINT8_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // color point B X 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COLORPOINTBX))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COLORPOINTBX, "ColorPointBX", ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // color point B Y 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COLORPOINTBY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COLORPOINTBY, "ColorPointBY", ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // color point B intensity 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_COLORPOINTBINTENSITY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_COLORPOINTBINTENSITY, "ColorPointBIntensity", ZclHelper.UINT8_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // move to hue command
            var command = new ZclCommand(this, COMMAND_MOVETOHUE, "MoveToHue", false);
            var parameter = new ZclValue(ZclHelper.UINT8_TYPE, "Hue");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, "Direction");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // move hue command
            command = new ZclCommand(this, COMMAND_MOVEHUE, "MoveHue", false);
            parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, "MoveMode");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "Rate");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // step hue command
            command = new ZclCommand(this, COMMAND_STEPHUE, "StepHue", false);
            parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, "StepMode");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "StepSize");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // move to saturation command
            command = new ZclCommand(this, COMMAND_MOVETOSATURATION, "MoveToSaturation", false);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "Saturation");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // move saturation command
            command = new ZclCommand(this, COMMAND_MOVESATURATION, "MoveSaturation", false);
            parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, "MoveMode");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "Rate");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // step saturation command
            command = new ZclCommand(this, COMMAND_STEPSATURATION, "StepSaturation", false);
            parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, "StepMode");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "StepSize");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // move hue and saturation command
            command = new ZclCommand(this, COMMAND_MOVETOHUEANDSATURATION, "MoveHueAndSaturation", false);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "Hue");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT8_TYPE, "Saturation");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // move to color command
            command = new ZclCommand(this, COMMAND_MOVETOCOLOR, "MoveToColor", false);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "ColorX");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "ColorY");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // move color command
            command = new ZclCommand(this, COMMAND_MOVECOLOR, "MoveColor", false);
            parameter = new ZclValue(ZclHelper.INT16_TYPE, "RateX");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.INT16_TYPE, "RateY");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // step color command
            command = new ZclCommand(this, COMMAND_STEPCOLOR, "StepColor", false);
            parameter = new ZclValue(ZclHelper.INT16_TYPE, "StepX");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.INT16_TYPE, "StepY");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // move to color temperature command
            command = new ZclCommand(this, COMMAND_MOVETOCOLORTEMPERATURE, "MoveToColorTemperature", false);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "ColorTemperature");
            command.AddInParam(parameter);
            parameter = new ZclValue(ZclHelper.UINT16_TYPE, "TransitionTime");
            command.AddInParam(parameter);
            m_commandList.Add(command.Id, command);

            // call parent class "post constructor"
            PostChildClassConstructor();
        }
    }
}