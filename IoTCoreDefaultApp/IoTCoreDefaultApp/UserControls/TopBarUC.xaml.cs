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
using Windows.UI.Xaml.Navigation;

namespace IoTCoreDefaultApp
{
    public sealed partial class TopBarUC : UserControl
    {
        private DispatcherTimer timer;

        public TopBarUC()
        {
            this.InitializeComponent();

            timer = new DispatcherTimer();
            timer.Tick += timer_Tick;
            timer.Interval = TimeSpan.FromSeconds(30);

            this.Loaded += async (sender, e) =>
            {
                await Dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
                {
                    UpdateDateTime();

                    timer.Start();
                });
            };
            this.Unloaded += (sender, e) =>
            {
                timer.Stop();
            };

        }
        
        public string ScreenName
        {
            get { return (string)GetValue(ScreenNameProperty); }
            set { SetValue(ScreenNameProperty, value);

                ClearBackground();

                switch (ScreenName)
                {
                    case "DeviceInfo":
                        DeviceInfo.Background = this.Resources["AccentBrush"] as Brush;
                        break;
                    case "CommandLine":
                        CommandLine.Background = this.Resources["AccentBrush"] as Brush;
                        break;
                    case "Browser":
                        WebBrowser.Background = this.Resources["AccentBrush"] as Brush;
                        break;
                    case "TutorialMain":
                        Tutorials.Background = this.Resources["AccentBrush"] as Brush;
                        break;
                    default:
                        break;
                }
            }
        }

        // Using a DependencyProperty as the backing store for ScreenName.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty ScreenNameProperty =
            DependencyProperty.Register("ScreenName", typeof(string), typeof(TopBarUC), new PropertyMetadata(0));


        private void UpdateDateTime()
        {
            var t = DateTime.Now;
            this.CurrentTime.Text = t.ToString("t", CultureInfo.CurrentCulture) + Environment.NewLine + t.ToString("d", CultureInfo.CurrentCulture);
        }

        private void ClearBackground()
        {
            WebBrowser.Background = this.Resources["TransparentBrush"] as Brush;
            DeviceInfo.Background = this.Resources["TransparentBrush"] as Brush;
            CommandLine.Background = this.Resources["TransparentBrush"] as Brush;
            Tutorials.Background = this.Resources["TransparentBrush"] as Brush;
        }

        private void ShutdownHelper(ShutdownKind kind)
        {
            ShutdownManager.BeginShutdown(kind, TimeSpan.FromSeconds(0.5));
        }


        #region TopBar Button Events

        private void timer_Tick(object sender, object e)
        {
            UpdateDateTime();
        }

        private void DeviceInfo_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(MainPage));
        }

        private void CommandLineButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(CommandLinePage));
        }

        private void WebBrowserButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(WebBrowserPage));
        }

        private void Tutorials_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(TutorialMainPage));
        }

        private void SettingsButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(Settings));
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

        #endregion TopBar Button Events

    }
}
