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
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices.WindowsRuntime;
using BridgeRT;
using System.Diagnostics;
using System.Threading;

namespace AdapterLib
{
    public sealed class Adapter : IAdapter
    {
        private const uint ERROR_SUCCESS = 0;
        private const uint ERROR_INVALID_HANDLE = 6;
        private const uint ERROR_INVALID_DATA = 13;
        private const uint ERROR_INVALID_PARAMETER = 87;
        private const uint ERROR_NOT_SUPPORTED = 50;

        public string Vendor { get; }

        public string AdapterName { get; }

        public string Version { get; }

        public string ExposedAdapterPrefix { get; }

        public string ExposedApplicationName { get; }

        public Guid ExposedApplicationGuid { get; }

        public IList<IAdapterSignal> Signals { get; }

        // A map of signal handle (object's hash code) and related listener entry
        private struct SIGNAL_LISTENER_ENTRY
        {
            // The signal object
            internal IAdapterSignal Signal;

            // The listener object
            internal IAdapterSignalListener Listener;

            //
            // The listener context that will be
            // passed to the signal handler
            //
            internal object Context;
        }
        private Dictionary<int, IList<SIGNAL_LISTENER_ENTRY>> m_signalListeners;

        // ZigBee command used to get new device notification, reportable attributes
        internal readonly DeviceAnnce m_deviceAnnonce = DeviceAnnce.Instance;
        internal readonly ZclReportAttributes m_reportAttributes = ZclReportAttributes.Instance;
        internal readonly ZclServerCommandHandler m_zclServerCommandHandler = ZclServerCommandHandler.Instance;

        public Adapter()
        {
            Windows.ApplicationModel.Package package = Windows.ApplicationModel.Package.Current;
            Windows.ApplicationModel.PackageId packageId = package.Id;
            Windows.ApplicationModel.PackageVersion versionFromPkg = packageId.Version;

            this.Vendor = AdapterHelper.ADAPTER_VENDOR;
            this.AdapterName = AdapterHelper.ADAPTER_NAME;

            // the adapter prefix must be something like "com.mycompany" (only alpha num and dots)
            // it is used by the Device System Bridge as root string for all services and interfaces it exposes
            this.ExposedAdapterPrefix = AdapterHelper.ADAPTER_DOMAIN + "." + this.Vendor.ToLower();
            this.ExposedApplicationGuid = Guid.Parse(AdapterHelper.ADAPTER_APPLICATION_GUID);

            if (null != package && null != packageId)
            {
                this.ExposedApplicationName = packageId.Name;
                this.Version = versionFromPkg.Major.ToString() + "." +
                               versionFromPkg.Minor.ToString() + "." +
                               versionFromPkg.Revision.ToString() + "." +
                               versionFromPkg.Build.ToString();
            }
            else
            {
                this.ExposedApplicationName = AdapterHelper.ADAPTER_DEFAULT_APPLICATION_NAME;
                this.Version = AdapterHelper.ADAPTER_DEFAULT_VERSION;
            }

            try
            {
                this.Signals = new List<IAdapterSignal>();
                this.m_signalListeners = new Dictionary<int, IList<SIGNAL_LISTENER_ENTRY>>();
            }
            catch (OutOfMemoryException ex)
            {
                Debug.WriteLine(ex);
                throw;
            }
        }

        public uint SetConfiguration([ReadOnlyArray] byte[] ConfigurationData)
        {
            // TODO: add configuration, e.g.: VID/PID of ZigBee dongle
            return ERROR_NOT_SUPPORTED;
        }

        public uint GetConfiguration(out byte[] ConfigurationDataPtr)
        {
            // TODO: add configuration
            ConfigurationDataPtr = null;

            return ERROR_NOT_SUPPORTED;
        }

        public uint Initialize()
        {
            try
            {
                m_xBeeModule.Initialize(out m_adapter);
            }
            catch (Exception ex)
            {
                return (uint)ex.HResult;
            }

            // discover ZigBee device
            CreateSignals();
            StartDeviceDiscovery();

            // add new device and change of attribute value notifications
            m_deviceAnnonce.OnReception += AddDeviceInDeviceList;
            m_reportAttributes.OnReception += ZclReportAttributeReception;
            m_xBeeModule.AddXZibBeeNotification(m_deviceAnnonce);
            m_xBeeModule.AddXZibBeeNotification(m_reportAttributes);
            m_xBeeModule.AddXZibBeeNotification(m_zclServerCommandHandler);

            return ERROR_SUCCESS;
        }

