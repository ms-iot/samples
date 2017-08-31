using IoTCoreDefaultApp.Utils;
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
using Windows.System;
using Windows.UI.Core;
using Windows.UI.ViewManagement;
using System.Globalization;
using System.Text.RegularExpressions;

namespace IoTCoreDefaultApp
{
    /// <summary>
    /// Page for WebBrowser
    /// </summary>
    public sealed partial class WebBrowserPage : Page
    {
        private string originalUrl = string.Empty;
        private bool forceRefresh = false;

        public WebBrowserPage()
        {
            this.InitializeComponent();

            ContentView.UnviewableContentIdentified += ContentView_UnviewableContentIdentified;
            ContentView.NavigationCompleted += ContentView_NavigationCompleted;
            ContentView.FrameNavigationStarting += ContentView_NavigationStarting;
            ContentView.LongRunningScriptDetected += ContentView_LongRunningScriptDetected;

            ContentView.NewWindowRequested += ContentView_OnNewWindowRequested;
            // Assume webView is defined in XAML
            ContentView.ContainsFullScreenElementChanged += ContentView_ContainsFullScreenElementChanged;

            //Initialize
            WebBackButton.IsEnabled = ContentView.CanGoBack;
            WebNextButton.IsEnabled = ContentView.CanGoForward;
            
            WebAddressText.AutoMaximizeSuggestionArea = true;

            WebAddressText.QueryIcon.HorizontalAlignment = HorizontalAlignment.Left;
            WebAddressText.QueryIcon.Arrange(new Rect(5, 5, 10, 10));

            InProgress = false;

            this.NavigationCacheMode = NavigationCacheMode.Enabled;

            this.DataContext = LanguageManager.GetInstance();
            
        }

        private bool _inProgress;

        public bool InProgress
        {
            get { return _inProgress; }
            set
            {
                _inProgress = value;
                ImageProgressBar.IsIndeterminate = value;
                if (true == value)
                {
                    WebRefreshButton.Content = "\xE711";
                }
                else
                {
                    WebRefreshButton.Content = "\xE72C";
                }

            }
        }
        private void OnPageLoaded(object sender, RoutedEventArgs e)
        {
            WebAddressText.Focus(FocusState.Pointer);
        }


        private void ClearButton_Click(object sender, RoutedEventArgs e)
        {
            WebAddressText.Text = string.Empty;
        }

        private List<string> ds;
        private void WebAddressText_TextChanged(AutoSuggestBox sender, AutoSuggestBoxTextChangedEventArgs args)
        {
            //Mark End InProgress when try to Edit
            InProgress = false;

            // Only get results when it was a user typing,
            // otherwise assume the value got filled in by TextMemberPath
            // or the handler for SuggestionChosen.
            if (args.Reason == AutoSuggestionBoxTextChangeReason.UserInput)
            {
                ds = new List<string>();
                ds.Add(Constants.WODUrl);
                ds.Add(Constants.IoTHacksterUrl);
                ds.Add(Constants.IoTGitHubUrl);

                //Set the ItemsSource to be your filtered dataset
                sender.ItemsSource = ds;
            }

        }

        private void WebAddressText_SuggestionChosen(AutoSuggestBox sender, AutoSuggestBoxSuggestionChosenEventArgs args)
        {
            // Set sender.Text. You can use args.SelectedItem to build your text string.
            sender.Text = args.SelectedItem.ToString();
        }

        private void WebAddressText_QuerySubmitted(AutoSuggestBox sender, AutoSuggestBoxQuerySubmittedEventArgs args)
        {

            if (args.ChosenSuggestion != null)
            {
                if (args.QueryText.Length > 0)
                {
                    WebAddressText.Text = args.QueryText;
                    DoWebNavigate();
                }
                // User selected an item from the suggestion list, take an action on it here.
            }
            else
            {
                if (args.QueryText.Length > 0)
                {
                    //Generic Search
                    DoWebNavigate();
                }
                // Use args.QueryText to determine what to do.
            }

        }

