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
    class ManagementLQI : ZigBeeCommand
    {
        public struct DeviceDescriptor
        {
            public UInt64 macAddress;
            public UInt16 networkAddress;
            public bool isEndDevice;
        };

        // cluster Id as specified in ZigBee standard
        private const UInt16 MANAGEMENT_LQI_CLUSTER_ID = 0x0031;
        private const UInt16 MANAGEMENT_LQI_RESPONSE_CLUSTER_ID = 0x8031;

        // from ZigBee standard
        private const int SIZEOF_NEIGHBOR_TABLE_ENTRY = 22;
        private const int NEIGHBOR_TABLE_ENTRY_MAC_ADDRESS_OFFSET = 8;
        private const int NEIGHBOR_TABLE_ENTRY_NETWORK_ADDRESS_OFFSET = (NEIGHBOR_TABLE_ENTRY_MAC_ADDRESS_OFFSET + AdapterHelper.MAC_ADDR_LENGTH);
        private const int NEIGHBOR_TABLE_ENTRY_DEVICE_TYPE_OFFSET = (NEIGHBOR_TABLE_ENTRY_NETWORK_ADDRESS_OFFSET + AdapterHelper.NETWORK_ADDRESS_LENGTH);
        private const byte NEIGHBOR_TABLE_ENTRY_DEVICE_TYPE_MASK = 0x11;
        private const byte END_DEVICE_TYPE = 0x02;

        private int m_nbOfNeighbors = 0;
        private List<DeviceDescriptor> m_neighborList = new List<DeviceDescriptor>();
        public List<DeviceDescriptor> NeighborList
        {
            get { return m_neighborList; }
        }

        public ManagementLQI()
        {
            m_isZdoCommand = true;
            m_clusterId = MANAGEMENT_LQI_CLUSTER_ID;
            m_responseClusterId = MANAGEMENT_LQI_RESPONSE_CLUSTER_ID;
            m_payload = new byte[1];
            m_payload[0] = 0;
        }

        public void GetNeighbors(XBeeModule xbeeModule, ZigBeeDevice device)
        {
            // clear list
            m_neighborList.Clear();

            // get 1st part of neighbor table
            m_payload[0] = 0;
            if (!SendCommand(xbeeModule, device))
            {
                m_neighborList.Clear();
                return;
            }

            // get rest of neighbor table
            // note that by Zigbee standard nb of entry in neighbor table can't exceed 1 byte
            m_payload[0] = Convert.ToByte(m_neighborList.Count);
            while (m_neighborList.Count < m_nbOfNeighbors)
            {
                if (!SendCommand(xbeeModule, device))
                {
                    m_neighborList.Clear();
                    return;
                }
            }
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if (!IsResponseOK(buffer))
            {
                return false;
            }

            // get number of neighbor
            int offset = GetZdoPayloadOffset();
            m_nbOfNeighbors = Convert.ToInt32(buffer[offset]);
            if (m_nbOfNeighbors == 0)
            {
                return true;
            }

            // get start index and count in neighbor list
            offset++;
            int startIndex = Convert.ToInt32(buffer[offset]);
            int neighborCount = Convert.ToInt32(buffer[offset+1]);

            // get neighbors from table
            offset += 2;
            for (int index = 0; index < neighborCount; index++)
            {
                // get mac address and network address from neighbor table entry
                DeviceDescriptor descriptor = new DeviceDescriptor();
                descriptor.networkAddress = AdapterHelper.UInt16FromZigBeeFrame(buffer, offset + NEIGHBOR_TABLE_ENTRY_NETWORK_ADDRESS_OFFSET);
                descriptor.macAddress = AdapterHelper.UInt64FromZigBeeFrame(buffer, offset + NEIGHBOR_TABLE_ENTRY_MAC_ADDRESS_OFFSET);
                byte deviceType = buffer[offset + NEIGHBOR_TABLE_ENTRY_DEVICE_TYPE_OFFSET];
                descriptor.isEndDevice = ((deviceType & NEIGHBOR_TABLE_ENTRY_DEVICE_TYPE_MASK) == END_DEVICE_TYPE);

                // add device descriptor in device list
                m_neighborList.Add(descriptor);

                // set offset of next entry
                offset += SIZEOF_NEIGHBOR_TABLE_ENTRY;
            }

            return true;
        }
    }
}
