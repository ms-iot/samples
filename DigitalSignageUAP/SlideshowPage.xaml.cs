/**
    Copyright(c) Microsoft Open Technologies, Inc.All rights reserved.
   The MIT License(MIT)
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
**/

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Core;
using Windows.UI.Popups;
using Windows.UI.Input;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Media.Imaging;
using Windows.UI.Xaml.Navigation;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.Storage.Pickers;
using Windows.Web.Http;
using System.Xml.Linq;
using System.Threading;
using System.Text;

namespace DigitalSignageUAP
{
    struct DisplayObject
    {
        public StorageFile file; // for image, and video that are saved to local
        public Uri uri; //for webpage
        public int duration; // only need this when it's a image or URI
    }
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class SlideshowPage : Page
    {
        const string szRootFolder = "DigitalSignage";
        const string szAudioFolder = "Audio";
        const string szDisplayFolder = "Display";
        List<string> imageExtensions = new List<string>(new string[] { ".bmp", ".gif", ".ico", ".jpg", ".png", ".wdp", ".tiff" }); // MSDN
        readonly string defaultConfigFilePath = @"Assets\config.xml";
        const string configValueName = "ConfigFilePath";        
        static List<object> audioList;
        static List<object> displayList;
        int currentIndexOfAudio = 0;
        int currentIndexOfDisplay = 0;
        static DispatcherTimer AcceptUserInputTimer;
        DispatcherTimer DisplayImageWEBTimer;
        StorageFolder localFolder = ApplicationData.Current.LocalFolder;
        static string currentConfigFilePath;
        ApplicationDataContainer localSettings = Windows.Storage.ApplicationData.Current.LocalSettings;
        BitmapImage imageSource = new BitmapImage();

        /// <summary>
        /// 
        /// </summary>
        public SlideshowPage()
        {
            this.InitializeComponent();

            // if we do not have the config in app's local settings, it means it's first time app launch
            // write the config path to local settings
            if (localSettings.Values[configValueName] == null)
            {
                localSettings.Values[configValueName] = defaultConfigFilePath;
            }
            
            // get the current config path from local settings
            currentConfigFilePath = (string)localSettings.Values[configValueName];

            this.NavigationCacheMode = Windows.UI.Xaml.Navigation.NavigationCacheMode.Enabled;

            Window.Current.CoreWindow.KeyDown += CoreWindow_KeyDown;
            this.PointerMoved += SlideshowPage_PointerMoved;
            DisplayImageWEBTimer = new DispatcherTimer();
            DisplayImageWEBTimer.Tick += DisplayImageWEBTimer_Tick;
            AcceptUserInputTimer = new DispatcherTimer();
            AcceptUserInputTimer.Interval = new TimeSpan(0, 0, 5);
            AcceptUserInputTimer.Tick += AcceptUserInputTimer_Tick;
            audioInstance.MediaEnded += audioInstance_MediaEnded;
            videoInstance.MediaEnded += videoInstance_MediaEnded;

            displayList = new List<object>();
            audioList = new List<object>();

            AcceptUserInputTimer.Start();
            StartSlideShow();

            GlobalTimerWrapper.StartReloadContentTimer(this); // when slideshow is being played back, we want it resync the config file and reload content at 12:00AM
        }

        private void CoreWindow_KeyDown(CoreWindow sender, KeyEventArgs args)
        {
            if (AcceptUserInputTimer.IsEnabled == false &&
                (videoInstance.Visibility == Visibility.Visible ||
                 audioInstance.Visibility == Visibility.Visible ||
                 imageInstance.Visibility == Visibility.Visible))
            {
                audioInstance.Visibility = Visibility.Collapsed;
                imageInstance.Visibility = Visibility.Collapsed;
                videoInstance.Visibility = Visibility.Collapsed;
                this.Frame.Navigate(typeof(MainPage));
            }
        }

        void updateTimer_Tick(object sender, object e)
        {
        }

        void DisplayImageWEBTimer_Tick(object sender, object e)
        {
            DisplayNext(); // move to the next one in display queue
        }

        void AcceptUserInputTimer_Tick(object sender, object e)
        {
            AcceptUserInputTimer.Stop();
        }

        void SlideshowPage_PointerMoved(object sender, Windows.UI.Xaml.Input.PointerRoutedEventArgs e)
        {
            if (AcceptUserInputTimer.IsEnabled == false)
            {
                audioInstance.Visibility = Visibility.Collapsed;
                imageInstance.Visibility = Visibility.Collapsed;
                videoInstance.Visibility = Visibility.Collapsed;
                this.Frame.Navigate(typeof(MainPage));
            }
        }

        public async void StartSlideShow()
        {
            await GetConfigAndParse();
            DisplayNext();
        }

        /// <summary>
        /// upload telemetry when navigating to this page
        /// </summary>
        /// <param name="e"></param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            videoInstance.Visibility = Visibility.Visible;
            audioInstance.Visibility = Visibility.Visible;
            imageInstance.Visibility = Visibility.Visible;
            base.OnNavigatedTo(e);
        }

