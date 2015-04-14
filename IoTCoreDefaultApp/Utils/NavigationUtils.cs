using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

namespace IoTCoreDefaultApp
{
    public static class NavigationUtils
    {
        public static void NavigateToScreen(Type screenType)
        {
            var rootFrame = Window.Current.Content as Frame;
            rootFrame.Navigate(screenType, null);
        }

        public static void GoBack()
        {
            var rootFrame = Window.Current.Content as Frame;
            rootFrame.GoBack();
        }
    }
}
