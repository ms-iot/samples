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
    class SimpleDescriptor : ZigBeeCommand
    {
        // cluster Id as specified in ZigBee standard
        private const UInt16 SIMPLE_DESCRIPTOR_CLUSTER_ID = 0x0004;
        private const UInt16 SIMPLE_DESCRIPTOR_RESPONSE_CLUSTER_ID = 0x8004;

        private const int MIN_DESCRIPTOR_SIZE = 8;

        private List<UInt16> m_inClusterList = new List<UInt16>();
        public List<UInt16> InClusterList
        {
            get { return m_inClusterList; }
        }
        private List<UInt16> m_outClusterList = new List<UInt16>();
        public List<UInt16> OutClusterList
        {
            get { return m_outClusterList; }
        }

        private UInt16 m_profileId = 0;
        public UInt16 ProfileId
        {
            get { return m_profileId; }
        }
        private UInt16 m_deviceId = 0;
        public UInt16 DeviceId
        {
            get { return m_deviceId; }
        }
        public SimpleDescriptor()
        {
            m_isZdoCommand = true;
            m_clusterId = SIMPLE_DESCRIPTOR_CLUSTER_ID;
            m_responseClusterId = SIMPLE_DESCRIPTOR_RESPONSE_CLUSTER_ID;
            // payload = network address of device then end point Id
            m_payload = new byte[AdapterHelper.NETWORK_ADDRESS_LENGTH + sizeof(byte)];
        }
        public void GetDescriptor(XBeeModule xbeeModule, ZigBeeDevice device, byte endPointId)
        {
            // sanity check
            if (null == device)
            {
                return;
            }

            // reset (in case descriptor has already been set by previous GetDescriptor call)
            Reset();

            // set payload
            // (network address of device then end point Id)
            byte[] tempBytes = AdapterHelper.ToZigBeeFrame(device.NetworkAddress);
            Array.Copy(tempBytes, m_payload, tempBytes.Length);
            m_payload[tempBytes.Length] = endPointId;

            // send command
            if (!SendCommand(xbeeModule, device))
            {
                Reset();
            }
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if (!IsResponseOK(buffer))
            {
                return false;
            }

            // verify network address matches with requested network address (1st part of payload)
            int offset = GetZdoPayloadOffset();
            for (int index = 0; index < AdapterHelper.NETWORK_ADDRESS_LENGTH; index++)
            {
                if (m_payload[index] != buffer[offset + index])
                {
                    return false;
                }
            }

            // get descriptor size
            offset += AdapterHelper.NETWORK_ADDRESS_LENGTH;
            int descriptorSize = Convert.ToInt32(buffer[offset]);
            if (descriptorSize < MIN_DESCRIPTOR_SIZE)
            {
                // "empty" or no descriptor => no info
                return false;
            }

            // verify end point Id matches with requested end point Id (2nd part of payload)
            offset++;
            if (m_payload[AdapterHelper.NETWORK_ADDRESS_LENGTH] != buffer[offset])
            {
                return false;
            }

            // get profile Id and device Id
            offset++;
            m_profileId = AdapterHelper.UInt16FromZigBeeFrame(buffer, offset);
            offset += sizeof(UInt16);
            m_deviceId = AdapterHelper.UInt16FromZigBeeFrame(buffer, offset);
            offset += sizeof(UInt16);

            // skip next byte
            offset++;

            // get nb of in clusters (1 byte converted to int)
            int nbOfInClusters = Convert.ToInt32(buffer[offset]);

            // get id of in clusters
            offset++;
            for (int index = 0; index < nbOfInClusters; index++)
            {
                UInt16 tempVal = AdapterHelper.UInt16FromZigBeeFrame(buffer, offset);
                m_inClusterList.Add(tempVal);
                offset += sizeof(UInt16);
            }

            // get nb of out clusters (1 byte converted to int)
            int nbOfOutClusters = Convert.ToInt32(buffer[offset]);

            // get id of out clusters
            offset++;
            for (int index = 0; index < nbOfOutClusters; index++)
            {
                UInt16 tempVal = AdapterHelper.UInt16FromZigBeeFrame(buffer, offset);
                m_outClusterList.Add(tempVal);
                offset += sizeof(UInt16);
            }

            return true;
        }

        private void Reset()
        {
            m_inClusterList.Clear();
            m_outClusterList.Clear();
            m_profileId = 0;
            m_deviceId = 0;
        }
    }
}
