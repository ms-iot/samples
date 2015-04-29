/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

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
*/
using System;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace IoTCoreDefaultApp
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

        private async void NextButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            var wifiAvailable = NetworkPresenter.WifiIsAvailable();
            SetPreferences();
            Type nextScreen;

            try
            {
                nextScreen = (await wifiAvailable) ? typeof(OOBENetwork) : typeof(MainPage);
            }
            catch (Exception)
            {
                nextScreen = typeof(MainPage);
            }

            await Window.Current.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                NavigationUtils.NavigateToScreen(nextScreen);
            });
        }
    }
}
