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
    class AdapterHelper
    {
        public const string ADAPTER_NAME = "ZigBee Device System Bridge";
        public const string ADAPTER_DOMAIN = "com";
        public const string ADAPTER_VENDOR = "Microsoft";
        public const string ADAPTER_APPLICATION_GUID = "{04AD845B-AA1B-4900-97B8-A67BBEF32B1C}";
        public const string ADAPTER_DEFAULT_APPLICATION_NAME = "ZigBeeDeviceSystemBridge";
        public const string ADAPTER_DEFAULT_VERSION = "0.0.0.0";

        // time out value must take into account that 
        //  - end device might be slow to respond (slow CPU, low power...)
        //  - response time depends on the number of mesh network hops 
        //    (if the device is several router away getting its response will take time) 
        public const Int32 MAX_WAIT_TIMEOUT = (15 * 1000);  // in ms 

        public const int MAC_ADDR_LENGTH = sizeof(UInt64);
        public const UInt64 BROADCAST_MAC_ADDRESS = 0x000000000000FFFF;
        public const UInt64 UNKNOWN_MAC_ADDRESS = 0x0000000000000000;

        public const int NETWORK_ADDRESS_LENGTH = sizeof(UInt16);
        public const UInt16 UNKNOWN_NETWORK_ADDRESS = 0xFFFE;

        // HRESULT code
        public const int S_OK = 0;
        public const int E_FAIL = -2147467259;     // 0x80004005 

        public static UInt16 ReverseBytes(UInt16 value)
        {
            return (UInt16)((value & 0xFFU) << 8 | (value & 0xFF00U) >> 8);
        }

        public static UInt32 ReverseBytes(UInt32 value)
        {
            return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
                   (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
        }
        public static UInt64 ReverseBytes(UInt64 value)
        {
            return (value & 0x00000000000000FFUL) << 56 | (value & 0x000000000000FF00UL) << 40 |
                   (value & 0x0000000000FF0000UL) << 24 | (value & 0x00000000FF000000UL) << 8 |
                   (value & 0x000000FF00000000UL) >> 8 | (value & 0x0000FF0000000000UL) >> 24 |
                   (value & 0x00FF000000000000UL) >> 40 | (value & 0xFF00000000000000UL) >> 56;
        }
        public static int AddByteToBuffer(ref byte[] outBuffer, ref byte[] bufferToAdd, int offsetInOutBuffer)
        {
            int index = 0;
            for (index = 0; index < bufferToAdd.Length; index++)
            {
                outBuffer[offsetInOutBuffer + index] = bufferToAdd[index];
            }
            return (index + offsetInOutBuffer);
        }

        public static byte[] ToXbeeFrame(UInt16 value)
        {
            // UInt16 are big endian in XBee frames
            if (BitConverter.IsLittleEndian)
            {
                value = AdapterHelper.ReverseBytes(value);
            }
            return BitConverter.GetBytes(value);
        }
        public static byte[] ToXbeeFrame(UInt64 value)
        {
            // numbers are big endian in XBee frames
            if (BitConverter.IsLittleEndian)
            {
                value = AdapterHelper.ReverseBytes(value);
            }
            return BitConverter.GetBytes(value);
        }
        public static UInt16 UInt16FromXbeeFrame(byte[] buffer, int offset)
        {
            // numbers are big endian in XBee frames
            UInt16 value = BitConverter.ToUInt16(buffer, offset);
            if (BitConverter.IsLittleEndian)
            {
                value = AdapterHelper.ReverseBytes(value);
            }
            return value;
        }
        public static UInt64 UInt64FromXbeeFrame(byte[] buffer, int offset)
        {
            // numbers are big endian in XBee frames
            UInt64 value = BitConverter.ToUInt64(buffer, offset);
            if (BitConverter.IsLittleEndian)
            {
                value = AdapterHelper.ReverseBytes(value);
            }
            return value;
        }
        public static byte[] ToZigBeeFrame(Int16 value)
        {
            // swap bytes of Int16 is like swap bytes of UInt16 
            return ToZigBeeFrame((UInt16)value);
        }
        public static byte[] ToZigBeeFrame(UInt16 value)
        {
            // numbers are little endian in ZigBee frames (ZDO and ZCL) 
            if (!BitConverter.IsLittleEndian)
            {
                value = AdapterHelper.ReverseBytes(value);
            }
            return BitConverter.GetBytes(value);
        }
        public static byte[] ToZigBeeFrame(Int32 value)
        {
            // swap bytes of Int32 is like swap bytes of UInt32 
            return ToZigBeeFrame((UInt32)value);
        }
        public static byte[] ToZigBeeFrame(UInt32 value)
        {
            // numbers are little endian in ZigBee frames (ZDO and ZCL) 
            if (!BitConverter.IsLittleEndian)
            {
                value = AdapterHelper.ReverseBytes(value);
            }
            return BitConverter.GetBytes(value);
        }
        public static byte[] ToZigBeeFrame(Int64 value)
        {
            // swap bytes of Int64 is like swap bytes of UInt64 
            return ToZigBeeFrame((UInt64)value);
        }
        public static byte[] ToZigBeeFrame(UInt64 value)
        {
            // numbers are little endian in ZigBee frames (ZDO and ZCL) 
            if (!BitConverter.IsLittleEndian)
            {
                value = AdapterHelper.ReverseBytes(value);
            }
            return BitConverter.GetBytes(value);
        }
        public static Int16 Int16FromZigBeeFrame(byte[] buffer, int offset)
        {
            return (Int16) UInt16FromZigBeeFrame(buffer, offset);
        }
        public static UInt16 UInt16FromZigBeeFrame(byte[] buffer, int offset)
        {
            // numbers are little endian in ZigBee frames (ZDO and ZCL) 
            UInt16 value = BitConverter.ToUInt16(buffer, offset);
            if (!BitConverter.IsLittleEndian)
            {
                value = AdapterHelper.ReverseBytes(value);
            }
            return value;
        }
        public static Int32 Int32FromZigBeeFrame(byte[] buffer, int offset)
        {
            return (Int32)UInt32FromZigBeeFrame(buffer, offset);
        }
        public static UInt32 UInt32FromZigBeeFrame(byte[] buffer, int offset)
        {
            // numbers are little endian in ZigBee frames (ZDO and ZCL) 
            UInt32 value = BitConverter.ToUInt32(buffer, offset);
            if (!BitConverter.IsLittleEndian)
            {
                value = AdapterHelper.ReverseBytes(value);
            }
            return value;
        }
        public static UInt64 UInt64FromZigBeeFrame(byte[] buffer, int offset)
        {
            // numbers are little endian in ZigBee frames (ZDO and ZCL) 
            UInt64 value = BitConverter.ToUInt64(buffer, offset);
            if (!BitConverter.IsLittleEndian)
            {
                value = AdapterHelper.ReverseBytes(value);
            }
            return value;
        }
    }
}
