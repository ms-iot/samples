/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

    The MIT License(MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

using OnBoardee;
using System;
using System.Globalization;
using System.Threading.Tasks;
using Windows.Networking.Connectivity;
using Windows.Storage;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;

namespace IoTCoreDefaultApp
{
    public sealed partial class MainPage : Page
    {
        private CoreDispatcher MainPageDispatcher;
        private DispatcherTimer timer;
        private OnboardingService OnboardingService;

        public MainPage()
        {
            this.InitializeComponent();

            UpdateBoardInfo();
            UpdateNetworkInfo();
            UpdateDateTime();

            MainPageDispatcher = Window.Current.Dispatcher;

            NetworkInformation.NetworkStatusChanged += NetworkInformation_NetworkStatusChanged;

            timer = new DispatcherTimer();
            timer.Tick += timer_Tick;
            timer.Interval = TimeSpan.FromSeconds(30);
            timer.Start();

            OnboardingService = new OnboardingService();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            if (!ApplicationData.Current.LocalSettings.Values.ContainsKey(Constants.HasDoneOOBEKey))
            {
                ApplicationData.Current.LocalSettings.Values[Constants.HasDoneOOBEKey] = Constants.HasDoneOOBEValue;
            }
            Task.Run(() => {
                OnboardingService.Initialize();
            });

            base.OnNavigatedTo(e);
        }

        private async void NetworkInformation_NetworkStatusChanged(object sender)
        {
            await MainPageDispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
            {
                UpdateNetworkInfo();
            });
        }

        private void timer_Tick(object sender, object e)
        {
            UpdateDateTime();
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
