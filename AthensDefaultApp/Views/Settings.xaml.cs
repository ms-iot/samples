using System.Collections.Generic;
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

        }

        private void TimeZoneComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {

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
            var network = new WifiNetwork();
            network.NetworkName = "Test Network Name";

            WifiListView.ItemsSource = new List<WifiNetwork>() { network };
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
                var listViewItem = listView.ContainerFromItem(item) as ListViewItem;
                listViewItem.ContentTemplate = WifiConnectState;
            }
        }

        private void ConnectButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            SwitchToItemState(sender as Button, WifiPasswordState);
        }

        private void NextButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            SwitchToItemState(sender as Button, WifiConnectingState);

            //NavigationUtils.NavigateToScreen(typeof(MainPage));
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
    }
}
