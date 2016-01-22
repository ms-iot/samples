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
    /// Read the 16-bit network address of the module. A value of 0xFFFE means the
    /// module has not joined a ZigBee network 
    /// </summary>
    class MY_Command : XBeeATCommand
    {
        private UInt16 m_networkAddress = AdapterHelper.UNKNOWN_NETWORK_ADDRESS;
        public UInt16 NetworkAddress
        {
            get { return m_networkAddress; }
        }
        // hardware version 
        public MY_Command()
        {
            m_atCommmand = new byte[] { (byte)'M', (byte)'Y' };
            m_responseSize = 5;  // 'MY' 0x00 0xXX 0xXX (16 bits network address)
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if (!IsResponseOK(buffer))
            {
                // response not OK => do nothing
                return false;
            }

            m_networkAddress = AdapterHelper.UInt16FromXbeeFrame(buffer, DATA_OFFSET_IN_RESPONSE_BUFFER);

            return true;
        }
    }
}
