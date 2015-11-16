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
    class OnOffCluster : ZclCluster
    {
        internal const UInt16 CLUSTER_ID = 0x0006;
        internal const string CLUSTER_NAME = "OnOff";

        internal const UInt16 ATTRIBUTE_ONOFF = 0x0000;
        internal const byte COMMAND_OFF = 0x00;
        internal const byte COMMAND_ON = 0x01;
        internal const byte COMMAND_TOGGLE = 0x02;

        // attribute and command specific to ZLL profile
        internal const UInt16 ATTRIBUTE_GLOBALSCENECONTROL_ZLL = 0x4000;
        internal const UInt16 ATTRIBUTE_ONTIME_ZLL = 0x4001;
        internal const UInt16 ATTRIBUTE_OFFWAITTIME_ZLL = 0x4002;
        internal const byte COMMAND_OFFWITHEFFECT_ZLL = 0x40;
        internal const byte COMMAND_ONWITHRECALLGLOBALSCENE_ZLL = 0x41;
        internal const byte COMMAND_ONWITHTIMEOFF_ZLL = 0x42;
        internal OnOffCluster(ZigBeeEndPoint endPoint, List<UInt16> supportedAttributes)
            : base(endPoint)
        {
            // set cluster name and cluster Id
            Name = CLUSTER_NAME;
            m_Id = CLUSTER_ID;

            // if supportedAttributes is null assume all attributes supported
            bool supportAllAttributesBydefault = false;
            if(supportedAttributes == null)
            {
                supportAllAttributesBydefault = true;
            }

            // OnOff attribute (read only)
            if(supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_ONOFF))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_ONOFF, "OnOff", ZclHelper.BOOLEAN_TYPE, true, false);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // IMPORTANT NOTE,  parameters MUST be added in parameter list of each command 
            // in the order specified in ZCL standard

            // Off command (no response required)
            var command = new ZclCommand(this, COMMAND_OFF, "Off", false);
            m_commandList.Add(command.Id, command);
            // On command (no response required)
            command = new ZclCommand(this, COMMAND_ON, "On", false);
            m_commandList.Add(command.Id, command);
            // Toggle command (no response required)
            command = new ZclCommand(this, COMMAND_TOGGLE, "Toggle", false);
            m_commandList.Add(command.Id, command);

            // add ZLL profile specific attributes and commands
            bool areZllAttributesSupported = false;
            if (supportAllAttributesBydefault ||
                supportedAttributes.Contains(ATTRIBUTE_GLOBALSCENECONTROL_ZLL))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_GLOBALSCENECONTROL_ZLL, "GlobalSceneControl", ZclHelper.BOOLEAN_TYPE, true, false);
                m_attributeList.Add(attrib.Id, attrib);
                areZllAttributesSupported = true;
            }
            if (supportAllAttributesBydefault ||
                supportedAttributes.Contains(ATTRIBUTE_ONTIME_ZLL))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_ONTIME_ZLL, "OnTime", ZclHelper.UINT16_TYPE, true, false);
                m_attributeList.Add(attrib.Id, attrib);
                areZllAttributesSupported = true;
            }
            if (supportAllAttributesBydefault ||
                supportedAttributes.Contains(ATTRIBUTE_OFFWAITTIME_ZLL))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_OFFWAITTIME_ZLL, "OnWaitTime", ZclHelper.UINT16_TYPE, true, false);
                m_attributeList.Add(attrib.Id, attrib);
                areZllAttributesSupported = true;
            }

            // add ZLL command if ZLL attributes are supported
            if (areZllAttributesSupported)
            {
                command = new ZclCommand(this, COMMAND_OFFWITHEFFECT_ZLL, "OffWithEffect", false);
                var parameter = new ZclValue(ZclHelper.UINT8_TYPE, "EffectIdentifier");
                command.AddInParam(parameter);
                parameter = new ZclValue(ZclHelper.UINT8_TYPE, "EffectVariant");
                command.AddInParam(parameter);
                m_commandList.Add(command.Id, command);

                command = new ZclCommand(this, COMMAND_ONWITHRECALLGLOBALSCENE_ZLL, "OnWithRecallGlobalScene", false);
                m_commandList.Add(command.Id, command);

                command = new ZclCommand(this, COMMAND_ONWITHTIMEOFF_ZLL, "OnWithTimeOff", false);
                parameter = new ZclValue(ZclHelper.UINT8_TYPE, "OnOffControl");
                command.AddInParam(parameter);
                parameter = new ZclValue(ZclHelper.UINT16_TYPE, "OnTime");
                command.AddInParam(parameter);
                parameter = new ZclValue(ZclHelper.UINT16_TYPE, "OffWaitTime");
                command.AddInParam(parameter);
                m_commandList.Add(command.Id, command);
            }

            // call parent class "post constructor"
            PostChildClassConstructor();
        }
    }
}
