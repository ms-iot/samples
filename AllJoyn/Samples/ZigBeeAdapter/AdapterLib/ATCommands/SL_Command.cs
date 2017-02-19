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
    /// <summary>
    /// Read the low 32 bits of the module's unique 64-bit address.
    /// </summary>
    class SL_Command : XBeeATCommand
    {
        private byte[] m_macAddressLowerPart = new byte[AdapterHelper.MAC_ADDR_LENGTH / 2];
        public byte[] MacAddressLowerPart
        {
            get { return m_macAddressLowerPart; }
        }
        // hardware version 
        public SL_Command()
        {
            m_atCommmand = new byte[] { (byte)'S', (byte)'L' };
            m_responseSize = 7;  // 'SL' 0x00 0xXXXXXXXX (low part of serial number)
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if (!IsResponseOK(buffer))
            {
                // response not OK => do nothing
                return false;
            }

            Array.Copy(buffer, DATA_OFFSET_IN_RESPONSE_BUFFER, m_macAddressLowerPart, 0, m_macAddressLowerPart.Length);

            return true;
        }
    }
}
