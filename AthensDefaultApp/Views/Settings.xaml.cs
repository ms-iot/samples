using System;
using Windows.Devices.WiFi;
using Windows.Security.Credentials;
using Windows.System.Threading;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace AthensDefaultApp
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

        public Settings()
        {
            this.InitializeComponent();

            visibleContent = BasicPreferencesGridView;

            SetupLanguages();
        }

        private void SetupLanguages()
        {
            languageManager = new LanguageManager();

            LanguageComboBox.ItemsSource = languageManager.LanguageDisplayNames;
            LanguageComboBox.SelectedItem = LanguageManager.GetCurrentLanguageDisplayName();

            KeyboardComboBox.ItemsSource = languageManager.LanguageDisplayNames;
            KeyboardComboBox.SelectedItem = LanguageManager.GetCurrentKeyboardLanguage();

            TimeZoneComboBox.ItemsSource = LanguageManager.GetSupportedTimeZones();
            TimeZoneComboBox.SelectedItem = LanguageManager.GetCurrentTimeZone();
        }

        private void SetupNetwork()
        {
            SetupEthernet();
            SetupWifi();
        }

        private void BackButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }

        private void NetworkListViewItem_Tapped(object sender, TappedRoutedEventArgs e)
        {
            if (NetworkGrid.Visibility == Visibility.Collapsed)
            {
                SetupNetwork();
                visibleContent.Visibility = Visibility.Collapsed;
                NetworkGrid.Visibility = Visibility.Visible;
                visibleContent = NetworkGrid;
            }
        }

        private void PreferencesListViewItem_Tapped(object sender, TappedRoutedEventArgs e)
        {
            if (BasicPreferencesGridView.Visibility == Visibility.Collapsed)
            {
                SetupLanguages();
                visibleContent.Visibility = Visibility.Collapsed;
                BasicPreferencesGridView.Visibility = Visibility.Visible;
                visibleContent = BasicPreferencesGridView;
            }            
        }

        private void LanguageComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var comboBox = sender as ComboBox;
            languageManager.UpdateLanguage(comboBox.SelectedItem as string);
        }

        private void KeyboardComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var comboBox = sender as ComboBox;
            languageManager.UpdateKeyboardLanguage(comboBox.SelectedItem as string);
        }

        private void TimeZoneComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var comboBox = sender as ComboBox;
            LanguageManager.ChangeTimeZone(comboBox.SelectedItem as string);
        }

        private void SetupEthernet()
        {
            var ethernetProfile = NetworkPresenter.GetDirectConnectionName();

            if (ethernetProfile.Equals("None found"))
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

        private void SetupWifi()
        {
            networkPresenter = new NetworkPresenter();

            WifiListView.ItemsSource = networkPresenter.GetAvailableNetworks();
        }

        private void WifiListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var listView = sender as ListView;

            foreach (var item in e.RemovedItems)
            {
                var listViewItem = listView.ContainerFromItem(item) as ListViewItem;
                listViewItem.ContentTemplate = WifiInitialState;
            }

            foreach (var item in e.AddedItems)
            {
                Automatic = true;
                var listViewItem = listView.ContainerFromItem(item) as ListViewItem;
                listViewItem.ContentTemplate = WifiConnectState;
            }
        }

        private async void ConnectButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            var button = sender as Button;
            var network = button.DataContext as WiFiAvailableNetwork;

            if (NetworkPresenter.IsNetworkOpen(network))
            {
                await ThreadPool.RunAsync((item) =>
                {
                    ConnectToWifi(button, network, null, Window.Current.Dispatcher);
                });
            }
            else
            {
                await Window.Current.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    SwitchToItemState(button, WifiPasswordState);
                });
            }
        }

        private async void ConnectToWifi(Button button, WiFiAvailableNetwork network, PasswordCredential credential, CoreDispatcher dispatcher)
        {
            var didConnect = credential == null ?
                networkPresenter.ConnectToNetwork(network, Automatic) :
                networkPresenter.ConnectToNetworkWithPassword(network, Automatic, credential);

            await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                SwitchToItemState(button, WifiConnectingState);
            });

            if (await didConnect)
            {
                await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    NavigationUtils.NavigateToScreen(typeof(MainPage));
                });
            }
            else
            {
                await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    var item = SwitchToItemState(button, WifiInitialState);
                    item.IsSelected = false;
                });
            }
        }

        private async void NextButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            var button = sender as Button;
            var network = button.DataContext as WiFiAvailableNetwork;
            await ThreadPool.RunAsync((item) =>
            {
                ConnectToWifi(button, network, null, Window.Current.Dispatcher);
            });
        }

        private void CancelButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            var item = SwitchToItemState(sender as Button, WifiInitialState);
            item.IsSelected = false;
        }

        private ListViewItem SwitchToItemState(Button sender, DataTemplate template)
        {
            var item = WifiListView.ContainerFromItem(sender.DataContext) as ListViewItem;
            item.ContentTemplate = template;

            return item;
        }

        private void ConnectAutomaticallyCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            var checkbox = sender as CheckBox;

            Automatic = checkbox.IsChecked ?? false;
        }
    }
}
