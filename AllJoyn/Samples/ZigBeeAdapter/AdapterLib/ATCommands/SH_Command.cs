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
    /// Read the high 32 bits of the module's unique 64-bit address
    /// </summary>
    class SH_Command : XBeeATCommand
    {
        private byte[] m_macAddressHightPart = new byte[AdapterHelper.MAC_ADDR_LENGTH / 2];
        public byte[] MacAddressHightPart
        {
            get { return m_macAddressHightPart; }
        }
        // hardware version 
        public SH_Command()
        {
            m_atCommmand = new byte[] { (byte)'S', (byte)'H' };
            m_responseSize = 7;  // 'SH' 0x00 0xXXXXXXXX (high part of serial number)
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if (!IsResponseOK(buffer))
            {
                // response not OK => do nothing
                return false;
            }

            Array.Copy(buffer, DATA_OFFSET_IN_RESPONSE_BUFFER, m_macAddressHightPart, 0, m_macAddressHightPart.Length);

            return true;
        }
    }
}
