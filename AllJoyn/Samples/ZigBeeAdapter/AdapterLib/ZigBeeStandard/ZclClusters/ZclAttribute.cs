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

using BridgeRT;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AdapterLib
{
    class ZclAttribute : ZigBeeCommand, IAdapterAttribute
    {
        private const byte COMMAND_ID_READ_ATTRIBUTE = 0x00;
        private const byte COMMAND_ID_READ_ATTRIBUTE_RESPONSE = 0x01;
        private const byte COMMAND_ID_WRITE_ATTRIBUTE = 0x02;
        private const byte COMMAND_ID_WRITE_ATTRIBUTE_RESPONSE = 0x04;

        // BridgeRT interface 
        //---------------------
        public IAdapterValue Value { get; }
        public E_ACCESS_TYPE Access { get; set; }
        public IDictionary<string, string> Annotations { get; }
        public SignalBehavior COVBehavior { get; set; }

        // ZigBee Implementation
        //------------------------

        // attribute Id
        private UInt16 m_Id = 0;
        internal UInt16 Id
        {
            get { return m_Id; }
        }

        private bool m_isReadOnly = false;
        internal bool IsReadOnly
        {
            get { return m_isReadOnly; }
        }

        private bool m_isOptional = false;
        internal bool IsOptional
        {
            get { return m_isOptional; }
        }

        private byte m_status = ZclHelper.ZCL_ERROR_SUCCESS;
        internal byte Status
        {
            get { return m_status; }
        }

        // attribute value
        private byte m_zigBeeType = ZclHelper.UNKNOWN_TYPE;
        private ZclCluster m_cluster = null;

        internal ZclAttribute(ZclCluster cluster, UInt16 id, String name, byte zigBeeType, bool isReadOnly = false, bool isOptional = false, bool isReportable = false)
        {
            // save parent cluster
            m_cluster = cluster;

            // attribute specific
            m_Id = id;
            m_isReadOnly = isReadOnly;
            m_isOptional = isOptional;
            m_zigBeeType = zigBeeType;

            // ZigBee command specific
            m_isZdoCommand = false;
            int offset = ZclHelper.GetCommandIdOffset(ref m_zclHeader, 0);
            m_zclHeader[offset] = COMMAND_ID_READ_ATTRIBUTE;

            // BridgeRT specific 
            Value = new ZclValue(zigBeeType, name);
            Annotations = new Dictionary<string, string>();
            if (isReadOnly)
            {
                Access = E_ACCESS_TYPE.ACCESS_READ;
            }
            else
            {
                Access = E_ACCESS_TYPE.ACCESS_READWRITE;
            }
            if (isReportable)
            {
                COVBehavior = SignalBehavior.Always;
            }
            else
            {
                COVBehavior = SignalBehavior.Unspecified;
            }
        }

        internal bool Read(out object value)
        {
            // set value(s) to null
            Value.Data = null;
            value = null;
            m_status = ZclHelper.ZCL_ERROR_SUCCESS;

            // set cluster Id
            m_clusterId = m_cluster.Id;
            m_responseClusterId = m_cluster.Id;

            // set read command Id
            int offset = ZclHelper.GetCommandIdOffset(ref m_zclHeader, 0);
            m_zclHeader[offset] = COMMAND_ID_READ_ATTRIBUTE;

            // set payload
            byte[] tempBuffer = AdapterHelper.ToZigBeeFrame(m_Id);
            m_payload = new byte[m_zclHeader.Length + tempBuffer.Length];
            Array.Copy(m_zclHeader, m_payload, m_zclHeader.Length);
            Array.Copy(tempBuffer, 0, m_payload, m_zclHeader.Length, tempBuffer.Length);

            // send command
            m_responseRequired = true;
            if (!SendCommand(m_cluster.EndPoint.Device.Module, m_cluster.EndPoint.Device, m_cluster.EndPoint))
            {
                m_status = ZclHelper.ZCL_ERROR_TIMEOUT;
                return false;
            }
            
            // set out value
            if (Value.Data == null)
            {
                return false;
            }

            value = Value.Data;

            return true;
        }

        internal bool Write(object value)
        {
            // sanity check
            if(value == null)
            {
                return false;
            }

            // get byte buffer from value to set
            byte[] attributeValueBuffer = ZclHelper.ToByteBufferWithType(value, m_zigBeeType);
            if (attributeValueBuffer == null)
            {
                return false;
            }

            // set cluster Id
            m_clusterId = m_cluster.Id;
            m_responseClusterId = m_cluster.Id;

            // set write command Id
            int offset = ZclHelper.GetCommandIdOffset(ref m_zclHeader, 0);
            m_zclHeader[offset] = COMMAND_ID_WRITE_ATTRIBUTE;

            // set payload
            byte[] tempBuffer = AdapterHelper.ToZigBeeFrame(m_Id);
            m_payload = new byte[m_zclHeader.Length + tempBuffer.Length + attributeValueBuffer.Length];
            Array.Copy(m_zclHeader, m_payload, m_zclHeader.Length);
            Array.Copy(tempBuffer, 0, m_payload, m_zclHeader.Length, tempBuffer.Length);
            Array.Copy(attributeValueBuffer, 0, m_payload, m_zclHeader.Length + tempBuffer.Length, attributeValueBuffer.Length);

            // send command
            m_responseRequired = false;
            if (!SendCommand(m_cluster.EndPoint.Device.Module, m_cluster.EndPoint.Device, m_cluster.EndPoint))
            {
                return false;
            }

            return true;
        }
        public override bool ParseResponse(byte[] buffer)
        {
            m_status = ZclHelper.ZCL_ERROR_FAILURE;

            if (!IsResponseOK(buffer))
            {
                return false;
            }

            // verify command Id
            int offset = GetZclCommandIdOffset(ref buffer);
            if (COMMAND_ID_READ_ATTRIBUTE_RESPONSE != buffer[offset])
            {
                return false;
            }

            // parse ZCL payload
            offset = GetZclPayloadOffset(ref buffer);

            // check attribute Id
            UInt16 id = AdapterHelper.UInt16FromZigBeeFrame(buffer, offset);
            if (id != m_Id)
            {
                return false;
            }

            offset += sizeof(UInt16);

            // check status
            m_status = buffer[offset];
            if (m_status != ZclHelper.ZCL_ERROR_SUCCESS)
            {
                return false;
            }

            offset += ZIGBEE_STATUS_LENGTH;

            // set data
            bool retValue = false;
            object value = null;
            
            // from ZCL command payload 
            // - 1st byte indicates the type
            // - following byte(s) contain the value
            byte type = buffer[offset];
            offset += sizeof(byte);
            retValue = ZclHelper.GetValue(type, ref buffer, ref offset, out value);
            Value.Data = value;

            return retValue;
        }
    }
}
