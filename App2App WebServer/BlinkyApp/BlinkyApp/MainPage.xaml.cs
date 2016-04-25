// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Threading.Tasks;
using Windows.Devices.Gpio;
using Windows.Storage;
using Windows.UI.Core;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace BlinkyWebService
{
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            InitializeComponent();
            InitGPIO();

            ApplicationData.Current.DataChanged += async (d, a) => await HandleDataChangedEvent(d, a);
        }

        private async Task HandleDataChangedEvent(ApplicationData data, object args)
        {
            try
            {
                var localSettings = ApplicationData.Current.LocalSettings;
                if (!localSettings.Values.ContainsKey("BlinkyState"))
                {
                    return;
                }

                string newState = localSettings.Values["BlinkyState"] as string;
                switch (newState)
                {
                    case "On":
                        if (LEDStatus == 0)
                        {
                            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => { FlipLED(); });
                        }
                        break;
                    case "Off":
                        if (LEDStatus == 1)
                        {
                            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => { FlipLED(); });
                        }
                        break;
                    case "Unspecified":
                    default:
                        // Do nothing 
                        break;
                }
            }
            catch (Exception)
            {
                // Do nothing
            }
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
            pin.Write(GpioPinValue.High);
            pin.SetDriveMode(GpioPinDriveMode.Output);

            GpioStatus.Text = "GPIO pin initialized correctly.";
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

        private int LEDStatus = 0;
        private const int LED_PIN = 5;
        private GpioPin pin;
        private SolidColorBrush redBrush = new SolidColorBrush(Windows.UI.Colors.Red);
        private SolidColorBrush grayBrush = new SolidColorBrush(Windows.UI.Colors.LightGray);
    }
}
