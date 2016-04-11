// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
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
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;
using IoTCoreDefaultApp.Utils;

namespace IoTCoreDefaultApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class TutorialMainPage : Page
    {
        private DispatcherTimer timer;

        public TutorialMainPage()
        {
            this.InitializeComponent();

#if !ALWAYS_SHOW_BLINKY
            if (DeviceTypeInformation.Type != DeviceTypes.RPI2 && DeviceTypeInformation.Type != DeviceTypes.DB410)
            {
                TutorialList.Items.Remove(HelloBlinkyGridViewItem);
            }
#endif
            this.NavigationCacheMode = NavigationCacheMode.Enabled;

            this.DataContext = LanguageManager.GetInstance();

            this.Loaded += (sender, e) =>
            {
                UpdateBoardInfo();
                UpdateDateTime();

                timer = new DispatcherTimer();
                timer.Tick += timer_Tick;
                timer.Interval = TimeSpan.FromSeconds(30);
                timer.Start();
            };
            this.Unloaded += (sender, e) =>
            {
                timer.Stop();
                timer = null;
            };
        }

        private void timer_Tick(object sender, object e)
        {
            UpdateDateTime();
        }

        private void UpdateBoardInfo()
        {
            if (DeviceTypeInformation.Type == DeviceTypes.DB410)
            {
                TutorialsImage.Source = new BitmapImage(new Uri("ms-appx:///Assets/DB410-tutorials.png"));
                GetStartedImage.Source = new BitmapImage(new Uri("ms-appx:///Assets/Tutorials/GetStarted-DB410.jpg"));
                HelloBlinkyTileImage.Source = new BitmapImage(new Uri("ms-appx:///Assets/Tutorials/HelloBlinkyTile-DB410.jpg"));
                GetConnectedImage.Source = new BitmapImage(new Uri("ms-appx:///Assets/Tutorials/GetConnected-DB410.jpg"));
            }
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

        private void DeviceInfo_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(MainPage));
        }

        private void TutorialList_ItemClick(object sender, ItemClickEventArgs e)
        {
            var item = e.ClickedItem as FrameworkElement;
            if (item == null)
            {
                return;
            }
            switch (item.Name)
            {
                case "HelloBlinky":
                    NavigationUtils.NavigateToScreen(typeof(TutorialHelloBlinkyPage), item.Name);
                    break;
                default:
                    NavigationUtils.NavigateToScreen(typeof(TutorialContentPage), item.Name);
                    break;
            }
        }
    }
}
