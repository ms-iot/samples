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

            usbConnectedDevicesWatcher = DeviceInformation.CreateWatcher(usbDevicesSelector);
            usbConnectedDevicesWatcher.EnumerationCompleted += DevicesEnumCompleted;
            usbConnectedDevicesWatcher.Updated += DevicesUpdated;
            usbConnectedDevicesWatcher.Removed += DevicesRemoved;
            usbConnectedDevicesWatcher.Start();
        }

        private async void DevicesEnumCompleted(DeviceWatcher sender, object args)
        {
            Debug.WriteLine("USB Devices Enumeration Completed");

            await dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
            {
                UpdateDevices();
            });
        }

        private async void DevicesUpdated(DeviceWatcher sender, DeviceInformationUpdate args)
        {
            Debug.WriteLine("Updated USB device: " + args.Id);

            await dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
            {
                UpdateDevices();
            });
        }

        private async void DevicesRemoved(DeviceWatcher sender, DeviceInformationUpdate args)
        {
            Debug.WriteLine("Removed USB device: " + args.Id);

            await dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
            {
                UpdateDevices();
            });
        }

        private async void UpdateDevices()
        {
            // Get a list of all enumerated usb devices              
            var deviceInformationCollection = await DeviceInformation.FindAllAsync(usbDevicesSelector);

            // Always start with a clean list                 
            devices.Clear();  

            if (deviceInformationCollection == null || deviceInformationCollection.Count == 0)  
            {
                return;
            }

            // If devices are found, enumerate them and add only enabled ones
            foreach (var device in deviceInformationCollection)
            {
                if (device.IsEnabled)
                {
                    devices.Add(device.Name);
                }
            }
        }

        public ObservableCollection<string> GetConnectedDevices()
        {
            return devices;
        }

        private ObservableCollection<string> devices = new ObservableCollection<string>();
        private DeviceWatcher usbConnectedDevicesWatcher;
    }
}
