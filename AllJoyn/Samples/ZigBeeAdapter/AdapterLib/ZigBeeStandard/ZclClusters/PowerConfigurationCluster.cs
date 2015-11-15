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
    class PowerConfigurationCluster : ZclCluster
    {
        internal const UInt16 CLUSTER_ID = 0x0001;
        internal const string CLUSTER_NAME = "Power Configuration";
        internal const UInt16 ATTRIBUTE_MAINS_VOLTAGE = 0x0000;
        internal const UInt16 ATTRIBUTE_MAINS_FREQUENCY = 0x0001;
        internal const UInt16 ATTRIBUTE_MAINS_ALARM_MASK = 0x0010;
        internal const UInt16 ATTRIBUTE_MAINS_VOLTAGE_MIN_THRESHOLD = 0x0011;
        internal const UInt16 ATTRIBUTE_MAINS_VOLTAGE_MAX_THRESHOLD = 0x0012;
        internal const UInt16 ATTRIBUTE_MAINS_VOLTAGE_DWELL = 0x0013;
        internal const UInt16 ATTRIBUTE_BATTERY_VOLTAGE = 0x0020;
        internal const UInt16 ATTRIBUTE_BATTERY_MANUFACTURER = 0x0030;
        internal const UInt16 ATTRIBUTE_BATTERY_SIZE = 0x0031;
        internal const UInt16 ATTRIBUTE_BATTERY_AHR_RATING = 0x0032;
        internal const UInt16 ATTRIBUTE_BATTERY_QUANTITY = 0x0033;
        internal const UInt16 ATTRIBUTE_BATTERY_RATED_VOLTAGE = 0x0034;
        internal const UInt16 ATTRIBUTE_BATTERY_ALARM_MASK = 0x0035;
        internal const UInt16 ATTRIBUTE_BATTERY_VOLTAGE_MIN_THRESHOLD = 0x0036;

        internal PowerConfigurationCluster(ZigBeeEndPoint endPoint, List<UInt16> supportedAttributes)
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

            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_MAINS_VOLTAGE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_MAINS_VOLTAGE, "Mains Voltage", 
                    ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_MAINS_FREQUENCY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_MAINS_FREQUENCY, "Mains Frequency", 
                    ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_MAINS_ALARM_MASK))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_MAINS_ALARM_MASK, "Mains Alarm Mask",
                    ZclHelper.BITMAP_8_BIT_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_MAINS_VOLTAGE_MIN_THRESHOLD))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_MAINS_VOLTAGE_MIN_THRESHOLD, "Mains Voltage Min Threshold", ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_MAINS_VOLTAGE_MAX_THRESHOLD))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_MAINS_VOLTAGE_MAX_THRESHOLD, "Mains Voltage Max Threshold",
                    ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_MAINS_VOLTAGE_DWELL))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_MAINS_VOLTAGE_DWELL, "Mains Voltage Dwell Trip Point",
                    ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_BATTERY_VOLTAGE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_BATTERY_VOLTAGE, "Battery Voltage",
                    ZclHelper.UINT8_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_BATTERY_MANUFACTURER))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_BATTERY_MANUFACTURER, "Battery Manufacturer",
                    ZclHelper.CHAR_STRING_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_BATTERY_SIZE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_BATTERY_SIZE, "Battery Size",
                    ZclHelper.ENUMERATION_8_BIT_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_BATTERY_AHR_RATING))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_BATTERY_AHR_RATING, "Battery AHr Rating",
                    ZclHelper.UINT16_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_BATTERY_QUANTITY))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_BATTERY_QUANTITY, "Battery Quantity",
                    ZclHelper.UINT8_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_BATTERY_RATED_VOLTAGE))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_BATTERY_RATED_VOLTAGE, "Battery Rated Voltage",
                    ZclHelper.UINT8_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_BATTERY_ALARM_MASK))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_BATTERY_ALARM_MASK, "Battery Alarm Mask",
                    ZclHelper.BITMAP_8_BIT_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_BATTERY_VOLTAGE_MIN_THRESHOLD))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_BATTERY_VOLTAGE_MIN_THRESHOLD, "Battery Voltage Min Threshold",
                    ZclHelper.UINT8_TYPE, false, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            // call parent class "post constructor"
            PostChildClassConstructor();
        }
    }
}
