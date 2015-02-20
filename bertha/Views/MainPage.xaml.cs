// © Copyright(C) Microsoft. All rights reserved.

using System;
using System.Globalization;
using Windows.Networking.Connectivity;
using Windows.Storage;
using Windows.System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media.Imaging;

namespace bertha
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

        private void NetworkInformation_NetworkStatusChanged(object sender)
        {
            UpdateNetworkInfo();
        }

        void timer_Tick(object sender, object e)
        {
            UpdateDateTime();
        }

        private void UpdateBoardInfo()
        {
            DeviceInfoPresenter presenter = new DeviceInfoPresenter();
            BoardName.Text = presenter.GetBoardName();
            BoardImage.Source = new BitmapImage(presenter.GetBoardImageUri());
        }

        private void UpdateDateTime()
        {
            var t = DateTime.Now;
            this.CurrentTime.Text = t.ToString("t", CultureInfo.CurrentCulture);
        }

        private void UpdateNetworkInfo()
        {
            this.DeviceName.Text = DeviceInfoPresenter.GetDeviceName();
            this.IPAddress1.Text = DeviceInfoPresenter.GetCurrentIpv4Address();
            this.NetworkName1.Text = DeviceInfoPresenter.GetCurrentNetworkName();
        }

        private DispatcherTimer timer;

        private void ShutdownButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            ShutdownDropdown.IsOpen = true;
        }

        private void ShutdownOption_Tapped(object sender, TappedRoutedEventArgs e)
        {
            ShutdownManager.BeginShutdown(TimeSpan.FromSeconds(0.5), ShutdownKind.Shutdown);
        }

        private void RestartOption_Tapped(object sender, TappedRoutedEventArgs e)
        {
            ShutdownManager.BeginShutdown(TimeSpan.FromSeconds(0.5), ShutdownKind.Restart);
        }
    }
}
