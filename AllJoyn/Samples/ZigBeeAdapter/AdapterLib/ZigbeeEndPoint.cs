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

using BridgeRT;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace AdapterLib
{
    //
    // ZigBeeEndPoint.
    // Description:
    // The class that implements IAdapterDevice from BridgeRT.
    //
    // Note that it isn't possible to map a ZigBee devices to IDeviceAdapter because ZigBee device doesn't contain anything
    // that can be mapped to method, attributes or signal which IDeviceAdapter contains. These are defined at end point level in ZigBee.
    class ZigBeeEndPoint :  IAdapterDevice,
                            IAdapterDeviceLightingService,
                            IAdapterDeviceControlPanel
    {
        // parent ZigBee device
        internal ZigBeeDevice m_device = null;
        internal ZigBeeDevice Device
        {
            get { return m_device; }
        }
        // Object Name
        public string Name { get; private set; }

        // Device information
        public string Vendor { get; private set; }

        public string Model { get; private set; }

        public string Version { get; private set; }

        public string FirmwareVersion { get; private set; }

        public string SerialNumber { get; private set; }

        public string Description { get; private set; }

        private LSFHandler m_lsfHandler = null;
        public ILSFHandler LightingServiceHandler
        {
            get { return m_lsfHandler; }
        }

        // Device properties
        public IList<IAdapterProperty> Properties
        {
            get
            {
                return GetZclAttributeList();
            }
        }

        // Device methods
        public IList<IAdapterMethod> Methods
        {
            get
            {
                return GetZclCommandList();
            }
        }

        // Device signals
        public IList<IAdapterSignal> Signals { get; }

        public IAdapterIcon Icon { get; }

        // Control Panel Handler
        public IControlPanelHandler ControlPanelHandler
        {
            get
            {
                return null;
            }
        }

        // command to instruct device to leave ZigBee network
        ManagementLeave m_managementLeave = null;

        internal ZigBeeEndPoint(byte id, UInt16 profileId, UInt16 DeviceId)
        {
            this.Id = id;
            this.Name = UNKNOWN_NAME;
            this.DeviceId = DeviceId;
            this.m_originalProfileId = profileId;
            this.m_commandProfileId = profileId;
            this.Vendor = UNKNOWN_MANUFACTURER;
            this.Model = UNKNOWN_MODEL;
            this.Version = UNKNOWN_VERSION;
            this.SerialNumber = id.ToString() + "." + profileId.ToString() + "." + DeviceId.ToString();
            this.Description = NO_DESCRIPTION;

            this.m_basicCluster = new BasicCluster(this);
            this.m_inClusters = new Dictionary<UInt16, ZclCluster>();
            this.m_outClusters = new Dictionary<UInt16, ZclCluster>();

            try
            {
                this.Signals = new List<IAdapterSignal>();
            }
            catch (OutOfMemoryException ex)
            {
                Debug.WriteLine(ex);
                throw;
            }
        }

        internal ZigBeeEndPoint(ZigBeeEndPoint Other)
        {
            this.m_device = Other.m_device;

            this.Id = Other.Id;
            this.DeviceId = Other.DeviceId;
            this.m_originalProfileId = Other.m_originalProfileId;
            this.m_commandProfileId = Other.m_commandProfileId;

            this.Name = Other.Name;
            this.Vendor = Other.Vendor;
            this.Model = Other.Model;
            this.Version = Other.Version;
            this.FirmwareVersion = Other.FirmwareVersion;
            this.SerialNumber = Other.SerialNumber;
            this.Description = Other.Description;

            this.m_basicCluster = Other.m_basicCluster;
            this.m_inClusters = Other.m_inClusters;
            this.m_outClusters = Other.m_outClusters;

            try
            {
                this.Signals = new List<IAdapterSignal>(Other.Signals);
            }
            catch (OutOfMemoryException ex)
            {
                Debug.WriteLine(ex);
                throw;
            }
        }

        //
        // ZigBee implementation
        //
        // note that profile ID that will be used to send command might be different
        // than the profile ID of the ZigBee device. For example, command will be sent using HA profile ID 
        // instead of ZLL profile ID for ZLL devices (as required by ZLL standard)
        private UInt16 m_originalProfileId = 0;
        private UInt16 m_commandProfileId = 0;
        internal UInt16 CommandProfileId
        {
            get { return m_commandProfileId; }
        }
        internal UInt16 DeviceId { get; private set; }
        internal byte Id { get; private set; }

        internal void Initialize(ZigBeeDevice device)
        {
            ZclAttribute attribute = null;

            // save away parent device
            m_device = device;
            m_managementLeave = new ManagementLeave(m_device);

            ZigBeeProfileLibrary profileLibrary = ZigBeeProfileLibrary.Instance;
            string profileName;
            string deviceType;
            profileLibrary.GetProfileAndDeviceNames(m_originalProfileId, DeviceId, out profileName, out deviceType, out m_commandProfileId);

            // set name and description
            this.Description = profileName + " - " + deviceType;
            this.Name = deviceType ;

            // get some information from Basic cluster, e.g.: manufacturer name, model name, HW version, application version...
            if (m_basicCluster.InternalAttributeList.TryGetValue(BasicCluster.ATTRIBUTE_MANUFACTURER_NAME, out attribute))
            {
                object value;
                if (attribute.Read(out value))
                {
                    this.Vendor = (String)value;
                }
            }
            if (m_basicCluster.InternalAttributeList.TryGetValue(BasicCluster.ATTRIBUTE_MODEL_IDENTIFIER, out attribute))
            {
                object value;
                if (attribute.Read(out value))
                {
                    this.Model = (String)value;
                }
            }
            if (m_basicCluster.InternalAttributeList.TryGetValue(BasicCluster.ATTRIBUTE_HW_VERSION, out attribute))
            {
                object value;
                if (attribute.Read(out value))
                {
                    this.FirmwareVersion = Convert.ToUInt32((byte)value).ToString();
                }
            }
            if (m_basicCluster.InternalAttributeList.TryGetValue(BasicCluster.ATTRIBUTE_APPLICATION_VERSION, out attribute))
            {
                object value;
                if (attribute.Read(out value))
                {
                    this.Version = Convert.ToUInt32((byte)value).ToString();
                }
            }

            // create AllJoyn LSF if this device is a light
            ZigBeeProfileLibrary.DeviceType zigBeeDeviceType;
            if (profileLibrary.IsLight(m_originalProfileId, DeviceId, out zigBeeDeviceType))
            {
                m_lsfHandler = new LSFHandler(this, zigBeeDeviceType);
            }

            // create signals
            CreateSignals();
        }

        public bool AddCluster(UInt16 clusterId, bool isInCluster)
        {
            var clusterFactory = ZclClusterFactory.Instance;
            if (!clusterFactory.IsClusterSupported(clusterId))
            {
                // cluster isn't supported
                return false;
            }

            // discover supported attributes
            List<UInt16> attributeIdList = null;
            var discoverAttributes = new ZclDiscoverAttributes();
            discoverAttributes.GetListOfAttributeIds(m_device.Module, m_device, this, clusterId, out attributeIdList);

            // create cluster instance
            var cluster = clusterFactory.CreateClusterInstance(this, clusterId, attributeIdList);
            if (cluster == null)
            {
                return false;
            }

            if (isInCluster)
            {
                m_inClusters.Add(cluster.Id, cluster);
            }
            else
            {
                m_outClusters.Add(cluster.Id, cluster);
            }

            return true;
        }
        internal ZclCluster GetCluster(UInt16 clusterId)
        {
            ZclCluster cluster = null;

            // 1st check in in cluster list then in out cluster list (if necessary)
            if (!m_inClusters.TryGetValue(clusterId, out cluster))
            {
                m_outClusters.TryGetValue(clusterId, out cluster);
            }

            return cluster;
        }

        private List<IAdapterMethod> GetZclCommandList()
        {
            var commandList = new List<IAdapterMethod>();

            // Enumerate all methods in clusters
            // note that:
            // - only enumerating in cluster is enough for DSB
            // - basic cluster shouldn't be enumerated
            //   since it is uniquely used to set end point information
            foreach (var cluster in m_inClusters.Values)
            {
                foreach (var command in cluster.CommandList)
                {
                    commandList.Add(command.Value);
                }
            }

            // add management leave command
            commandList.Add(m_managementLeave);

            return commandList;
        }

        private List<IAdapterProperty> GetZclAttributeList()
        {
            var attributeList = new List<IAdapterProperty>();

            // Enumerate all attributes in clusters
            // note that:
            // - in cluster is enough for DSB
            // - attributes of basic cluster shouldn't be enumerated
            //   since they're uniquely used to set end point information hence "exposed"
            //   through the end point itself
            foreach (var cluster in m_inClusters.Values)
            {
                attributeList.Add(cluster);
            }

            return attributeList;
        }

        private void CreateSignals()
        {
            // change of value signal
            AdapterSignal changeOfAttributeValue = new AdapterSignal(Constants.CHANGE_OF_VALUE_SIGNAL);
            changeOfAttributeValue.AddParam(Constants.COV__PROPERTY_HANDLE);
            changeOfAttributeValue.AddParam(Constants.COV__ATTRIBUTE_HANDLE);
            Signals.Add(changeOfAttributeValue);
        }

        private const string UNKNOWN_MANUFACTURER = "Unknown";
        private const string UNKNOWN_MODEL = "Unknown";
        private const string UNKNOWN_VERSION = "";
        private const string UNKNOWN_NAME = "Unknown";
        private const string NO_DESCRIPTION = "No description";

        private readonly Dictionary<UInt16, ZclCluster> m_inClusters;
        private readonly Dictionary<UInt16, ZclCluster> m_outClusters;
        private BasicCluster m_basicCluster;
    }
}
