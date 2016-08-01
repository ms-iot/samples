// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.ObjectModel;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Media.Capture;
using Windows.Media.MediaProperties;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace AudioInSample
{
    /// <summary>
    /// Audio In sample that allows you to record an audio clip and play it back
    /// </summary>
    public sealed partial class MainPage : Page
    {
        Windows.Media.Capture.MediaCapture audioCapture;
        MediaCaptureInitializationSettings captureInitSettings;
        ObservableCollection<DeviceInformation> deviceList;
        bool isRecording = false;
        
        string audioFileName = null;

        public MainPage()
        {
            this.InitializeComponent();
            deviceList = new ObservableCollection<DeviceInformation>();
            deviceListView.ItemsSource = deviceList;
            outputTextBlock.Text = "Select an audio device to start recording.";
            media.MediaEnded += Media_MediaEnded;
        }

        private void Media_MediaEnded(object sender, RoutedEventArgs e)
        {
            outputTextBlock.Text = "Done playing.";
            refreshUI();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            EnumerateAudioDevice();
            refreshUI();
        }

        private async void startRecord(object sender, RoutedEventArgs e)
        {
            var selected = deviceListView.SelectedItem as DeviceInformation;
            if (selected != null)
            {
                InitCaptureSettings(selected.Id);
                await InitMediaCapture();

                var storageFile = await Windows.Storage.KnownFolders.VideosLibrary.CreateFileAsync("audioOut.mp3", Windows.Storage.CreationCollisionOption.GenerateUniqueName);
                audioFileName = storageFile.Name;
                MediaEncodingProfile profile = null;
                profile = MediaEncodingProfile.CreateM4a(Windows.Media.MediaProperties.AudioEncodingQuality.Auto);
                await audioCapture.StartRecordToStorageFileAsync(profile, storageFile);
                isRecording = true;
                outputTextBlock.Text = "Recording...";
            }
            else
            {
                outputTextBlock.Text = "Error: No audio device selected.";
            }

            refreshUI();
        }

        private async void EnumerateAudioDevice()
        {
            deviceList.Clear();
            var devices = await DeviceInformation.FindAllAsync(DeviceClass.AudioCapture);
            if (devices.Count > 0)
            {
                for (var i = 0; i < devices.Count; i++)
                {
                    deviceList.Add(devices[i]);
                }
            }
        }

        private void InitCaptureSettings(string id)
        {
            // Set the capture setting
            captureInitSettings = null;
            captureInitSettings = new Windows.Media.Capture.MediaCaptureInitializationSettings();

            captureInitSettings.AudioDeviceId = id;

            captureInitSettings.StreamingCaptureMode = Windows.Media.Capture.StreamingCaptureMode.AudioAndVideo;
        }

        private async Task InitMediaCapture()
        {
            audioCapture = null;
            audioCapture = new Windows.Media.Capture.MediaCapture();

            // for dispose purpose
            (App.Current as App).MediaCapture = audioCapture;
            await audioCapture.InitializeAsync(captureInitSettings);

        }

        private async void endRecord(object sender, RoutedEventArgs e)
        {
            if (isRecording)
            {
                await audioCapture.StopRecordAsync();
                isRecording = false;
                refreshUI();
                outputTextBlock.Text = "Recording stopped.";
            }
        }

        private async void refreshUI()
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                startRecordButton.IsEnabled = !isRecording;
                endRecordButton.IsEnabled = isRecording;
            });
        }

        private async void playRecordedAudio(object sender, RoutedEventArgs e)
        {
            Windows.Storage.StorageFile storageFile = await Windows.Storage.KnownFolders.VideosLibrary.GetFileAsync(audioFileName);
            var stream = await storageFile.OpenAsync(Windows.Storage.FileAccessMode.Read);
            
            if (null != stream)
            {
                media.SetSource(stream, storageFile.ContentType);
                media.Play();
                outputTextBlock.Text = "Playing audio...";

                startRecordButton.IsEnabled = endRecordButton.IsEnabled = false;
            }
            else
            {
                outputTextBlock.Text = "Error: No audio file found";
            }
        }
    }
}
