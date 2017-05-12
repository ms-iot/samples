// Copyright (c) Microsoft. All rights reserved.
// Project derived by Marius Zaharia - lecampusazure.net

using System;
using Windows.Devices.Gpio;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Media;
using System.Threading.Tasks;

namespace Blinky
{
    public sealed partial class MainPage : Page
    {
        private const int LED_PIN = 47; // onboard OK LED
        private GpioPin pin;
        private GpioPinValue pinValue;
        private DispatcherTimer timer;
        private SolidColorBrush blackBrush = new SolidColorBrush(Windows.UI.Colors.Black);
        private SolidColorBrush greenBrush = new SolidColorBrush(Windows.UI.Colors.GreenYellow);

        public MainPage()
        {
            InitializeComponent();

            timer = new DispatcherTimer();
            timer.Interval = TimeSpan.FromMilliseconds(10000);
            timer.Tick += Timer_Tick;
            InitGPIO();

            if (pin != null)
            {
                timer.Start();
                CallSOS();
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
            pinValue = GpioPinValue.High;
            pin.Write(pinValue);
            pin.SetDriveMode(GpioPinDriveMode.Output);

            GpioStatus.Text = "GPIO pin initialized correctly.";

        }


        private void Timer_Tick(object sender, object e)
        {
            CallSOS();
        }



        private async Task CallSOS()
        {
       
            await LetterS();

            await PauseBetweenLetters();

            await LetterO();

            await PauseBetweenLetters();

            await LetterS();

            await PauseBetweenLetters();
        }

        private async Task PauseBetweenLetters()
        {
            // pause between letters
            LetterText.Text = "";

            await Task.Delay(600);

        }

        private async Task LetterO()
        {
            // "O" : 3 "lines" (long on + short off)
            LetterText.Text = "O";

            for (int i = 0; i < 3; i++)
            {
                BlinkOn();
                await Task.Delay(600);
                BlinkOff();
                await Task.Delay(200);
            }
        }

        private async Task LetterS()
        {
            // "S" : 3 "points" (shorts)
            LetterText.Text = "S";

            for (int i = 0; i < 3; i++)
            {
                BlinkOn();
                await Task.Delay(150);
                BlinkOff();
                await Task.Delay(150);
            }
        }

        private void BlinkOn()
        {
            pinValue = GpioPinValue.High;
            pin.Write(pinValue);
            LED.Fill = greenBrush;
        }

        private void BlinkOff()
        {
            pinValue = GpioPinValue.Low;
            pin.Write(pinValue);
            LED.Fill = blackBrush;
        }

}
}
