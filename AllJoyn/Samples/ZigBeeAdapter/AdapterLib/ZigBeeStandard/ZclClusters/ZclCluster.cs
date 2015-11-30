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

namespace AdapterLib
{
    abstract class ZclCluster : IAdapterProperty
    {
        // cluster Id
        protected UInt16 m_Id = 0;
        // end point to which this cluster belongs
        protected ZigBeeEndPoint m_endPoint;
        internal ZigBeeEndPoint EndPoint
        {
            get { return m_endPoint; }
        }

        // BridgeRT interface
        public string Name { get; protected set; }
        public string InterfaceHint { get; private set; }
        public IList<IAdapterAttribute> Attributes { get; }

        internal ZclCluster(ZigBeeEndPoint endPoint)
        {
            m_endPoint = endPoint;
            Attributes = new List<IAdapterAttribute>();
        }
        internal UInt16 Id
        {
            get { return m_Id; }
        }
        protected Dictionary<UInt16, ZclAttribute> m_attributeList = new Dictionary<UInt16, ZclAttribute>();
        internal Dictionary<UInt16, ZclAttribute> InternalAttributeList
        {
            get { return m_attributeList; }
        }

        // As per ZCL standard clusters have 2 sides: client and server. ZigBee DSB acts as client of clusters 
        //  - ZclCommand list stores command that ZigBee DSB can send to server side of the cluster (these commands are exposed to AllJoyn)
        //  - ZclServerCommand list stores command that ZigBee DSB can receive from server side of cluster (these commands are NOT exposed to AllJoyn)
        //    Note that server command that are sent in response to client command should not be part of this list because they are handled as part of ZclCommand
        // 
        protected Dictionary<byte, ZclCommand> m_commandList = new Dictionary<byte, ZclCommand>();
        internal Dictionary<byte, ZclCommand> CommandList
        {
            get { return m_commandList; }
        }
        protected List<ZclServerCommand> m_serverCommandList = new List<ZclServerCommand>();

        internal void PostChildClassConstructor()
        {
            // set interface hint 
            // (set it to "" to use default value from BridgeRT)
            InterfaceHint = AdapterHelper.ADAPTER_DOMAIN + "." + AdapterHelper.ADAPTER_VENDOR.ToLower() + "." + m_endPoint.Name.ToLower() + "." + Name.ToLower();

            // set list of attributes for BridgeRT
            foreach (var attribute in m_attributeList)
            {
                Attributes.Add(attribute.Value);
            }

            // add server command in server command handler
            foreach(var command in m_serverCommandList)
            {
                command.AddToServerCommandHandler();
            }
        }

        protected void SignalAttributeValueChange(ZclAttribute attribute)
        {
            m_endPoint.Device.ZigBeeAdatper.SignalChangeOfAttributeValue(m_endPoint, this, attribute);
        }
    }
}
