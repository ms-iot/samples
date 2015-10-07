// Copyright (c) Microsoft. All rights reserved.


using System;
using Windows.Devices.WiFi;
using Windows.Security.Credentials;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace IoTCoreDefaultApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Settings : Page
    {
        private LanguageManager languageManager;
        private UIElement visibleContent;
        private NetworkPresenter networkPresenter;
        private bool Automatic = true;
        private string CurrentPassword = string.Empty;

        public Settings()
        {
            this.InitializeComponent();

            visibleContent = BasicPreferencesGridView;

            this.NavigationCacheMode = Windows.UI.Xaml.Navigation.NavigationCacheMode.Enabled;

            this.DataContext = LanguageManager.GetInstance();

            this.Loaded += (sender, e) =>
            {
                SetupLanguages();
            };
        }

        private void SetupLanguages()
        {
            languageManager = LanguageManager.GetInstance();

            LanguageListBox.ItemsSource = languageManager.LanguageDisplayNames;
            LanguageListBox.SelectedItem = LanguageManager.GetCurrentLanguageDisplayName();
        }

        private void SetupNetwork()
        {
            SetupEthernet();
            SetupWifi();
        }

        private void BackButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }

        private void LanguageListBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var listBox = sender as ListBox;
            if (listBox.SelectedItem == null)
            {
                return;
            }

            languageManager.UpdateLanguage(listBox.SelectedItem as string);
        }


        private void SetupEthernet()
        {
            var ethernetProfile = NetworkPresenter.GetDirectConnectionName();

            if (ethernetProfile == null)
            {
                NoneFoundText.Visibility = Visibility.Visible;
                DirectConnectionStackPanel.Visibility = Visibility.Collapsed;
            }
            else
            {
                NoneFoundText.Visibility = Visibility.Collapsed;
                DirectConnectionStackPanel.Visibility = Visibility.Visible;
            }
        }

        private async void SetupWifi()
        {
            networkPresenter = new NetworkPresenter();

            if (await NetworkPresenter.WifiIsAvailable())
            {
                var networks = await networkPresenter.GetAvailableNetworks();

                if (networks.Count > 0)
                {
                    WifiListView.ItemsSource = networks;
                    var connectedNetwork = networkPresenter.GetCurrentWifiNetwork();

                    if (connectedNetwork != null)
                    {
                        SwitchToItemState(connectedNetwork, WifiConnectedState, true);
                    }

                    NoWifiFoundText.Visibility = Visibility.Collapsed;
                    WifiListView.Visibility = Visibility.Visible;

                    return;
                }
            }

            NoWifiFoundText.Visibility = Visibility.Visible;
            WifiListView.Visibility = Visibility.Collapsed;
        }

        private void WifiListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var listView = sender as ListView;

            foreach (var item in e.RemovedItems)
            {
                SwitchToItemState(item, WifiInitialState, true);
            }

            foreach (var item in e.AddedItems)
            {
                Automatic = true;
                SwitchToItemState(item, WifiConnectState, true);
            }
        }

        private void ConnectButton_Clicked(object sender, RoutedEventArgs e)
        {
            var button = sender as Button;
            var network = button.DataContext as WiFiAvailableNetwork;
            if (NetworkPresenter.IsNetworkOpen(network))
            {
                ConnectToWifi(network, null, Window.Current.Dispatcher);
            }
            else
            {
                SwitchToItemState(network, WifiPasswordState, false);
            }
        }

        private async void ConnectToWifi(WiFiAvailableNetwork network, PasswordCredential credential, CoreDispatcher dispatcher)
        {
            var didConnect = credential == null ?
                networkPresenter.ConnectToNetwork(network, Automatic) :
                networkPresenter.ConnectToNetworkWithPassword(network, Automatic, credential);

            await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                SwitchToItemState(network, WifiConnectingState, false);
            });

            DataTemplate nextState = (await didConnect) ? WifiConnectedState : WifiInitialState;

            await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                var item = SwitchToItemState(network, nextState, false);
                item.IsSelected = false; 
            });
        }

        private void NextButton_Clicked(object sender, RoutedEventArgs e)
        {
            var button = sender as Button;
            PasswordCredential credential;

            if (string.IsNullOrEmpty(CurrentPassword))
            {
                credential = null;
            }
            else
            {
                credential = new PasswordCredential()
                {
                    Password = CurrentPassword
                };
            }

            var network = button.DataContext as WiFiAvailableNetwork;
            ConnectToWifi(network, credential, Window.Current.Dispatcher);
        }

        private void CancelButton_Clicked(object sender, RoutedEventArgs e)
        {
            var button = sender as Button;
            var item = SwitchToItemState(button.DataContext, WifiInitialState, false);
            item.IsSelected = false;
        }

        private ListViewItem SwitchToItemState(object dataContext, DataTemplate template, bool forceUpdate)
        {
            if (forceUpdate)
            {
                WifiListView.UpdateLayout();
            }
            var item = WifiListView.ContainerFromItem(dataContext) as ListViewItem;
            if (item != null)
            {
                item.ContentTemplate = template;
            }

            return item;
        }

        private void ConnectAutomaticallyCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            var checkbox = sender as CheckBox;

            Automatic = checkbox.IsChecked ?? false;
        }

        private void WifiPasswordBox_PasswordChanged(object sender, RoutedEventArgs e)
        {
            var passwordBox = sender as PasswordBox;
            CurrentPassword = passwordBox.Password;
        }

        private void ListView_ItemClick(object sender, ItemClickEventArgs e)
        {
            var item = e.ClickedItem as FrameworkElement;
            if (item == null)
            {
                return;
            }
            switch (item.Name)
            {
                case "PreferencesListViewItem":
                    if (BasicPreferencesGridView.Visibility == Visibility.Collapsed)
                    {
                        visibleContent.Visibility = Visibility.Collapsed;
                        BasicPreferencesGridView.Visibility = Visibility.Visible;
                        visibleContent = BasicPreferencesGridView;
                    }
                    break;
                case "NetworkListViewItem":
                    if (NetworkGrid.Visibility == Visibility.Collapsed)
                    {
                        SetupNetwork();
                        visibleContent.Visibility = Visibility.Collapsed;
                        NetworkGrid.Visibility = Visibility.Visible;
                        visibleContent = NetworkGrid;
                    }
                    break;
            }
        }

        private void RefreshButton_Click(object sender, RoutedEventArgs e)
        {
            SetupWifi();
        }
    }
}
