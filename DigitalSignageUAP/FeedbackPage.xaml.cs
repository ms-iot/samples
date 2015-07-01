using DigitalSignageUAP.Common;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Microsoft.Diagnostics.Tracing;
using System.Diagnostics;

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

namespace DigitalSignageUAP
{
    /// <summary>
    /// The feedback page. Upload a telemetry event with user comments/feedback
    /// </summary>
    public sealed partial class FeedbackPage : Page
    {
        bool thumbUp = true; 
        private NavigationHelper navigationHelper;
        private ObservableDictionary defaultViewModel = new ObservableDictionary();
        private bool AliasTextBoxFirstTimeFocus = true, CommentTextBoxFirstTimeFocus = true;
        private bool OnScreenKeyboardVisible = false;


        /// <summary>
        /// This can be changed to a strongly typed view model.
        /// </summary>
        public ObservableDictionary DefaultViewModel
        {
            get { return this.defaultViewModel; }
        }

        /// <summary>
        /// NavigationHelper is used on each page to aid in navigation and 
        /// process lifetime management
        /// </summary>
        public NavigationHelper NavigationHelper
        {
            get { return this.navigationHelper; }
        }


        public FeedbackPage()
        {
            this.InitializeComponent();
            this.navigationHelper = new NavigationHelper(this);
            this.navigationHelper.LoadState += navigationHelper_LoadState;
            this.navigationHelper.SaveState += navigationHelper_SaveState;

            /* 
             * Register on screen keyboards (OSK) with Textboxes. Since the OSK does not currently support
             * multiple text boxes, the workaround is to instantiate a new keyboard for each text box 
             */
            SIP_AliasTextBox.RegisterEditControl(AliasTextbox);
            SIP_CommentTextBox.RegisterEditControl(CommentTextbox);

            SIP_AliasTextBox.OutputString = "Your alias";
            SIP_CommentTextBox.OutputString = "Please share a few lines of feedback on this demo with us. We will use the feedback to improve the next version.";
        }

        /// <summary>
        /// Populates the page with content passed during navigation. Any saved state is also
        /// provided when recreating a page from a prior session.
        /// </summary>
        /// <param name="sender">
        /// The source of the event; typically <see cref="NavigationHelper"/>
        /// </param>
        /// <param name="e">Event data that provides both the navigation parameter passed to
        /// <see cref="Frame.Navigate(Type, Object)"/> when this page was initially requested and
        /// a dictionary of state preserved by this page during an earlier
        /// session. The state will be null the first time a page is visited.</param>
        private void navigationHelper_LoadState(object sender, LoadStateEventArgs e)
        {
        }

        /// <summary>
        /// Preserves state associated with this page in case the application is suspended or the
        /// page is discarded from the navigation cache.  Values must conform to the serialization
        /// requirements of <see cref="SuspensionManager.SessionState"/>.
        /// </summary>
        /// <param name="sender">The source of the event; typically <see cref="NavigationHelper"/></param>
        /// <param name="e">Event data that provides an empty dictionary to be populated with
        /// serializable state.</param>
        private void navigationHelper_SaveState(object sender, SaveStateEventArgs e)
        {
        }

        #region NavigationHelper registration

        /// The methods provided in this section are simply used to allow
        /// NavigationHelper to respond to the page's navigation methods.
        /// 
        /// Page specific logic should be placed in event handlers for the  
        /// <see cref="GridCS.Common.NavigationHelper.LoadState"/>
        /// and <see cref="GridCS.Common.NavigationHelper.SaveState"/>.
        /// The navigation parameter is available in the LoadState method 
        /// in addition to page state preserved during an earlier session.

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            navigationHelper.OnNavigatedTo(e);
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            navigationHelper.OnNavigatedFrom(e);
        }

        #endregion
        
        private void ThumbupButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            ThumbupButton.Background = this.thumbButtonTappedBrush;
            ThumbdownButton.Background = this.thumbButtonUnSelectedBrush;
            thumbUp = true;
        }

        private void ThumbdownButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            ThumbdownButton.Background = this.thumbButtonTappedBrush;
            ThumbupButton.Background = this.thumbButtonUnSelectedBrush;
            thumbUp = false;
        }

        private void AliasTextbox_GotFocus(object sender, RoutedEventArgs e)
        {
            if (AliasTextBoxFirstTimeFocus)
            {
                AliasTextbox.Text = "";
                AliasTextbox.Foreground = this.TextboxEditBrush;
                AliasTextBoxFirstTimeFocus = false;
            }
            /* Workaround since we need two soft keyboards for the two text inputs on this page */
            if (OnScreenKeyboardVisible == true)
            {
                SIP_CommentTextBox.Visibility = Visibility.Collapsed;
                SIP_AliasTextBox.Visibility = Visibility.Visible;
            }
        }

        private void CommentTextbox_GotFocus(object sender, RoutedEventArgs e)
        {
            if (CommentTextBoxFirstTimeFocus)
            {
                CommentTextbox.Text = "";
                CommentTextbox.Foreground = this.TextboxEditBrush;
                CommentTextBoxFirstTimeFocus = false;
            }

            /* Workaround since we need two soft keyboards for the two text inputs on this page */
            if (OnScreenKeyboardVisible == true)
            {
                SIP_AliasTextBox.Visibility = Visibility.Collapsed;
                SIP_CommentTextBox.Visibility = Visibility.Visible;
            }

        }

        private void SendFeedbackButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            if (CommentTextBoxFirstTimeFocus)
            {
                MessageDialog dialog = new MessageDialog("Please say something...");
                dialog.Commands.Add(new UICommand("OK", DoNothingCommandInvokeHandler));
                dialog.ShowAsync();
                return;
            }

            // MessageDialog is not supported on Athens
            /*
            MessageDialog ResultDialog = new MessageDialog("Thanks for your feedback, it has been successfully uploaded!");
            ResultDialog.Commands.Add(new UICommand("OK", CommandInvokeHandler));
            ResultDialog.ShowAsync();
            */

            // Get back to Main page
            this.Frame.Navigate(typeof(MainPage));
        }

        void CommandInvokeHandler(IUICommand command)
        {
            this.Frame.Navigate(typeof(MainPage));
        }

        private void OSK_Button_Click(object sender, RoutedEventArgs e)
        {
            /* Toggle on screen keyboard visibility */
            OnScreenKeyboardVisible = !OnScreenKeyboardVisible;
            if(OnScreenKeyboardVisible == true)
            {
                SIP_AliasTextBox.Visibility = Visibility.Visible;
                SIP_CommentTextBox.Visibility = Visibility.Visible;
            }
            else
            {
                SIP_CommentTextBox.Visibility = Visibility.Collapsed;
                SIP_AliasTextBox.Visibility = Visibility.Collapsed;
            }

            /* Need to set focus so the on screen keyboard's content buffer gets updated */
            AliasTextbox.Focus(FocusState.Programmatic);
        }

        void DoNothingCommandInvokeHandler(IUICommand command)
        {
        }

        private void backButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(MainPage));
        }
    }
}
