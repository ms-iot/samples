// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.Devices.Gpio;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace GpioInterrupt
{
    public sealed class StartupTask : IBackgroundTask
    {
        BackgroundTaskDeferral deferral;
        GpioPin pin;

        public void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();

            pin = GpioController.GetDefault().OpenPin(6);
            //Ignore changes in value of less than 50ms
            pin.DebounceTimeout = new TimeSpan(0, 0, 0, 0, 50);                   
            pin.SetDriveMode(GpioPinDriveMode.Input);
            pin.ValueChanged += Pin_ValueChanged;
        }

        private void Pin_ValueChanged(GpioPin sender, GpioPinValueChangedEventArgs args)
        {
            System.Diagnostics.Debug.WriteLine("New pin value: " + args.Edge.ToString());
        }
    }
}
