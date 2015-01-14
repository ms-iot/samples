// © Copyright (C) Microsoft. All rights reserved.


using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.Devices.Enumeration;
using Windows.Devices.Gpio;

namespace Blinky
{
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            InitializeComponent();

            timer = new DispatcherTimer();
            timer.Interval = TimeSpan.FromMilliseconds(500);
            timer.Tick += Timer_Tick;
            timer.Start();

            InitGPIO();
        }

        private async void InitGPIO()
        {
            try
            {
                var deviceId = GpioController.GetDeviceSelector("GPIO_S5");
                var deviceInfos = await DeviceInformation.FindAllAsync(deviceId, null);
                var controller = await GpioController.FromIdAsync(deviceInfos[0].Id);
                GpioPinInfo pinInfo;
                controller.Pins.TryGetValue(0, out pinInfo);
                pinInfo.TryOpenOutput(GpioPinValue.High, GpioSharingMode.Exclusive, out outPin);
                GpioStatus.Text = "GPIO pin initialized correctly.";
            }
            catch (Exception)
            {
                GpioStatus.Text = "There were problems initializing the GPIO pin.";
            }
        }

        private void FlipLED()
        {
            if (LEDStatus == 0)
            {
                LEDStatus = 1;
                if (outPin != null)
                {
                    // to turn on the LED, we need to push the pin 'low'
                    outPin.Value = GpioPinValue.Low;
                }
                LED.Fill = redBrush;
            }
            else
            {
                LEDStatus = 0;
                if (outPin != null)
                {
                    outPin.Value = GpioPinValue.High;
                }
                LED.Fill = grayBrush;
            }
        }

        private void TurnOffLED()
        {
            if (LEDStatus == 1)
            {
                FlipLED();
            }
        }

        private void Timer_Tick(object sender, object e)
        {
            FlipLED();
        }

        private void Delay_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {            
            if (timer == null)
            {
                return;
            }
            if (e.NewValue == Delay.Minimum)
            {
                DelayText.Text = "Stopped";
                timer.Stop();
                TurnOffLED();
            }
            else
            {
                DelayText.Text = e.NewValue + "ms";
                timer.Interval = TimeSpan.FromMilliseconds(e.NewValue);
                timer.Start();
            }
        }

        private int LEDStatus = 0;
        private GpioOutputPin outPin;
        private DispatcherTimer timer;
        private SolidColorBrush redBrush = new SolidColorBrush(Windows.UI.Colors.Red);
        private SolidColorBrush grayBrush = new SolidColorBrush(Windows.UI.Colors.LightGray);
    }
}
