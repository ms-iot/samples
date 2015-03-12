using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace AthensDefaultApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class OOBEWelcome : Page
    {
        private LanguageManager languageManager;

        public OOBEWelcome()
        {
            this.InitializeComponent();

            SetupLanguages();
            SetupCountryOrRegion();
        }

        private void SetupLanguages()
        {
            languageManager = new LanguageManager();

            LanguageComboBox.ItemsSource = languageManager.LanguageDisplayNames;
            LanguageComboBox.SelectedItem = LanguageManager.GetCurrentLanguageDisplayName();
        }

        private void SetupCountryOrRegion()
        {
            RegionComboBox.SelectedItem = LanguageManager.GetCurrentRegion();
        }

        private void SetPreferences()
        {
            var selectedLanguage = LanguageComboBox.SelectedItem as string;
            languageManager.UpdateLanguage(selectedLanguage);
        }

        private void SetupKeyboardLayout()
        {
            LanguageComboBox.SelectedItem = LanguageManager.GetCurrentKeyboardLanguage();
        }

        private void NextButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            SetPreferences();
        }
    }
}
