// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Foundation;
using Windows.Graphics.Display;
using Windows.Media.Capture;
using Windows.Media.MediaProperties;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace VideoCaptureSample
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        MediaCaptureInitializationSettings captureInitSettings;
        List<DeviceInformation> deviceList;
        MediaEncodingProfile profile;

        public MediaCapture mediaCapture;
        public bool isRecording;
        public string fileName;

        public MainPage()
        {
            this.InitializeComponent();
            EnumerateCameras();
        }

        private async Task StartMediaCaptureSession()
        {
            await StopMediaCaptureSession();

            var storageFile = await Windows.Storage.KnownFolders.VideosLibrary.CreateFileAsync("cameraCapture.wmv", Windows.Storage.CreationCollisionOption.GenerateUniqueName);
            fileName = storageFile.Name;

            await mediaCapture.StartRecordToStorageFileAsync(profile, storageFile);
            await mediaCapture.StartPreviewAsync();
            isRecording = true;
        }

        private async Task StopMediaCaptureSession()
        {
            if (isRecording)
            {
                await mediaCapture.StopPreviewAsync();
                await mediaCapture.StopRecordAsync();
                isRecording = false;
            }
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
            mediaCapture = new Windows.Media.Capture.MediaCapture();
            await mediaCapture.InitializeAsync(captureInitSettings);

            // Add video stabilization effect during Live Capture
            Windows.Media.Effects.VideoEffectDefinition def = new Windows.Media.Effects.VideoEffectDefinition(Windows.Media.VideoEffects.VideoStabilization);
            await mediaCapture.AddVideoEffectAsync(def, MediaStreamType.VideoRecord);

            profile = Windows.Media.MediaProperties.MediaEncodingProfile.CreateMp4(Windows.Media.MediaProperties.VideoEncodingQuality.Qvga);

            // Use MediaEncodingProfile to encode the profile
            System.Guid MFVideoRotationGuild = new System.Guid("C380465D-2271-428C-9B83-ECEA3B4A85C1");
            int MFVideoRotation = ConvertVideoRotationToMFRotation(VideoRotation.None);
            profile.Video.Properties.Add(MFVideoRotationGuild, PropertyValue.CreateInt32(MFVideoRotation));

            // add the mediaTranscoder 
            var transcoder = new Windows.Media.Transcoding.MediaTranscoder();
            transcoder.AddVideoEffect(Windows.Media.VideoEffects.VideoStabilization);

            // wire to preview XAML element
            capturePreview.Source = mediaCapture;
            DisplayInformation.AutoRotationPreferences = DisplayOrientations.None;
        }

        private int ConvertVideoRotationToMFRotation(VideoRotation rotation)
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

        private async void startCapture(object sender, RoutedEventArgs e)
        {
            await StartMediaCaptureSession();
        }

        private async void stopCapture(object sender, RoutedEventArgs e)
        {
            await StopMediaCaptureSession();
        }

        private async void playVideo(object sender, RoutedEventArgs e)
        {
            Windows.Storage.StorageFile storageFile = await Windows.Storage.KnownFolders.VideosLibrary.GetFileAsync(fileName);

            using (var stream = await storageFile.OpenAsync(Windows.Storage.FileAccessMode.Read))
            {
                if (null != stream)
                {
                    media.Visibility = Visibility.Visible;
                    media.SetSource(stream, storageFile.ContentType);
                    media.Play();
                }
            }
        }
    }
}
