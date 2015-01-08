// © Copyright(C) Microsoft. All rights reserved.

using System;
using System.Globalization;
using Windows.Networking.Connectivity;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media.Imaging;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace bertha
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();

            var hasSeenOOBE = ApplicationData.Current.LocalSettings.Values["SeenOOBE"];

            if (hasSeenOOBE == null)
            {
                ApplicationData.Current.LocalSettings.Values["SeenOOBE"] = true;
            }

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
            this.MACAddress1.Text = DeviceInfoPresenter.GetCurrentMACAddress();
        }

        private DispatcherTimer timer;

		private void TelnetSession_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(RemoteShell));
        }

        private void Settings_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(Settings));
        }

        private void ConnectToPC_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(ConnectToPC));
        }

        private void MakeLEDBlink_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(BlinkLED));
        }

		private void GetCoding_Tapped(object sender, TappedRoutedEventArgs e)
		{

		}

		private void ConnectToCloud_Tapped(object sender, TappedRoutedEventArgs e)
		{

		}

		private void Tutorials_Tapped(object sender, TappedRoutedEventArgs e)
		{

		}

		private static void SwapUnderline(TextBlock hide, TextBlock show)
		{
			hide.Visibility = Visibility.Collapsed;
			show.Visibility = Visibility.Visible;
		}

		private void TelnetSession_PointerEntered(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(TelnetText, TelnetTextUnderline);
		}

		private void TelnetSession_PointerExited(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(TelnetTextUnderline, TelnetText);
		}

		private void MakeLEDBlink_PointerEntered(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(LEDText, LEDTextUnderline);
		}

		private void MakeLEDBlink_PointerExited(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(LEDTextUnderline, LEDText);
		}

		private void ConnectToPC_PointerEntered(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(ConnectText, ConnectTextUnderline);
		}

		private void ConnectToPC_PointerExited(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(ConnectTextUnderline, ConnectText);
		}

		private void GetCoding_PointerEntered(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(CodingText, CodingTextUnderline);
		}

		private void GetCoding_PointerExited(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(CodingTextUnderline, CodingText);
		}

		private void ConnectToCloud_PointerEntered(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(CloudText, CloudTextUnderline);
		}

		private void ConnectToCloud_PointerExited(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(CloudTextUnderline, CloudText);
		}

		private void Tutorials_PointerEntered(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(TutorialText, TutorialTextUnderline);
		}

		private void Tutorials_PointerExited(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(TutorialTextUnderline, TutorialText);
		}

		private void Settings_PointerEntered(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(SettingsText, SettingsTextUnderline);
		}

		private void Settings_PointerExited(object sender, PointerRoutedEventArgs e)
		{
			SwapUnderline(SettingsTextUnderline, SettingsText);
		}

		private void ShutdownButton_Tapped(object sender, TappedRoutedEventArgs e)
		{
			ShutdownDropdown.IsOpen = true;
		}

		private void ShutdownOption_Tapped(object sender, TappedRoutedEventArgs e)
		{

		}

		private void RestartOption_Tapped(object sender, TappedRoutedEventArgs e)
		{

		}
	}
}
