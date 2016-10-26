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
    class ZclReportAttributes : ZigBeeCommand
    {
        /// note that ZCL report attributes command is received each time a ZigBee device 
        /// report values of one or more of its attributes.
        /// This command will never be sent by ZigBee adapter

        // Singleton class 
        private static readonly ZclReportAttributes instance = new ZclReportAttributes();
        public static ZclReportAttributes Instance
        {
            get { return instance; }
        }

        private const byte COMMAND_ID_REPORT_ATTRIBUTES = 0x0A;

        // notification action
        public struct SOURCE_INFO
        {
            public UInt64 macAddress;
            public UInt16 networkAddress;
            public byte endpointId;
            public UInt16 clusterId;
        }
        public Action<SOURCE_INFO, UInt16, object> OnReception = null;
        private ZclReportAttributes()
        {
            // ZigBee command specific 
            m_isZdoCommand = false;
        }

        public override bool ParseResponse(byte[] buffer)
        {
            SOURCE_INFO sourceInfo = new SOURCE_INFO();

            // verify command Id
            int offset = GetZclCommandIdOffset(ref buffer);
            if (COMMAND_ID_REPORT_ATTRIBUTES != buffer[offset])
            {
                return false;
            }

            if (OnReception == null)
            {
                // can't signal anything 
                // however this is not an error => return true
                return true;
            }

            // get Mac address, network address, endpoint Id and cluster Id of source 
            offset = 0;
            sourceInfo.macAddress = AdapterHelper.UInt64FromXbeeFrame(buffer, offset);
            offset = AdapterHelper.MAC_ADDR_LENGTH;

            sourceInfo.networkAddress = AdapterHelper.UInt16FromXbeeFrame(buffer, offset);
            offset += AdapterHelper.NETWORK_ADDRESS_LENGTH;

            sourceInfo.endpointId = buffer[offset];
            offset++;

            // skip destination end point
            offset++;

            sourceInfo.clusterId = AdapterHelper.UInt16FromXbeeFrame(buffer, offset);

            // parse ZCL payload
            offset = GetZclPayloadOffset(ref buffer);
            while (offset < buffer.Length)
            {
                object value = null;

                // from ZCL report attribute payload 
                // - 1st byte is the attribute Id
                // - 2nd byte indicates the type
                // - following byte(s) contain the value
                UInt16 attributeId = AdapterHelper.UInt16FromZigBeeFrame(buffer, offset);
                offset += sizeof(UInt16);
                byte type = buffer[offset];
                offset += sizeof(byte);
                if(!ZclHelper.GetValue(type, ref buffer, ref offset, out value))
                {
                    // give up if attribute can't be retrieved
                    break;
                }

                // execute notification callback asynchronously
                Task.Run(() => { OnReception(sourceInfo, attributeId, value); });
            }

            return true;
        }
    }
}
