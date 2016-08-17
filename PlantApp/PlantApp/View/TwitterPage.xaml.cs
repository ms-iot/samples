using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Tweetinvi;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace PlantApp
{
    /// <summary>
    /// This page allows you to tweet and see past history tweets
    /// </summary>
    public sealed partial class TwitterPage : Page
    {
        public TwitterPage()
        {
            this.InitializeComponent();
            //need to check for null
            //var ListOfTweets = Timeline.GetHomeTimeline();
            //var ListOfTweetsIntoListView = new ListViewItem();
            this.HistoryTweetsListView = new ListView();
           // foreach(Tweetinvi.Models.ITweet tweet in ListOfTweets)
            //{
                //this.HistoryTweetsListView.Columns.
                //ListOfTweetsIntoListView.SubItems
            //}
            //Auth.SetUserCredentials("5no3TPnFYR9oArbzprNa8QpbY", "9kPVZaI2aLOxDD5S1m6EuDsRnDyeovqxuZA7oe43LlmidVin6U", "764234424552558592-zqHPE9eIW5PIOw36J5FELnMGgSA9mKp", "0f9gFGGQ5ahP0AuR8gT1ARtcRjVzyGmYcICXErWTUIila");
        }

        private void TwitterCalendarAppBar_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(HistoryPage));
        }

        private void TwitterSettingsAppBar_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(SettingsPage));
        }

        private void TwitterHomeAppBar_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(MainPage));
        }

        private void Tweet_Click(object sender, RoutedEventArgs e)
        {
            Auth.SetUserCredentials("5no3TPnFYR9oArbzprNa8QpbY", "9kPVZaI2aLOxDD5S1m6EuDsRnDyeovqxuZA7oe43LlmidVin6U", "764234424552558592-zqHPE9eIW5PIOw36J5FELnMGgSA9mKp", "0f9gFGGQ5ahP0AuR8gT1ARtcRjVzyGmYcICXErWTUIila");
            var firstTweet = Tweet.PublishTweet("HelloWorld");
        }

        private void TwitterTwitterAppBar_Click(object sender, RoutedEventArgs e)
        {

        }
    }
}
