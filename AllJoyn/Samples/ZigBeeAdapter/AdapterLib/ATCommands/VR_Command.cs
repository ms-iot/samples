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
    /// Read firmware version of the module. The firmware version returns 4 hexadecimal values (2 bytes)
    /// "ABCD".  Digits ABC are the main release number and D is the revision number from the main release.
    /// "B" is a variant designator.
    /// </summary>
    class VR_Command : XBeeATCommand
    {
        private UInt16 m_SWVersion = 0;
        public UInt16 SWVersion
        {
            get { return m_SWVersion; }
        }
        // hardware version 
        public VR_Command()
        {
            m_atCommmand = new byte[] { (byte)'V', (byte)'R' };
            m_responseSize = 5;  // 'VR' 0x00 0x1X 0xXX (version)
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if (!IsResponseOK(buffer))
            {
                // response not OK => do nothing
                return false;
            }

            m_SWVersion = AdapterHelper.UInt16FromXbeeFrame(buffer, DATA_OFFSET_IN_RESPONSE_BUFFER);

            return true;
        }
    }
}
