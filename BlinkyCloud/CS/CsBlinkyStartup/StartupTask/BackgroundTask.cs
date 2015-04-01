// © Copyright (C) Microsoft. All rights reserved.

using System;
using Windows.ApplicationModel.Background;
using Windows.Devices.Gpio;
using Windows.System.Threading;
using System.Diagnostics;
using Microsoft.IoT.Maker;

namespace StartupTask
{
    public sealed class BackgroundTask : IBackgroundTask
    {
        MakerClient svc = new MakerClient();
        BackgroundTaskDeferral _deferral;
        public void Run(IBackgroundTaskInstance taskInstance)
        {
            _deferral = taskInstance.GetDeferral();
            timer = ThreadPoolTimer.CreatePeriodicTimer(Timer_Tick, TimeSpan.FromMilliseconds(1000));
            InitGPIO();

            svc.CommandReceived += OnCommand;      // Handle commands from cloud

            svc.SendEvent("BlinkyStart", "");      // Send event to cloud
        }

        private async void InitGPIO()
        {
            var gpio = await GpioController.GetDefaultAsync();

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
            if (LEDStatus == 0)
            {
                LEDStatus = 1;
                if (pin != null)
                    pin.Write(GpioPinValue.High);
            }
            else
            {
                LEDStatus = 0;
                if (pin != null)
                    pin.Write(GpioPinValue.Low);
            }
            Debug.WriteLine("LEDStatus=" + LEDStatus);

            svc.SendData("LEDStatus", LEDStatus);    // Upload LEDStatus to cloud
        }

        private void OnCommand(object sender, CommandReceivedEventArgs e)
        {
            switch (e.CommandName)
            {
                case "BlinkOn":
                    if (timer == null)
                        timer = ThreadPoolTimer.CreatePeriodicTimer(Timer_Tick, TimeSpan.FromMilliseconds(1000));
                    break;
                case "BlinkOff":
                    if (timer != null)
                        timer.Cancel();
                    timer = null;
                    break;
            }
        }

        private int LEDStatus = 0;
        private const int LED_PIN = 0;
        private GpioPin pin;
        private ThreadPoolTimer timer = null;
    }
}
