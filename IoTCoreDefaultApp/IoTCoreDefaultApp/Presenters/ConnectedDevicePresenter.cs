// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.UI.Core;
using System.Diagnostics;

namespace IoTCoreDefaultApp
{
    public class ConnectedDevicePresenter
    {
        private CoreDispatcher dispatcher;
        const string usbDevicesSelector = "(System.Devices.InterfaceClassGuid:=\"{" + Constants.GUID_DEVINTERFACE_USB_DEVICE + "}\")";

        public ConnectedDevicePresenter(CoreDispatcher dispatcher)
        {
            this.dispatcher = dispatcher;

            // Always start with a clean list                 
            devices.Clear();

            usbConnectedDevicesWatcher = DeviceInformation.CreateWatcher(usbDevicesSelector);
            usbConnectedDevicesWatcher.Added += DevicesAdded;
            usbConnectedDevicesWatcher.Removed += DevicesRemoved;
            usbConnectedDevicesWatcher.Updated += DevicesUpdated;
            usbConnectedDevicesWatcher.EnumerationCompleted += DevicesEnumCompleted;
            usbConnectedDevicesWatcher.Start();
        }

        private async void DevicesAdded(DeviceWatcher sender, DeviceInformation args)
        {
            Debug.WriteLine("USB Devices Added: " + args.Id);

            var device = new ConnectedDevice() { Id = args.Id, Name = args.Name };
            await dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
            {
                if (!devices.Contains(device))
                {
                    devices.Add(device);
                }
            });
        }

        private void DevicesEnumCompleted(DeviceWatcher sender, object args)
        {
            Debug.WriteLine("USB Devices Enumeration Completed");
        }

        private async void DevicesUpdated(DeviceWatcher sender, DeviceInformationUpdate args)
        {
            Debug.WriteLine("Updated USB device: " + args.Id);

            var deviceInfo = await DeviceInformation.CreateFromIdAsync(args.Id);
            var device = new ConnectedDevice() { Id = args.Id, Name = deviceInfo.Name };
            await dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
            {
                if (!deviceInfo.IsEnabled && devices.Contains(device))
                {
                    devices.Remove(device);
                }
                else if (deviceInfo.IsEnabled && !devices.Contains(device))
                {
                    devices.Add(device);
                }
            });
        }

        private async void DevicesRemoved(DeviceWatcher sender, DeviceInformationUpdate args)
        {
            Debug.WriteLine("Removed USB device: " + args.Id);

            var deviceInfo = await DeviceInformation.CreateFromIdAsync(args.Id);
            var device = new ConnectedDevice() { Id = args.Id, Name = deviceInfo.Name };
            await dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
            {
                if (devices.Contains(device))
                {
                    devices.Remove(device);
                }
            });
        }

        public ObservableCollection<ConnectedDevice> GetConnectedDevices()
        {
            return devices;
        }

        public class ConnectedDevice
        {
            public string Id { get; set; }
            public string Name { get; set; }

            public override int GetHashCode()
            {
                return Id.GetHashCode();
            }
            public override bool Equals(object other)
            {
                var otherConnectedDevice = other as ConnectedDevice;
                if (otherConnectedDevice == null)
                {
                    return false;
                }

                return otherConnectedDevice.Id.Equals(Id);
            }
            public override string ToString() { return Name; }
        }

        private ObservableCollection<ConnectedDevice> devices = new ObservableCollection<ConnectedDevice>();
        private DeviceWatcher usbConnectedDevicesWatcher;
    }
}
