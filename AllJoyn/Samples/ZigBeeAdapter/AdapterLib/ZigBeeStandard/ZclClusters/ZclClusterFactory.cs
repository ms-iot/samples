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
    class ZclClusterFactory
    {
        // Singleton class 
        private static readonly ZclClusterFactory instance = new ZclClusterFactory();

        // list of supported cluster Id
        // DON'T ADD Basic Cluster Id in that list. 
        // Basic Cluster is supported by all devices and must not be part of the supported cluster list
        // otherwise any ZigBee device/end point might be recognized as a supported device
        private UInt16[] m_clusterIdList = {
            PowerConfigurationCluster.CLUSTER_ID,
            OnOffCluster.CLUSTER_ID,
            LevelControlCluster.CLUSTER_ID,
            AlarmCluster.CLUSTER_ID,
            DoorLockCluster.CLUSTER_ID,
            ColorControlCluster.CLUSTER_ID,
            TemperatureCluster.CLUSTER_ID,
            RelativeHumidityCluster.CLUSTER_ID,
            IASZoneCluster.CLUSTER_ID,
        };

        private ZclClusterFactory()
        {
        }

        public bool IsClusterSupported(UInt16 clusterId)
        {
            return m_clusterIdList.Contains(clusterId);
        }
        public ZclCluster CreateClusterInstance(ZigBeeEndPoint endPoint, UInt16 clusterId, List<UInt16> supportedAttributes)
        {
            // create instance of cluster depending on its Id, 
            // e.g.: create OnOffCluster instance if id correspond to OnOff cluster
            ZclCluster cluster = null;
            if(IsClusterSupported(clusterId))
            {
                switch (clusterId)
                {
                    case PowerConfigurationCluster.CLUSTER_ID:
                        cluster = new PowerConfigurationCluster(endPoint, supportedAttributes);
                        break;
                    case OnOffCluster.CLUSTER_ID:
                        cluster = new OnOffCluster(endPoint, supportedAttributes);
                        break;
                    case LevelControlCluster.CLUSTER_ID:
                        cluster = new LevelControlCluster(endPoint, supportedAttributes);
                        break;
                    case AlarmCluster.CLUSTER_ID:
                        cluster = new AlarmCluster(endPoint, supportedAttributes);
                        break;
                    case DoorLockCluster.CLUSTER_ID:
                        cluster = new DoorLockCluster(endPoint, supportedAttributes);
                        break;
                    case ColorControlCluster.CLUSTER_ID:
                        cluster = new ColorControlCluster(endPoint, supportedAttributes);
                        break;
                    case TemperatureCluster.CLUSTER_ID:
                        cluster = new TemperatureCluster(endPoint, supportedAttributes);
                        break;
                    case RelativeHumidityCluster.CLUSTER_ID:
                        cluster = new RelativeHumidityCluster(endPoint, supportedAttributes);
                        break;
                    case IASZoneCluster.CLUSTER_ID:
                        cluster = new IASZoneCluster(endPoint, supportedAttributes);
                        break;
                }
            }
            return cluster;
        }
        public static ZclClusterFactory Instance
        {
            get { return instance; }
        }
    }
}
