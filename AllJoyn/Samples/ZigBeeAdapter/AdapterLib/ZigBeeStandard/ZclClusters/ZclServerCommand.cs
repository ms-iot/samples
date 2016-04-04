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
    class ZclServerCommandHandler : ZigBeeCommand
    {
        /// ZCL server command is sent by a ZigBee endpoint/cluster to ZigBee DSB
        /// note that: 
        /// - ZigBee DSB implements very often not to say always the client side of a ZCL cluster 
        /// - ZCL server command that are response to ZCL client command are 
        ///   are directly handled by ZclCluster and ZclCommand classes
        /// 
        /// Server command will never be sent by ZigBee Adapter nor exposed to AllJoyn.
        /// 

        // Singleton class 
        private static readonly ZclServerCommandHandler instance = new ZclServerCommandHandler();
        public static ZclServerCommandHandler Instance
        {
            get { return instance; }
        }

        private List<ZclServerCommand> m_commandList = new List<ZclServerCommand>();
        public void AddCommand(ZclServerCommand command)
        {
            lock(m_commandList)
            {
                m_commandList.Add(command);
            }
        }
        public void RemoveCommands(ZigBeeDevice device)
        {
            // sanity check
            if(device == null)
            {
                return;
            }

            // remove all server command associated with a specific device
            lock(m_commandList)
            {
                // 1st build list of elements to remove then remove them from list
                List<ZclServerCommand> elementsToRemove = new List<ZclServerCommand>();
                foreach(var element in m_commandList)
                {
                    if(element.MatchDevice(device))
                    {
                        elementsToRemove.Add(element);
                    }
                }

                foreach(var element in elementsToRemove)
                {
                    m_commandList.Remove(element);
                }
            }
        }
        public override bool ParseResponse(byte[] buffer)
        {
            if(m_commandList.Count == 0)
            {
                // no server command registered
                return false;
            }

            // get Mac address, endpoint Id, cluster Id and command Id of source 
            int offset = 0;
            UInt64 macAddress = AdapterHelper.UInt64FromXbeeFrame(buffer, offset);
            offset = AdapterHelper.MAC_ADDR_LENGTH;

            // skip network address (mac address is enough) 
            offset += AdapterHelper.NETWORK_ADDRESS_LENGTH;

            byte endpointId = buffer[offset];
            offset++;

            // skip destination end point
            offset++;

            UInt16 clusterId = AdapterHelper.UInt16FromXbeeFrame(buffer, offset);

            offset = GetZclCommandIdOffset(ref buffer);
            byte commandId = buffer[offset];

            // find element that matches Mac address, network address, endpoint Id and cluster Id
            var element = Find(macAddress, endpointId, clusterId, commandId);
            if(element == null)
            {
                // no corresponding element for that Zcl server command
                return false;
            }

            if(element.OnReception == null)
            {
                // no notification hence nothing else to do
                return true;
            }

            // get parameters
             offset = GetZclPayloadOffset(ref buffer);
            // fill in out parameters
            foreach (var parameter in element.ParamList)
            {
                object value;
                if (!ZclHelper.GetValue(parameter.ZigBeeType, ref buffer, ref offset, out value))
                {
                    // can't get one of the out parameters => nothing else to do
                    // note that return value is true because server command has been found but can't be parsed
                    return true;
                }

                parameter.Data = value;
            }

            // execute notification callback asynchronously
            Task.Run(() => { element.OnReception(element); });

            return true;
        }

        private ZclServerCommand Find(UInt64 macAddress, byte endpointId, UInt16 clusterId, byte commandId)
        {
            lock(m_commandList)
            {
                foreach(var tempElement in m_commandList)
                {
                    if(tempElement.Match(macAddress, endpointId, clusterId, commandId))
                    {
                        return tempElement;
                    }
                }
            }
            return null;
        }
    }

    class ZclServerCommand
    {
        private byte m_commandId = 0x00;
        private ZclCluster m_cluster = null;
        
        public ZclServerCommand(ZclCluster cluster, byte commandId)
        {
            m_cluster = cluster;
            m_commandId = commandId;
        }
        // command parameter lists
        // 
        // Note:
        // - IMPORTANT: ZCL parameters MUST be added in ZCL parameter list  
        //   in the order specified in ZCL standard
        // - an internal parameter list must be kept 
        //   because BridgeRT can set input parameter list and doesn't guarantee
        //   ZCL order will be respected 
        //   
        private readonly List<ZclValue> m_paramList = new List<ZclValue>();
        public List<ZclValue> ParamList
        {
            get { return m_paramList;  }
        }
        public void AddParam(ZclValue parameter)
        {
            m_paramList.Add(parameter);
        }

        public ZclValue GetParam(string name)
        {
            return m_paramList.FirstOrDefault(s => s.Name == name);
        }
        public void AddToServerCommandHandler()
        {
            ZclServerCommandHandler serverCommandHandler = ZclServerCommandHandler.Instance;
            serverCommandHandler.AddCommand(this);
        }

        public bool Match(UInt64 macAddress, byte endpointId, UInt16 clusterId, byte commandId)
        {
            if(m_commandId == commandId &&
               m_cluster.Id == clusterId &&
               m_cluster.EndPoint.Id == endpointId &&
               m_cluster.EndPoint.Device.MacAddress == macAddress)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        public bool MatchDevice(ZigBeeDevice device)
        {
            if (m_cluster.EndPoint.Device.MacAddress == device.MacAddress)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        public Action<ZclServerCommand> OnReception = null;
    }
}
