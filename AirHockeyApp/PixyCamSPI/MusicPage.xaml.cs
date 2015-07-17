// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Storage;
using Windows.System.Threading;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace AirHockeyApp
{
    public sealed partial class MusicPage : Page
    {
        Dictionary<string, string> songPathDictionary;
        private SoundPlayer soundPlayerX, soundPlayerY;
        IAsyncAction mainThread;
        Robot robot;
        bool stopThread = false;
        string xSong, ySong;
        bool isPlaying = false;
        int tempo = 150;

        private const float FAST_ACCEL_Y = 900000, MEDIUM_ACCEL_Y = 700000, BASE_ACCEL_Y = 400000;
        private const float BASE_ACCEL_X = 1800000;

        public MusicPage()
        {
            this.InitializeComponent();
            outputTextBlock.Text = "";

            songPathDictionary = new Dictionary<string, string>();

            robot = new Robot();
        }

        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            await populateComboBoxes();
            xMotorComboBox.SelectedIndex = yMotorComboBox.SelectedIndex = 0;
        }

        protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            base.OnNavigatingFrom(e);


            stopAll();

            if (mainThread != null)
                mainThread.Cancel();

            MusicPlayer.Stop();

            if (robot != null)
                robot.Close();
        }

        private async Task populateComboBoxes()
        {
            yMotorComboBox.Items.Clear();
            xMotorComboBox.Items.Clear();

            xMotorComboBox.Items.Add("None");
            yMotorComboBox.Items.Add("None");

            var songList = await getFileList();
            foreach (string songPath in songList)
            {
                xMotorComboBox.Items.Add(songPath);
                yMotorComboBox.Items.Add(songPath);
            }

            tempoComboBox.Items.Clear();
            for (int i = 100; i <= 300; i++)
            {
                tempoComboBox.Items.Add(i);
            }
            tempoComboBox.SelectedValue = 150;

            xPitchCorrectComboBox.Items.Clear();
            yPitchCorrectComboBox.Items.Clear();

            for (int i = 1; i < 40; i++)
            {
                xPitchCorrectComboBox.Items.Add(i);
                yPitchCorrectComboBox.Items.Add(i);
            }

            xPitchCorrectComboBox.SelectedValue = soundPlayerX.PitchCorrect;
            yPitchCorrectComboBox.SelectedValue = soundPlayerY.PitchCorrect;
        }

        private async Task<List<string>> getFileList()
        {
            List<string> songList = new List<string>();
            StorageFolder InstallationFolder = Windows.ApplicationModel.Package.Current.InstalledLocation;
            var songFolder = await InstallationFolder.GetFolderAsync(@"Assets\Songs");
            var songFiles = await songFolder.GetFilesAsync();
            foreach (StorageFile file in songFiles)
            {
                songList.Add(file.Name);
                songPathDictionary.Add(file.Name, file.Path);
            }

            return songList;
        }

        private void playButton_Click(object sender, RoutedEventArgs e)
        {
            if (!isPlaying)
            {
                startSongThread();
                updatePlayButton();
            }
            else
            {
                stopAll();
                updatePlayButton();
            }
        }

        private void stopAll()
        {
            stopThread = true;
            robot.Stop();
            MusicPlayer.Stop();
            showMessage("Stopped");
        }

        private void homeButton_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(MainPage));
        }

        private async void playSong()
        {
            List<SoundPlayer> playerList = new List<SoundPlayer>();
            soundPlayerX = new SoundPlayer(robot.StepperX, Config.MAX_MALLET_OFFSET_X);
            soundPlayerY = new SoundPlayer(robot.StepperY, Config.MAX_MALLET_OFFSET_Y);

            var partX = await MusicPlayer.ReadNotesFromFile(@"Assets\Songs\" + xSong);
            if (partX != null)
            {
                soundPlayerX.SetSong(partX);
                playerList.Add(soundPlayerX);
            }

            var partY = await MusicPlayer.ReadNotesFromFile(@"Assets\Songs\" + ySong);
            if (partY != null)
            {
                soundPlayerY.SetSong(partY);
                playerList.Add(soundPlayerY);
            }

            MusicPlayer.PlaySong(tempo * 10, playerList.ToArray());

            showMessage("Done.");

            isPlaying = false;
        }

        private void xMotorComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            xSong = (string)xMotorComboBox.SelectedItem;
        }

        private void yMotorComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ySong = (string)yMotorComboBox.SelectedItem;
        }

        private void tempoComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            tempo = (int)tempoComboBox.SelectedValue;
        }

        private void xPitchCorrectComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            soundPlayerX.PitchCorrect = (int)xPitchCorrectComboBox.SelectedValue;
        }

        private void yPitchCorrectComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            soundPlayerY.PitchCorrect = (int)yPitchCorrectComboBox.SelectedValue;
        }

        private void startSongThread()
        {
            stopThread = false;
            isPlaying = true;

            updatePlayButton();

            mainThread = ThreadPool.RunAsync((s) =>
            {
                try
                {
                    showMessage("Resetting...");
                    warmUp();

                    if (!stopThread)
                    {
                        showMessage("Playing you a song...");
                        playSong();
                    }
                }
                catch (Exception err)
                {
                    showMessage("Error: " + err.Message);
                }
            }, WorkItemPriority.High);
        }

        private void warmUp()
        {
            robot.StepperX.SetMaxSpeed(Config.MOTOR_X_MAX_SPEED);
            robot.StepperY.SetMaxSpeed(Config.MOTOR_Y_MAX_SPEED);

            robot.StepperX.SetAcceleration(Config.MOTOR_X_ACCELERATION);
            robot.StepperY.SetAcceleration(Config.MOTOR_Y_ACCELERATION);

            try
            {
                showMessage("Resetting motors...");
                robot.MoveMotorsToZero();
            }
            catch (Exception) { }
        }

        private async void updatePlayButton()
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                playButton.Content = (isPlaying) ? "Stop" : "Play";
            });
        }

        private async void showMessage(string text)
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                outputTextBlock.Text = text;
            });
        }

    }
}
