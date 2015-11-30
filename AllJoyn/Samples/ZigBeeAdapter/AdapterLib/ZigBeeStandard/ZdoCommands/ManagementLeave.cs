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
    class ManagementLeave : ZigBeeCommand, IAdapterMethod
    {
        // public properties
        public string Name { get; private set; }
        public string Description { get; private set; }

        // command parameter lists
        //
        public IList<IAdapterValue> InputParams { get; set; }
        public IList<IAdapterValue> OutputParams { get; private set; }

        public int HResult { get; private set; }

        // cluster Id as specified in ZigBee standard
        private const UInt16 MANAGEMENT_LEAVE_CLUSTER_ID = 0x0034;
        private const UInt16 MANAGEMENT_LEAVE_RESPONSE_CLUSTER_ID = 0x8034;
        private const string EXPOSED_NAME = "DeviceManagementLeaveNetwork";
        private const string COMMAND_DESCRIPTION = "Instruct device to leave ZigBee network";

        private byte m_zigBeeStatus = ZdoHelper.ZDO_ERROR_SUCCESS;
        internal byte ZigBeeStatus
        {
            get { return m_zigBeeStatus; }
        }
        private ZigBeeDevice m_device = null;
        internal ZigBeeDevice Device
        {
            get { return m_device;  }
        }

        internal ManagementLeave(ZigBeeDevice device)
        {
            m_isZdoCommand = true;
            m_clusterId = MANAGEMENT_LEAVE_CLUSTER_ID;
            m_responseClusterId = MANAGEMENT_LEAVE_RESPONSE_CLUSTER_ID;
            m_device = device;

            Name = EXPOSED_NAME;
            Description = COMMAND_DESCRIPTION;
            InputParams = new List<IAdapterValue>();
            OutputParams = new List<IAdapterValue>();
        }

        internal void Send()
        {
            byte[] tempBuffer = AdapterHelper.ToZigBeeFrame(m_device.MacAddress);
            m_payload = new byte[tempBuffer.Length + 1];

            Array.Copy(tempBuffer, m_payload, tempBuffer.Length);
            m_payload[tempBuffer.Length] = 0;
            
            // send command 
            m_zigBeeStatus = ZdoHelper.ZDO_ERROR_SUCCESS;
            if (!SendCommand(m_device.Module, m_device))
            {
                m_zigBeeStatus = ZdoHelper.ZDO_ERROR_TIMEOUT;
            }
            HResult = ZdoHelper.ZigBeeStatusToHResult(m_zigBeeStatus);
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if (!IsResponseOK(buffer))
            {
                m_zigBeeStatus = ZdoHelper.APS_ILLEGAL_REQUEST;
                return false;
            }

            // get status
            int offset = GetZdoPayloadOffset() - ZIGBEE_STATUS_LENGTH;
            m_zigBeeStatus = buffer[offset];

            return true;
        }
    }
}