        public uint Shutdown()
        {
            m_xBeeModule.Shutdown();

            return ERROR_SUCCESS;
        }

        public uint EnumDevices(
            ENUM_DEVICES_OPTIONS Options,
            out IList<IAdapterDevice> DeviceListPtr,
            out IAdapterIoRequest RequestPtr)
        {
            RequestPtr = null;

            DeviceListPtr = new List<IAdapterDevice>();

            // Add all end points in all ZigBee devices to the list
            // if device list not locked 
            // note that:
            // - an endpoint is a device for BridgeRT
            // - endpoints will be notified as ZigBee devices arrive 
            if (Monitor.TryEnter(m_deviceMap))
            {
                try
                {
                    foreach (var zigBeeDeviceItem in m_deviceMap)
                    {
                        foreach (var endPointItem in zigBeeDeviceItem.Value.EndPointList)
                        {
                            DeviceListPtr.Add(endPointItem.Value);
                        }
                    }
                }
                finally
                {
                    // use try/finally to ensure m_deviceList is unlocked in every case
                    Monitor.Exit(m_deviceMap);
                }
            }

            return ERROR_SUCCESS;
        }

        public uint GetProperty(
            IAdapterProperty Property,
            out IAdapterIoRequest RequestPtr)
        {
            RequestPtr = null;

            // sanity check
            if (Property == null)
            {
                return ERROR_INVALID_PARAMETER;
            }

            // cast back IAdapterProperty to ZclCluster
            ZclCluster cluster = (ZclCluster)Property;

            // read all attributes for the attribute
            foreach (var item in cluster.InternalAttributeList)
            {
                var attribute = item.Value;
                object value;
                if(!attribute.Read(out value))
                {
                    // give up at 1st read error
                    return (uint) ZclHelper.ZigBeeStatusToHResult(attribute.Status);
                }
            }

            return ERROR_SUCCESS;
        }

        public uint SetProperty(
            IAdapterProperty Property,
            out IAdapterIoRequest RequestPtr)
        {
            RequestPtr = null;

            // sanity check
            if (Property == null)
            {
                return ERROR_INVALID_PARAMETER;
            }

            // cast back IAdapterProperty to ZclCluster
            ZclCluster cluster = (ZclCluster)Property;

            // write new value for all attributes
            // note that it is assumed that BridgeRT has set the new value
            // for each attribute
            foreach (var item in cluster.InternalAttributeList)
            {
                var attribute = item.Value;
                if (attribute.Write(attribute.Value.Data))
                {
                    // give up at 1st write error
                    return (uint)ZclHelper.ZigBeeStatusToHResult(attribute.Status);
                }
            }

            return ERROR_SUCCESS;
        }

        public uint GetPropertyValue(
            IAdapterProperty Property,
            string AttributeName,
            out IAdapterValue ValuePtr,
            out IAdapterIoRequest RequestPtr)
        {
            ValuePtr = null;
            RequestPtr = null;

            // sanity check
            if (Property == null)
            {
                return ERROR_INVALID_PARAMETER;
            }

            // cast back IAdapterProperty to ZclCluster
            ZclCluster cluster = (ZclCluster) Property;

            // look for the attribute
            foreach(var item in cluster.InternalAttributeList)
            {
                var attribute = item.Value;
                if (attribute.Value.Name == AttributeName)
                {
                    object value;
                    if(attribute.Read(out value))
                    {
                        ValuePtr = attribute.Value;
                        return ERROR_SUCCESS;
                    }
                    else
                    {
                        return (uint)ZclHelper.ZigBeeStatusToHResult(attribute.Status);
                    }
                }
            }

            return ERROR_NOT_SUPPORTED;
        }

