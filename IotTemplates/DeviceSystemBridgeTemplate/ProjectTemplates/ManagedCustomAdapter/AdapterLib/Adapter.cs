using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices.WindowsRuntime;
using BridgeRT;

namespace AdapterLib
{
    public sealed class Adapter : IAdapter
    {
        private const uint ERROR_SUCCESS = 0;
        private const uint ERROR_INVALID_HANDLE = 6;

        // Device Arrival and Device Removal Signal Indices
        private const int DEVICE_ARRIVAL_SIGNAL_INDEX = 0;
        private const int DEVICE_ARRIVAL_SIGNAL_PARAM_INDEX = 0;
        private const int DEVICE_REMOVAL_SIGNAL_INDEX = 1;
        private const int DEVICE_REMOVAL_SIGNAL_PARAM_INDEX = 0;

        public string Vendor { get; }

        public string AdapterName { get; }

        public string Version { get; }

        public string ExposedAdapterPrefix { get; }

        public string ExposedApplicationName { get; }

        public Guid ExposedApplicationGuid { get; }

        public IList<IAdapterSignal> Signals { get; }

        public Adapter()
        {
            Windows.ApplicationModel.Package package = Windows.ApplicationModel.Package.Current;
            Windows.ApplicationModel.PackageId packageId = package.Id;
            Windows.ApplicationModel.PackageVersion versionFromPkg = packageId.Version;

            this.Vendor = "$safeusername$";
            this.AdapterName = "$saferootprojectname$";

            // the adapter prefix must be something like "com.mycompany" (only alpha num and dots)
            // it is used by the Device System Bridge as root string for all services and interfaces it exposes
            this.ExposedAdapterPrefix = "com." + this.Vendor.ToLower();
            this.ExposedApplicationGuid = Guid.Parse("$appguid$");

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
                this.ExposedApplicationName = "DeviceSystemBridge";
                this.Version = "0.0.0.0";
            }

            try
            {
                this.Signals = new List<IAdapterSignal>();
                this.devices = new List<IAdapterDevice>();
                this.signalListeners = new Dictionary<int, IList<SIGNAL_LISTENER_ENTRY>>();

                //Create Adapter Signals
                this.createSignals();
            }
            catch (OutOfMemoryException ex)
            {
                throw;
            }
        }

        public uint SetConfiguration([ReadOnlyArray] byte[] ConfigurationData)
        {
            return ERROR_SUCCESS;
        }

        public uint GetConfiguration(out byte[] ConfigurationDataPtr)
        {
            ConfigurationDataPtr = null;

            return ERROR_SUCCESS;
        }

        public uint Initialize()
        {
            return ERROR_SUCCESS;
        }

        public uint Shutdown()
        {
            return ERROR_SUCCESS;
        }

        public uint EnumDevices(
            ENUM_DEVICES_OPTIONS Options,
            out IList<IAdapterDevice> DeviceListPtr,
            out IAdapterIoRequest RequestPtr)
        {
            RequestPtr = null;

            try
            {
                DeviceListPtr = new List<IAdapterDevice>(this.devices);
            }
            catch (OutOfMemoryException ex)
            {
                throw;
            }

            return ERROR_SUCCESS;
        }

        public uint GetProperty(
            IAdapterProperty Property,
            out IAdapterIoRequest RequestPtr)
        {
            RequestPtr = null;

            return ERROR_SUCCESS;
        }

        public uint SetProperty(
            IAdapterProperty Property,
            out IAdapterIoRequest RequestPtr)
        {
            RequestPtr = null;

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

            // find corresponding attribute
            foreach(var attribute in ((AdapterProperty)Property).Attributes)
            {
                if(attribute.Value.Name == AttributeName)
                {
                    ValuePtr = attribute.Value;
                    return ERROR_SUCCESS;
                }
            }

            return ERROR_INVALID_HANDLE;
        }

        public uint SetPropertyValue(
            IAdapterProperty Property,
            IAdapterValue Value,
            out IAdapterIoRequest RequestPtr)
        {
            RequestPtr = null;

            // find corresponding attribute
            foreach (var attribute in ((AdapterProperty)Property).Attributes)
            {
                if (attribute.Value.Name == Value.Name)
                {
                    attribute.Value.Data = Value.Data;
                    return ERROR_SUCCESS;
                }
            }

            return ERROR_INVALID_HANDLE;
        }

