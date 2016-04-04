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
    class DeviceAnnce : ZigBeeCommand
    {
        /// note that device_annce ZDO command is received each time a new ZigBee devices 
        /// pops up on the network.
        /// This command will never be sent by ZigBee adapter

        // Singleton class 
        private static readonly DeviceAnnce instance = new DeviceAnnce();
        public static DeviceAnnce Instance
        {
            get { return instance; }
        }
        // cluster Id as specified in ZigBee standard
        private const UInt16 DEVICE_ANNCE_CLUSTER_ID = 0x0013;

        // notification action
        public Action<UInt16, UInt64, bool> OnReception { get; set; }

        private DeviceAnnce()
        {
            m_isZdoCommand = true;
            m_isNotification = true;
            m_clusterId = DEVICE_ANNCE_CLUSTER_ID;
            m_responseClusterId = DEVICE_ANNCE_CLUSTER_ID;
            m_payload = null;
            OnReception = null;
        }

        public override bool ParseResponse(byte[] buffer)
        {
            UInt16 networkAddress;
            UInt64 macAddress;

            if (!IsResponseOK(buffer))
            {
                return false;
            }

            // verify network address matches with payload (requested network address)
            int offset = GetZdoPayloadOffset();

            // skip 1st byte (ZDO command ref)
            offset++;

            // get network and Mac address
            networkAddress = AdapterHelper.UInt16FromZigBeeFrame(buffer, offset);
            offset += sizeof(UInt16);
            macAddress = AdapterHelper.UInt64FromZigBeeFrame(buffer, offset);

            // ignore capability (last byte)

            if (null != OnReception)
            {
                // execute notification callback asynchronously
                // can't determine is device is an end device from DeviceAnnce => assume it is one by default
                Task.Run(() => { OnReception(networkAddress, macAddress, true); } );
            }

            return true;
        }
    }
}
