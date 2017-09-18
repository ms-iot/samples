// Copyright (c) Microsoft. All rights reserved.


using System;
using Windows.Networking.Connectivity;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace IoTCoreDefaultApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class OOBENetwork : Page
    {
        private CoreDispatcher OOBENetworkPageDispatcher;

        public OOBENetwork()
        {
            this.InitializeComponent();
            OOBENetworkPageDispatcher = Window.Current.Dispatcher;

            NetworkInformation.NetworkStatusChanged += NetworkInformation_NetworkStatusChanged;
            NetworkGrid.NetworkConnected += NetworkGrid_NetworkConnected;


            this.NavigationCacheMode = Windows.UI.Xaml.Navigation.NavigationCacheMode.Enabled;

            this.DataContext = LanguageManager.GetInstance();

            this.Loaded += async (sender, e) =>
            {
                await OOBENetworkPageDispatcher.RunAsync(CoreDispatcherPriority.Low, async () => {
                    await NetworkGrid.SetupNetworkAsync();
                });
            };
        }

        private async void NetworkInformation_NetworkStatusChanged(object sender)
        {
            await OOBENetworkPageDispatcher.RunAsync(CoreDispatcherPriority.Low, async () =>
            {
                await NetworkGrid.SetupNetworkAsync();
            });
        }

        private async void NetworkGrid_NetworkConnected(object sender, EventArgs e)
        {
            await OOBENetworkPageDispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                await CortanaHelper.LaunchCortanaToConsentPageAsyncIfNeeded();
                NavigationUtils.NavigateToScreen(typeof(MainPage));
            });
        }
       
        private void BackButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }

        private async void SkipButton_Clicked(object sender, RoutedEventArgs e)
        {
            await CortanaHelper.LaunchCortanaToConsentPageAsyncIfNeeded();
            NavigationUtils.NavigateToScreen(typeof(MainPage));
        }

    }
}
