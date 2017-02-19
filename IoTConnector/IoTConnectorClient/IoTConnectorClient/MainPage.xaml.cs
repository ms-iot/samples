﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Networking;
using Windows.Networking.Connectivity;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;


// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace IoTConnectorClient
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {

        private static Uri ConnectUri = new Uri(@"http://localhost:"+Constants.PORT+"/");
        private HttpManager httpmanager;
        public MainPage()
        {
            this.InitializeComponent();
            httpmanager = new HttpManager();
        }

        private async void Page_Loaded(object sender, RoutedEventArgs e)
        {
            await this.httpmanager.Initialize();
            this.webView.Navigate(ConnectUri);

        }
    }
}
