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
using BridgeRT;
using System.Diagnostics;

namespace AdapterLib
{
    //
    // ZclCommand.
    // Description:
    // The class that implements ZigBeeCommand and IAdapterMethod from BridgeRT.
    //
    class ZclCommand : ZigBeeCommand, IAdapterMethod
    {
        private const byte COMMAND_DEFAULT_RESPONSE = 0x0B;

        private ZclCluster m_cluster = null;

        // public properties
        public string Name { get; private set; }
        public string Description { get; private set; }

        // command parameter lists
        // 
        // Note:
        // - IMPORTANT: ZCL parameters MUST be added in ZCL parameter list  
        //   in the order specified in ZCL standard
        // - an internal parameter list must be kept 
        //   because BridgeRT can set input parameter list and doesn't guarantee
        //   ZCL order will be respected 
        //   
        private readonly List<ZclValue> m_zclInParamList;
        public IList<IAdapterValue> InputParams { get; set; }
        public IList<IAdapterValue> OutputParams { get; private set; }

        public int HResult { get; private set; }

        //
        // ZclCommand implementation
        //
        private byte m_Id = 0;
        internal byte Id
        {
            get { return m_Id; }
        }
        private byte m_zigBeeStatus = ZclHelper.ZCL_ERROR_SUCCESS;
        private bool m_useDefaultResponse = true;

        internal ZclCommand(ZclCluster cluster , byte id, String name, bool specificResponseRequired)
        {
            m_cluster = cluster;
            m_Id = id;
            m_zigBeeStatus = ZclHelper.ZCL_ERROR_SUCCESS;

            // as far as ZigBeeCommand class is concerned a response will always be required
            // ZCL will use the Default Response if ZclCommand require no specific response
            m_responseRequired = true;
            if (specificResponseRequired)
            {
                m_useDefaultResponse = false;
            }
            else
            {
                m_useDefaultResponse = true;
            }

            this.Name = m_cluster.Name+ "_" + name;
            this.Description = null;
            this.HResult = ZclHelper.ZigBeeStatusToHResult(m_zigBeeStatus);

            m_isZdoCommand = false;
            ZclHelper.SetClusterSpecificHeader(ref m_zclHeader, 0);
            int offset = ZclHelper.GetCommandIdOffset(ref m_zclHeader, 0);
            m_zclHeader[offset] = m_Id;

            m_zclInParamList = new List<ZclValue>();
            InputParams = new List<IAdapterValue>();
            OutputParams = new List<IAdapterValue>();
        }

        internal void AddInParam(ZclValue parameter)
        {
            // add parameter in both list: "internal" ZCL list as well "BridgeRT" list
            // note that reason for 2 list is explained above
            m_zclInParamList.Add(parameter);
            InputParams.Add(parameter);
        }
        internal void AddOutParam(ZclValue parameter)
        {
            OutputParams.Add(parameter);
        }
        internal IAdapterValue GetInputParamByName(String name)
        {
            IAdapterValue parameter = null;

            foreach (var value in InputParams)
            {
                if(name == value.Name)
                {
                    // found parameter with matching name
                    parameter = value;
                    break;
                }
            }

            return parameter;
        }
        internal void Send()
        {
            // set cluster Id
            m_clusterId = m_cluster.Id;
            m_responseClusterId = m_cluster.Id;

            // add header to payload
            m_payload = new byte[m_zclHeader.Length];
            Array.Copy(m_zclHeader, m_payload, m_zclHeader.Length);

            // add command parameters to payload
            foreach (var item in m_zclInParamList)
            {
                // copy in parameter data from "BridgeRT" in parameter to 
                // "internal" ZCL parameter
                IAdapterValue value = GetInputParamByName(item.Name);
                item.Data = value.Data;

                // add parameter in ZCL payload
                byte[] tempBuffer = item.ToByteBuffer();
                if (tempBuffer == null)
                {
                    // can't set current parameter => give up with sending
                    m_zigBeeStatus = ZclHelper.ZCL_ERROR_INVALID_VALUE;
                    HResult = ZclHelper.ZigBeeStatusToHResult(m_zigBeeStatus);
                    return;
                }

                int previousLength = m_payload.Length;
                Array.Resize(ref m_payload, previousLength + tempBuffer.Length);
                Array.Copy(tempBuffer, 0, m_payload, previousLength, tempBuffer.Length);
            }

            // send command 
            m_zigBeeStatus = ZclHelper.ZCL_ERROR_SUCCESS;
            if (!SendCommand(m_cluster.EndPoint.Device.Module, m_cluster.EndPoint.Device, m_cluster.EndPoint))
            {
                m_zigBeeStatus = ZclHelper.ZCL_ERROR_TIMEOUT;
            }
            HResult = ZclHelper.ZigBeeStatusToHResult(m_zigBeeStatus);

            return;
        }
        public override bool ParseResponse(byte[] buffer)
        {
            m_zigBeeStatus = ZclHelper.ZCL_ERROR_SUCCESS;

            if (!m_responseRequired)
            {
                // no response require => leave with success
                return true;
            }

            // verify response status
            if (!IsResponseOK(buffer))
            {
                m_zigBeeStatus = ZclHelper.ZCL_ERROR_FAILURE;
                return false;
            }

            // parse response
            int offset = GetZclPayloadOffset(ref buffer);
            if (m_useDefaultResponse)
            {
                // Default response to command

                // verify response command Id and sent command Id
                if (COMMAND_DEFAULT_RESPONSE != buffer[offset - 1] ||
                   m_Id != buffer[offset])
                {
                    m_zigBeeStatus = ZclHelper.ZCL_ERROR_FAILURE;
                    return false;
                }

                // save away status
                offset++;
                m_zigBeeStatus = buffer[offset];
            }
            else
            {
                // specific response to command

                // verify response command Id
                if (m_Id != buffer[offset - 1])
                {
                    m_zigBeeStatus = ZclHelper.ZCL_ERROR_FAILURE;
                    return false;
                }

                // fill in out parameters
                foreach (var item in OutputParams)
                {
                    // cast back out param
                    var outParam = (ZclValue)item;

                    object value;
                    if (!ZclHelper.GetValue(outParam.ZigBeeType, ref buffer, ref offset, out value))
                    {
                        // can't get one of the out parameters => give up
                        m_zigBeeStatus = ZclHelper.ZCL_ERROR_FAILURE;
                        return false;
                    }

                    outParam.Data = value;
                }
            }

            return true;
        }

    }
}
