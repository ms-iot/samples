// © Copyright(C) Microsoft.All rights reserved.

using System;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Media;

namespace bertha
{
    public sealed partial class BlinkLED : Page
    {
        private DispatcherTimer Timer;
        private SolidColorBrush RedBrush = new SolidColorBrush(Colors.Red);
        private SolidColorBrush GrayBrush = new SolidColorBrush(Colors.LightGray);
        private LED Blinky;

        public BlinkLED()
        {
            InitializeComponent();
            try
            {
                Blinky = new LED();
                GpioStatus.Text = "GPIO pin initialized correctly.";
            }
            catch (Exception)
            {
                GpioStatus.Text = "There were problems initializing the GPIO pin.";
            }

            Timer = new DispatcherTimer();
            Timer.Interval = TimeSpan.FromMilliseconds(500);
            Timer.Tick += Timer_Tick;
            Timer.Start();
        }

        private void FlipLED(int status)
        {
            if (status == 0)
            {
                LED.Fill = GrayBrush;
            }
            else
            {
                LED.Fill = RedBrush;
            }
        }

        private void Timer_Tick(object sender, object e)
        {
            FlipLED(Blinky.FlipLED());
        }

        private void Delay_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
            if (Timer == null)
            {
                return;
            }

            if (e.NewValue == Delay.Minimum)
            {
                DelayText.Text = "Stopped";
                Timer.Stop();
                FlipLED(Blinky.TurnOffLED());
            }
            else
            {
                DelayText.Text = e.NewValue + "ms";
                Timer.Interval = TimeSpan.FromMilliseconds(e.NewValue);
                Timer.Start();
            }
        }

        private void BackButton_Tapped(object sender, Windows.UI.Xaml.Input.TappedRoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }
    }
}
