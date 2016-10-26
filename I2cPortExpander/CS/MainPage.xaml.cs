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
using Windows.Devices.Enumeration;
using Windows.Devices.I2c;
// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace I2cPortExpander
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        // use these constants for controlling how the I2C bus is setup
        private const byte PORT_EXPANDER_I2C_ADDRESS = 0x20; // 7-bit I2C address of the port expander
        private const byte PORT_EXPANDER_IODIR_REGISTER_ADDRESS = 0x00; // IODIR register controls the direction of the GPIO on the port expander
        private const byte PORT_EXPANDER_GPIO_REGISTER_ADDRESS = 0x09; // GPIO register is used to read the pins input
        private const byte PORT_EXPANDER_OLAT_REGISTER_ADDRESS = 0x0A; // Output Latch register is used to set the pins output high/low
        private const double TIMER_INTERVAL = 500; // value is milliseconds and denotes the timer interval
        private const double BUTTON_STATUS_CHECK_TIMER_INTERVAL = 50;

        private byte LED_GPIO_PIN = 0x01; // using GPIO pin 0 on the port expander for the LED
        private byte PUSHBUTTON_GPIO_PIN = 0x02; // using GPIO pin 1 on the port expander for reading the toggle button status

        private byte iodirRegister; // local copy of I2C Port Expander IODIR register
        private byte gpioRegister; // local copy of I2C Port Expander GPIO register
        private byte olatRegister; // local copy of I2C Port Expander OLAT register

        private I2cDevice i2cPortExpander;
        private DispatcherTimer ledTimer;
        private DispatcherTimer buttonStatusCheckTimer;

        private bool isLedOn = false;
        private bool isButtonPressed = false;

        private SolidColorBrush redBrush = new SolidColorBrush(Windows.UI.Colors.Red);
        private SolidColorBrush grayBrush = new SolidColorBrush(Windows.UI.Colors.LightGray);

        public MainPage()
        {
            this.InitializeComponent();

            // Register for the unloaded event so we can clean up upon exit 
            Unloaded += MainPage_Unloaded;

            InitializeSystem();
        }

        private async void InitializeSystem()
        {
            byte[] i2CWriteBuffer;
            byte[] i2CReadBuffer;
            byte bitMask;
            
          
            var i2cSettings = new I2cConnectionSettings(PORT_EXPANDER_I2C_ADDRESS);
            var controller = await I2cController.GetDefaultAsync();
            i2cPortExpander = controller.GetDevice(i2cSettings);


            // initialize I2C Port Expander registers
            try
            {
                // initialize local copies of the IODIR, GPIO, and OLAT registers
                i2CReadBuffer = new byte[1];

                // read in each register value on register at a time (could do this all at once but
                // for example clarity purposes we do it this way)
                i2cPortExpander.WriteRead(new byte[] { PORT_EXPANDER_IODIR_REGISTER_ADDRESS }, i2CReadBuffer);
                iodirRegister = i2CReadBuffer[0];

                i2cPortExpander.WriteRead(new byte[] { PORT_EXPANDER_GPIO_REGISTER_ADDRESS }, i2CReadBuffer);
                gpioRegister = i2CReadBuffer[0];

                i2cPortExpander.WriteRead(new byte[] { PORT_EXPANDER_OLAT_REGISTER_ADDRESS }, i2CReadBuffer);
                olatRegister = i2CReadBuffer[0];

                // configure the LED pin output to be logic high, leave the other pins as they are.
                olatRegister |= LED_GPIO_PIN;
                i2CWriteBuffer = new byte[] { PORT_EXPANDER_OLAT_REGISTER_ADDRESS, olatRegister };
                i2cPortExpander.Write(i2CWriteBuffer);

                // configure only the LED pin to be an output and leave the other pins as they are.
                // input is logic low, output is logic high
                bitMask = (byte)(0xFF ^ LED_GPIO_PIN); // set the LED GPIO pin mask bit to '0', all other bits to '1'
                iodirRegister &= bitMask;
                i2CWriteBuffer = new byte[] { PORT_EXPANDER_IODIR_REGISTER_ADDRESS, iodirRegister };
                i2cPortExpander.Write(i2CWriteBuffer);

            }
            catch (Exception e)
            {
                ButtonStatusText.Text = "Failed to initialize I2C port expander: " + e.Message;
                return;
            }

            // setup our timers, one for the LED blink interval, the other for checking button status
            
            ledTimer = new DispatcherTimer();
            ledTimer.Interval = TimeSpan.FromMilliseconds(TIMER_INTERVAL);
            ledTimer.Tick += LedTimer_Tick;
            ledTimer.Start();

            buttonStatusCheckTimer = new DispatcherTimer();
            buttonStatusCheckTimer.Interval = TimeSpan.FromMilliseconds(BUTTON_STATUS_CHECK_TIMER_INTERVAL);
            buttonStatusCheckTimer.Tick += ButtonStatusCheckTimer_Tick;
            buttonStatusCheckTimer.Start();
        }

        private void MainPage_Unloaded(object sender, object args)
        {
            /* Cleanup */
            i2cPortExpander.Dispose();
        }

        private void FlipLED()
        {
            byte bitMask;
            if (isLedOn == true)
            {
                // turn off the LED
                isLedOn = false;
                olatRegister |= LED_GPIO_PIN;
                i2cPortExpander.Write(new byte[] { PORT_EXPANDER_OLAT_REGISTER_ADDRESS, olatRegister });
                Led.Fill = grayBrush;
            }
            else
            {
                // turn on the LED
                isLedOn = true;
                bitMask = (byte)(0xFF ^ LED_GPIO_PIN);
                olatRegister &= bitMask;
                i2cPortExpander.Write(new byte[] { PORT_EXPANDER_OLAT_REGISTER_ADDRESS, olatRegister });
                Led.Fill = redBrush;
            }
        }

        private void TurnOffLED()
        {
            isLedOn = false;
            olatRegister |= LED_GPIO_PIN;
            i2cPortExpander.Write(new byte[] { PORT_EXPANDER_OLAT_REGISTER_ADDRESS, olatRegister });
            Led.Fill = grayBrush;
        }

        private void CheckButtonStatus()
        {
            byte[] readBuffer = new byte[1];
            i2cPortExpander.WriteRead(new byte[] { PORT_EXPANDER_GPIO_REGISTER_ADDRESS }, readBuffer);

            // a button press results in a logic low for the GPIO pin
            if ((byte)(readBuffer[0] & PUSHBUTTON_GPIO_PIN) == 0x00)
            {
                ButtonStatusText.Text = "Button Status: Pressed";
                isButtonPressed = true;
            }
            else
            {
                ButtonStatusText.Text = "Button Status: Released";
                isButtonPressed = false;
            }
        }

        private void LedTimer_Tick(object sender, object e)
        {
            if (isButtonPressed == false)
            {
                FlipLED();
            }
        }

        private void ButtonStatusCheckTimer_Tick(object sender, object e)
        {
            CheckButtonStatus();
        }

        private void Delay_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            if (ledTimer == null)
            {
                return;
            }
            if (e.NewValue == Delay.Minimum)
            {
                DelayText.Text = "Stopped";
                ledTimer.Stop();
                TurnOffLED();
            }
            else
            {
                DelayText.Text = e.NewValue + "ms";
                ledTimer.Interval = TimeSpan.FromMilliseconds(e.NewValue);
                ledTimer.Start();
            }
        }
    }
}
