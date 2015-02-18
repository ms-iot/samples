// © Copyright (C) Microsoft. All rights reserved.


using System;
using Windows.ApplicationModel.Background;
using Windows.Devices.Gpio;
using Windows.System.Threading;

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
            var gpio = await GpioController.GetDefaultAsync();
            pin = gpio.OpenPin(LED_PIN);

            if (pin == null)
            {
                return;
            }

            pin.Write(GpioPinValue.High);
            pin.SetDriveMode(GpioPinDriveMode.Output);
        }

        private void Timer_Tick(ThreadPoolTimer timer)
        {
            if (pin != null)
            {
                if (LEDStatus == 0)
                {
                    LEDStatus = 1;
                    pin.Write(GpioPinValue.High);
                }
                else
                {
                    LEDStatus = 0;
                    pin.Write(GpioPinValue.Low);
                }
            }
        }

        private int LEDStatus = 0;
        private const int LED_PIN = 0;
        private GpioPin pin;
        private ThreadPoolTimer timer;
    }
}
