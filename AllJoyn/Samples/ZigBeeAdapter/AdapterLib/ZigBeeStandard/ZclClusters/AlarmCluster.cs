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
    class AlarmCluster : ZclCluster
    {
        internal const UInt16 CLUSTER_ID = 0x0009;
        internal const string CLUSTER_NAME = "Alarm";
        internal const UInt16 ATTRIBUTE_ALARM_COUNT = 0x0000;

        internal AlarmCluster(ZigBeeEndPoint endPoint, List<UInt16> supportedAttributes)
            : base(endPoint)
        {
            // set cluster name and cluster Id
            Name = CLUSTER_NAME;
            m_Id = CLUSTER_ID;

            // if supportedAttributes is null assume all attributes supported
            bool supportAllAttributesBydefault = false;
            if (supportedAttributes == null)
            {
                supportAllAttributesBydefault = true;
            }

            if (supportAllAttributesBydefault || supportedAttributes.Contains(ATTRIBUTE_ALARM_COUNT))
            {
                var attrib = new ZclAttribute(this, ATTRIBUTE_ALARM_COUNT, "Alarm Count", ZclHelper.UINT16_TYPE, true, true);
                m_attributeList.Add(attrib.Id, attrib);
            }
            //TODO: Add Cluster Commands

            // call parent class "post constructor"
            PostChildClassConstructor();
        }
    }
}
