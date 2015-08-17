using I2CCompass.Sensors;
using I2CCompass.ViewModels;
using Windows.UI.Xaml.Controls;

namespace I2CCompass.Views
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();

            DataContext = new MainPageViewModel(new HMC5883L());
            //_compass.CompassReadingChanged += _compass_CompassReadingChanged;
        }

        //private async void _compass_CompassReadingChanged(object sender, CompassReading e)
        //{
        //    await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, new Windows.UI.Core.DispatchedHandler(() =>
        //    {
        //        headingTextBlock.Text = string.Format("{0}°", e.Heading);
        //    }));
        //}

        //private void StartContinuousMeasurements_Click(object sender, RoutedEventArgs e)
        //{
        //    _compass.StartContinuousMeasurements();
        //}
    }
}
