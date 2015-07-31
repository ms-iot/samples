// Copyright (c) Microsoft. All rights reserved.

using System;
using Windows.Devices.Gpio;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace PushButton
{
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            InitializeComponent();
            InitGPIO();
        }

        private void InitGPIO()
        {
            var gpio = GpioController.GetDefault();

            // Show an error if there is no GPIO controller
            if (gpio == null)
            {
                GpioStatus.Text = "There is no GPIO controller on this device.";
                return;
            }

            buttonPin = gpio.OpenPin(BUTTON_PIN);
            ledPin = gpio.OpenPin(LED_PIN);

            // Initialize LED to the OFF state by first writing a HIGH value
            // We write HIGH because the LED is wired in a active LOW configuration
            ledPin.Write(GpioPinValue.High); 
            ledPin.SetDriveMode(GpioPinDriveMode.Output);

            // Check if input pull-up resistors are supported
            if (buttonPin.IsDriveModeSupported(GpioPinDriveMode.InputPullUp))
                buttonPin.SetDriveMode(GpioPinDriveMode.InputPullUp);
            else
                buttonPin.SetDriveMode(GpioPinDriveMode.Input);

            // Set a debounce timeout to filter out switch bounce noise from a button press
            buttonPin.DebounceTimeout = TimeSpan.FromMilliseconds(50);

            // Register for the ValueChanged event so our buttonPin_ValueChanged 
            // function is called when the button is pressed
            buttonPin.ValueChanged += buttonPin_ValueChanged;

            GpioStatus.Text = "GPIO pins initialized correctly.";
        }

        private void buttonPin_ValueChanged(GpioPin sender, GpioPinValueChangedEventArgs e)
        {
            // toggle the state of the LED every time the button is pressed
            if (e.Edge == GpioPinEdge.FallingEdge)
            {
                ledPinValue = (ledPinValue == GpioPinValue.Low) ?
                    GpioPinValue.High : GpioPinValue.Low;
                ledPin.Write(ledPinValue);
            }

            // need to invoke UI updates on the UI thread because this event
            // handler gets invoked on a separate thread.
            var task = Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => {
                if (e.Edge == GpioPinEdge.FallingEdge)
                {
                    ledEllipse.Fill = (ledPinValue == GpioPinValue.Low) ? 
                        redBrush : grayBrush;
                    GpioStatus.Text = "Button Pressed";
                }
                else
                {
                    GpioStatus.Text = "Button Released";
                }
            });
        }

        private const int LED_PIN = 6;
        private const int BUTTON_PIN = 5;
        private GpioPin ledPin;
        private GpioPin buttonPin;
        private GpioPinValue ledPinValue = GpioPinValue.High;
        private SolidColorBrush redBrush = new SolidColorBrush(Windows.UI.Colors.Red);
        private SolidColorBrush grayBrush = new SolidColorBrush(Windows.UI.Colors.LightGray);
    }
}
