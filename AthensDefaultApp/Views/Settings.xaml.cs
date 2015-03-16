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

        }

        private void BackButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }

        private void NetworkListViewItem_Tapped(object sender, TappedRoutedEventArgs e)
        {
            visibleContent.Visibility = Visibility.Collapsed;
        }

        private void PreferencesListViewItem_Tapped(object sender, TappedRoutedEventArgs e)
        {
            if (BasicPreferencesGridView.Visibility == Visibility.Collapsed)
            {
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
    }
}
