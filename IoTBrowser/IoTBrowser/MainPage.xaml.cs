/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

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
*/

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

        private async void Go_Web_Click(object sender, RoutedEventArgs e)
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

        private void Go_IoTPortal_Click(object sender, RoutedEventArgs e)
        {
            Web_Address.Text = "https://devx.windows-int.com/en-us/iot";
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
