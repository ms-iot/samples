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
    class ZclDiscoverAttributes : ZigBeeCommand
    {
        private const byte COMMAND_ID_DISCOVER_ATTRIBUTES = 0x0C;
        private const byte COMMAND_ID_DISCOVER_ATTRIBUTES_RESPONSE = 0x0D;

        private const byte START_ATTRIBUTE_ID_LOW = 0x00;
        private const byte START_ATTRIBUTE_ID_HIGH = 0x00;
        private const byte MAX_ATTRIBUTES = 16;
        private static byte[] m_zclPayload = { START_ATTRIBUTE_ID_LOW, START_ATTRIBUTE_ID_HIGH, MAX_ATTRIBUTES };

        private byte m_status = ZclHelper.ZCL_ERROR_SUCCESS;
        private List<UInt16> m_attributeIdList = new List<UInt16>();
        private bool m_discoveryCompleted = false;
        public ZclDiscoverAttributes()
        {
            // ZigBee command specific 
            m_isZdoCommand = false;
            int offset = ZclHelper.GetCommandIdOffset(ref m_zclHeader, 0);
            m_zclHeader[offset] = COMMAND_ID_DISCOVER_ATTRIBUTES;
        }

        public bool GetListOfAttributeIds(XBeeModule xbeeModule, ZigBeeDevice device, ZigBeeEndPoint endPoint, UInt16 clusterId, out List<UInt16> attributeIdList)
        {
            m_status = ZclHelper.ZCL_ERROR_SUCCESS;

            // reset list of attribute Ids
            attributeIdList = null;
            m_attributeIdList.Clear();
            m_discoveryCompleted = false;

            // set cluster Id
            m_clusterId = clusterId;
            m_responseClusterId = clusterId;

            // add header and ZCL payload to command payload
            while (!m_discoveryCompleted)
            {
                m_payload = new byte[m_zclHeader.Length + m_zclPayload.Length];
                Array.Copy(m_zclHeader, m_payload, m_zclHeader.Length);
                // update start Id in Zcl payload if necessary
                if (m_attributeIdList.Count != 0)
                {
                    UInt16 lastId = m_attributeIdList.Last();
                    lastId++;
                    byte[] newStartId = AdapterHelper.ToZigBeeFrame(lastId);
                    Array.Copy(newStartId, 0, m_zclPayload, 0, newStartId.Length);
                }
                Array.Copy(m_zclPayload, 0, m_payload, m_zclHeader.Length, m_zclPayload.Length);

                // send command
                if (!SendCommand(xbeeModule, device, endPoint) ||
                    m_status != ZclHelper.ZCL_ERROR_SUCCESS)
                {
                    return false;
                }
            }

            // set out value
            attributeIdList = m_attributeIdList;

            return true;
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if (!IsResponseOK(buffer))
            {
                return false;
            }

            // verify command Id
            int offset = GetZclCommandIdOffset(ref buffer);
            if (COMMAND_ID_DISCOVER_ATTRIBUTES_RESPONSE != buffer[offset])
            {
                return false;
            }

            // parse ZCL payload
            offset = GetZclPayloadOffset(ref buffer);

            // set discovery complete status
            m_discoveryCompleted = Convert.ToBoolean(buffer[offset]);
            offset++;

            while (offset < buffer.Length)
            {
                UInt16 id = AdapterHelper.UInt16FromZigBeeFrame(buffer, offset);
                m_attributeIdList.Add(id);
                offset += sizeof(UInt16);
                // skip type
                offset++;
            }

            return true;
        }
    }
}
