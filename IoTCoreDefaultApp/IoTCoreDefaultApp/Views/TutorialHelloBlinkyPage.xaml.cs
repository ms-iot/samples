// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Devices.Gpio;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using IoTCoreDefaultApp.Utils;

namespace IoTCoreDefaultApp
{
    public sealed partial class TutorialHelloBlinkyPage : Page
    {
        private DispatcherTimer timer;
        private DispatcherTimer blinkyTimer;
        private int LEDStatus = 0;
        private readonly int LED_PIN = 47; // on-board LED on the Rpi2
        private GpioPin pin;
        private SolidColorBrush redBrush = new SolidColorBrush(Windows.UI.Colors.Red);
        private SolidColorBrush grayBrush = new SolidColorBrush(Windows.UI.Colors.LightGray);
        private string docName;
        private Windows.ApplicationModel.Resources.ResourceLoader loader;

        public TutorialHelloBlinkyPage()
        {
            this.InitializeComponent();

            if (DeviceTypeInformation.Type == DeviceTypes.DB410)
            {
                LED_PIN = 115; // on-board LED on the DB410c
            }

            var rootFrame = Window.Current.Content as Frame;
            rootFrame.Navigated += RootFrame_Navigated;
            Unloaded += MainPage_Unloaded;

            this.NavigationCacheMode = NavigationCacheMode.Enabled;

            this.DataContext = LanguageManager.GetInstance();

            this.Loaded += (sender, e) =>
            {
                UpdateDateTime();

                timer = new DispatcherTimer();
                timer.Tick += timer_Tick;
                timer.Interval = TimeSpan.FromSeconds(30);
                timer.Start();

                blinkyTimer = new DispatcherTimer();
                blinkyTimer.Interval = TimeSpan.FromMilliseconds(500);
                blinkyTimer.Tick += Timer_Tick;

                loader = new Windows.ApplicationModel.Resources.ResourceLoader();
                BlinkyStartStop.Content = loader.GetString("BlinkyStart");
            };
        }

        private void RootFrame_Navigated(object sender, NavigationEventArgs e)
        {
            docName = e.Parameter as string;
            if (docName != null)
            {
                NextButton.Visibility = (NavigationUtils.IsNextTutorialButtonVisible(docName) ? Visibility.Visible : Visibility.Collapsed);
            }
        }

        private void timer_Tick(object sender, object e)
        {
            UpdateDateTime();
        }

        private void UpdateDateTime()
        {
            var t = DateTime.Now;
            this.CurrentTime.Text = t.ToString("t", CultureInfo.CurrentCulture) + Environment.NewLine + t.ToString("d", CultureInfo.CurrentCulture);
        }

        private void ShutdownButton_Clicked(object sender, RoutedEventArgs e)
        {
            ShutdownDropdown.IsOpen = true;
        }

        private void ShutdownDropdown_Opened(object sender, object e)
        {
            var w = ShutdownListView.ActualWidth;
            if (w == 0)
            {
                // trick to recalculate the size of the dropdown
                ShutdownDropdown.IsOpen = false;
                ShutdownDropdown.IsOpen = true;
            }
            var offset = -(ShutdownListView.ActualWidth - ShutdownButton.ActualWidth);
            ShutdownDropdown.HorizontalOffset = offset;
        }

        private void ShutdownHelper(ShutdownKind kind)
        {
            ShutdownManager.BeginShutdown(kind, TimeSpan.FromSeconds(0.5));
        }

        private void ShutdownListView_ItemClick(object sender, ItemClickEventArgs e)
        {
            var item = e.ClickedItem as FrameworkElement;
            if (item == null)
            {
                return;
            }
            switch (item.Name)
            {
                case "ShutdownOption":
                    ShutdownHelper(ShutdownKind.Shutdown);
                    break;
                case "RestartOption":
                    ShutdownHelper(ShutdownKind.Restart);
                    break;
            }
        }

        private void SettingsButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(Settings));
        }

        private void BackButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }

        private void DeviceInfo_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(MainPage));
        }

        private void MainPage_Unloaded(object sender, object args)
        {
            Stop();
        }

        private void Start()
        {
            GpioController gpio = null;
            try
            {
                gpio = GpioController.GetDefault();
            }
            catch (Exception)
            {
                // the error will be handled below
            }

            // Show an error if there is no GPIO controller
            if (gpio == null)
            {
                pin = null;
                GpioStatus.Text = loader.GetString("NoGPIOController");
                return;
            }

            pin = gpio.OpenPin(LED_PIN);

            // Show an error if the pin wasn't initialized properly
            if (pin == null)
            {
                GpioStatus.Text = loader.GetString("ProblemsInitializingGPIOPin");
                return;
            }

            pin.Write(GpioPinValue.High);
            pin.SetDriveMode(GpioPinDriveMode.Output);

            GpioStatus.Text = loader.GetString("GPIOPinInitializedCorrectly");

            blinkyTimer.Start();
            BlinkyStartStop.Content = loader.GetString("BlinkyStop");
        }

        private void Stop()
        {
            TurnOffLED();
            blinkyTimer.Stop();
            if (pin != null)
            {
                pin.Dispose();
                pin = null;
            }
            BlinkyStartStop.Content = loader.GetString("BlinkyStart");
        }

        private void FlipLED()
        {
            if (LEDStatus == 0)
            {
                LEDStatus = 1;
                if (pin != null)
                {
                    pin.Write(GpioPinValue.High);
                }
                LED.Fill = redBrush;
            }
            else
            {
                LEDStatus = 0;
                if (pin != null)
                {
                    pin.Write(GpioPinValue.Low);
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
            if (blinkyTimer == null)
            {
                return;
            }
            if (e.NewValue == Delay.Minimum)
            {
                DelayText.Text = loader.GetString("BlinkyStopped");
                blinkyTimer.Stop();
                TurnOffLED();
            }
            else
            {
                DelayText.Text = e.NewValue + "ms";
                blinkyTimer.Interval = TimeSpan.FromMilliseconds(e.NewValue);
                blinkyTimer.Start();
            }
        }

        private void BlinkyStartStop_Click(object sender, RoutedEventArgs e)
        {
            if (pin == null)
            {
                Start();
            }
            else
            {
                Stop();
            }
        }

        private void NextButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToNextTutorialFrom(docName);
        }

    }
}
