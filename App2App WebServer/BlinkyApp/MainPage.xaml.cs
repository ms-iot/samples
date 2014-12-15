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
using Windows.ApplicationModel.AppService;
using Windows.Devices.Gpio;
using Windows.Foundation.Collections;
using Windows.UI.Core;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace BlinkyWebService
{
    public sealed partial class MainPage : Page
    {
        AppServiceConnection _appServiceConnection;

        public MainPage()
        {
            InitializeComponent();
            InitGPIO();
            InitAppSvc();
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

            pin = gpio.OpenPin(LED_PIN);

            // Show an error if the pin wasn't initialized properly
            if (pin == null)
            {
                GpioStatus.Text = "There were problems initializing the GPIO pin.";
                return;
            }

            pin.Write(GpioPinValue.High);
            pin.SetDriveMode(GpioPinDriveMode.Output);

            GpioStatus.Text = "GPIO pin initialized correctly.";
        }

        private async void InitAppSvc()
        {
            // Initialize the AppServiceConnection
            _appServiceConnection = new AppServiceConnection();
            _appServiceConnection.PackageFamilyName = "WebServer_hz258y3tkez3a";
            _appServiceConnection.AppServiceName = "App2AppComService";

            // Send a initialize request 
            var res = await _appServiceConnection.OpenAsync();
            if (res == AppServiceConnectionStatus.Success)
            {
                var message = new ValueSet();
                message.Add("Command", "Initialize");
                var response = await _appServiceConnection.SendMessageAsync(message);
                if (response.Status != AppServiceResponseStatus.Success)
                {
                    throw new Exception("Failed to send message");
                }
                _appServiceConnection.RequestReceived += OnMessageReceived;
            }
        }

        private async void OnMessageReceived(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
        {
            var message = args.Request.Message;
            string newState = message["State"] as string;
            switch (newState)
            {
                case "On":
                    {
                        await Dispatcher.RunAsync(
                              CoreDispatcherPriority.High,
                             () =>
                             {
                                 TurnOnLED();
                             }); 
                        break;
                    }
                case "Off":
                    {
                        await Dispatcher.RunAsync(
                        CoreDispatcherPriority.High,
                        () =>
                        {
                            TurnOffLED();
                         });
                        break;
                    }
                case "Unspecified":
                default:
                    {
                        // Do nothing 
                        break;
                    }
            }
        }

        private void FlipLED()
        {
            if (LEDStatus == 0)
            {
                LEDStatus = 1;
                if (pin != null)
                {
                    // to turn on the LED, we need to push the pin 'low'
                    pin.Write(GpioPinValue.Low);
                }
                LED.Fill = redBrush;
                StateText.Text = "On";
            }
            else
            {
                LEDStatus = 0;
                if (pin != null)
                {
                    pin.Write(GpioPinValue.High);
                }
                LED.Fill = grayBrush;
                StateText.Text = "Off"; 
            }
        }

        private void TurnOffLED()
        {
            if (LEDStatus == 1)
            {
                FlipLED();
            }
        }
        private void TurnOnLED()
        {
            if (LEDStatus == 0)
            {
                FlipLED();
            }
        }

        private int LEDStatus = 0;
        private const int LED_PIN = 5;
        private GpioPin pin;
        private SolidColorBrush redBrush = new SolidColorBrush(Windows.UI.Colors.Red);
        private SolidColorBrush grayBrush = new SolidColorBrush(Windows.UI.Colors.LightGray);
    }
}
