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
    class ActiveEndPoints : ZigBeeCommand
    {
        // cluster Id as specified in ZigBee standard
        private const UInt16 ACTIVE_ENDPOINTS_CLUSTER_ID = 0x0005;
        private const UInt16 ACTIVE_ENDPOINTS_RESPONSE_CLUSTER_ID = 0x8005;

        private List<byte> m_endPointList = new List<byte>();
        public List<byte> EndPointList
        {
            get { return m_endPointList;  }
        }
        public ActiveEndPoints()
        {
            m_isZdoCommand = true;
            m_clusterId = ACTIVE_ENDPOINTS_CLUSTER_ID;
            m_responseClusterId = ACTIVE_ENDPOINTS_RESPONSE_CLUSTER_ID;
            m_payload = new byte[AdapterHelper.NETWORK_ADDRESS_LENGTH];
        }
        public void GetEndPoints(XBeeModule xbeeModule, ZigBeeDevice device)
        {
            // sanity check
            if (null == device)
            {
                return;
            }

            // reset end point list
            m_endPointList.Clear();

            // set payload
            m_payload = AdapterHelper.ToZigBeeFrame(device.NetworkAddress);

            // send command
            if (!SendCommand(xbeeModule, device))
            {
                m_endPointList.Clear();
                return;
            }
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if (!IsResponseOK(buffer))
            {
                return false;
            }

            // verify network address matches with payload (requested network address)
            int offset = GetZdoPayloadOffset();
            for (int index = 0; index < m_payload.Length; index++)
            {
                if (m_payload[index] != buffer[offset + index])
                {
                    return false;
                }
            }

            // get number of end points
            offset += m_payload.Length;
            int nbOfEndPoints = Convert.ToInt32(buffer[offset]);
            if (nbOfEndPoints == 0)
            {
                return true;
            }

            // get end points
            offset++;
            for (int index = 0; index < nbOfEndPoints; index++)
            {
                m_endPointList.Add(buffer[offset + index]);
            }

            return true;
        }
    }
}
