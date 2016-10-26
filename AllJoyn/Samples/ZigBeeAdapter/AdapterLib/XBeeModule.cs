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

using System.Diagnostics;

namespace AdapterLib
{
    class XBeeModule
    {
        // XBEE related constant
        //----------------------------
        private const int CHECKSUM_LENGTH = 1;
        private byte[] COMMAND_PREFIX = new byte[] { 0x7E };

        // XBee AT command
        private byte[] AT_COMMAND_LOCAL_SEND = new byte[] { 0x08 };
        private byte[] AT_COMMAND_LOCAL_RECEIVE = new byte[] { 0x88 };
        private byte[] AT_COMMAND_REMOTE_SEND = new byte[] { 0x17 };
        private byte[] AT_COMMAND_REMOTE_RECEIVE = new byte[] { 0x97 };

        // XBee ZDO or ZCL command
        private byte[] EXPLICIT_ADDRESSING_FRAME = new byte[] { 0x11 };
        private byte[] EXPLICIT_RX_INDICATOR = new byte[] { 0x91 };

        // response to command
        private byte[] XBEE_COMMAND_OK = new byte[] { 0x00 };

        // XBee USB dongle
        private SerialController m_serialController = new SerialController();

        // command ID
        private byte[] m_commandId = { 0 };

        // list of response or notification awaited from XBee
        // -> key is the command Id for responses or the AT command for notification, Value the received buffer
        private Dictionary<byte, XBeeATCommand> m_pendingATCmdList = new Dictionary<byte, XBeeATCommand>();
        private Dictionary<byte, ZigBeeCommand> m_pendingZigBeeCmdList = new Dictionary<byte, ZigBeeCommand>();
        private Dictionary<UInt16, ZigBeeCommand> m_ZigBeeNotificationList = new Dictionary<UInt16, ZigBeeCommand>();

        private object m_locker = new object();
        private byte[] m_currentResponse = null;

        // module information
        private UInt16 m_HWVersion = 0;
        public UInt16 HWVersion
        {
            get { return m_HWVersion; }
        }

        private UInt16 m_SWVersion = 0;
        public UInt16 SWVersion
        {
            get { return m_SWVersion; }
        }

        public void Initialize(out ZigBeeDevice adapter)
        {
            // initialize communication with XBee module
            try
            {
                m_serialController.InitializeAsync().Wait();
            }
            catch (Exception ex)
            {
                Debug.WriteLine("{0}.{1}: An exception occurred:\n    {2}", this.GetType().Name, nameof(this.Initialize), ex);
                throw;
            }
            m_serialController.OnByteReception += GetBytesFromModule;

            // make sure there is a valid command Id
            NextCommandId();

            // get information about XBee module
            //-----------------------------------

            // get hardware version
            HV_Command hvCommand = new HV_Command();
            if (hvCommand.SendAndWaitResponse(this))
            {
                m_HWVersion = hvCommand.HWVersion;
            }

            // get software version
            VR_Command vrCommand = new VR_Command();
            if (vrCommand.SendAndWaitResponse(this))
            {
                m_SWVersion = vrCommand.SWVersion;
            }

            // get MAC address
            byte[] macAddress = null;
            SL_Command slCommand = new SL_Command();
            SH_Command shCommand = new SH_Command();
            if (slCommand.SendAndWaitResponse(this) &&
               shCommand.SendAndWaitResponse(this))
            {
                macAddress = new byte[shCommand.MacAddressHightPart.Length + slCommand.MacAddressLowerPart.Length];
                Array.Copy(shCommand.MacAddressHightPart, 0, macAddress, 0, shCommand.MacAddressHightPart.Length);
                Array.Copy(slCommand.MacAddressLowerPart, 0, macAddress, shCommand.MacAddressHightPart.Length, slCommand.MacAddressLowerPart.Length);
            }

            MY_Command myCommand = new MY_Command();
            myCommand.SendAndWaitResponse(this);

            // set RX indicator mode 
            // note this API mode is necessary to get response to ZDO and ZCL commands
            AO_Command aoCommand = new AO_Command();
            aoCommand.SetRxIndicatorMode(this);

            adapter = new ZigBeeDevice(myCommand.NetworkAddress, AdapterHelper.UInt64FromXbeeFrame(macAddress, 0), false);
        }

        public void Shutdown()
        {
            if (null != m_serialController)
            {
                m_serialController.Shutdown();
            }
        }

        public void AddXZibBeeNotification(ZigBeeCommand command)
        {
            // only add one notification per command
            if (!m_ZigBeeNotificationList.ContainsKey(command.ClusterId))
            {
                m_ZigBeeNotificationList.Add(command.ClusterId, command);
            }
        }

