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

#region my reference namespace
using Windows.Media.Capture;
using Windows.Media.Audio;
using Windows.Storage;
using Windows.Media.MediaProperties;


#endregion
// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace AudioInSample
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        Windows.Media.Capture.MediaCapture audioCapture;
        MediaCaptureInitializationSettings captureInitSettings;
        List<Windows.Devices.Enumeration.DeviceInformation> deviceList;

       // string audioFileName = "audioOut.mp3";
        string audioFileName=null;
        void startoPlay()
        {
            media.Play();
        }
        public MainPage()
        {
            this.InitializeComponent();
            EnumerateAudioDevice();           

        }

        private async void startRecord(object sender, RoutedEventArgs e)
        {
            var storageFile = await Windows.Storage.KnownFolders.VideosLibrary.CreateFileAsync("audioOut.mp3", Windows.Storage.CreationCollisionOption.GenerateUniqueName);
            audioFileName = storageFile.Name;
            MediaEncodingProfile profile = null;
            profile = MediaEncodingProfile.CreateM4a(Windows.Media.MediaProperties.AudioEncodingQuality.Auto);
            await audioCapture.StartRecordToStorageFileAsync(profile, storageFile);

        }
          private async void EnumerateAudioDevice()
        {
            var devices = await Windows.Devices.Enumeration.DeviceInformation.FindAllAsync(Windows.Devices.Enumeration.DeviceClass.AudioCapture);
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
            // Set the capture setting
            captureInitSettings = null;
            captureInitSettings = new Windows.Media.Capture.MediaCaptureInitializationSettings();

            captureInitSettings.AudioDeviceId = "";

            captureInitSettings.StreamingCaptureMode = Windows.Media.Capture.StreamingCaptureMode.AudioAndVideo;
            if (deviceList.Count > 0)
            {
                captureInitSettings.AudioDeviceId = deviceList[0].Id;
            }
        }
        private async void InitMediaCapture()
        {
            audioCapture = null;
            audioCapture = new Windows.Media.Capture.MediaCapture();

            // for dispose purpose
            (App.Current as App).MediaCapture = audioCapture;
            await audioCapture.InitializeAsync(captureInitSettings);
           // CreateProfile();

        }
        //public void CreateProfile()
        //{
        //    _profile = Windows.Media.MediaProperties.MediaEncodingProfile.CreateMp3(Windows.Media.MediaProperties.AudioEncodingQuality.Auto);
        //}

        private async void endRecord(object sender, RoutedEventArgs e)
        {
            await audioCapture.StopRecordAsync();
        }

        private async void playRecordedAudio(object sender, RoutedEventArgs e)
        {
            Windows.Storage.StorageFile storageFile = await Windows.Storage.KnownFolders.VideosLibrary.GetFileAsync(audioFileName);
            var stream = await storageFile.OpenAsync(Windows.Storage.FileAccessMode.Read);


            if (null != stream)
            {
                media.SetSource(stream, storageFile.ContentType);

                media.Play();
            }
        }
    }
}