        /// <summary>
        /// upload telemetry when navigating from this page (leaving this page)
        /// </summary>
        /// <param name="e"></param>
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="command"></param>
        void InvalidConfigDialogCommandInvokeHandler(IUICommand command)
        {
            audioInstance.Visibility = Visibility.Collapsed;
            imageInstance.Visibility = Visibility.Collapsed;
            videoInstance.Visibility = Visibility.Collapsed;
            this.Frame.Navigate(typeof(MainPage));
        }

        /// <summary>
        /// check if next one is image, continue the audio and display the image for seconds defined by the timer interval
        /// of next is video, pause the audio, and play back the video
        /// </summary>
        async void DisplayNext()
        {
            if (displayList.Count == 0)
            {
                MessageDialog dialog = new MessageDialog("You've entered an invalid or empty config file, it's been reset to default.");
                localSettings.Values[configValueName] = defaultConfigFilePath;
                dialog.Commands.Add(new UICommand("OK", new UICommandInvokedHandler(InvalidConfigDialogCommandInvokeHandler)));
                dialog.ShowAsync(); // show a dialog with only one button to return to homepage
                return;
            }

            DisplayObject currentDO = (DisplayObject) displayList[currentIndexOfDisplay];

            if (currentDO.uri != null) // we're dealing with a WEB Page, show the WebView instance
            {
                videoInstance.Stop();
                videoInstance.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                imageInstance.Visibility = Windows.UI.Xaml.Visibility.Collapsed;

                DisplayImageWEBTimer.Interval = new TimeSpan(0, 0, currentDO.duration);
                DisplayImageWEBTimer.Start();

                PlayAudio();
            }
            else // it must be StorageFile, i.e. image or video
            {
                if (imageExtensions.Contains(currentDO.file.FileType.ToLower())) // image, will start or resume audio play back
                {
                    videoInstance.Stop();
                    videoInstance.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                    imageInstance.Visibility = Windows.UI.Xaml.Visibility.Visible;
                    imageSource = new BitmapImage(new Uri(currentDO.file.Path));
                    imageInstance.Width = imageSource.DecodePixelHeight = (int)this.ActualWidth;
                    imageInstance.Source = imageSource;
                    DisplayImageWEBTimer.Interval = new TimeSpan(0, 0, currentDO.duration);
                    DisplayImageWEBTimer.Start();
                    PlayAudio();
                }
                else // video, we'll pause audio playback
                {
                    audioInstance.Pause();
                    videoInstance.Source = new Uri(currentDO.file.Path);
                    audioInstance.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                    imageInstance.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
                    videoInstance.Visibility = Windows.UI.Xaml.Visibility.Visible;
                    videoInstance.Play();
                }
            }

            currentIndexOfDisplay = (++currentIndexOfDisplay) % displayList.Count; // make the index in a loop
        }

