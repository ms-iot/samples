// Copyright (c) Microsoft. All rights reserved.

using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using AirHockeyHelper;

#pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed

namespace AirHockeyApp
{
    public sealed partial class MainPage : Page
    {

        public MainPage()
        {
            // Start the global stopwatch
            if (!Global.Stopwatch.IsRunning)
            {
                Global.Stopwatch.Start();
            }

            this.InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
        }

        private void diagnosticsButton_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(DiagnosticsPage), GameMode.Diagnostics);
        }

        private void gameButton_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(DiagnosticsPage), GameMode.Game);
        }

        private void mirrorButton_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(DiagnosticsPage), GameMode.Mirror);
        }

        private void testButton_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(DiagnosticsPage), GameMode.Test);
        }
    }
}
