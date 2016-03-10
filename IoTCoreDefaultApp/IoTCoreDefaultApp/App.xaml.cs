// Copyright (c) Microsoft. All rights reserved.


using System;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Devices.Enumeration;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

// The Blank Application template is documented at http://go.microsoft.com/fwlink/?LinkId=402347&clcid=0x409

namespace IoTCoreDefaultApp
{
    public class InboundPairingEventArgs
    {
        public InboundPairingEventArgs(DeviceInformation di)
        {
            DeviceInfo = di;
        }
        public DeviceInformation DeviceInfo
        {
            get;
            private set;
        }
    }
    // Callback handler delegate type for Inbound pairing requests
    public delegate void InboundPairingRequestedHandler(object sender, InboundPairingEventArgs inboundArgs);

    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    sealed partial class App : Application
    {
        // Handler for Inbound pairing requests
        public static event InboundPairingRequestedHandler InboundPairingRequested;

        // Don't try and make discoverable if this has already been done
        private static bool isDiscoverable = false;

        public static bool IsBluetoothDiscoverable
        {
            get
            {
                return isDiscoverable;
            }

            set
            {
                isDiscoverable = value;
            }
        }

        /// <summary>
        /// Initializes the singleton application object.  This is the first line of authored code
        /// executed, and as such is the logical equivalent of main() or WinMain().
        /// </summary>
        public App()
        {
            // System.Diagnostics.Debugger.Break();
            this.InitializeComponent();
            this.Suspending += OnSuspending;
        }

        /// <summary>
        /// Invoked when the application is launched normally by the end user.  Other entry points
        /// will be used such as when the application is launched to open a specific file.
        /// </summary>
        /// <param name="e">Details about the launch request and process.</param>
        protected override void OnLaunched(LaunchActivatedEventArgs e)
        {

            /*#if DEBUG
                        if (System.Diagnostics.Debugger.IsAttached)
                        {
                            this.DebugSettings.EnableFrameRateCounter = true;
                        }
            #endif*/

            Frame rootFrame = Window.Current.Content as Frame;

            // Do not repeat app initialization when the Window already has content,
            // just ensure that the window is active
            if (rootFrame == null)
            {
                // Create a Frame to act as the navigation context and navigate to the first page
                rootFrame = new Frame();
                // Set the default language
                rootFrame.Language = Windows.Globalization.ApplicationLanguages.Languages[0];

                rootFrame.NavigationFailed += OnNavigationFailed;

                if (e.PreviousExecutionState == ApplicationExecutionState.Terminated)
                {
                    //TODO: Load state from previously suspended application
                }

                // Place the frame in the current Window
                Window.Current.Content = rootFrame;
            }

            if (rootFrame.Content == null)
            {
                // When the navigation stack isn't restored navigate to the first page,
                // configuring the new page by passing required information as a navigation
                // parameter

#if !FORCE_OOBE_WELCOME_SCREEN
                if (ApplicationData.Current.LocalSettings.Values.ContainsKey(Constants.HasDoneOOBEKey))
                {
                    rootFrame.Navigate(typeof(MainPage), e.Arguments);
                }
                else
#endif
                {
                    rootFrame.Navigate(typeof(OOBEWelcome), e.Arguments);
                }
                
            }
            // Ensure the current window is active
            Window.Current.Activate();

            Screensaver.InitializeScreensaver();
        }

        protected override void OnActivated(IActivatedEventArgs args)
        {
            // Spot if we are being activated due to inbound pairing request
            if (args.Kind == ActivationKind.DevicePairing)
            {
                // Ensure the main app loads first
                OnLaunched(null);

                // Get the arguments, which give information about the device which wants to pair with this app
                var devicePairingArgs = (DevicePairingActivatedEventArgs)args;
                var di = devicePairingArgs.DeviceInformation;

                // Automatically switch to Bluetooth Settings page
                NavigationUtils.NavigateToScreen(typeof(Settings));

                int bluetoothSettingsIndex = 2;
                Frame rootFrame = Window.Current.Content as Frame;
                ListView settingsListView = null;
                settingsListView = (rootFrame.Content as FrameworkElement).FindName("SettingsChoice") as ListView;
                settingsListView.Focus(FocusState.Programmatic);
                bluetoothSettingsIndex = Math.Min(bluetoothSettingsIndex, settingsListView.Items.Count - 1);
                settingsListView.SelectedIndex = bluetoothSettingsIndex;
                // Appropriate Bluetooth Listview grid content is forced by App_InboundPairingRequested call to SwitchToSelectedSettings

                // Fire the event letting subscribers know there's a new inbound request.
                // In this case Scenario should be subscribed.
                if (InboundPairingRequested != null)
                {
                    InboundPairingEventArgs inboundEventArgs = new InboundPairingEventArgs(di);
                    InboundPairingRequested(this, inboundEventArgs);
                }
            }
        }

        /// <summary>
        /// Invoked when Navigation to a certain page fails
        /// </summary>
        /// <param name="sender">The Frame which failed navigation</param>
        /// <param name="e">Details about the navigation failure</param>
        void OnNavigationFailed(object sender, NavigationFailedEventArgs e)
        {
            throw new Exception("Failed to load Page " + e.SourcePageType.FullName);
        }

        /// <summary>
        /// Invoked when application execution is being suspended.  Application state is saved
        /// without knowing whether the application will be terminated or resumed with the contents
        /// of memory still intact.
        /// </summary>
        /// <param name="sender">The source of the suspend request.</param>
        /// <param name="e">Details about the suspend request.</param>
        private void OnSuspending(object sender, SuspendingEventArgs e)
        {
            var deferral = e.SuspendingOperation.GetDeferral();
            //TODO: Save application state and stop any background activity
            deferral.Complete();
        }
    }
}