        private void WebGoButton_Click(object sender, RoutedEventArgs e)
        {
            forceRefresh = true;
            //completed, and showing refresh
            if (!InProgress)
            {
                DoWebNavigate();
            }
            else
            {
                //Stop Navigation
                InProgress = false;
                ContentView.Stop();
            }

        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
           
            if (null == e || null == e.Parameter)
            {
                //Check existing url
                if (WebAddressText.Text.Length > 0)
                {
                    originalUrl = WebAddressText.Text;
                }
                else
                {
                    WebAddressText.Text = Constants.WODUrl;
                    DoWebNavigate();
                }

            }
            else
            {
                WebAddressText.Text = e.Parameter.ToString();
                DoWebNavigate();
            }
        }

        /// <summary>
        /// WebAddressText KeyUp 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void WebAddressText_KeyUp(object sender, KeyRoutedEventArgs e)
        {
            WebAddressText.IsSuggestionListOpen = true;
            //Handling Enter Action
            if (e.Key == Windows.System.VirtualKey.Enter)
            {
                DoWebNavigate();
            }
        }

        /// <summary>
        /// Allows or disallows to be allowed, for future  to restirct sites (TBD)
        /// TODO: Expand as needed
        /// </summary>
        /// <param name="url"></param>
        /// <returns></returns>
        private bool IsAllowedUri(Uri url)
        {
            return true;
        }


        /// <summary>
        /// Placeholder to skip the Non-Uri
        /// TODO: Expand as needed 
        /// </summary>
        /// <param name="uriText"></param>
        /// <returns></returns>
        private bool SkipNonUri(string uriText)
        {
            if (uriText.ToLower().Equals("about:blank") || uriText.ToLower().StartsWith("javascript:void"))
                return true;
            else
                return false;

        }


        /// <summary>
        /// WebView Navigation Stating Event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void ContentView_NavigationStarting(object sender, WebViewNavigationStartingEventArgs args)
        {
            //string url = "";
            //try {
            //    url = args.Uri.ToString();
            //}
            //finally
            //{
            //    WebAddressText.Text = url;
            //    InProgress = true;
            //}

            if (SkipNonUri( args.Uri.AbsoluteUri.ToString()) )
            {
                InProgress = false;

            } else
            {
                InProgress = true;
                WebAddressText.IsSuggestionListOpen = false;
                //WebAddressText.Text = args.Uri.ToString();
            }
            
            // Cancel navigation if URL is not allowed. (Implemetation of IsAllowedUri not shown.)
            if (!IsAllowedUri(args.Uri))
            {
                args.Cancel = true;
                //Check if this is taken care by Navigation Completed
                InProgress = false;
            }
            
        }

        /// <summary>
        /// Event to indicate that the content is fully loaded in the webview. If you need to invoke script, it is best to wait for this event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        void ContentView_DOMContentLoaded(WebView sender, WebViewDOMContentLoadedEventArgs args)
        {
            
        }

        /// <summary>
        /// Event to indicate webview has completed the navigation, either with success or failure.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        void ContentView_NavigationCompleted(WebView sender, WebViewNavigationCompletedEventArgs args)
        {            
            //InProgress = false;
            if (args.IsSuccess)
            {
                WebAddressText.Text = args.Uri.ToString();
                //Store it
                originalUrl = WebAddressText.Text;
                forceRefresh = false;

                //Check and Enable History
                WebBackButton.IsEnabled = ContentView.CanGoBack;
                WebNextButton.IsEnabled = ContentView.CanGoForward;

                InProgress = false;
                /*
                if (originalUrl.Length > 0 )
                {
                    WebAddressText.Text = originalUrl;
                    originalUrl = string.Empty;
                    DoWebNavigate();
                }*/
            }
            else
            {
                //Do Some action if Failure
                //Check if need Wifi Connection
                //TODO: Do action on Redirect behavior
                DisplayMessage(Common.GetLocalizedText("NetworkNotConnected"), Common.GetLocalizedText("NetworkNotConnected2"), true);
                InProgress = false;
            }
        }

