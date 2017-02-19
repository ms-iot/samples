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
using Windows.Media.Playback;

namespace AudioInSample
{
    /// <summary>
    /// Audio In sample that allows you to record an audio clip and play it back
    /// </summary>
    public sealed partial class MainPage : Page
    {
        Windows.Media.Capture.MediaCapture audioCapture;
        MediaCaptureInitializationSettings captureInitSettings;
        ObservableCollection<DeviceInformation> captureDeviceList;
        ObservableCollection<DeviceInformation> renderDeviceList;
        Windows.Media.Playback.MediaPlayer mediaPlayer;

        bool isRecording = false;
        
        string audioFileName = null;

        public MainPage()
        {
            this.InitializeComponent();

            mediaPlayer = new MediaPlayer();

            captureDeviceList = new ObservableCollection<DeviceInformation>();
            captureDeviceListView.ItemsSource = captureDeviceList;

            renderDeviceList = new ObservableCollection<DeviceInformation>();
            renderDeviceListView.ItemsSource = renderDeviceList;

            renderDeviceListView.SelectionChanged += RenderDeviceListView_SelectionChanged;

            outputTextBlock.Text = "Ready to record.";
            mediaPlayer.MediaEnded += MediaPlayer_MediaEnded;
        }

        private void RenderDeviceListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            mediaPlayer.AudioDevice = renderDeviceList[renderDeviceListView.SelectedIndex];
        }


        private async void MediaPlayer_MediaEnded(MediaPlayer sender, object args)
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                outputTextBlock.Text = "Done playing.";
                refreshUI();
            });

        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            EnumerateAudioDevice();
            refreshUI();
        }

        private async void startRecord(object sender, RoutedEventArgs e)
        {
            var selected = captureDeviceListView.SelectedItem as DeviceInformation;
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
            captureDeviceList.Clear();
            var devices = await DeviceInformation.FindAllAsync(DeviceClass.AudioCapture);
            if (devices.Count > 0)
            {
                for (var i = 0; i < devices.Count; i++)
                {
                    captureDeviceList.Add(devices[i]);
                }
                captureDeviceListView.SelectedItem = devices[0];
                
            }

            renderDeviceList.Clear();
            devices = await DeviceInformation.FindAllAsync(DeviceClass.AudioRender);
            if (devices.Count > 0)
            {
                for (var i = 0; i < devices.Count; i++)
                {
                    renderDeviceList.Add(devices[i]);
                }
                renderDeviceListView.SelectedItem = devices[0];
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

            media.SetMediaPlayer(mediaPlayer);
            media.Source = Windows.Media.Core.MediaSource.CreateFromStorageFile(storageFile);
            mediaPlayer.Play();
            outputTextBlock.Text = "Playing audio...";
            startRecordButton.IsEnabled = endRecordButton.IsEnabled = false;
        }
    }
}
