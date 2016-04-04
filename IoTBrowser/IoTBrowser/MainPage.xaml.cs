// Copyright (c) Microsoft. All rights reserved.

using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace IoTBrowser
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }

        private void Go_Web_Click(object sender, RoutedEventArgs e)
        {
            DoWebNavigate();
        }


        private void Web_Address_KeyUp(object sender, KeyRoutedEventArgs e)
        {
            if (e.Key == Windows.System.VirtualKey.Enter)
            {
                DoWebNavigate();
            }
        }

        private void DoWebNavigate()
        {
            DismissMessage();

            try
            {
                if (Web_Address.Text.Length > 0)
                {
                    webView.Navigate(new Uri(Web_Address.Text));
                }
                else
                {
                    DisplayMessage("You need to enter a web address.");
                }
            }
            catch (Exception e)
            {
                DisplayMessage("Error: " + e.Message);
            }
        }

        private void Go_WOD_Click(object sender, RoutedEventArgs e)
        {
            Web_Address.Text = "https://www.windowsondevices.com";
            DoWebNavigate();
        }

        private void Go_Hackster_Click(object sender, RoutedEventArgs e)
        {
            Web_Address.Text = "https://www.hackster.io/windowsiot";
            DoWebNavigate();
        }

        private void Go_GitHub_Click(object sender, RoutedEventArgs e)
        {
            Web_Address.Text = "https://github.com/ms-iot";
            DoWebNavigate();
        }

        private void DisplayMessage(String message)
        {
            Message.Text = message;
            MessageStackPanel.Visibility = Visibility.Visible;
            webView.Visibility = Visibility.Collapsed;

        }

        private void OnMessageDismiss_Click(object sender, RoutedEventArgs e)
        {
            DismissMessage();
        }

        private void DismissMessage()
        {
            webView.Visibility = Visibility.Visible;
            MessageStackPanel.Visibility = Visibility.Collapsed;
        }
    }
}