        public byte SendZigBeeCommand(ZigBeeCommand command)
        {
            // set ZigBee command sequence number
            byte[] tempSequenceNumber = ZigBeeCommand.NextSequenceNumber();
            SendZigBeeCommandInternal(command, true, tempSequenceNumber);
            return tempSequenceNumber[0];
        }
        public void SendZigBeeCommandNoResponse(ZigBeeCommand command)
        {
            // set ZigBee command sequence number
            byte[] tempSequenceNumber = ZigBeeCommand.NextSequenceNumber();
            SendZigBeeCommandInternal(command, false, tempSequenceNumber);
        }
        private async void SendZigBeeCommandInternal(ZigBeeCommand command, bool responseRequired, byte[] tempSequenceNumber)
        {
            int headerSize = 0;
            int commandSize = 0;
            int nbOfBytesToSend = 0;
            int offset = 0;

            // allocate send buffer
            headerSize = COMMAND_PREFIX.Length + sizeof(UInt16);
            byte[] zigBeeCommandHeader = command.GetHeader();
            commandSize = EXPLICIT_ADDRESSING_FRAME.Length + m_commandId.Length + zigBeeCommandHeader.Length ;
            if (command.IsZdoCommand)
            {
                // note that sequence number is part of payload in case of ZCL command
                commandSize += tempSequenceNumber.Length;
            }
            if (null != command.Payload)
            {
                commandSize += command.Payload.Length;
            }
            nbOfBytesToSend = headerSize + commandSize + CHECKSUM_LENGTH;
            byte[] buffer = new byte[nbOfBytesToSend];

            // add headers (frame and XBee command)
            byte[] tempCmdId = NextCommandId();
            offset = SetCommandHeader(ref buffer, offset, commandSize, ref EXPLICIT_ADDRESSING_FRAME, ref tempCmdId);

            // add payload
            offset = AdapterHelper.AddByteToBuffer(ref buffer, ref zigBeeCommandHeader, offset);
            if (command.IsZdoCommand)
            {
                offset = AdapterHelper.AddByteToBuffer(ref buffer, ref tempSequenceNumber, offset);
            }
            if (null != command.Payload)
            {
                byte[] payload = command.Payload;
                if (!command.IsZdoCommand &&
                    payload.Length >= 2)
                {
                    // sequence number is 2nd byte of payload for ZCL command
                    payload[1] = tempSequenceNumber[0];
                }
                offset = AdapterHelper.AddByteToBuffer(ref buffer, ref payload, offset);
            }

            // add checksum
            buffer[offset] = CheckSum(ref buffer, headerSize, commandSize);

            if (responseRequired)
            {
                lock (m_locker)
                {
                    // add new awaited response in list
                    m_pendingZigBeeCmdList.Add(tempSequenceNumber[0], command);
                }
            }

            // send command
            await m_serialController.WriteAsync(buffer);

            Debug.WriteLineIf(Logger.IsVerbose(), "command sent", command.ToString());
        }

        public void SendATCmdLocalNoResponse(XBeeATCommand command)
        {
            byte[] tempCmdId = NextCommandId();
            SendATCmdLocalInternal(command, false, tempCmdId);
        }
        public byte SendATCmdLocal(XBeeATCommand command)
        {
            byte[] tempCmdId = NextCommandId();
            SendATCmdLocalInternal(command, true, tempCmdId);
            return tempCmdId[0];
        }
        private async void SendATCmdLocalInternal(XBeeATCommand command, bool responseRequired, byte[] tempCmdId)
        {
            int headerSize = 0;
            int commandSize = 0;
            int nbOfBytesToSend = 0;
            int offset = 0;
            byte[] atCommand = command.ATCommand;

            // allocate send buffer
            headerSize = COMMAND_PREFIX.Length + sizeof(UInt16);
            commandSize = AT_COMMAND_LOCAL_SEND.Length + m_commandId.Length + atCommand.Length;
            if (null != command.Payload)
            {
                commandSize += command.Payload.Length;
            }
            nbOfBytesToSend = headerSize + commandSize + CHECKSUM_LENGTH;
            byte[] buffer = new byte[nbOfBytesToSend];

            // add headers
            offset = SetCommandHeader(ref buffer, offset, commandSize, ref AT_COMMAND_LOCAL_SEND, ref tempCmdId);

            // add payload
            offset = AdapterHelper.AddByteToBuffer(ref buffer, ref atCommand, offset);
            if (null != command.Payload)
            {
                byte[] payload = command.Payload;
                offset = AdapterHelper.AddByteToBuffer(ref buffer, ref payload, offset);
            }

            // add checksum
            buffer[offset] = CheckSum(ref buffer, headerSize, commandSize);

            if (responseRequired)
            {
                lock (m_locker)
                {
                    // add new awaited response in list
                    m_pendingATCmdList.Add(tempCmdId[0], command);
                }
            }

            // send command
            await m_serialController.WriteAsync(buffer);

            Debug.WriteLineIf(Logger.IsVerbose(), "command sent", command.ToString());
        }

