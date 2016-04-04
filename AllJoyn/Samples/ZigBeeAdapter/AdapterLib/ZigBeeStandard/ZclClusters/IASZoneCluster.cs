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
    class IASZoneCluster : ZclCluster
    {
        internal const UInt16 CLUSTER_ID = 0x0500;
        internal const string CLUSTER_NAME = "IASZone";

        internal const UInt16 ATTRIBUTE_ZONESTATE = 0x0000;
        internal const UInt16 ATTRIBUTE_ZONETYPE = 0x0001;
        internal const UInt16 ATTRIBUTE_ZONESTATUS = 0x0002;
        internal const UInt16 ATTRIBUTE_IASCIEADDRESS = 0x0010;

        // note that:
        //  - Zone Enroll Request and Zone Enroll Response aren't supported because
        //    they are part of ZigBee commissioning phase which isn't supported by ZigBee DSB
        internal const byte COMMAND_ZONESTATUSCHANGENOTIFICATION = 0x00;
        private const string PARAM_ZONESTATUS = "ZoneStatus";
        private const string PARAM_EXTENDEDSTATUS = "ExtendedStatus";

        internal IASZoneCluster(ZigBeeEndPoint endPoint, List<UInt16> supportedAttributes)
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

            // Zone State attribute (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_ZONESTATE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_ZONESTATE, "ZoneState", ZclHelper.ENUMERATION_8_BIT_TYPE, true);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // Zone Type attribute (read only)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_ZONETYPE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_ZONETYPE, "ZoneType", ZclHelper.ENUMERATION_16_BIT_TYPE, true);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // Zone Status attribute (read only, mandatory and "reportable")
            // note that ZCL standard indicates that this attribute isn't reportable, however it also specifies a server command: Zone status change notification command to  
            // notify change of zone status => ZclAttribute is then kind of reportable through a server command mechanism (change of value signal should consequently be available 
            // for that attribute)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_ZONESTATUS))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_ZONESTATUS, "ZoneStatus", ZclHelper.BITMAP_16_BIT_TYPE, true, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // IAS CIE address attribute (read/write)
            if (supportAllAttributesBydefault ||
               supportedAttributes.Contains(ATTRIBUTE_IASCIEADDRESS))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_IASCIEADDRESS, "IasCieAddress", ZclHelper.IEEE_ADDRESS_TYPE, false);
                m_attributeList.Add(attrib.Id, attrib);
            }

            // IMPORTANT NOTE,  parameters MUST be added in parameter list of each command 
            // in the order specified in ZCL standard
            
            // Zone status change notification command
            // note that this command is server command hence received by ZigBee DSB (as opposed to ZclCommand which are sent by ZigBee DSB)  
            var command = new ZclServerCommand(this, COMMAND_ZONESTATUSCHANGENOTIFICATION);
            var parameter = new ZclValue(ZclHelper.ENUMERATION_16_BIT_TYPE, PARAM_ZONESTATUS);
            command.AddParam(parameter);
            parameter = new ZclValue(ZclHelper.ENUMERATION_8_BIT_TYPE, PARAM_EXTENDEDSTATUS);
            command.AddParam(parameter);
            command.OnReception += ZoneStatusChangeNotification;
            m_serverCommandList.Add(command);

            // call parent class "post constructor"
            PostChildClassConstructor();
        }

        private void ZoneStatusChangeNotification(ZclServerCommand command)
        {
            ZclAttribute attribute = null;

            // update Zone status attribute and notify change
            if(!m_attributeList.TryGetValue(ATTRIBUTE_ZONESTATUS, out attribute))
            {
                attribute.Value.Data = command.GetParam(PARAM_ZONESTATUS).Data;
                SignalAttributeValueChange(attribute);
            }
        }
    }
}
