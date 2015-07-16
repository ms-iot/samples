/**
   Copyright(c) Microsoft Open Technologies, Inc.All rights reserved.
  The MIT License(MIT)
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files(the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions :
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
**/

using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using AirHockeyHelper2;

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

        private void songButton_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(MusicPage), null);
        }

        private void testButton_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(DiagnosticsPage), GameMode.Test);
        }
    }
}
