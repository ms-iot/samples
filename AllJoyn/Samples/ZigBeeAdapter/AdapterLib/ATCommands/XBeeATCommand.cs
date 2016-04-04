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
using System.Threading;

namespace AdapterLib
{
    abstract class XBeeATCommand
    {
        protected byte[] m_atCommmand = null;
        protected static int VARIABLE_RESPONSE_SIZE = -1;
        protected int m_responseSize = 0;
        protected byte[] m_payload = null;

        protected static int DATA_OFFSET_IN_RESPONSE_BUFFER = 3;
        protected static byte RESPONSE_OK = 0;

        private AutoResetEvent m_responseParsedEv = new AutoResetEvent(false);

        public void SendNoWaitResponse(XBeeModule xbeeModule, bool sendLocal = true)
        {
            if (sendLocal)
            {
                xbeeModule.SendATCmdLocalNoResponse(this);
            }
            else
            {
                // not yet implemented
            }
        }
        public bool SendAndWaitResponse(XBeeModule xbeeModule, bool sendLocal = true)
        {
            if (sendLocal)
            {
                byte commandId = xbeeModule.SendATCmdLocal(this);
                if(!m_responseParsedEv.WaitOne(AdapterHelper.MAX_WAIT_TIMEOUT))
                {
                    xbeeModule.RemovePendingATCmd(commandId);
                    return false;
                }
                else
                {
                    return true;
                }
            }
            else
            {
                // not yet implemented
                return false;
            }
        }
        public byte[] ATCommand
        {
            get { return m_atCommmand;  }
        }
        public byte[] Payload
        {
            get { return m_payload; }
        }
        public void SignalResponseReceive()
        {
            m_responseParsedEv.Set();
        }
        public abstract bool ParseResponse(byte[] buffer);
        protected bool IsResponseOK(byte[] buffer)
        {
            // verify response size (if not variable)
            if (m_responseSize != buffer.Length &&
                m_responseSize != VARIABLE_RESPONSE_SIZE)
            {
                // wrong response size 
                return false;
            }
            
            // verify response correspond to AT command
            for (int index = 0; index < m_atCommmand.Length; index++)
            {
                if (buffer[index] != m_atCommmand[index])
                {
                    // AT command doesn't match 
                    return false;
                }
            }

            // verify response if OK
            if (buffer[m_atCommmand.Length] != RESPONSE_OK)
            {
                // response not OK 
                return false;
            }

            return true;
        }
    }
}
