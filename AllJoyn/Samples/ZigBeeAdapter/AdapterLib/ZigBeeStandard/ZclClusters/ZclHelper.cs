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
    class ZclHelper
    {
        // ZCL error codes
        public const byte ZCL_ERROR_SUCCESS = 0x00;
        public const byte ZCL_ERROR_FAILURE = 0x01;
        public const byte ZCL_ERROR_NOT_AUTHORIZED = 0x7E;
        public const byte ZCL_ERROR_MALFORMED_COMMAND = 0x80;
        public const byte ZCL_ERROR_UNSUP_CLUSTER_COMMAND = 0x81;
        public const byte ZCL_ERROR_UNSUP_GENERAL_COMMAND = 0x82;
        public const byte ZCL_ERROR_INVALID_FIELD = 0x85;
        public const byte ZCL_ERROR_UNSUPPORTED_ATTRIBUTE = 0x86;
        public const byte ZCL_ERROR_INVALID_VALUE = 0x87;
        public const byte ZCL_ERROR_TIMEOUT = 0x94;

        // supported data types
        private const int ZCL_TYPE_CODE_LENGTH = sizeof(byte);

        public const byte UNKNOWN_TYPE = 0xFF;
        public const byte BOOLEAN_TYPE = 0x10;
        public const byte BITMAP_8_BIT_TYPE = 0x18;
        public const byte BITMAP_16_BIT_TYPE = 0x19;
        public const byte UINT8_TYPE = 0x20;
        public const byte UINT16_TYPE = 0x21;
        public const byte UINT32_TYPE = 0x23;
        public const byte INT8_TYPE = 0x28;
        public const byte INT16_TYPE = 0x29;
        public const byte INT32_TYPE = 0x2B;
        public const byte ENUMERATION_8_BIT_TYPE = 0x30;
        public const byte ENUMERATION_16_BIT_TYPE = 0x31;
        public const byte CHAR_STRING_TYPE = 0x42;
        public const byte IEEE_ADDRESS_TYPE = 0xF0;

        // ZCL payload
        public const byte FRAME_CONTROL_GENERAL_COMMAND = 0x00;
        public const byte FRAME_CONTROL_CLUSTER_SPECIFIC_COMMAND = 0x01;
        public const int ZCL_FRAME_CONTROL_LENGTH = sizeof(byte);
        public const int ZCL_COMMAND_ID_LENGTH = sizeof(byte);
        public const int ZCL_SEQUENCE_NUMBER_LENGTH = sizeof(byte);
        public const byte ZCL_FRAME_CONTROL_MANUFACTURER_CODE_BITMASK = 0x04;
        public const int ZCL_MANUFACTURER_CODE_LENGTH = sizeof(UInt16);

        public static bool CreateDefaultValue(byte zigBeeType, out object value)
        {
            value = null;

            switch (zigBeeType)
            {
                case BOOLEAN_TYPE:
                    value = false;
                    break;

                case CHAR_STRING_TYPE:
                    value = (String) "";
                    break;

                case INT8_TYPE:
                    value = (sbyte)0;
                    break;

                case INT16_TYPE:
                    value = (Int16)0;
                    break;

                case INT32_TYPE:
                    value = (Int32)0;
                    break;

                case UINT8_TYPE:
                    value = (byte) 0;
                    break;

                case UINT16_TYPE:
                    value = (UInt16) 0;
                    break;

                case UINT32_TYPE:
                    value = (UInt32)0;
                    break;

                case ENUMERATION_8_BIT_TYPE:
                    value = (byte)0;
                    break;

                case ENUMERATION_16_BIT_TYPE:
                    value = (UInt16)0;
                    break;

                case BITMAP_8_BIT_TYPE:
                    value = (byte)0;
                    break;

                case BITMAP_16_BIT_TYPE:
                    value = (UInt16)0;
                    break;

                case IEEE_ADDRESS_TYPE:
                    value = (UInt64)0;
                    break;
            }

            if (value != null)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        public static bool GetValue(byte type, ref byte[] buffer, ref int offset, out object value)
        {
            value = null;

            switch(type)
            {
                case BOOLEAN_TYPE:
                    {
                        if(buffer.Length >= offset + sizeof(bool))
                        {
                            bool tempVal = Convert.ToBoolean(buffer[offset]);
                            value = tempVal;
                            offset += sizeof(bool);
                        }
                    }
                    break;

                case CHAR_STRING_TYPE:
                    {
                        if(buffer.Length >= offset + sizeof(byte))
                        {
                            int length = Convert.ToInt32(buffer[offset]);
                            if (length != 0 &&
                                buffer.Length >= (offset + (length + 1) * sizeof(byte)))
                            {
                                offset += sizeof(byte);
                                String tempVal = Encoding.UTF8.GetString(buffer, offset, length);
                                value = tempVal;
                                offset += (length + 1) * sizeof(byte);
                            }
                        }
                    }
                    break;

                case INT8_TYPE:
                    {
                        if (buffer.Length >= offset + sizeof(sbyte))
                        {
                            sbyte tempVal = (sbyte)buffer[offset];
                            value = tempVal;
                            offset += sizeof(sbyte);
                        }
                    }
                    break;

                case ENUMERATION_8_BIT_TYPE:        // expected fall through
                case BITMAP_8_BIT_TYPE:             // expected fall through
                case UINT8_TYPE:
                    {
                        if (buffer.Length >= offset + sizeof(byte))
                        {
                            byte tempVal = buffer[offset];
                            value = tempVal;
                            offset += sizeof(byte);
                        }
                    }
                    break;

                case INT16_TYPE:
                    {
                        if (buffer.Length >= offset + sizeof(Int16))
                        {
                            value = AdapterHelper.Int16FromZigBeeFrame(buffer, offset);
                            offset += sizeof(Int16);
                        }
                    }
                    break;

                case ENUMERATION_16_BIT_TYPE:        // expected fall through
                case BITMAP_16_BIT_TYPE:             // expected fall through
                case UINT16_TYPE:
                    {
                        if (buffer.Length >= offset + sizeof(UInt16))
                        {
                            value = AdapterHelper.UInt16FromZigBeeFrame(buffer, offset);
                            offset += sizeof(UInt16);
                        }
                    }
                    break;

                case INT32_TYPE:
                    {
                        if (buffer.Length >= offset + sizeof(Int32))
                        {
                            value = AdapterHelper.Int32FromZigBeeFrame(buffer, offset);
                            offset += sizeof(Int32);
                        }
                    }
                    break;

                case UINT32_TYPE:
                    {
                        if (buffer.Length >= offset + sizeof(UInt32))
                        {
                            value = AdapterHelper.UInt32FromZigBeeFrame(buffer, offset);
                            offset += sizeof(UInt32);
                        }
                    }
                    break;

                case IEEE_ADDRESS_TYPE:
                    {
                        if (buffer.Length >= offset + sizeof(UInt64))
                        {
                            value = AdapterHelper.UInt64FromZigBeeFrame(buffer, offset);
                            offset += sizeof(UInt64);
                        }
                    }
                    break;
            }

            if (value != null)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        public static byte[] ToByteBuffer(object value)
        {
            byte[] buffer = null;
            if (value is byte ||
                value is sbyte)
            {
                buffer = new byte[sizeof(byte)];
                buffer[0] = (byte)value;
            }
            else if (value is Int16)
            {
                buffer = AdapterHelper.ToZigBeeFrame((Int16)value);
            }
            else if (value is UInt16)
            {
                buffer = AdapterHelper.ToZigBeeFrame((UInt16)value);
            }
            else if (value is Int32)
            {
                buffer = AdapterHelper.ToZigBeeFrame((Int32)value);
            }
            else if (value is UInt32)
            {
                buffer = AdapterHelper.ToZigBeeFrame((UInt32)value);
            }
            else if (value is Int64)
            {
                buffer = AdapterHelper.ToZigBeeFrame((Int64)value);
            }
            else if (value is UInt64)
            {
                buffer = AdapterHelper.ToZigBeeFrame((UInt64)value);
            }

            return buffer;
        }
        public static byte[] ToByteBufferWithType(object value, byte zigBeeType)
        {
            byte[] buffer = null;
            byte[] valueBuffer = null;

            // get buffer that correspond to the vale
            valueBuffer = ZclHelper.ToByteBuffer(value);
            if(valueBuffer == null)
            {
                return null;
            }

            // add ZigBee type to returned buffer
            buffer = new byte[ZclHelper.ZCL_TYPE_CODE_LENGTH + valueBuffer.Length];
            buffer[0] = zigBeeType;
            Array.Copy(valueBuffer, 0, buffer, ZclHelper.ZCL_TYPE_CODE_LENGTH, valueBuffer.Length);

            return buffer;
        }
        public static int GetCommandIdOffset(ref byte[] buffer, int startIndex)
        {
            int offset = ZclHelper.ZCL_FRAME_CONTROL_LENGTH + ZCL_SEQUENCE_NUMBER_LENGTH;
            if ((buffer[startIndex] & ZclHelper.ZCL_FRAME_CONTROL_MANUFACTURER_CODE_BITMASK) != 0)
            {
                offset += ZclHelper.ZCL_MANUFACTURER_CODE_LENGTH;
            }

            return offset;
        }

        public static int GetPayloadOffset(ref byte[] buffer, int startIndex)
        {
            int offset = ZclHelper.ZCL_FRAME_CONTROL_LENGTH + ZCL_SEQUENCE_NUMBER_LENGTH + ZclHelper.ZCL_COMMAND_ID_LENGTH;
            if ((buffer[startIndex] & ZclHelper.ZCL_FRAME_CONTROL_MANUFACTURER_CODE_BITMASK) != 0)
            {
                offset += ZclHelper.ZCL_MANUFACTURER_CODE_LENGTH;
            }

            return offset;
        }

        public static void SetClusterSpecificHeader(ref byte[] buffer, int startIndex)
        {
            buffer[startIndex] = FRAME_CONTROL_CLUSTER_SPECIFIC_COMMAND;
        }

        public static int ZigBeeStatusToHResult(byte zigBeeError)
        {
            int hresult = AdapterHelper.E_FAIL;

            switch (zigBeeError)
            {
                case ZCL_ERROR_SUCCESS:
                    hresult = AdapterHelper.S_OK;
                    break;

                case ZCL_ERROR_NOT_AUTHORIZED:
                    {
                        System.UnauthorizedAccessException ex = new UnauthorizedAccessException();
                        hresult = ex.HResult;
                    }
                    break;

                case ZCL_ERROR_INVALID_FIELD:       // fall through OK
                case ZCL_ERROR_INVALID_VALUE:       // fall through OK
                case ZCL_ERROR_MALFORMED_COMMAND:
                    {
                        System.ArgumentException ex = new ArgumentException();
                        hresult = ex.HResult;
                    }
                    break;

                case ZCL_ERROR_UNSUP_CLUSTER_COMMAND:       // fall through OK
                case ZCL_ERROR_UNSUP_GENERAL_COMMAND:       // fall through OK
                case ZCL_ERROR_UNSUPPORTED_ATTRIBUTE:
                    {
                        System.NotSupportedException ex = new NotSupportedException();
                        hresult = ex.HResult;
                    }
                    break;

                case ZCL_ERROR_TIMEOUT:
                    {
                        System.TimeoutException ex = new TimeoutException();
                        hresult = ex.HResult;
                    }
                    break;
            }
            return hresult;
        }
    }
}
