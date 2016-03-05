// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.ApplicationModel.AppService;
using Windows.Foundation.Collections;
using Windows.Devices.Gpio;


// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace BlinkyService
{
    public sealed class StartupTask : IBackgroundTask
    {
        //Gpio Pin and Controller are shared between all connections. 
        private static GpioPin pin;
        private static GpioController controller;

        //Each client gets it's own connection
        BackgroundTaskDeferral deferral;
        AppServiceConnection connection;

        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();
            //Print out the FamilyName
            //This string is needed to connect from a client
            System.Diagnostics.Debug.WriteLine(Windows.ApplicationModel.Package.Current.Id.FamilyName);


            //If this is the first time this background task is activated, init the gpio controller and pin
            if (pin == null && controller == null)
            {
                controller = await GpioController.GetDefaultAsync();
                pin = controller.OpenPin(5);
                pin.SetDriveMode(GpioPinDriveMode.Output);
            }

            //Check to determine whether this activation was caused by an incoming app service connection
            var appServiceTrigger = taskInstance.TriggerDetails as AppServiceTriggerDetails;
            if (appServiceTrigger != null)
            {
                //Verify that the app service connection is requesting the "BlinkyService" that this class provides
                if (appServiceTrigger.Name.Equals("BlinkyService"))
                {
                    //Store the connection and subscribe to the "RequestRecieved" event to be notified when clients send messages
                    connection = appServiceTrigger.AppServiceConnection;
                    connection.RequestReceived += Connection_RequestReceived;
                }
                else
                {
                    deferral.Complete();
                }

            }

        }

        private void Connection_RequestReceived(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
        {
            var messageDeferral = args.GetDeferral();

            //The message is provided as a ValueSet (IDictionary<String,Object)
            //The only message this server understands is with the name "requestedPinValue" and values of "Low" and "High"
            ValueSet message = args.Request.Message;
            string requestedPinValue = (string)message["requestedPinValue"];
            

            if (message.ContainsKey("requestedPinValue"))
            {
  
                if (requestedPinValue.Equals("High"))
                {
                    pin.Write(GpioPinValue.High);
                }
                else if (requestedPinValue.Equals("Low"))
                {
                    pin.Write(GpioPinValue.Low);
                }
                else
                {
                    System.Diagnostics.Debug.WriteLine("Reqested pin value is not understood: " + requestedPinValue);
                    System.Diagnostics.Debug.WriteLine("Valid values are 'High' and 'Low'");
                }
               
            }
            else
            {
                System.Diagnostics.Debug.WriteLine("Message not understood");
                System.Diagnostics.Debug.WriteLine("Valid command is: requestedPinValue");
            }

            messageDeferral.Complete();
        }
    }
}
