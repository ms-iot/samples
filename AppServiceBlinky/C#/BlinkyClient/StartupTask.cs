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

namespace BlinkyClient
{
    public sealed class StartupTask : IBackgroundTask
    {

        AppServiceConnection connection;
        BackgroundTaskDeferral deferral;
        ThreadPoolTimer timer;
        string requestedPinValue;


        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();


            //Connect to the "BlinkyService" implemented in the "BlinkyService" solution
            connection = new AppServiceConnection();
            connection.AppServiceName = "BlinkyService";
            connection.PackageFamilyName = "BlinkyService-uwp_gpek5j0d8wyr0";
            AppServiceConnectionStatus status = await connection.OpenAsync();

            if (status != AppServiceConnectionStatus.Success)
            {
                deferral.Complete();
                return;
            }

            //Send a message with the name "requestedPinValue" and the value "High"
            //These work like loosely typed input parameters to a method
            requestedPinValue = "High";
            var message = new ValueSet();
            message["requestedPinValue"] = requestedPinValue;
            AppServiceResponse response = await connection.SendMessageAsync(message);

            //If the message was successful, start a timer to send alternating requestedPinValues to blink the LED
            if (response.Status == AppServiceResponseStatus.Success)
            {
                timer = ThreadPoolTimer.CreatePeriodicTimer(this.Tick, TimeSpan.FromMilliseconds(500));
            }

        }

        private async void Tick(ThreadPoolTimer timer)
        {
            if (requestedPinValue.Equals("High"))
            {
                requestedPinValue = "Low";
            }
            else
            {
                requestedPinValue = "High";
            }
            var message = new ValueSet();
            message["requestedPinValue"] = requestedPinValue;
            await connection.SendMessageAsync(message);
        }
    }
}
