using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.UI.Core;

namespace IoTCoreDefaultApp
{
    public class ConnectedDevicePresenter
    {
        private CoreDispatcher dispatcher;

        public ConnectedDevicePresenter(CoreDispatcher dispatcher)
        {
            this.dispatcher = dispatcher;
            var deviceClass = "(System.Devices.InterfaceClassGuid:=\"{" + Constants.GUID_DEVINTERFACE_USB_DEVICE + "}\")";

            usbConnectedDevicesWatcher = DeviceInformation.CreateWatcher(deviceClass);
            usbConnectedDevicesWatcher.Added += DevicesAdded;
            usbConnectedDevicesWatcher.Start();
        }

        private async void DevicesAdded(DeviceWatcher sender, DeviceInformation args)
        {
            await dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
            {
                devices.Add(args.Name);
            });
        }

        public ObservableCollection<string> GetConnectedDevices()
        {
            return devices;
        }

        private ObservableCollection<string> devices = new ObservableCollection<string>();
        private DeviceWatcher usbConnectedDevicesWatcher;
    }
}
