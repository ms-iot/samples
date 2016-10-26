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
    class DoorLockCluster : ZclCluster
    {
        internal const UInt16 CLUSTER_ID = 0x0101;
        internal const string CLUSTER_NAME = "DoorLock";

        internal const UInt16 ATTRIBUTE_LOCKSTATE = 0x0000;
        internal const UInt16 ATTRIBUTE_LOCKTYPE = 0x0001;
        internal const UInt16 ATTRIBUTE_ACTUATORENABLED = 0x0002;
        internal const UInt16 ATTRIBUTE_DOORSTATE = 0x0003;
        internal const UInt16 ATTRIBUTE_DOOROPENEVENTS = 0x0004;
        internal const UInt16 ATTRIBUTE_DOORCLOSEDEVENTS = 0x0005;
        internal const UInt16 ATTRIBUTE_OPENPERIOD = 0x0006;

        internal const byte COMMAND_LOCKDOOR = 0x00;
        internal const byte COMMAND_UNLOCKDOOR = 0x01;
        internal DoorLockCluster(ZigBeeEndPoint endPoint, List<UInt16> supportedAttributes)
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

            // Lock State attribute (read only, mandatory, reportable))
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_LOCKSTATE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_LOCKSTATE, "LockState", ZclHelper.ENUMERATION_8_BIT_TYPE, true, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // Lock Type attribute (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_LOCKTYPE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_LOCKTYPE, "LockType", ZclHelper.ENUMERATION_8_BIT_TYPE, true, false);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // Actuator Enabled attribute (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_ACTUATORENABLED))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_ACTUATORENABLED, "ActuatorEnabled", ZclHelper.BOOLEAN_TYPE, true, false);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // Door State attribute (read only, optional, reportable)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_DOORSTATE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_DOORSTATE, "DoorState", ZclHelper.ENUMERATION_8_BIT_TYPE, true, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // Door Open Events attribute 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_DOOROPENEVENTS))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_DOOROPENEVENTS, "DoorOpenEvents", ZclHelper.UINT32_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // Door Close Events attribute
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_DOORCLOSEDEVENTS))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_DOORCLOSEDEVENTS, "DoorCloseEvents", ZclHelper.UINT32_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // Open Period attribute 
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_OPENPERIOD))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_OPENPERIOD, "OpenPeriod", ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // IMPORTANT NOTE,  parameters MUST be added in parameter list of each command 
            // in the order specified in ZCL standard

            // Lock door command 
            var command = new ZclCommand(this, COMMAND_LOCKDOOR, "LockDoor", true);
            var parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, "Status");
            command.AddOutParam(parameter);
            m_commandList.Add(command.Id, command);

            // Unlock door command 
            command = new ZclCommand(this, COMMAND_UNLOCKDOOR, "UnlockDoor", true);
            parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, "Status");
            command.AddOutParam(parameter);
            m_commandList.Add(command.Id, command);

            // call parent class "post constructor"
            PostChildClassConstructor();
        }
    }
}
