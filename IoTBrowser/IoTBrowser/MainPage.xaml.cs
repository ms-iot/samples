// Copyright (c) Microsoft. All rights reserved.

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

        private async void DoWebNavigate()
        {
            try
            {
                if (Web_Address.Text.Length > 0)
                {
                    webView.Navigate(new Uri(Web_Address.Text));
                }
                else
                {
                    MessageDialog dlg = new MessageDialog("you need to enter a web address.");
                    await dlg.ShowAsync();
                }
            }
            catch (Exception e)
            {
                MessageDialog dlg = new MessageDialog("Error: " + e.Message);
                await dlg.ShowAsync();

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
    }
}
