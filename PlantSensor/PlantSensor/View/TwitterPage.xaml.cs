// Copyright (c) Microsoft. All rights reserved.
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using Tweetinvi;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace PlantSensor
{
    //twitterList class here
    public class twitterListClass
    {
        public string Tweet { get; set;}
    }
    /// <summary>
    /// This page allows you to tweet and see past history tweets
    /// </summary>
    public sealed partial class TwitterPage : Page
    {
        Timer twitterTimerHistory;
        Timer twitterTimerLive;
        public TwitterPage()
        {
            this.InitializeComponent();
            twitterTimerHistory = new Timer(timerTwitterHistory, this, 0, 1000);
            //var user = User.GetUserFromScreenName("Plant_App_Pi");
            //var tweets = Timeline.GetUserTimeline(user);
            //foreach(String twitterInUI in tweets)
            //{

            //}
            //get list of user timeline tweets
            //foreach loop: get all of the tweets
            //add them into history timeline tweets
        }

        protected override void OnNavigatedTo(NavigationEventArgs navArgs)
        {
            Debug.WriteLine("Twitter Page reached");
        }

        private async void timerTwitterHistory(object state)
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                //goes in constructor HistoryTweetList.ItemsSource = App.Twitterresult;
            });
        }

        private async void twitterTimerLiveMethod(object state)
        {
            Auth.SetUserCredentials("5no3TPnFYR9oArbzprNa8QpbY", "9kPVZaI2aLOxDD5S1m6EuDsRnDyeovqxuZA7oe43LlmidVin6U", "764234424552558592-zqHPE9eIW5PIOw36J5FELnMGgSA9mKp", "0f9gFGGQ5ahP0AuR8gT1ARtcRjVzyGmYcICXErWTUIila");
            String Tweet = determineTweet();
            //var firstTweet = Tweet.PublishTweet(Tweet);
            String stringIntoFile = Tweet + "," + DateTime.Now + Environment.NewLine;
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                //instead of adding string to list, add object from class
                HistoryTweetList.Items.Add(stringIntoFile);
            });

            Windows.Storage.FileIO.AppendTextAsync(App.TwitterFile, stringIntoFile);
        }

        private string determineTweet()
        {
            return "HelloWorld";
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

        private void TwitterTwitterAppBar_Click(object sender, RoutedEventArgs e)
        {

        }

        private void toggleSwitchTwitter_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if(toggleSwitch.IsOn == true)
                {
                    twitterTimerLive = new Timer(twitterTimerLiveMethod, this, 0, 3000);
                }
                else
                {
                    twitterTimerLive.Change(Timeout.Infinite, Timeout.Infinite);
                }
            }
        }
    }
}
