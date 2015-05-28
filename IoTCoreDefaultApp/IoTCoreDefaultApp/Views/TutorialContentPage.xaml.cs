using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace IoTCoreDefaultApp
{
    public sealed partial class TutorialContentPage : Page
    {
        private DispatcherTimer timer;
        private string docName;

        public TutorialContentPage()
        {
            this.InitializeComponent();

            var rootFrame = Window.Current.Content as Frame;
            rootFrame.Navigated += RootFrame_Navigated;

            UpdateDateTime();

            timer = new DispatcherTimer();
            timer.Tick += timer_Tick;
            timer.Interval = TimeSpan.FromSeconds(30);
            timer.Start();
        }

        private void RootFrame_Navigated(object sender, NavigationEventArgs e)
        {
            docName = e.Parameter as string;
            if (docName != null)
            {
                LoadDocument(docName);
                NextButton.Visibility = (NavigationUtils.IsNextTutorialButtonVisible(docName) ? Visibility.Visible : Visibility.Collapsed);
            }
        }

        private void LoadDocument(string docname)
        {
            TutorialText.Source = new Uri(string.Format(CultureInfo.InvariantCulture, "ms-appx-web:///assets/tutorials/{0}.htm", docname));
        }

        private void timer_Tick(object sender, object e)
        {
            UpdateDateTime();
        }

        private void UpdateDateTime()
        {
            var t = DateTime.Now;
            this.CurrentTime.Text = t.ToString("t", CultureInfo.CurrentCulture);
        }

        private void ShutdownButton_Clicked(object sender, RoutedEventArgs e)
        {
            ShutdownDropdown.IsOpen = true;
        }

        private void ShutdownHelper(ShutdownKind kind)
        {
            ShutdownManager.BeginShutdown(kind, TimeSpan.FromSeconds(0.5));
        }

        private void ShutdownListView_ItemClick(object sender, ItemClickEventArgs e)
        {
            var item = e.ClickedItem as FrameworkElement;
            if (item == null)
            {
                return;
            }
            switch (item.Name)
            {
                case "ShutdownOption":
                    ShutdownHelper(ShutdownKind.Shutdown);
                    break;
                case "RestartOption":
                    ShutdownHelper(ShutdownKind.Restart);
                    break;
            }
        }

        private void SettingsButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(Settings));
        }

        private void BackButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }

        private void DeviceInfo_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(MainPage));
        }

        private void NextButton_Clicked(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToNextTutorialFrom(docName);
        }
    }
}
