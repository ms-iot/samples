// Copyright (c) Microsoft. All rights reserved.

using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using IoTCoreDefaultApp.Utils;

namespace IoTCoreDefaultApp
{
    public static class NavigationUtils
    {
        public static void NavigateToScreen(Type screenType)
        {
            var rootFrame = Window.Current.Content as Frame;
            rootFrame.Navigate(screenType, null);
        }

        public static void NavigateToScreen(Type screenType, object parameter)
        {
            var rootFrame = Window.Current.Content as Frame;
            rootFrame.Navigate(screenType, parameter);
        }

        public static void GoBack()
        {
            var rootFrame = Window.Current.Content as Frame;
            rootFrame.GoBack();
        }

        public static void NavigateToNextTutorialFrom(string docName)
        {
            var index = Array.FindIndex(Constants.TutorialDocNames, s => s.Equals(docName));
            if (index <= -1 || index >= (Constants.TutorialDocNames.Length - 1))
            {
                return;
            }
            ++index;
            var newDocName = Constants.TutorialDocNames[index];
            if (newDocName == "HelloBlinky")
            {
#if (!ALWAYS_SHOW_BLINKY)
                if (DeviceTypeInformation.IsRaspberryPi || DeviceTypeInformation.Type == DeviceTypes.DB410)
#endif
                {
                    NavigationUtils.NavigateToScreen(typeof(TutorialHelloBlinkyPage), newDocName);
                    return;
                }
// Disable unreachable code warning
#pragma warning disable 162
                newDocName = Constants.TutorialDocNames[++index];
#pragma warning restore 162
            }

            NavigationUtils.NavigateToScreen(typeof(TutorialContentPage), newDocName);
        }

        public static bool IsNextTutorialButtonVisible(string docName)
        {
            var index = Array.FindIndex(Constants.TutorialDocNames, s => s.Equals(docName));
            if (index == -1 || index == (Constants.TutorialDocNames.Length - 1))
            {
                return false;
            }
            return true;
        }
    }
}
