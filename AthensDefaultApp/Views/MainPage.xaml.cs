// © Copyright(C) Microsoft. All rights reserved.

using System;
using System.Globalization;
using Windows.Networking.Connectivity;
using Windows.Storage;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;

namespace AthensDefaultApp
{
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();

            UpdateBoardInfo();
            UpdateNetworkInfo();
            UpdateDateTime();

            NetworkInformation.NetworkStatusChanged += NetworkInformation_NetworkStatusChanged;

            timer = new DispatcherTimer();
            timer.Tick += timer_Tick;
            timer.Interval = TimeSpan.FromSeconds(30);
            timer.Start();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            if (!ApplicationData.Current.LocalSettings.Values.ContainsKey(Constants.HasDoneOOBEKey))
            {
                ApplicationData.Current.LocalSettings.Values[Constants.HasDoneOOBEKey] = Constants.HasDoneOOBEValue;
            }

            base.OnNavigatedTo(e);
        }

        private async void NetworkInformation_NetworkStatusChanged(object sender)
        {
            await Window.Current.Dispatcher.RunAsync(CoreDispatcherPriority.Low, () => 
            {
                UpdateNetworkInfo();
            });
        }

        private async void timer_Tick(object sender, object e)
        {
            await Window.Current.Dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
            {
                UpdateDateTime();
            });
        }

        private void UpdateBoardInfo()
        {
            BoardName.Text = DeviceInfoPresenter.GetBoardName();
            BoardImage.Source = new BitmapImage(DeviceInfoPresenter.GetBoardImageUri());
        }

        private void UpdateDateTime()
        {
            var t = DateTime.Now;
            this.CurrentTime.Text = t.ToString("t", CultureInfo.CurrentCulture);
        }

        private void UpdateNetworkInfo()
        {
            this.DeviceName.Text = DeviceInfoPresenter.GetDeviceName();
            this.IPAddress1.Text = NetworkPresenter.GetCurrentIpv4Address();
            this.NetworkName1.Text = NetworkPresenter.GetCurrentNetworkName() ?? "Not connected";
        }

        private DispatcherTimer timer;

        private void ShutdownButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            ShutdownDropdown.IsOpen = true;
        }

        private void ShutdownOption_Tapped(object sender, TappedRoutedEventArgs e)
        {
            ShutdownManager.BeginShutdown(ShutdownKind.Shutdown, TimeSpan.FromSeconds(0.5));
        }

        private void RestartOption_Tapped(object sender, TappedRoutedEventArgs e)
        {
            ShutdownManager.BeginShutdown(ShutdownKind.Restart, TimeSpan.FromSeconds(0.5));
        }

        private void SettingsButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(Settings));
        }
    }
}
