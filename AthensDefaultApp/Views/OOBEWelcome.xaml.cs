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
        }

        private void SetupLanguages()
        {
            languageManager = new LanguageManager();

            LanguagesListView.ItemsSource = languageManager.LanguageDisplayNames;
            LanguagesListView.SelectedItem = LanguageManager.GetCurrentLanguageDisplayName();
        }

        private void SetPreferences()
        {
            var selectedLanguage = LanguagesListView.SelectedItem as string;
            languageManager.UpdateLanguage(selectedLanguage);
        }

        private void NextButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            SetPreferences();
            NavigationUtils.NavigateToScreen(typeof(OOBENetwork));
        }
    }
}
