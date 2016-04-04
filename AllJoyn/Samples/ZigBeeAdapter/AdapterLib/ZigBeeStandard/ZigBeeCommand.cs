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
    abstract class ZigBeeCommand
    {
        // length OK status for ZigBee command (see ZigBee standard)
        // note that: 
        //  - status value depend on type of command: ZDO or ZCL.
        //    they are consequently defined in ZDO or ZCL helper class
        //  - OK status value is the same for ZDO and ZCL
        protected static int ZIGBEE_STATUS_LENGTH = sizeof(byte);

        public const byte UNSUPPORTED_ATTRIBUTE_STATUS = 0x86;

        // default value for ZigBee command (see ZigBee standard)
        private static byte[] SOURCE_ENDPOINT = { 0 };
        private static byte[] DESTINATION_ENDPOINT = { 0 };
        private static byte[] BROADCAST_RADIUS = { 0 };
        private static byte[] TRANSMIT_OPTIONS = { 0 };
        private static byte[] RECEIVE_OPTIONS = { 0 };
        private static byte[] PROFILE_ID = { 0, 0 };
        private static byte[] DEFAULT_CLUSTER_ID = { 0, 0 };

        // sequence number of ZigBee command
        private static byte[] m_sequenceNumber = { 0 };

        public static byte[] NextSequenceNumber()
        {
            byte[] previousSequenceNumber = new byte[m_sequenceNumber.Length];
            Array.Copy(m_sequenceNumber, previousSequenceNumber, m_sequenceNumber.Length);

            // Note that ZigBee ZCL standard indicates that next sequence number
            // after 0xff must be 0x00
            m_sequenceNumber[0]++;

            // return previous sequence number
            return previousSequenceNumber;
        }

        // ZCL header for read attribute
        // 1 - Frame control (1 byte)
        //     note: that manufacturer specific bit must be set to 0 in frame control
        //     => No Manufacturer code in ZCL header
        // 2 - Sequence number (1 byte)
        //     note: this byte will be dynamically set when command will be sent
        // 3 - Command Id (1 byte)
        //     note: this byte will be set by each specific ZCL command
        //
        // Note: ZCL header of responses can be different in size and contain the manufacturer code
        //       this depends on the device manufacturer
        protected byte[] m_zclHeader = { ZclHelper.FRAME_CONTROL_GENERAL_COMMAND, 0x00, 0x00 };

        private AutoResetEvent m_responseParsedEv = new AutoResetEvent(false);
        private ZigBeeDevice m_destination = null;
        private ZigBeeEndPoint m_endPoint = null;

        protected bool m_responseRequired = true;
        protected UInt16 m_clusterId = 0;
        protected UInt16 m_responseClusterId = 0;
        protected byte[] m_payload = null;
        protected bool m_isZdoCommand = true;
        protected bool m_isNotification = false;

        public bool IsZdoCommand
        {
            get { return m_isZdoCommand; }
        }
        public UInt16 ClusterId
        {
            get { return m_clusterId; }
        }

        public byte[] Payload
        {
            get { return m_payload; }
        }
        public void SignalResponseReceive()
        {
            m_responseParsedEv.Set();
        }

        public byte[] GetHeader()
        {
            byte[] destinationEndPoint = null;
            byte[] profileId = null;
            if (m_endPoint == null)
            {
                destinationEndPoint = DESTINATION_ENDPOINT;
                profileId = PROFILE_ID;
            }
            else
            {
                destinationEndPoint = new byte[sizeof(byte)];
                destinationEndPoint[0] = m_endPoint.Id ;
                profileId = AdapterHelper.ToXbeeFrame(m_endPoint.CommandProfileId);
            }
            // set destination (mac address and network address)
            byte[] macAddress = null;
            byte[] networkAddress = null;
            if (null != m_destination)
            {
                // send to specific device
                macAddress = AdapterHelper.ToXbeeFrame(m_destination.MacAddress);
                networkAddress = AdapterHelper.ToXbeeFrame(m_destination.NetworkAddress);
            }
            else
            {
                // broadcast
                macAddress = AdapterHelper.ToXbeeFrame(AdapterHelper.BROADCAST_MAC_ADDRESS);
                networkAddress = AdapterHelper.ToXbeeFrame(AdapterHelper.UNKNOWN_NETWORK_ADDRESS);
            }

            // set cluster ID
            byte[] clusterId = AdapterHelper.ToXbeeFrame(m_clusterId);

            // create and set header buffer
            int offset = 0;
            int headerSize = macAddress.Length + networkAddress.Length + SOURCE_ENDPOINT.Length + DESTINATION_ENDPOINT.Length +
                clusterId.Length + PROFILE_ID.Length + BROADCAST_RADIUS.Length + TRANSMIT_OPTIONS.Length ;
            byte[] header = new byte[headerSize];

            offset = AdapterHelper.AddByteToBuffer(ref header, ref macAddress, offset);
            offset = AdapterHelper.AddByteToBuffer(ref header, ref networkAddress, offset);
            offset = AdapterHelper.AddByteToBuffer(ref header, ref SOURCE_ENDPOINT, offset);
            offset = AdapterHelper.AddByteToBuffer(ref header, ref destinationEndPoint, offset);
            offset = AdapterHelper.AddByteToBuffer(ref header, ref clusterId, offset);
            offset = AdapterHelper.AddByteToBuffer(ref header, ref profileId, offset);
            offset = AdapterHelper.AddByteToBuffer(ref header, ref BROADCAST_RADIUS, offset);
            offset = AdapterHelper.AddByteToBuffer(ref header, ref TRANSMIT_OPTIONS, offset);

            return header;
        }

        protected bool SendCommand(XBeeModule xbeeModule, ZigBeeDevice destination, ZigBeeEndPoint endPoint = null)
        {
            bool retVal = false;

            // send ZigBee command then wait for response if required
            m_destination = destination;
            m_endPoint = endPoint;
            if (m_responseRequired)
            {
                byte sequenceNumber = xbeeModule.SendZigBeeCommand(this);
                retVal = m_responseParsedEv.WaitOne(AdapterHelper.MAX_WAIT_TIMEOUT);
                if(!retVal)
                {
                    // no response received in time
                    xbeeModule.RemovePendingZigBeeCmd(sequenceNumber);
                }
            }
            else
            {
                xbeeModule.SendZigBeeCommandNoResponse(this);
                retVal = true;
            }

            return retVal;
        }

        protected bool IsResponseOK(byte[] buffer)
        {
            int offset = 0;

            // verify cluster Id
            offset = AdapterHelper.MAC_ADDR_LENGTH;
            offset += AdapterHelper.NETWORK_ADDRESS_LENGTH;
            offset += SOURCE_ENDPOINT.Length;
            offset += DESTINATION_ENDPOINT.Length;

            UInt16 tempValue = AdapterHelper.UInt16FromXbeeFrame(buffer, offset);
            if (tempValue != m_responseClusterId)
            {
                return false;
            }

            if(!m_isZdoCommand)
            {
                // there is nothing more to control for ZCL
                //
                //  - cluster ID is hard to control (Phillips Hue light bulb respond with ZHA profile even though they are ZLL profile)  
                //  - status of ZCL command is inside ZCL payload and depends on each ZCL command
                return true;
            }

            // verify profile Id
            offset += DEFAULT_CLUSTER_ID.Length;
            byte[] profileId = PROFILE_ID;

            for (int index = 0; index < profileId.Length; index++)
            {
                if (buffer[index + offset] != profileId[index])
                {
                    return false;
                }
            }

            // verify status only for response to ZDO command (ZDO notification don't have status) 
            if (!m_isNotification)
            {
                offset = GetZdoPayloadOffset() - ZIGBEE_STATUS_LENGTH;
                if (buffer[offset] != ZdoHelper.ZDO_ERROR_SUCCESS)
                {
                    return false;
                }
            }

            return true;
        }

        private static int GetZigBeePayloadOffset()
        {
            // return offset of ZigBee frame payload in XBee response buffer
            // note:
            // - offset doesn't count header of all ZigBee module frames
            //   (start delimiter, response length and frame type)

            int offset = AdapterHelper.MAC_ADDR_LENGTH;
            offset += AdapterHelper.NETWORK_ADDRESS_LENGTH;
            offset += SOURCE_ENDPOINT.Length;
            offset += DESTINATION_ENDPOINT.Length;
            offset += DEFAULT_CLUSTER_ID.Length;
            offset += PROFILE_ID.Length;
            offset += RECEIVE_OPTIONS.Length;

            return offset;
        }

        public static int GetSequenceNumberOffestInZdoFrame()
        {
            // return ZDO sequence number offset in response buffer
            return GetZigBeePayloadOffset();
        }

        public static int GetSequenceNumberOffestInZclFrame()
        {
            // return ZCL sequence number offset in response buffer
            return GetZigBeePayloadOffset() + ZclHelper.ZCL_FRAME_CONTROL_LENGTH;
        }

        protected int GetZdoPayloadOffset()
        {
            // return offset of the "real" payload of the ZDO response in response buffer
            // note that ZDO notification have no sequence number and no status
            int offset = ZigBeeCommand.GetSequenceNumberOffestInZdoFrame();
            if (!m_isNotification)
            {
                offset += m_sequenceNumber.Length + ZIGBEE_STATUS_LENGTH;
            }
            return offset;
        }

        protected int GetZclCommandIdOffset(ref byte[] buffer)
        {
            int offset = GetZigBeePayloadOffset();
            offset += ZclHelper.GetCommandIdOffset(ref buffer, offset);
            return offset;
        }

        protected int GetZclPayloadOffset(ref byte[] buffer)
        {
            int offset = GetZigBeePayloadOffset();
            offset += ZclHelper.GetPayloadOffset(ref buffer, offset);
            return offset;
        }

        public abstract bool ParseResponse(byte[] buffer);
    }
}
