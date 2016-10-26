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
    /// Hardware Version. Read the hardware version of the module.version of the module.  
    /// This command can be used to distinguish among different hardware platforms. 
    /// The upper byte returns a value that is unique to each module type.  The lower 
    /// byte indicates the hardware revision.
    /// </summary>
    class HV_Command : XBeeATCommand
    {
        private UInt16 m_HWVersion = 0;
        public UInt16 HWVersion
        {
            get { return m_HWVersion; }
        }
        // hardware version 
        public HV_Command()
        {
            m_atCommmand = new byte[] { (byte)'H', (byte)'V' };
            m_responseSize = 5;  // 'HV' 0x00 0x1E or 0x1A 0xXX (version)
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if (!IsResponseOK(buffer))
            {
                // response not OK => do nothing
                return false;
            }

            m_HWVersion = AdapterHelper.UInt16FromXbeeFrame(buffer, DATA_OFFSET_IN_RESPONSE_BUFFER);

            return true;
        }
    }
}
