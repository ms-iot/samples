// Copyright (c) Microsoft. All rights reserved.

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
using Windows.ApplicationModel.Background;
using Windows.ApplicationModel.AppService;
using Windows.UI.Core;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace NotepadServiceClientApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {

        AppServiceConnection connection;
        public MainPage()
        {
            this.InitializeComponent();
            this.Loaded += MainPage_Loaded;
        }


        //For more details about basic AppServices, see the "AppServiceBlinky" sample
        //This app allows the user to request messages from the NotepadService 
        //The "GetNewMessagesButton" is only enabled when there are new messages with the service
        private async void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            GetNewMessagesButton.IsEnabled = false;
            connection = new AppServiceConnection();
            connection.AppServiceName = "NotepadService";
            connection.PackageFamilyName = "NotepadService-uwp_gpek5j0d8wyr0";
            AppServiceConnectionStatus status = await connection.OpenAsync();
            if (status != AppServiceConnectionStatus.Success)
            {
                Messages.Text = "Could not connnect to Notepad Service";
            }
            else
            {
                connection.RequestReceived += Connection_RequestReceived;
                GetNewMessagesButton.Click += GetNewMessagesButton_Click;
            }

        }


        //When the user requests new messages, send a request to the server and print them out
        private async void GetNewMessagesButton_Click(object sender, RoutedEventArgs e)
        {
            var message = new ValueSet();
            message.Add("operation", "getMessages");
            var result = await connection.SendMessageAsync(message);
            if (result.Status == AppServiceResponseStatus.Success)
            {
                var response = result.Message;
                string[] notes = (string[])response["notes"];
                foreach (string note in notes)
                {
                    Messages.Text += note + "\n";
                }
            }
            else
            {
                Messages.Text = "Could not connect to Notepad service";
            }
            GetNewMessagesButton.IsEnabled = false;
            
        }

        //When the service notifies us of new messages, enable the button to allow the user to request messages
        private async void Connection_RequestReceived(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
        {
            ValueSet message = args.Request.Message;
            if (message["operation"].Equals("newMessagesAdded"))
            {
                //Incoming messages come in on a background thread and so you need to go through the dispatcher to modify the button on the UI thread
                var dispatcher = GetNewMessagesButton.Dispatcher;
                await dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    GetNewMessagesButton.IsEnabled = true;
                });
                
            }
            
        }
    }
}