        /// <summary>
        /// when an audio is finished, call this function to move to next audio
        /// this could also be called when a video is finished, pick up from where it was paused
        /// </summary>
        void PlayAudio()
        {
            if (audioList.Count == 0)
                return;

            if (audioInstance.CurrentState == MediaElementState.Paused)
            {
                videoInstance.Visibility = Visibility.Collapsed;
                audioInstance.Visibility = Visibility.Visible;
                audioInstance.Play();
            }
            else if (audioInstance.CurrentState != MediaElementState.Playing)
            {
                audioInstance.Source = new System.Uri(((DisplayObject)audioList[currentIndexOfAudio]).file.Path);
                videoInstance.Visibility = Visibility.Collapsed;
                audioInstance.Visibility = Visibility.Visible;
                audioInstance.Play();
                currentIndexOfAudio = (++currentIndexOfAudio) % audioList.Count;
            }
        }
        
        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public async Task GetConfigAndParse()
        {
            displayList.Clear(); // clear the list since this function is called when the app resyncs content at mid night
            audioList.Clear(); // see above

            StorageFolder displayFolder = await Windows.Storage.ApplicationData.Current.LocalFolder.CreateFolderAsync("DigitalSignal\\Display", CreationCollisionOption.OpenIfExists);
            StorageFolder audioFolder = await Windows.Storage.ApplicationData.Current.LocalFolder.CreateFolderAsync("DigitalSignal\\Audio", CreationCollisionOption.OpenIfExists);

            try
            {
                HttpClient configClient = new HttpClient();
                string configStr = File.ReadAllText(currentConfigFilePath);
                XElement xele = XElement.Parse(configStr);

                foreach (XElement xe in xele.Elements())
                {
                    StorageFolder tmp = xe.Name == "Audio" ? audioFolder : displayFolder;
                    List<object> tmpList = xe.Name == "Audio" ? audioList : displayList;

                    foreach (XElement fileElement in xe.Elements()) // audio
                    {
                        if (fileElement.Attribute("type") != null && fileElement.Attribute("type").Value == "webpage") // the display is a webpage, not image, not video, we create and store a new Uri Object. Only url display type has type attribute
                        {
                            DisplayObject DO = new DisplayObject();
                            DO.uri = new Uri(fileElement.Attribute("path").Value);
                            DO.duration = Convert.ToInt32(fileElement.Attribute("duration").Value);
                            tmpList.Add(DO);
                        }
                        else
                        {
                            DisplayObject DO = new DisplayObject();
                            string filename = fileElement.Attribute("path").Value.Substring(fileElement.Attribute("path").Value.LastIndexOf('/') + 1);
                            StorageFile file = await tmp.CreateFileAsync(filename, CreationCollisionOption.ReplaceExisting);
                            Uri fileElementUri;

                            if (Uri.TryCreate(fileElement.Attribute("path").Value, UriKind.Absolute, out fileElementUri)
                                && Uri.CheckSchemeName(fileElementUri.Scheme))      // check if URL or local media
                            {
                                HttpClient client = new HttpClient();
                                HttpResponseMessage response = await client.GetAsync(fileElementUri);
                                await FileIO.WriteBufferAsync(file, await response.Content.ReadAsBufferAsync());
                            }
                            else
                            {
                                byte[] bytes = File.ReadAllBytes(filename);
                                await FileIO.WriteBufferAsync(file, WindowsRuntimeBufferExtensions.AsBuffer(bytes));
                            }

                            if (fileElement.Attribute("duration") != null) // this is an image
                                DO.duration = Convert.ToInt32(fileElement.Attribute("duration").Value);

                            DO.file = file;

                            tmpList.Add(DO);
                        }
                    }
                }
            }
            catch (Exception)
            {
                // use this to capture any config xml parsing error, we don't expose what they are
                // but only popup a dialog to user saying something is wrong..
            }
        }

        /// <summary>
        /// when a video is finished, move to next displayObject
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void videoInstance_MediaEnded(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            // videoInstance.IsFullWindow = false;
            DisplayNext();
        }

        /// <summary>
        /// a audio loop, when one audio is finished, move to next one
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void audioInstance_MediaEnded(object sender, Windows.UI.Xaml.RoutedEventArgs e)
        {
            PlayAudio();
        }
    }
}
