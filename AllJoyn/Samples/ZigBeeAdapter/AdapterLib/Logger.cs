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

using System.Diagnostics;

namespace AdapterLib
{
    class Logger
    {
        private enum LogLevel
        {
            None,
            Verbose
        }

        private const LogLevel m_logLevel = LogLevel.None;
        private const bool m_doTraceRxBuffer = false;
        private const bool m_doTraceDeviceDiscovery = false;
        public static bool IsVerbose()
        {
            return (m_logLevel == LogLevel.Verbose);
        }
        [ConditionalAttribute("DEBUG")]
        public static void TraceRxBuffer(byte[] buffer)
        {
            const byte EXPLICIT_RX_INDICATOR = 0x91;

            if (!m_doTraceRxBuffer)
            {
                return;
            }

            if (buffer?.Length >= 4)
            {
                if(buffer[3] == EXPLICIT_RX_INDICATOR)
                {
                    TraceExplicitRxIndicator(buffer);
                }
                else
                {
                    Debug.WriteLine("Rx buffer (length {0}):", buffer.Length);
                    TraceBytes(ref buffer, 0, buffer.Length);
                }
                Debug.WriteLine("\n");
            }
        }

        [ConditionalAttribute("DEBUG")]
        private static void TraceExplicitRxIndicator(byte[] buffer)
        {
            int offset = 0;

            Debug.WriteLine("ZDO/ZCL Rx buffer (length {0}):", buffer.Length);
            Debug.WriteLine("+++ beginning of Digi payload");
            TraceBytes(ref buffer, 0, 3);
            offset = 3;

            Debug.WriteLine("Explicit Rx Indicator");
            TraceBytes(ref buffer, offset, sizeof(byte));
            offset++;

            Debug.WriteLine("Source Mac address");
            TraceBytes(ref buffer, offset, AdapterHelper.MAC_ADDR_LENGTH);
            offset += AdapterHelper.MAC_ADDR_LENGTH;

            Debug.WriteLine("Source net address");
            TraceBytes(ref buffer, offset, AdapterHelper.NETWORK_ADDRESS_LENGTH);
            offset += AdapterHelper.NETWORK_ADDRESS_LENGTH;

            Debug.WriteLine("source Endpoint");
            TraceBytes(ref buffer, offset, sizeof(byte));
            offset++;

            Debug.WriteLine("destination Endpoint");
            TraceBytes(ref buffer, offset, sizeof(byte));
            offset++;

            Debug.WriteLine("Cluster ID");
            TraceBytes(ref buffer, offset, 2);
            offset += 2;

            Debug.WriteLine("Profile ID");
            TraceBytes(ref buffer, offset, 2);
            offset += 2;

            Debug.WriteLine("Receive option");
            TraceBytes(ref buffer, offset, sizeof(byte));
            offset++;

            Debug.WriteLine("--- end of Digi payload");
            Debug.WriteLine("+++ beginning of ZDO/ZCL payload");
            TraceBytes(ref buffer, offset, (buffer.Length - 1 - offset));
            Debug.WriteLine("--- end of ZDO/ZCL payload");

            Debug.WriteLine("Checksum");
            Debug.WriteLine("0x{0:X2}", buffer[buffer.Length - 1]);
        }

        [ConditionalAttribute("DEBUG")]
        private static void TraceBytes(ref byte[] buffer, int offset, int count)
        {
            var tempString = new StringBuilder("");
            for (int index = offset; index < offset + count; index++)
            {
                tempString.AppendFormat("0x{0:X2} ", buffer[index]);
            }
            Debug.WriteLine(tempString.ToString());
        }
        [ConditionalAttribute("DEBUG")]
        public static void TraceDeviceDetailed(UInt64 macAddress, byte endPointId, SimpleDescriptor descriptor)
        {
            if (!m_doTraceDeviceDiscovery ||
                descriptor == null)
            {
                return;
            }

            Debug.WriteLine("Device mac address 0x{0:X8}, EndPoint: 0x{1:X2}, Profile ID: 0x{2:X4}, Device ID: 0x{3:X4}", macAddress, endPointId, descriptor.ProfileId, descriptor.DeviceId);
        }
        [ConditionalAttribute("DEBUG")]
        public static void TraceDevice(UInt16 networkAddress, UInt64 macAddress)
        {
            if (!m_doTraceDeviceDiscovery)
            {
                return;
            }

            Debug.WriteLine("Device mac address 0x{0:X8}, Network address: 0x{1:X2}", macAddress, networkAddress);
        }
    }
}