        public uint SetPropertyValue(
            IAdapterProperty Property,
            IAdapterValue Value,
            out IAdapterIoRequest RequestPtr)
        {
            RequestPtr = null;

            // sanity check
            if (Property == null)
            {
                return ERROR_INVALID_PARAMETER;
            }

            // cast back IAdapterProperty to ZclCluster
            ZclCluster cluster = (ZclCluster)Property;

            // look for the attribute and write new data
            foreach (var item in cluster.InternalAttributeList)
            {
                var attribute = item.Value;
                if (attribute.Value.Name == Value.Name)
                {
                    if (attribute.Write(Value.Data))
                    {
                        return ERROR_SUCCESS;
                    }
                    else
                    {
                        return (uint)ZclHelper.ZigBeeStatusToHResult(attribute.Status);
                    }
                }
            }

            return ERROR_NOT_SUPPORTED;
        }

        public uint CallMethod(
            IAdapterMethod Method,
            out IAdapterIoRequest RequestPtr)
        {
            RequestPtr = null;

            // sanity check
            if (Method == null)
            {
                return ERROR_INVALID_PARAMETER;
            }

            // IAdapterMethod is either a ZclCommand or a ManagementLeave command,
            // cast back IAdapterMethod to ZclCommand first then to ManagementLeave if cast failed 
            try
            {
                var command = (ZclCommand) Method;
                command.Send();
                return (uint)command.HResult;
            }
            catch (InvalidCastException e)
            {
                // send the leave command and remove devices (hence all end points of the ZigBee device)
                //
                // Note that ManagementLeave is THE unique ZdoCommand exposed to AllJoyn
                //
                var command = (ManagementLeave)Method;
                command.Send();

                if (command.ZigBeeStatus != ZdoHelper.ZDO_NOT_SUPPORTED)
                {
                    // async device removal
                    Task.Run(() => RemoveDevice(command.Device));
                }

                return (uint)command.HResult;
            }
        }

        public uint RegisterSignalListener(
            IAdapterSignal Signal,
            IAdapterSignalListener Listener,
            object ListenerContext)
        {
            // sanity check
            if (Signal == null || Listener == null)
            {
                return ERROR_INVALID_PARAMETER;
            }

            int signalHashCode = Signal.GetHashCode();

            SIGNAL_LISTENER_ENTRY newEntry;
            newEntry.Signal = Signal;
            newEntry.Listener = Listener;
            newEntry.Context = ListenerContext;

            lock (m_signalListeners)
            {
                if (m_signalListeners.ContainsKey(signalHashCode))
                {
                    m_signalListeners[signalHashCode].Add(newEntry);
                }
                else
                {
                    var newEntryList = new List<SIGNAL_LISTENER_ENTRY> { newEntry };
                    m_signalListeners.Add(signalHashCode, newEntryList);
                }
            }

            return ERROR_SUCCESS;
        }

        public uint UnregisterSignalListener(
            IAdapterSignal Signal,
            IAdapterSignalListener Listener)
        {
            return ERROR_SUCCESS;
        }

        private uint CreateSignals()
        {
            // create device arrival signal and add it to list
            AdapterSignal deviceArrival = new AdapterSignal(Constants.DEVICE_ARRIVAL_SIGNAL);
            deviceArrival.AddParam(Constants.DEVICE_ARRIVAL__DEVICE_HANDLE);
            Signals.Add(deviceArrival);

            // create device removal signal and add it to list
            AdapterSignal deviceRemoval = new AdapterSignal(Constants.DEVICE_REMOVAL_SIGNAL);
            deviceRemoval.AddParam(Constants.DEVICE_REMOVAL__DEVICE_HANDLE);
            Signals.Add(deviceRemoval);

            return ERROR_SUCCESS;
        }

        internal void NotifyBridgeRT(ZigBeeEndPoint endPoint, string signalName, string paramName)
        {
            // find device arrival signal in list
            var deviceArrival = Signals.OfType<AdapterSignal>().FirstOrDefault(s => s.Name == signalName);
            if (deviceArrival == null)
            {
                // no device arrival signal
                return;
            }

            // set parameter value
            var param = deviceArrival.Params.FirstOrDefault(p => p.Name == paramName);
            if(param == null)
            {
                // signal doesn't have the expected parameter
                return;
            }
            param.Data = endPoint;

            NotifySignalListeners(deviceArrival);
        }

