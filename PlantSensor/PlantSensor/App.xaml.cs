// Copyright (c) Microsoft. All rights reserved.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace PlantSensor
{    
    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    sealed partial class App : Application
    {
        //this helps find the data from the sensors
        public static SensorDataProvider SensorProvider;

        //these are the files that the app reads from for the sensors
        public static Windows.Storage.StorageFile BrightnessFile;
        public static Windows.Storage.StorageFile TemperatureFile;
        public static Windows.Storage.StorageFile SoilMoistureFile;
        public static Windows.Storage.StorageFile TwitterFile;

        //this is the folder in which the files are stored
        static Windows.Storage.StorageFolder storageFolder;

        //when the files are read, then they are stored in these lists
        public static IList<string> Brightnessresult;
        public static IList<string> Temperatureresult;
        public static IList<string> SoilMoistureresult;
        public static IList<string> Twitterresult;

        //these lists are where new data is temporarily stored so that we are not 
        //readong and writing to files that often
        public static List<String> BrightnessList;
        public static List<String> TemperatureList;
        public static List<String> SoilMoistureList;

        //this variable holds the settings for the plant
        public static Settings PlantSettings;

        /// <summary>
        /// Initializes the singleton application object.  This is the first line of authored code
        /// executed, and as such is the logical equivalent of main() or WinMain().
        /// </summary>
        public App()
        {
            PlantSettings = new Settings();
            this.InitializeComponent();
            this.Suspending += OnSuspending;
            SensorProvider = new SensorDataProvider();
        }

        public async Task setUpFile()
        {
            storageFolder = Windows.Storage.ApplicationData.Current.LocalFolder;
            try
            {
                BrightnessFile = await storageFolder.GetFileAsync(FileNames.BrightnessfileName);
            }
            catch (FileNotFoundException e)
            {
                BrightnessFile = await storageFolder.CreateFileAsync(FileNames.BrightnessfileName);
            }

            try
            {
                TemperatureFile = await storageFolder.GetFileAsync(FileNames.TemperaturefileName);
            }
            catch (FileNotFoundException e)
            {
                TemperatureFile = await storageFolder.CreateFileAsync(FileNames.TemperaturefileName);
            }

            try
            {
                SoilMoistureFile = await storageFolder.GetFileAsync(FileNames.SoilMoisturefileName);
                Debug.WriteLine("Old Files are used");
            }
            catch (FileNotFoundException e)
            {
                SoilMoistureFile = await storageFolder.CreateFileAsync(FileNames.SoilMoisturefileName);
                Debug.WriteLine("New Files were created");
            }

            try
            {
                TwitterFile = await storageFolder.GetFileAsync(FileNames.TwitterfileName);
                Debug.WriteLine("Old twitter Files are used");
            }
            catch (FileNotFoundException e)
            {
                TwitterFile = await storageFolder.CreateFileAsync(FileNames.TwitterfileName);
                Debug.WriteLine("new twitter Files are used");
            }

            Brightnessresult = await Windows.Storage.FileIO.ReadLinesAsync(BrightnessFile);
            Temperatureresult = await Windows.Storage.FileIO.ReadLinesAsync(TemperatureFile);
            SoilMoistureresult = await Windows.Storage.FileIO.ReadLinesAsync(SoilMoistureFile);
            Twitterresult = await Windows.Storage.FileIO.ReadLinesAsync(TwitterFile);

            BrightnessList = new List<string>();
            TemperatureList = new List<string>();
            SoilMoistureList = new List<string>();

        }
        /// <summary>
        /// Invoked when the application is launched normally by the end user.  Other entry points
        /// will be used such as when the application is launched to open a specific file.
        /// </summary>
        /// <param name="e">Details about the launch request and process.</param>
        protected async override void OnLaunched(LaunchActivatedEventArgs e)
        {
            await setUpFile();
            await App.SensorProvider.mcp3008.Initialize();
            await App.SensorProvider.BMP280.Initialize();
            try
            {
                PlantSettings = await Settings.Load("settings.txt");
            }
            catch
            {

            }
#if DEBUG
            if (System.Diagnostics.Debugger.IsAttached)
            {
                //this.DebugSettings.EnableFrameRateCounter = true;
            }
#endif
            Frame rootFrame = Window.Current.Content as Frame;

            // Do not repeat app initialization when the Window already has content,
            // just ensure that the window is active
            if (rootFrame == null)
            {
                // Create a Frame to act as the navigation context and navigate to the first page
                rootFrame = new Frame();

                rootFrame.NavigationFailed += OnNavigationFailed;

                if (e.PreviousExecutionState == ApplicationExecutionState.Terminated)
                {
                    //TODO: Load state from previously suspended application
                }

                // Place the frame in the current Window
                Window.Current.Content = rootFrame;
            }

            if (e.PrelaunchActivated == false)
            {
                if (rootFrame.Content == null)
                {
                    // When the navigation stack isn't restored navigate to the first page,
                    // configuring the new page by passing required information as a navigation
                    // parameter
                    rootFrame.Navigate(typeof(MainPage), e.Arguments);
                }
                // Ensure the current window is active
                Window.Current.Activate();
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
