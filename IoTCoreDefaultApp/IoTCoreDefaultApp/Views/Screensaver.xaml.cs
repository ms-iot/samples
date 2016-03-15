// Copyright (c) Microsoft. All rights reserved.

using System;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media.Imaging;

namespace IoTCoreDefaultApp
{
    // The screensaver class triggers automatically after a certain amount of inactivity
    // from keyboard or pointer events.
    // Note: You should only use this screen saver if your device has a pointer or keyboard,
    // or you won't be able to see the app after the screensaver starts.
    //
    // To re-use this screensaver, simply include this class and add the following line
    // to the end of App.OnLaunched:
    // Screensaver.InitializeScreensaver();
    public sealed partial class Screensaver : UserControl
    {
        private static DispatcherTimer timeoutTimer;
        private static Popup screensaverContainer;

        /// <summary>
        /// Initializes the screensaver
        /// </summary>
        public static void InitializeScreensaver()
        {
            screensaverContainer = new Popup()
            {
                Child = new Screensaver(),
                Margin = new Thickness(0),
                IsOpen = false
            };
            //Set screen saver to activate after 1 minute
            timeoutTimer = new DispatcherTimer() { Interval = TimeSpan.FromMinutes(1) };
            timeoutTimer.Tick += TimeoutTimer_Tick;
            Window.Current.Content.AddHandler(UIElement.KeyDownEvent, new KeyEventHandler(App_KeyDown), true);
            Window.Current.Content.AddHandler(UIElement.PointerMovedEvent, new PointerEventHandler(App_PointerEvent), true);
            Window.Current.Content.AddHandler(UIElement.PointerPressedEvent, new PointerEventHandler(App_PointerEvent), true);
            Window.Current.Content.AddHandler(UIElement.PointerReleasedEvent, new PointerEventHandler(App_PointerEvent), true);
            Window.Current.Content.AddHandler(UIElement.PointerEnteredEvent, new PointerEventHandler(App_PointerEvent), true);
            if (IsScreensaverEnabled)
            {
                timeoutTimer.Start();
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether the screen saver should listen for inactivity and 
        /// show after a little while. The default is <c>false</c>.
        /// </summary>
        public static bool IsScreensaverEnabled
        {
            get
            {
                if (ApplicationData.Current.LocalSettings.Values.ContainsKey(Constants.EnableScreensaverKey))
                {
                    return (bool)ApplicationData.Current.LocalSettings.Values[Constants.EnableScreensaverKey];
                }
                return false;
            }
            set
            {
                ApplicationData.Current.LocalSettings.Values[Constants.EnableScreensaverKey] = value;
                if (timeoutTimer != null)
                {
                    if (value)
                    {
                        timeoutTimer.Start();
                    }
                    else
                    {
                        timeoutTimer.Stop();
                    }
                }
            }
        }

        // Triggered when there hasn't been any key or pointer events in a while
        private static void TimeoutTimer_Tick(object sender, object e)
        {
            ShowScreensaver();
        }

        private static void ShowScreensaver()
        { 
            timeoutTimer.Stop();
            var bounds = Windows.UI.Core.CoreWindow.GetForCurrentThread().Bounds;
            var view = (Screensaver)screensaverContainer.Child;
            view.Width = bounds.Width;
            view.Height = bounds.Height;
            view.image.Width = view.Width / 5; //Make screensaver image 1/5 the width of the screen
            screensaverContainer.IsOpen = true;
        }

        private static void App_KeyDown(object sender, KeyRoutedEventArgs args)
        {
            if (IsScreensaverEnabled)
            {
                ResetScreensaverTimeout();
            }
        }

        private static void App_PointerEvent(object sender, PointerRoutedEventArgs e)
        {
            if (IsScreensaverEnabled)
            {
                ResetScreensaverTimeout();
            }
        }

        // Resets the timer and starts over.
        private static void ResetScreensaverTimeout()
        {
            timeoutTimer.Stop();
            timeoutTimer.Start();
            screensaverContainer.IsOpen = false;
        }

        private DispatcherTimer moveTimer;
        private Random randomizer = new Random();

        private Screensaver()
        {
            this.InitializeComponent();
            moveTimer = new DispatcherTimer() { Interval = TimeSpan.FromSeconds(10) };
            moveTimer.Tick += MoveTimer_Tick;
            this.Loaded += ScreenSaver_Loaded;
            this.Unloaded += ScreenSaver_Unloaded;
            image.Source = new BitmapImage(DeviceInfoPresenter.GetBoardImageUri());
        }

        protected override void OnPointerMoved(PointerRoutedEventArgs e)
        {
            base.OnPointerMoved(e);
            if (IsScreensaverEnabled)
            {
                ResetScreensaverTimeout();
            }
        }

        private void MoveTimer_Tick(object sender, object e)
        {
            var left = randomizer.NextDouble() * (this.ActualWidth - image.ActualWidth);
            var top = randomizer.NextDouble() * (this.ActualHeight - image.ActualHeight);
            image.Margin = new Thickness(left, top, 0, 0);
        }

        private void ScreenSaver_Unloaded(object sender, RoutedEventArgs e)
        {
            moveTimer.Stop();
        }

        private void ScreenSaver_Loaded(object sender, RoutedEventArgs e)
        {
            moveTimer.Start();
            MoveTimer_Tick(moveTimer, null);
        }
    }
}