        internal void NotifySignalListeners(IAdapterSignal signal)
        {
            int signalHashCode = signal.GetHashCode();

            IList<SIGNAL_LISTENER_ENTRY> listenerList = null;
            lock (m_signalListeners)
            {
                if(m_signalListeners.ContainsKey(signalHashCode))
                {
                    // make a local copy of the listener list
                    listenerList = m_signalListeners[signalHashCode].ToArray();
                }
                else
                {
                    // can't do anything 
                    return;
                }

            }

            // call out event handlers out of the lock to avoid
            // deadlock risk
            foreach (SIGNAL_LISTENER_ENTRY entry in listenerList)
            {
                IAdapterSignalListener listener = entry.Listener;
                object listenerContext = entry.Context;
                listener.AdapterSignalHandler(signal, listenerContext);
            }
        }

        private void RemoveDevice(ZigBeeDevice zigBeeDevice)
        {
            // remove device from list (it will be garbaged collect later)
            lock (m_deviceMap)
            {
                m_deviceMap.Remove(zigBeeDevice.MacAddress);
            }

            // remove server commands that belong to that device
            m_zclServerCommandHandler.RemoveCommands(zigBeeDevice);

            // notify AllJoyn/BridgeRT
            // 
            // note that a ZigBee endpoint is a exposed as a device on AllJoyn
            // => BridgeRT should be notified of removal of each enpoint of the removed ZigBee device
            foreach (var endPoint in zigBeeDevice.EndPointList)
            {
                NotifyBridgeRT(endPoint.Value, Constants.DEVICE_REMOVAL_SIGNAL, Constants.DEVICE_REMOVAL__DEVICE_HANDLE);
            }
        }

        private XBeeModule m_xBeeModule = new XBeeModule();
        internal XBeeModule XBeeModule
        {
            get { return m_xBeeModule; }
        }

        private ZigBeeDevice m_adapter = null;
        private ZclClusterFactory m_zclClusterFactory = ZclClusterFactory.Instance;
        private ZigBeeProfileLibrary m_zigBeeProfileLibrary = ZigBeeProfileLibrary.Instance;
        private readonly Dictionary<UInt64, ZigBeeDevice> m_deviceMap = new Dictionary<UInt64, ZigBeeDevice>();
        internal Dictionary<UInt64, ZigBeeDevice> DeviceList
        {
            get { return m_deviceMap; }
        }

        private void StartDeviceDiscovery()
        {
            // async discovery process
            Task.Run(() => DiscoverDevices());
        }

        private void DiscoverDevices()
        {
            ManagementLQI lqiCommand = new ManagementLQI();

            // lock device list and clear it
            lock(m_deviceMap)
            {
                m_deviceMap.Clear();
            }

            // get direct neighbor and add them in list
            //
            // note: for now, only deal with direct neighbor (no hop)
            //       going through ZigBee network nodes and graph will come later
            lqiCommand.GetNeighbors(m_xBeeModule, m_adapter);
            foreach (var deviceDescriptor in lqiCommand.NeighborList)
            {
                // async add device in list so device details can be queried in parallel and
                // one detail discovery won't block the others
                Task.Run(() => { AddDeviceInDeviceList(deviceDescriptor.networkAddress, deviceDescriptor.macAddress, deviceDescriptor.isEndDevice); });
            }
        }
        private void AddDeviceInDeviceList(UInt16 networkAddress, UInt64 macAddress, bool isEndDevice)
        {
            bool deviceFound = false;
            bool addDeviceInList = false;
            ZigBeeDevice device = null;

            Logger.TraceDevice(networkAddress, macAddress);

            lock (m_deviceMap)
            {
                deviceFound = m_deviceMap.TryGetValue(macAddress, out device);
            }

            if(!deviceFound)
            {
                // the device isn't in the list yet
                device = new ZigBeeDevice(networkAddress, macAddress, isEndDevice);

                // get end points and supported clusters
                ActiveEndPoints activeEndPointsCommand = new ActiveEndPoints();
                activeEndPointsCommand.GetEndPoints(m_xBeeModule, device);

                foreach (var endPointId in activeEndPointsCommand.EndPointList)
                {
                    SimpleDescriptor descriptor = new SimpleDescriptor();
                    descriptor.GetDescriptor(m_xBeeModule, device, endPointId);
                    Logger.TraceDeviceDetailed(device.MacAddress, endPointId, descriptor);
                    foreach (var clusterId in descriptor.InClusterList)
                    {
                        if (m_zclClusterFactory.IsClusterSupported(clusterId) &&
                            device.AddClusterToEndPoint(true, endPointId, descriptor.ProfileId, descriptor.DeviceId, clusterId, this))
                        {
                            // only add device in list if at least 1 cluster is supported
                            addDeviceInList = true;
                        }
                    }
                    foreach (var clusterId in descriptor.OutClusterList)
                    {
                        if (m_zclClusterFactory.IsClusterSupported(clusterId) &&
                            device.AddClusterToEndPoint(false, endPointId, descriptor.ProfileId, descriptor.DeviceId, clusterId, this))
                        {
                            // only add device in list if at least 1 cluster is supported
                            addDeviceInList = true;
                        }
                    }
                }
            }
            else
            {
                // the device is already in list so just refresh its network address.
                // note that mac address will never change but network address can, e.g.: if end device connects to another router
                device.NetworkAddress = networkAddress;
            }

            // add device in list if necessary
            if (addDeviceInList)
            {
                lock (m_deviceMap)
                {
                    // add device in list if it has not been added between beginning of this routine and now
                    ZigBeeDevice tempDevice = null;
                    if(!m_deviceMap.TryGetValue(device.MacAddress, out tempDevice))
                    {
                        m_deviceMap.Add(device.MacAddress, device);
                    }
                    else
                    {
                        // device has been added => update network address of already existing device
                        // give up with device that has been created and use the already existing device instead
                        tempDevice.NetworkAddress = device.NetworkAddress;
                        device = tempDevice;
                    }
                }
            }

            // notify devices to bridgeRT
            // note end points are devices as far as BridgeRT is concerned
            foreach (var endpoint in device.EndPointList)
            {
                NotifyBridgeRT(endpoint.Value, Constants.DEVICE_ARRIVAL_SIGNAL, Constants.DEVICE_ARRIVAL__DEVICE_HANDLE);
            }
        }

