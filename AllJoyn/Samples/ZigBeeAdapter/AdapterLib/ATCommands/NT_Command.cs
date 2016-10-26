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
    /// Set/Read the node discovery timeout. When the network discovery (ND) command is 
    /// issued, the NT value is included in the transmission to provide all remote 
    /// devices with a response timeout. Remote devices wait a random time, less than 
    /// NT, before sending their response.
    /// </summary>
    class NT_Command : XBeeATCommand
    {
        private UInt16 m_nodeDiscoveryTimeOut = 0;
        public UInt16 NodeDiscoveryTimeOut
        {
            get { return m_nodeDiscoveryTimeOut; }
        }
        // hardware version 
        public NT_Command()
        {
            m_atCommmand = new byte[] { (byte)'N', (byte)'T' };
            m_responseSize = 5;  // 'NT' 0x00 0xXXXX (note discovery time-out)
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if (!IsResponseOK(buffer))
            {
                // response not OK => do nothing
                return false;
            }

            m_nodeDiscoveryTimeOut = AdapterHelper.UInt16FromXbeeFrame(buffer, DATA_OFFSET_IN_RESPONSE_BUFFER);

            return true;
        }
    }
}
