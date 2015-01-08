// © Copyright(C) Microsoft. All rights reserved.

using System;
using Windows.Networking.Connectivity;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;

namespace bertha
{
    public sealed partial class Settings : Page
    {
        private Grid VisibleContent;

        public Settings()
        {
            this.InitializeComponent();

            UpdateDeviceInfo();
            VisibleContent = DeviceInfoContent;
            UpdateAbout();
        }

        private void UpdateDeviceInfo()
        {
            DeviceInfoPresenter presenter = new DeviceInfoPresenter();

            DeviceName.Text = DeviceInfoPresenter.GetDeviceName();
            IPAddress1.Text = DeviceInfoPresenter.GetCurrentIpv4Address();
            Processor1.Text = presenter.GetProcessor();
            NetworkName1.Text = DeviceInfoPresenter.GetCurrentNetworkName();
            MACAddress1.Text = DeviceInfoPresenter.GetCurrentMACAddress();
            RAM1.Text = DeviceInfoPresenter.GetRAM();
            Edition.Text = DeviceInfoPresenter.GetWindowsEdition();
            Activation1.Text = DeviceInfoPresenter.GetActivationString();
        }

        private void UpdateDefaultAppList()
        {
            //TODO
        }

        private void UpdateAbout()
        {
            AboutPage.Navigate(new Uri("http://microsoft.com"));
        }

        private void SwapContent(Grid newContent)
        {
            if (VisibleContent == DeviceInfoContent)
            {
                NetworkInformation.NetworkStatusChanged -= NetworkInformation_NetworkStatusChanged;
            }

            VisibleContent.Visibility = Visibility.Collapsed;
            VisibleContent = newContent;
            VisibleContent.Visibility = Visibility.Visible;
        }

        private void NetworkInformation_NetworkStatusChanged(object sender)
        {
            UpdateDeviceInfo();
        }

        private void BackButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }

        private void DeviceInfo_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NetworkInformation.NetworkStatusChanged += NetworkInformation_NetworkStatusChanged;
            UpdateDeviceInfo();
            SwapContent(DeviceInfoContent);
        }

        private void DefaultApp_Tapped(object sender, TappedRoutedEventArgs e)
        {
            UpdateDefaultAppList();
            SwapContent(DefaultAppContent);
        }

        private void Network_Tapped(object sender, TappedRoutedEventArgs e)
        {
            //UpdateNetwork();
            //SwapContent(NetworkContent);
        }

        private void Accounts_Tapped(object sender, TappedRoutedEventArgs e)
        {
            //UpdateAccounts();
            //SwapContent(AccountsContent)
        }

        private void About_Tapped(object sender, TappedRoutedEventArgs e)
        {
            SwapContent(AboutContent);
        }
    }
}
