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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace IoTConnector
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class AboutPage : Page
    {
        private Page prevPage;
        public AboutPage()
        {
            this.InitializeComponent();
        }
        /// <summary>
        /// Back Button trigger event
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Back_Click(object sender, RoutedEventArgs e)
        {
            this.Frame.GoBack();
        }
        private void DisplayBrowser(string uri)
        {
            this.Info.Visibility = Visibility.Collapsed;
            this.policyGrid.Visibility = Visibility.Visible;
            this.policyView.Navigate(new Uri(uri));
        }
        /// <summary>
        /// navigate to internal web viewer, display policy page
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void Policy_Clicked(Windows.UI.Xaml.Documents.Hyperlink sender, Windows.UI.Xaml.Documents.HyperlinkClickEventArgs args)
        {
            DisplayBrowser(("https://go.microsoft.com/fwlink/?LinkId=521839"));
        }
        /// <summary>
        /// exit out of browser, return to about page
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Close_Browser(object sender, RoutedEventArgs e)
        {

            this.policyGrid.Visibility = Visibility.Collapsed;
            this.Info.Visibility = Visibility.Visible;
        }
        /// <summary>
        /// navigate to browser, display legal page
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void Legal_Click(Windows.UI.Xaml.Documents.Hyperlink sender, Windows.UI.Xaml.Documents.HyperlinkClickEventArgs args)
        {
            DisplayBrowser(("http://go.microsoft.com/fwlink/p/?LinkID=530144"));
        }

        private void Sample_Click(Windows.UI.Xaml.Documents.Hyperlink sender, Windows.UI.Xaml.Documents.HyperlinkClickEventArgs args)
        {
            DisplayBrowser("https://developer.microsoft.com/en-us/windows/iot/docs/cloudintro");
        }

        private void Dashboard_Click(Windows.UI.Xaml.Documents.Hyperlink sender, Windows.UI.Xaml.Documents.HyperlinkClickEventArgs args)
        {
            DisplayBrowser("http://go.microsoft.com/fwlink/?LinkID=708576");
        }
    }
}
