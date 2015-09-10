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

using DigitalSignageUAP.Common;
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

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

namespace DigitalSignageUAP
{
    /// <summary>
    /// A simple browser that user can browse web pages
    /// </summary>
    public sealed partial class BrowserPage : Page
    {
        private NavigationHelper navigationHelper;
        private ObservableDictionary defaultViewModel = new ObservableDictionary();

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

        public BrowserPage()
        {
            this.InitializeComponent(); 
            this.NavigationCacheMode = Windows.UI.Xaml.Navigation.NavigationCacheMode.Enabled;
            this.navigationHelper = new NavigationHelper(this);
            this.navigationHelper.LoadState += navigationHelper_LoadState;
            this.navigationHelper.SaveState += navigationHelper_SaveState;

            /* Register the URL textbox with an on-screen keyboard control. Note that currently this
             * keyboard does not support inputting into browser controls 
             */
            SIP_AddressBar.RegisterEditControl(AddressBar);
            SIP_AddressBar.RegisterHost(this);
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

        /// <summary>
        /// self explained
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ForwardButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            if (browser.CanGoForward)
                browser.GoForward();
        }

        /// <summary>
        /// self explained
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void backButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            if(browser.CanGoBack)
                browser.GoBack();
        }

        /// <summary>
        /// when navigation complete, set the address bar to the final URI
        /// and remove focus from Address bar
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void browser_NavigationCompleted(WebView sender, WebViewNavigationCompletedEventArgs args)
        {
            AddressBar.Text = args.Uri.AbsoluteUri;
            this.Focus(FocusState.Keyboard);
        }

        /// <summary>
        /// self-explained
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        void refreshStopButtonNavigationComplete_Tapped(object sender, TappedRoutedEventArgs e)
        {
            browser.Refresh();
        }

        public void RenderPage()
        {
            try
            {
                browser.Navigate(new UriBuilder(AddressBar.Text).Uri);
            }
            catch (Exception)
            {
                /* we don't actually handle the exceptions here, due to the nature of web pages
                 * java script exceptions pop up very often. We merely want it continue without impacting the user experience
                 * because java script exceptions will launch JIT debugger */
            }
        }

        /// <summary>
        /// Start navigate
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void AddressBar_KeyDown(object sender, KeyRoutedEventArgs e)
        {
            
            if(e.Key == Windows.System.VirtualKey.Enter)
            {
                RenderPage();
            }
        }

        private void homepageButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            this.Frame.Navigate(typeof(MainPage));
        }

        private void OSK_Button_Click(object sender, RoutedEventArgs e)
        {
            /* Toggle the on screen keyboard visibility */
            if(SIP_AddressBar.Visibility == Visibility.Collapsed)
            {
                SIP_AddressBar.Visibility = Visibility.Visible;
            }
            else
            {
                SIP_AddressBar.Visibility = Visibility.Collapsed;
            }

            /* Need to set focus so the on screen keyboard's content buffer gets updated */
            AddressBar.Focus(FocusState.Programmatic);
        }
    }
}
