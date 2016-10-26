// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.ApplicationModel.AppService;
using Windows.Foundation.Collections;
using Windows.System.Threading;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace NotepadClientBackgroundApplication
{
    public sealed class StartupTask : IBackgroundTask
    {
        AppServiceConnection connection;
        BackgroundTaskDeferral deferral;
        ThreadPoolTimer timer;
        int count = 1;

        //For a detailed introduction to AppServices on IoT, see the "AppServiceBlinky" sample
        //This app connects to the service and pushes messages every 30 seconds. 
        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();
            connection = new AppServiceConnection();
            connection.AppServiceName = "NotepadService";
            connection.PackageFamilyName = "NotepadService-uwp_gpek5j0d8wyr0";
            AppServiceConnectionStatus status = await connection.OpenAsync();
            if (status != AppServiceConnectionStatus.Success)
            {
                return;
            }

            var message = new ValueSet();

            //Send a message with an operation type of "postNote" and the desired "newNote"
            //Then, if successful, start a timer and send a new message periodically
            message.Add("operation", "postNote");
            message.Add("newNote", "Hello, this is my first note. I will add another one every 30 seconds");
            AppServiceResponse response = await connection.SendMessageAsync(message);
            if (response.Status == AppServiceResponseStatus.Success)
            {
                var result = response.Message["Result"].ToString();
                timer = ThreadPoolTimer.CreatePeriodicTimer(this.Tick, TimeSpan.FromSeconds(30));
                System.Diagnostics.Debug.WriteLine(result);
            }
            
            
        }


        public async void Tick(ThreadPoolTimer sender)
        {
            count++;
            var message = new ValueSet();
            message.Add("operation", "postNote");
            message.Add("newNote", "Here is new note #" + count.ToString());
            AppServiceResponse response = await connection.SendMessageAsync(message);
            if (response.Status == AppServiceResponseStatus.Success)
            {
                var result = response.Message["Result"].ToString();
                System.Diagnostics.Debug.WriteLine(result);
                
            }
        }
    }
}
