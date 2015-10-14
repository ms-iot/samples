// Copyright (c) Microsoft. All rights reserved.

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
using Windows.Devices.Gpio;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace ShiftRegister
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        // use these constants for controlling how the I2C bus is setup
        private const double TIMER_INTERVAL = 100; // value is milliseconds and denotes the timer interval
        private const double TIME_DELAY = 1;

        private const int SRCLK_PIN = 18; // GPIO 18 is pin 12 on RPI2 header
        private GpioPin shiftRegisterClock;
        private const int SER_PIN = 27; // GPIO 27 is pin 13 on the RPI2 header
        private GpioPin serial;
        private const int RCLK_PIN = 5; // GPIO 5 is pin 29 on RPI2 header
        private GpioPin registerClock;
        private const int OE_PIN = 6; // GPIO 6 is pin 31 on RPI2 header
        private GpioPin outputEnable;
        private const int SRCLR_PIN = 12; // GPIO 12 is pin 32 on RPI2 header
        private GpioPin shiftRegisterClear;
        private DispatcherTimer timer;
        private byte pinMask = 0x01;
        private bool areLedsInverted = true;

        private SolidColorBrush redBrush = new SolidColorBrush(Windows.UI.Colors.Red);
        private SolidColorBrush grayBrush = new SolidColorBrush(Windows.UI.Colors.LightGray);

        public MainPage()
        {
            this.InitializeComponent();

            // Register for the unloaded event so we can clean up upon exit 
            Unloaded += MainPage_Unloaded;

            InitializeSystem();
        }

        private void InitializeSystem()
        {
            // initialize the GPIO pins we will use for bit-banging our serial data to the shift register
            var gpio = GpioController.GetDefault();

            // Show an error if there is no GPIO controller
            if (gpio == null)
            {
                GpioStatus.Text = "There is no GPIO controller on this device.";
                return;
            }

            shiftRegisterClock = gpio.OpenPin(SRCLK_PIN);
            serial = gpio.OpenPin(SER_PIN);
            registerClock = gpio.OpenPin(RCLK_PIN);
            outputEnable = gpio.OpenPin(OE_PIN);
            shiftRegisterClear = gpio.OpenPin(SRCLR_PIN);

            // reset the pins to a known state
            shiftRegisterClock.Write(GpioPinValue.Low);
            shiftRegisterClock.SetDriveMode(GpioPinDriveMode.Output);
            serial.Write(GpioPinValue.Low);
            serial.SetDriveMode(GpioPinDriveMode.Output);
            registerClock.Write(GpioPinValue.Low);
            registerClock.SetDriveMode(GpioPinDriveMode.Output);
            outputEnable.Write(GpioPinValue.Low);
            outputEnable.SetDriveMode(GpioPinDriveMode.Output);
            shiftRegisterClear.Write(GpioPinValue.Low);
            shiftRegisterClear.SetDriveMode(GpioPinDriveMode.Output);

            // clear out the shift register to ensure a clean start            
            // the shift register has a minimum setup time of 200ns 
            // don't really need to delay here but to really ensure this and
            // for working with slower shift registers
            registerClock.Write(GpioPinValue.High);
            registerClock.Write(GpioPinValue.Low);
            shiftRegisterClear.Write(GpioPinValue.High);

            GpioStatus.Text = "GPIO pin initialized correctly.";

            try
            {
                timer = new DispatcherTimer();
                timer.Interval = TimeSpan.FromMilliseconds(TIMER_INTERVAL);
                timer.Tick += Timer_Tick;
                timer.Start();
            }
            catch (Exception e)
            {
                System.Diagnostics.Debug.WriteLine("Exception: {0}", e.Message);
                return;
            }
        }

        private void MainPage_Unloaded(object sender, object args)
        {
            // Cleanup
            shiftRegisterClock.Dispose();
            serial.Dispose();
            registerClock.Dispose();
            outputEnable.Dispose();
            shiftRegisterClear.Dispose();
        }

        private void SendDataBit()
        {
            if ((pinMask & 0x80) > 0)
            {
                serial.Write(GpioPinValue.High);
                shiftRegisterClock.Write(GpioPinValue.High);
                shiftRegisterClock.Write(GpioPinValue.Low);
                registerClock.Write(GpioPinValue.High);
                registerClock.Write(GpioPinValue.Low);
            }
            else
            {
                serial.Write(GpioPinValue.Low);
                shiftRegisterClock.Write(GpioPinValue.High);
                shiftRegisterClock.Write(GpioPinValue.Low);
                registerClock.Write(GpioPinValue.High);
                registerClock.Write(GpioPinValue.Low);
            }

            pinMask <<= 1;
            if (areLedsInverted)
            {
                if (pinMask == 0)
                {
                    pinMask = 0x01;
                }
            }
            else
            {
                pinMask |= 0x01;
                if (pinMask == 0xFF)
                {
                    pinMask &= 0xFE;
                }
            }

        }

        private void Timer_Tick(object sender, object e)
        {
            SendDataBit();
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
            }
            else
            {
                DelayText.Text = e.NewValue + "ms";
                timer.Interval = TimeSpan.FromMilliseconds(e.NewValue);
                timer.Start();
            }
        }

        private void ToggleButtonClicked(object sender, RoutedEventArgs e)
        {
            pinMask ^= 0xFF;
            if (areLedsInverted)
            {
                areLedsInverted = false;
                ToggleButton.Background = grayBrush;
            }
            else
            {
                areLedsInverted = true;
                ToggleButton.Background = redBrush;
            }
        }
    }
}