        private int SetCommandHeader(ref byte[] buffer, int initialOffset, int commandSize, ref byte[] xbeeCommand, ref byte[] commandId)
        {
            int offset = initialOffset;

            // add headers (frame and at command)
            offset = AdapterHelper.AddByteToBuffer(ref buffer, ref COMMAND_PREFIX, offset);

            UInt16 tempWord = Convert.ToUInt16(commandSize);
            byte[] tempBytes = AdapterHelper.ToXbeeFrame(tempWord);
            offset = AdapterHelper.AddByteToBuffer(ref buffer, ref tempBytes, offset);

            offset = AdapterHelper.AddByteToBuffer(ref buffer, ref xbeeCommand, offset);
            offset = AdapterHelper.AddByteToBuffer(ref buffer, ref commandId, offset);

            return offset;
        }

        void GetBytesFromModule(byte[] buffer, int nbOfBytes)
        {
            // sanity check
            if (0 == buffer.Length ||
               0 == nbOfBytes)
            {
                return;
            }

            AppendBytesToCurrentResponse(ref buffer, nbOfBytes);
            while(CheckResponse()) ;
        }

        private bool CheckResponse()
        {
            bool checkNextResponse = false;

            int headerSize = 0;
            bool signalResponseReceived = false;
            byte[] response = null;
            XBeeATCommand atCommand = null;
            ZigBeeCommand zigBeeCommand = null;

            lock(m_locker)
            {
                if (COMMAND_PREFIX[0] != m_currentResponse[0])
                {
                    // response buffer is invalid => discard
                    m_currentResponse = null;
                    return checkNextResponse;
                }

                headerSize = COMMAND_PREFIX.Length + sizeof(UInt16);
                if (m_currentResponse.Length <= headerSize)
                {
                    // not enough bytes => wait for more
                    return checkNextResponse;
                }

                // get size of response
                int payloadLength = (int)AdapterHelper.UInt16FromXbeeFrame(m_currentResponse, COMMAND_PREFIX.Length);
                int fullResponseLength = payloadLength + headerSize + CHECKSUM_LENGTH;

                if (m_currentResponse.Length < fullResponseLength)
                {
                    // not enough bytes => wait for more
                    return checkNextResponse;
                }

                // verify checksum
                if (IsCheckSumOk(ref m_currentResponse, headerSize, payloadLength + CHECKSUM_LENGTH))
                {
                    // trace Rx buffer to debug output
                    Logger.TraceRxBuffer(m_currentResponse);

                    // get frame type
                    byte frameType = m_currentResponse[headerSize];

                    // find corresponding command or corresponding notification
                    if (frameType == AT_COMMAND_LOCAL_RECEIVE[0] ||
                        frameType == AT_COMMAND_REMOTE_RECEIVE[0])
                    {
                        // get command ID
                        byte commandId = m_currentResponse[headerSize + AT_COMMAND_LOCAL_RECEIVE.Length];
                        if (m_pendingATCmdList.TryGetValue(commandId, out atCommand))
                        {
                            Debug.WriteLineIf(Logger.IsVerbose(), "got response", atCommand.ToString());

                            // command found => parse response and remove from list
                            int responseHeaderLength = sizeof(byte) + AT_COMMAND_LOCAL_RECEIVE.Length;
                            payloadLength -= responseHeaderLength;
                            response = new byte[payloadLength];
                            Array.Copy(m_currentResponse, headerSize + responseHeaderLength, response, 0, payloadLength);

                            atCommand.ParseResponse(response);
                            RemovePendingATCmd(commandId);
                            signalResponseReceived = true;
                        }
                        else
                        {
                            Debug.WriteLineIf(Logger.IsVerbose(), "unmanaged response to AT COMMAND or AT COMMAND notification");
                        }
                    }
                    else if (frameType == EXPLICIT_RX_INDICATOR[0])
                    {
                        int responseHeaderLength = sizeof(byte);
                        payloadLength -= responseHeaderLength;
                        response = new byte[payloadLength];
                        Array.Copy(m_currentResponse, headerSize + responseHeaderLength, response, 0, payloadLength);

                        byte zdoSequenceNumber = m_currentResponse[headerSize + EXPLICIT_RX_INDICATOR.Length + ZigBeeCommand.GetSequenceNumberOffestInZdoFrame()];
                        byte zclSequenceNumber = m_currentResponse[headerSize + EXPLICIT_RX_INDICATOR.Length + ZigBeeCommand.GetSequenceNumberOffestInZclFrame()];
                        if ((m_pendingZigBeeCmdList.TryGetValue(zdoSequenceNumber, out zigBeeCommand) && zigBeeCommand.IsZdoCommand)
                            ||
                            (m_pendingZigBeeCmdList.TryGetValue(zclSequenceNumber, out zigBeeCommand) && !zigBeeCommand.IsZdoCommand))
                        {
                            Debug.WriteLineIf(Logger.IsVerbose(), "got response to ZDO or ZCL command", zigBeeCommand.ToString());

                            zigBeeCommand.ParseResponse(response);
                            if (zigBeeCommand.IsZdoCommand)
                            {
                                RemovePendingZigBeeCmd(zdoSequenceNumber);
                            }
                            else
                            {
                                RemovePendingZigBeeCmd(zclSequenceNumber);
                            }
                            signalResponseReceived = true;
                        }
                        else
                        {
                            // check if this is an awaited notification
                            foreach (var notification in m_ZigBeeNotificationList)
                            {
                                // "try" parsing response to figure out if it
                                // correspond to a awaited notification
                                if (notification.Value.ParseResponse(response))
                                {
                                    // corresponding notification found
                                    Debug.WriteLineIf(Logger.IsVerbose(), "notification of ZDO or ZCL command", notification.Value.ToString());
                                    break;
                                }
                            }
                            Debug.WriteLineIf(Logger.IsVerbose(), "unmanaged response to ZDO or ZCL command");
                        }
                    }
                    else
                    {
                        Debug.WriteLineIf(Logger.IsVerbose(), "received frame OK but not handled");
                    }
                }

                // remove response from buffer
                ClearResponseBuffer(fullResponseLength);
                if (m_currentResponse != null &&
                   m_currentResponse.Length != 0)
                {
                    checkNextResponse = true;
                }
            }

            if (signalResponseReceived)
            {
                if (null != atCommand)
                {
                    atCommand.SignalResponseReceive();
                }
                else if (null != zigBeeCommand)
                {
                    zigBeeCommand.SignalResponseReceive();
                }
            }

            return checkNextResponse;
        }

