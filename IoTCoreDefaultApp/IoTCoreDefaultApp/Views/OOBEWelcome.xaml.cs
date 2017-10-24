// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Globalization;
using Windows.Foundation;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace IoTCoreDefaultApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class OOBEWelcome : Page
    {
        private LanguageManager languageManager;
        private DispatcherTimer timer;
        private DispatcherTimer countdown;


        public OOBEWelcome()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = Windows.UI.Xaml.Navigation.NavigationCacheMode.Enabled;

            this.DataContext = LanguageManager.GetInstance();
            languageManager = LanguageManager.GetInstance();

            timer = new DispatcherTimer();
            timer.Tick += timer_Tick;
            timer.Interval = TimeSpan.FromSeconds(60);

            countdown = new DispatcherTimer();
            countdown.Tick += countdown_Tick;
            countdown.Interval = TimeSpan.FromMilliseconds(100);

            this.Loaded += async (sender, e) =>
            {
                await Dispatcher.RunAsync(CoreDispatcherPriority.Low, () =>
                {
                    SetupLanguages();
                    UpdateBoardInfo();
                    UpdateNetworkInfo();

                    timer.Start();
                });
            };

            Unloaded += MainPage_Unloaded;
        }

        private void MainPage_Unloaded(object sender, RoutedEventArgs e)
        {
            timer.Stop();
            countdown.Stop();
        }

        private async void countdown_Tick(object sender, object e)
        {
            var value = DefaultLanguageProgress.Value + DefaultLanguageProgress.SmallChange * 5;
            DefaultLanguageProgress.Value = value;
            if (value >= DefaultLanguageProgress.Maximum)
            {
                countdown.Stop();
                await Window.Current.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    NavigationUtils.NavigateToScreen(typeof(MainPage));
                });
            }
        }

        private void timer_Tick(object sender, object e)
        {
            timer.Stop();
            ChooseDefaultLanguage.Visibility = Visibility.Visible;
            CancelButton.Visibility = Visibility.Visible;
            countdown.Start();
        }

        private void SetupLanguages()
        {
            LanguagesListView.ItemsSource = languageManager.LanguageDisplayNames;
            LanguagesListView.SelectedItem = LanguageManager.GetCurrentLanguageDisplayName();

            SetPreferences();
        }

        private void SetPreferences()
        {
            if(null == LanguagesListView.SelectedItem)
            {
                NavigationUtils.NavigateToScreen(typeof(MainPage));

            }
            else
            {
                LangApplyStack.Visibility = Visibility.Collapsed;
                string selectedLanguage = LanguagesListView.SelectedItem as string;

                //Check existing lang Tuple
                var currentLangTuple = languageManager.GetLanguageTuple(languageManager.GetLanguageTagFromDisplayName(selectedLanguage));
                SpeechSupport.Text = currentLangTuple.Item2 ? languageManager["SpeechSupportText"] : languageManager["SpeechNotSupportText"];
                
                if (LanguageManager.GetCurrentLanguageDisplayName().Equals(selectedLanguage))
                {
                    //Do Nothing
                    return;
                }

                //Check if selected language is part of ffu
                var newLang = languageManager.CheckUpdateLanguage(selectedLanguage);
                               
                if (LanguageManager.GetDisplayNameFromLanguageTag(newLang.Item4).Equals(selectedLanguage))
                {
                    //Update
                    var langReturned = languageManager.UpdateLanguage(selectedLanguage);
                           
                    //ffu list, Show user to restart to use the System Languages
                    if (newLang.Item1)
                    {
                        Common.LangApplyRebootRequired = true;
                        LangApplyStack.Visibility = Visibility.Visible;
                    }
                    //else
                    //skip providing option to restart app
                }
                else
                {
                    if (newLang.Item2)
                    {

                        //Stop Automatic counter to switch to next screen
                        timer.Stop();
                        countdown.Stop();
                        ChooseDefaultLanguage.Visibility = Visibility.Collapsed;
                        CancelButton.Visibility = Visibility.Collapsed;

                        //If different, show the popup for confirmation
                        PopupText2.Text = LanguageManager.GetDisplayNameFromLanguageTag(newLang.Item4);
                        PopupYes.Content = LanguageManager.GetDisplayNameFromLanguageTag(newLang.Item4);

                        PopupText1.Text = languageManager["LanguagePopupText1"];
                        PopupText3.Text = languageManager["LanguagePopupText3"];

                        PopupNo.Content = LanguagesListView.SelectedItem as string;
                        
                        double hOffset = (Window.Current.Bounds.Width) / 4;
                        double vOffset = (Window.Current.Bounds.Height) / 2;

                        StandardPopup.VerticalOffset = vOffset;
                        StandardPopup.HorizontalOffset = hOffset;

                        if (!StandardPopup.IsOpen) { StandardPopup.IsOpen = true; }
                    } 
                    else
                    { 
                        //Just update silently in the background and dont ask for restart app
                        var langReturned = languageManager.UpdateLanguage(selectedLanguage);

                    }
                    
                }
                
            }
        }

        // Handles the Click event on the Button inside the Popup control
        private void PopupYes_Clicked(object sender, RoutedEventArgs e)
        {
            var curLang = LanguageManager.GetCurrentLanguageDisplayName();
            if (curLang.Equals(PopupYes.Content as string))
            {
                Common.LangApplyRebootRequired = false;
                LangApplyStack.Visibility = Visibility.Collapsed;

            }
            else
            {
                //Update
                var langReturned = languageManager.UpdateLanguage(PopupYes.Content as string, true);

                Common.LangApplyRebootRequired = true;
                LangApplyStack.Visibility = Visibility.Visible;
                
            }
            LanguagesListView.SelectedItem = LanguageManager.GetCurrentLanguageDisplayName();
            if (StandardPopup.IsOpen) { StandardPopup.IsOpen = false; }

            //Check existing lang Tuple
            var currentLangTuple = languageManager.GetLanguageTuple(languageManager.GetLanguageTagFromDisplayName(LanguagesListView.SelectedItem as string));
            SpeechSupport.Text = currentLangTuple.Item2 ? languageManager["SpeechSupportText"] : languageManager["SpeechNotSupportText"];
        }

        private void PopupNo_Clicked(object sender, RoutedEventArgs e)
        {
            var curLang = LanguageManager.GetCurrentLanguageDisplayName();

            if (curLang.Equals(PopupNo.Content as string))
            {
                Common.LangApplyRebootRequired = false;
                LangApplyStack.Visibility = Visibility.Collapsed;

            }
            else
            {
                var langReturned = languageManager.UpdateLanguage(PopupNo.Content as string, false);
                                
            }
            LanguagesListView.SelectedItem = LanguageManager.GetCurrentLanguageDisplayName();
            if (StandardPopup.IsOpen) { StandardPopup.IsOpen = false; }

            //Check existing lang Tuple
            var currentLangTuple = languageManager.GetLanguageTuple(languageManager.GetLanguageTagFromDisplayName(LanguagesListView.SelectedItem as string));
            SpeechSupport.Text = currentLangTuple.Item2 ? languageManager["SpeechSupportText"] : languageManager["SpeechNotSupportText"];

            //Enable only if selected lang supports speech or image localization
            if(currentLangTuple.Item1 || currentLangTuple.Item2)
            {
                Common.LangApplyRebootRequired = true;
                LangApplyStack.Visibility = Visibility.Visible;
            }
        }
               
        private void LangApplyYes_Click(object sender, RoutedEventArgs e)
        {
            Windows.ApplicationModel.Core.CoreApplication.Exit();
        }


        private void CancelButton_Clicked(object sender, RoutedEventArgs e)
        {
            countdown.Stop();
            ChooseDefaultLanguage.Visibility = Visibility.Collapsed;
            CancelButton.Visibility = Visibility.Collapsed;
        }

        private async void NextButton_Clicked(object sender, RoutedEventArgs e)
        {
            var networkPresenter = new NetworkPresenter();
            var wifiAvailable = networkPresenter.WifiIsAvailable();
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

            await Window.Current.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                // If the next screen is the main-page, navigate there, but also launch Cortana to its Consent Page independently
                if (nextScreen == typeof(MainPage))
                {
                    await CortanaHelper.LaunchCortanaToConsentPageAsyncIfNeeded();
                }
                NavigationUtils.NavigateToScreen(nextScreen);
            });
        }

        private void LanguagesListBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (LanguageManager.GetCurrentLanguageDisplayName().Equals(LanguagesListView.SelectedItem as string))
            {
                Common.LangApplyRebootRequired = false;
                LangApplyStack.Visibility = Visibility.Collapsed;

                //Do Nothing
                return;
            }

            SetPreferences();
           
        }

        private void UpdateBoardInfo()
        {
            ulong version = 0;
            if (!ulong.TryParse(Windows.System.Profile.AnalyticsInfo.VersionInfo.DeviceFamilyVersion, out version))
            {
                OSVersion.Text = languageManager["OSVersionNotAvailable"];
            }
            else
            {
                OSVersion.Text = String.Format(CultureInfo.InvariantCulture, "{0}.{1}.{2}.{3}",
                    (version & 0xFFFF000000000000) >> 48,
                    (version & 0x0000FFFF00000000) >> 32,
                    (version & 0x00000000FFFF0000) >> 16,
                    version & 0x000000000000FFFF);
            }
        }

        private void UpdateNetworkInfo()
        {
            this.DeviceName.Text = DeviceInfoPresenter.GetDeviceName();
            this.IPAddress1.Text = NetworkPresenter.GetCurrentIpv4Address();
        }

    }
}
