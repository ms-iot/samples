using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.Devices.Gpio;
using Windows.System.Threading;

namespace BlinkyHeadlessCS
{
    public sealed class StartupTask : IBackgroundTask
    {
        BackgroundTaskDeferral _deferral;
        private int LEDStatus = 0;
        private const int LED_PIN = 5;
        private GpioPin pin;
        private ThreadPoolTimer timer;

        public void Run(IBackgroundTaskInstance taskInstance)
        {
            _deferral = taskInstance.GetDeferral();

            timer = ThreadPoolTimer.CreatePeriodicTimer(Timer_Tick, TimeSpan.FromMilliseconds(500));
            InitGPIO();
        }
        private void InitGPIO()
        {
            var gpio = GpioController.GetDefault();

            if (gpio == null)
            {
                pin = null;
                return;
            }

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
    }
}