        /// <summary>
        /// Apply some constraints to take action on long running scripts
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        void ContentView_LongRunningScriptDetected(WebView sender, WebViewLongRunningScriptDetectedEventArgs args)
        {
            //Halt script running more than 30 secs 
            if (args.ExecutionTime > new TimeSpan(0, 0, 30))
            {
                args.StopPageScriptExecution = true;
                InProgress = false;
            }

        }
        private void DoWebNavigate()
        {
            //Skip Navigation Scenario
            if ( WebAddressText.Text.Trim().Equals( originalUrl ) && !forceRefresh )
            {
                return;
            }

            //Validate Empty
            //1. User clears out and enter
            //2. User clears out and press space and enter
            if (WebAddressText.Text.Trim().Length == 0 )
            {
                //NoAction
                return;
            }

            //Change icon to Inprogress and then Complete
            InProgress = true;
            DismissMessage();

            try
            {
                var url = WebAddressText.Text.Trim();

                if (url.Length > 0)
                {
                    //Defaulting to http
                    if (!(url.StartsWith("http://") || url.StartsWith("https://")))
                    {
                        url = "http://" + url;
                    }

                    //TODO: What if user clicks search on search
                    if (!Uri.IsWellFormedUriString(url, UriKind.Absolute))
                    {
                        //Enable Search 
                        url = string.Format("http://{0}/search?q={1}", Constants.BingHomePageUrl, WebAddressText.Text);
                    }

                    //Navigate
                    ContentView.Navigate(new Uri(url));
                }
                else
                {   
                    ContentView.Navigate(new Uri(Constants.WODUrl));
                }
            }
            catch (Exception e)
            {
                //DisplayMessage("Error: " + e.Message);
                //Direct to bing.com
                WebAddressText.Text = Constants.BingHomePageUrl;
            }
        }


        private void BrowseTo(string url)
        {
            WebAddressText.Text = url;
            DoWebNavigate();
        }


        private void DisplayMessage(String message1, string message2 = "", bool toSettings = false)
        {
            Message1.Text = message1;
            Message2.Text = message2;
            SettingsDirect.Visibility = Visibility.Collapsed;

            if (toSettings)
            {
                SettingsDirect.Visibility = Visibility.Visible;
            }
            MessageStackPanel.Visibility = Visibility.Visible;
            ContentView.Visibility = Visibility.Collapsed;
        }

        /// <summary>
        /// Invokes Settings->Network ListView Item
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnSettings_Click(object sender, RoutedEventArgs e)
        {
            NavigationUtils.NavigateToScreen(typeof(Settings), "NetworkListViewItem");
        }

        private void OnMessageDismiss_Click(object sender, RoutedEventArgs e)
        {
            DismissMessage();
        }

        private void DismissMessage()
        {
            ContentView.Visibility = Visibility.Visible;
            MessageStackPanel.Visibility = Visibility.Collapsed;
        }

        private void BackButton_Clicked(object sender, RoutedEventArgs e)
        {
            if (ContentView.CanGoBack)
            {
                InProgress = true;
                ContentView.GoBack();
            }
        }

        private void NextButton_Clicked(object sender, RoutedEventArgs e)
        {
            if (ContentView.CanGoForward)
            {
                InProgress = true;
                ContentView.GoForward();
            }
        }

        /// <summary>
        /// Event is fired by webview when the content is not a webpage, such as a file download
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        async void ContentView_UnviewableContentIdentified(WebView sender, WebViewUnviewableContentIdentifiedEventArgs args)
        {
            // We turn around and hand the Uri to the system launcher to launch the default handler for it
            await Windows.System.Launcher.LaunchUriAsync(args.Uri);
        }

        private void ContentView_OnNewWindowRequested(WebView sender, WebViewNewWindowRequestedEventArgs e)
        {
            WebAddressText.Text = e.Uri.ToString();
            DoWebNavigate();
        }


        private void ContentView_ContainsFullScreenElementChanged(WebView sender, object args)
        {

            var applicationView = ApplicationView.GetForCurrentView();

            if (sender.ContainsFullScreenElement)
            {
                applicationView.TryEnterFullScreenMode();
            }
            else if (applicationView.IsFullScreenMode)
            {
                applicationView.ExitFullScreenMode();
            }
        }

        private void WebAddressText_FocusEngaged(Control sender, FocusEngagedEventArgs args)
        {
            WebAddressText.IsSuggestionListOpen = true;
        }

        private void WebAddressText_LostFocus(object sender, RoutedEventArgs e)
        {
            WebAddressText.Text = originalUrl;
        }
    }
}
