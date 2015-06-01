/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

    The MIT License(MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

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