        private void ZclReportAttributeReception(ZclReportAttributes.SOURCE_INFO deviceInfo, UInt16 attributeId, object newValue)
        {
            ZigBeeDevice device = null;
            ZigBeeEndPoint endPoint = null;
            ZclCluster cluster = null;
            ZclAttribute attribute = null;

            // look for corresponding ZigBee device
            lock (m_deviceMap)
            {
                if (!m_deviceMap.TryGetValue(deviceInfo.macAddress, out device))
                {
                    // unknown device => do nothing
                    return;
                }
            }

            // look for corresponding end point
            if (!device.EndPointList.TryGetValue(deviceInfo.endpointId, out endPoint))
            {
                // unknown end point => do nothing
                return;
            }

            // look for corresponding cluster
            cluster = endPoint.GetCluster(deviceInfo.clusterId);
            if (cluster == null)
            {
                // unknown cluster => do nothing
                return;
            }

            // look for the corresponding attribute
            if (cluster.InternalAttributeList.TryGetValue(attributeId, out attribute))
            {
                // unknown attribute => do nothing
                return;
            }

            // update value
            attribute.Value.Data = newValue;

            // signal value of attribute has changed
            SignalChangeOfAttributeValue(endPoint, cluster, attribute);
        }

        internal void SignalChangeOfAttributeValue(ZigBeeEndPoint endPoint, ZclCluster cluster, ZclAttribute attribute)
        {
            // find change of value signal of that end point (end point == bridgeRT device)
            var covSignal = endPoint.Signals.OfType<AdapterSignal>().FirstOrDefault(s => s.Name == Constants.CHANGE_OF_VALUE_SIGNAL);
            if (covSignal == null)
            {
                // no change of value signal
                return;
            }

            // set property and attribute param of COV signal
            // note that 
            // - ZCL cluster correspond to BridgeRT property 
            // - ZCL attribute correspond to BridgeRT attribute 
            var param = covSignal.Params.FirstOrDefault(p => p.Name == Constants.COV__PROPERTY_HANDLE);
            if (param == null)
            {
                // signal doesn't have the expected parameter
                return;
            }
            param.Data = cluster;

            param = covSignal.Params.FirstOrDefault(p => p.Name == Constants.COV__ATTRIBUTE_HANDLE);
            if (param == null)
            {
                // signal doesn't have the expected parameter
                return;
            }
            param.Data = attribute;

            // signal change of value to BridgeRT
            NotifySignalListeners(covSignal);
        }
        
        public UInt64 MacAddress
        {
            get { return m_adapter.MacAddress; }
        }
    }
}

