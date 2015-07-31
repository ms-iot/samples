// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

#region mynamespace
using Windows.Media.Capture;
using Windows.Media.Effects;
using Windows.Graphics.Display;

#endregion
// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace VideoCaptureSample
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        MediaCaptureInitializationSettings captureInitSettings;
        List<Windows.Devices.Enumeration.DeviceInformation> deviceList;
        Windows.Media.MediaProperties.MediaEncodingProfile profile;
        Windows.Media.Capture.MediaCapture mediaCapture;

        bool recording = false;
        bool previewing = false;

        public string fileName;
        public MainPage()
        {
            this.InitializeComponent();
            EnumerateCameras();
        }

        // Start the Video Capture
        private async void StartMediaCaptureSession()
        {   
            var storageFile = await Windows.Storage.KnownFolders.VideosLibrary.CreateFileAsync("cameraCapture.wmv", Windows.Storage.CreationCollisionOption.GenerateUniqueName);
            fileName = storageFile.Name;

            await mediaCapture.StartRecordToStorageFileAsync(profile, storageFile);
            recording = true;

            // start the preview      
            capturePreview.Source=mediaCapture;                
            await mediaCapture.StartPreviewAsync();            
        }

        // Stop the video capture
        private async void StopMediaCaptureSession()
        {
            await mediaCapture.StopRecordAsync();
            recording = false;
            (App.Current as App).IsRecording = false;

            //stop the preview
            await mediaCapture.StopPreviewAsync();
            

        }

        private async void EnumerateCameras()
        {
            var devices = await Windows.Devices.Enumeration.DeviceInformation.FindAllAsync(Windows.Devices.Enumeration.DeviceClass.VideoCapture);
            deviceList = new List<Windows.Devices.Enumeration.DeviceInformation>();

            if (devices.Count > 0)
            {
                for(var i = 0; i < devices.Count; i++)
                {
                    deviceList.Add(devices[i]);
                }

                InitCaptureSettings();
                InitMediaCapture();

            }
        }

        private void InitCaptureSettings()
        {
            // Set the Capture Setting
            captureInitSettings = null;
            captureInitSettings = new Windows.Media.Capture.MediaCaptureInitializationSettings();
            captureInitSettings.AudioDeviceId = "";
            captureInitSettings.VideoDeviceId = "";
            captureInitSettings.StreamingCaptureMode = Windows.Media.Capture.StreamingCaptureMode.AudioAndVideo;
            captureInitSettings.PhotoCaptureSource = Windows.Media.Capture.PhotoCaptureSource.VideoPreview;
            
            if (deviceList.Count > 0)
            {
                captureInitSettings.VideoDeviceId = deviceList[0].Id;
            }

        }

        private async void InitMediaCapture()
        {
            mediaCapture = null;
            mediaCapture = new Windows.Media.Capture.MediaCapture();

            // for dispose purpose
            (App.Current as App).MediaCapture = mediaCapture;
            (App.Current as App).PreviewElement = capturePreview;

            await mediaCapture.InitializeAsync(captureInitSettings);

            // Add video stabilization effect during Live Capture
            //await _mediaCapture.AddEffectAsync(MediaStreamType.VideoRecord, Windows.Media.VideoEffects.VideoStabilization, null); //this will be deprecated soon
            Windows.Media.Effects.VideoEffectDefinition def = new Windows.Media.Effects.VideoEffectDefinition(Windows.Media.VideoEffects.VideoStabilization);
            await mediaCapture.AddVideoEffectAsync(def, MediaStreamType.VideoRecord);

           CreateProfile();

            // start preview

            capturePreview.Source = mediaCapture;
            
            DisplayInformation.AutoRotationPreferences = DisplayOrientations.None;

        //// set the video Rotation
        //    _mediaCapture.SetPreviewRotation(VideoRotation.Clockwise90Degrees);
        //    _mediaCapture.SetRecordRotation(VideoRotation.Clockwise90Degrees);
        }

        //Create a profile
        private void CreateProfile()
        {
            profile = Windows.Media.MediaProperties.MediaEncodingProfile.CreateMp4(Windows.Media.MediaProperties.VideoEncodingQuality.Qvga);

            // Use MediaEncodingProfile to encode the profile
            System.Guid MFVideoRotationGuild = new System.Guid("C380465D-2271-428C-9B83-ECEA3B4A85C1");
            int MFVideoRotation = ConvertVideoRotationToMFRotation(VideoRotation.None);
            profile.Video.Properties.Add(MFVideoRotationGuild, PropertyValue.CreateInt32(MFVideoRotation));


            // add the mediaTranscoder 
            var transcoder = new Windows.Media.Transcoding.MediaTranscoder();
            transcoder.AddVideoEffect(Windows.Media.VideoEffects.VideoStabilization);
            


        }
        int ConvertVideoRotationToMFRotation(VideoRotation rotation)
        {
            int MFVideoRotation = 0;
            switch (rotation)
            {
                case VideoRotation.Clockwise90Degrees:
                    MFVideoRotation = 90;
                    break;
                case VideoRotation.Clockwise180Degrees:
                    MFVideoRotation = 180;
                    break;
                case VideoRotation.Clockwise270Degrees:
                    MFVideoRotation = 270;
                    break;
            }
            return MFVideoRotation;
        }

        private void startCapture(object sender, RoutedEventArgs e)
        {
            StartMediaCaptureSession();
        }

        private void stopCapture(object sender, RoutedEventArgs e)
        {
            StopMediaCaptureSession();
        }

        // open the file in stream here

        private async void playVideo(object sender, RoutedEventArgs e)
        {
            Windows.Storage.StorageFile storageFile = await Windows.Storage.KnownFolders.VideosLibrary.GetFileAsync(fileName);

            var stream = await storageFile.OpenAsync(Windows.Storage.FileAccessMode.Read);

            // mediaControl is a MediaElement defined in XAML
            if (null != stream)
            {
                media.Visibility = Visibility.Visible;
                media.SetSource(stream, storageFile.ContentType);

                media.Play();
            }

        }

    }
}
