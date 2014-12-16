using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;

using Windows.System.Threading;
using Windows.Devices.Enumeration;
using Windows.Devices.Gpio;

namespace StartupTask
{
    public sealed class BackgroundTask : IBackgroundTask
    {
        BackgroundTaskDeferral _deferral;
        public void Run(IBackgroundTaskInstance taskInstance)
        {
            _deferral = taskInstance.GetDeferral();

            timer = ThreadPoolTimer.CreatePeriodicTimer(Timer_Tick, TimeSpan.FromMilliseconds(500));
            InitGPIO();
        }

        private async void InitGPIO()
        {
            var deviceId = GpioController.GetDeviceSelector("GPIO_S5");
            var deviceInfos = await DeviceInformation.FindAllAsync(deviceId, null);
            var controller = await GpioController.FromIdAsync(deviceInfos[0].Id);
            GpioPinInfo pinInfo;
            controller.Pins.TryGetValue(0, out pinInfo);
            pinInfo.TryOpenOutput(GpioPinValue.Low, GpioSharingMode.Exclusive, out outPin);
        }

        private void Timer_Tick(ThreadPoolTimer timer)
        {
            if (outPin != null)
            {
                if (LEDStatus == 0)
                {
                    LEDStatus = 1;
                    outPin.Value = GpioPinValue.High;
                }
                else
                {
                    LEDStatus = 0;
                    outPin.Value = GpioPinValue.Low;
                }
            }
        }

        private int LEDStatus = 0;
        private GpioOutputPin outPin;
        private ThreadPoolTimer timer;
    }
}
