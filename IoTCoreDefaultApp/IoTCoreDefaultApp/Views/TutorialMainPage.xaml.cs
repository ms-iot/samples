// Copyright (c) Microsoft. All rights reserved.

using IoTCoreDefaultApp.Utils;
using System;
using System.Globalization;
using Windows.System;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;

namespace IoTCoreDefaultApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class TutorialMainPage : Page
    {
        public TutorialMainPage()
        {
            this.InitializeComponent();

#if !ALWAYS_SHOW_BLINKY
            if (!DeviceTypeInformation.IsRaspberryPi && DeviceTypeInformation.Type != DeviceTypes.DB410)
            {
                TutorialList.Items.Remove(HelloBlinkyGridViewItem);
            }
#endif
            this.NavigationCacheMode = NavigationCacheMode.Enabled;

            this.DataContext = LanguageManager.GetInstance();

        }

        private void UpdateBoardInfo()
        {
            if (DeviceTypeInformation.Type == DeviceTypes.DB410)
            {
                TutorialsImage.Source = new BitmapImage(new Uri("ms-appx:///Assets/DB410-tutorials.png"));
                GetStartedImage.Source = new BitmapImage(new Uri("ms-appx:///Assets/Tutorials/GetStarted-DB410.jpg"));
                HelloBlinkyTileImage.Source = new BitmapImage(new Uri("ms-appx:///Assets/Tutorials/HelloBlinkyTile-DB410.jpg"));
                GetConnectedImage.Source = new BitmapImage(new Uri("ms-appx:///Assets/Tutorials/GetConnected-DB410.jpg"));
            }
        }

        private void TutorialList_ItemClick(object sender, ItemClickEventArgs e)
        {
            var item = e.ClickedItem as FrameworkElement;
            if (item == null)
            {
                return;
            }
            switch (item.Name)
            {
                case "HelloBlinky":
                    NavigationUtils.NavigateToScreen(typeof(TutorialHelloBlinkyPage), item.Name);
                    break;
                default:
                    NavigationUtils.NavigateToScreen(typeof(TutorialContentPage), item.Name);
                    break;
            }
        }



    }
}