        public uint CallMethod(
            IAdapterMethod Method,
            out IAdapterIoRequest RequestPtr)
        {
            RequestPtr = null;

            return ERROR_SUCCESS;
        }

        public uint RegisterSignalListener(
            IAdapterSignal Signal,
            IAdapterSignalListener Listener,
            object ListenerContext)
        {
            if (Signal == null || Listener == null)
            {
                return ERROR_INVALID_HANDLE;
            }

            int signalHashCode = Signal.GetHashCode();

            SIGNAL_LISTENER_ENTRY newEntry;
            newEntry.Signal = Signal;
            newEntry.Listener = Listener;
            newEntry.Context = ListenerContext;

            lock (this.signalListeners)
            {
                if (this.signalListeners.ContainsKey(signalHashCode))
                {
                    this.signalListeners[signalHashCode].Add(newEntry);
                }
                else
                {
                    IList<SIGNAL_LISTENER_ENTRY> newEntryList;

                    try
                    {
                        newEntryList = new List<SIGNAL_LISTENER_ENTRY>();
                    }
                    catch (OutOfMemoryException ex)
                    {
                        throw;
                    }

                    newEntryList.Add(newEntry);
                    this.signalListeners.Add(signalHashCode, newEntryList);
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

        public uint NotifySignalListener(IAdapterSignal Signal)
        {
            if (Signal == null)
            {
                return ERROR_INVALID_HANDLE;
            }

            int signalHashCode = Signal.GetHashCode();

            lock (this.signalListeners)
            {
                IList<SIGNAL_LISTENER_ENTRY> listenerList = this.signalListeners[signalHashCode];
                foreach (SIGNAL_LISTENER_ENTRY entry in listenerList)
                {
                    IAdapterSignalListener listener = entry.Listener;
                    object listenerContext = entry.Context;
                    listener.AdapterSignalHandler(Signal, listenerContext);
                }
            }

            return ERROR_SUCCESS;
        }

        public uint NotifyDeviceArrival(IAdapterDevice Device)
        {
            if (Device == null)
            {
                return ERROR_INVALID_HANDLE;
            }

            IAdapterSignal deviceArrivalSignal = this.Signals[DEVICE_ARRIVAL_SIGNAL_INDEX];
            IAdapterValue signalParam = deviceArrivalSignal.Params[DEVICE_ARRIVAL_SIGNAL_PARAM_INDEX];
            signalParam.Data = Device;
            this.NotifySignalListener(deviceArrivalSignal);

            return ERROR_SUCCESS;
        }

        public uint NotifyDeviceRemoval(IAdapterDevice Device)
        {
            if (Device == null)
            {
                return ERROR_INVALID_HANDLE;
            }

            IAdapterSignal deviceRemovalSignal = this.Signals[DEVICE_REMOVAL_SIGNAL_INDEX];
            IAdapterValue signalParam = deviceRemovalSignal.Params[DEVICE_REMOVAL_SIGNAL_PARAM_INDEX];
            signalParam.Data = Device;
            this.NotifySignalListener(deviceRemovalSignal);

            return ERROR_SUCCESS;
        }

        private void createSignals()
        {
            try
            {
                // Device Arrival Signal
                AdapterSignal deviceArrivalSignal = new AdapterSignal(Constants.DEVICE_ARRIVAL_SIGNAL);
                AdapterValue deviceHandle_arrival = new AdapterValue(
                                                            Constants.DEVICE_ARRIVAL__DEVICE_HANDLE,
                                                            null);
                deviceArrivalSignal.Params.Add(deviceHandle_arrival);

                // Device Removal Signal
                AdapterSignal deviceRemovalSignal = new AdapterSignal(Constants.DEVICE_REMOVAL_SIGNAL);
                AdapterValue deviceHandle_removal = new AdapterValue(
                                                            Constants.DEVICE_REMOVAL__DEVICE_HANDLE,
                                                            null);
                deviceRemovalSignal.Params.Add(deviceHandle_removal);

                // Add Signals to the Adapter Signals
                this.Signals.Add(deviceArrivalSignal);
                this.Signals.Add(deviceRemovalSignal);
            }
            catch (OutOfMemoryException ex)
            {
                throw;
            }
        }

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

        // List of Devices
        private IList<IAdapterDevice> devices;

        // A map of signal handle (object's hash code) and related listener entry
        private Dictionary<int, IList<SIGNAL_LISTENER_ENTRY>> signalListeners;
    }
}
