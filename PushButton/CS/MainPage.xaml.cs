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
using Windows.Devices.Gpio;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Media;

namespace PushButton
{
    public sealed partial class MainPage : Page
    {
        private GpioPinValue pushButtonValue;
        public MainPage()
        {
            InitializeComponent();

            timer = new DispatcherTimer();
            timer.Interval = TimeSpan.FromMilliseconds(500);
            timer.Tick += Timer_Tick;
            timer.Start();

            Unloaded += MainPage_Unloaded;

            InitGPIO();
        }

        private void InitGPIO()
        {
            var gpio = GpioController.GetDefault();

            // Show an error if there is no GPIO controller
            if (gpio == null)
            {
                pin = null;
                GpioStatus.Text = "There is no GPIO controller on this device.";
                return;
            }
            pushButton = gpio.OpenPin(PB_PIN);
            pin = gpio.OpenPin(LED_PIN);

            // Show an error if the pin wasn't initialized properly
            if (pin == null)
            {
                GpioStatus.Text = "There were problems initializing the GPIO LED pin.";
                return;
            }
            if (pushButton == null)
            {
                GpioStatus.Text = "There were problems initializing the GPIO Push Button pin.";
                return;
            }

            pushButton.SetDriveMode(GpioPinDriveMode.Input);
            pin.SetDriveMode(GpioPinDriveMode.Output);

            GpioStatus.Text = "GPIO pin initialized correctly.";
        }

        private void MainPage_Unloaded(object sender, object args)
        {
            // Cleanup
            pin.Dispose();
            pushButton.Dispose();
        }

        private void FlipLED()
        {
            pushButtonValue = pushButton.Read();
            if (pushButtonValue == GpioPinValue.High)
            {
                LED.Fill = redBrush;
                pin.Write(GpioPinValue.High);
            }
            else if (pushButtonValue == GpioPinValue.Low)
            {
                LED.Fill = grayBrush;
                pin.Write(GpioPinValue.Low);
            }
        }

       

       private void Timer_Tick(object sender, object e)
        {
            FlipLED();
        }

       
        /// <summary>
        /// 
        /// </summary>
        private const int LED_PIN = 27;
        private const int PB_PIN = 5;
        private GpioPin pin;
        private GpioPin pushButton;
        private DispatcherTimer timer;
        private SolidColorBrush redBrush = new SolidColorBrush(Windows.UI.Colors.Red);
        private SolidColorBrush grayBrush = new SolidColorBrush(Windows.UI.Colors.LightGray);
    }
}