        public void RemovePendingATCmd(byte id)
        {
            lock (m_locker)
            {
                m_pendingATCmdList.Remove(id);
            }
        }
        public void RemovePendingZigBeeCmd(byte id)
        {
            lock (m_locker)
            {
                m_pendingZigBeeCmdList.Remove(id);
            }
        }
        private void ClearResponseBuffer(int length)
        {
            lock(m_locker)
            {
                if (m_currentResponse.Length <= length)
                {
                    m_currentResponse = null;
                }
                else
                {
                    int newLenght = m_currentResponse.Length - length;
                    for (int index = 0; index < newLenght; index++)
                    {
                        m_currentResponse[index] = m_currentResponse[index + length];
                    }
                    Array.Resize(ref m_currentResponse, newLenght);
                }
            }
        }
        private void AppendBytesToCurrentResponse(ref byte[] buffer, int nbOfBytes)
        {
            lock(m_locker)
            {
                int offset = 0;

                if (null == m_currentResponse)
                {
                    m_currentResponse = new byte[nbOfBytes];
                }
                else
                {
                    offset = m_currentResponse.Length;
                    Array.Resize(ref m_currentResponse, m_currentResponse.Length + nbOfBytes);
                }

                for (int index = 0; index < nbOfBytes; index++)
                {
                    m_currentResponse[offset + index] = buffer[index];
                }
            }
        }
        private byte[] NextCommandId()
        {
            byte[] previousCommandId = new byte[m_commandId.Length];
            Array.Copy(m_commandId, previousCommandId, m_commandId.Length);

            m_commandId[0]++;
            if (0 == m_commandId[0])
            {
                m_commandId[0] = 1;
            }

            return previousCommandId;
        }
        byte CheckSum(ref byte[] buffer, int offset, int lenght)
        {
	        // subtracts bytes from 0xFF, in order to determine the proper
	        // checksum byte so all bytes added together result in 0x00.

	        byte checkSum = 0xFF;

	        for (int index= 0; index < lenght; index++)
	        {
		        checkSum -= buffer[offset + index];
	        }

	        return checkSum;
        }

        bool IsCheckSumOk(ref byte[] buffer, int startIndex, int length)
        {
	        byte checkSum = 0x00;

	        for (int i = 0; i < length; i++)
	        {
		        checkSum += buffer[startIndex + i];
	        }

	        if (0xFF == checkSum)
	        {
		        return true;
	        }
	        else
	        {
		        return false;
	        }
        }
    }
}
