// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.ApplicationModel.AppService;
using Windows.Foundation.Collections;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace NotepadService
{
    //See the "AppServiceBlinky" sample for a basic introduction to AppServices for IoT devices
    //Each connection causes a new instance to be spun up in the same process
    public sealed class StartupTask : IBackgroundTask
    {
        //Notepad is static to allow a single instance to be shared among all connections 
        private static Notepad notePad;
        BackgroundTaskDeferral deferral;
        AppServiceConnection appServiceConnection;

        public void Run(IBackgroundTaskInstance taskInstance)
        { 

            deferral = taskInstance.GetDeferral();
            System.Diagnostics.Debug.WriteLine(Windows.ApplicationModel.Package.Current.Id.FamilyName);

            if (notePad == null)
            {
                notePad = new Notepad();
            }


            var appServiceTrigger = taskInstance.TriggerDetails as AppServiceTriggerDetails;

            if (appServiceTrigger != null)
            {
                if (appServiceTrigger.Name.Equals("NotepadService"))
                {
                    appServiceConnection = appServiceTrigger.AppServiceConnection;
                    appServiceConnection.RequestReceived += AppServiceConnection_RequestReceived;
                    notePad.NoteAdded += NotePad_NoteAdded;
                }
                else
                {
                    deferral.Complete();
                }
            }
        }

        //Every time the underlying notepad gets a new note, send a message back to the client indicating that a new message is available
        private async void NotePad_NoteAdded(object sender, EventArgs e)
        {
            ValueSet message = new ValueSet();
            message.Add("operation", "newMessagesAdded");
            await appServiceConnection.SendMessageAsync(message);
        }

        //This AppService supports recieving two types of request: "postNote" and "getMessages"
        //When a note is sent from the clients the service will pass it to the shared notpad and send a confirmation response to the client
        //When the client requests messages the repsonse will contain a string[] of all the messages on the notepad
        private async void AppServiceConnection_RequestReceived(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
        {
            var messageDefferal = args.GetDeferral();
            var message = args.Request.Message;
            string operation = (string)message["operation"];
            if (operation.Equals("postNote"))
            {
                await notePad.AddNote((message["newNote"].ToString()));
                ValueSet returnMessage = new ValueSet();
                returnMessage.Add("Result", "Note Added");
                var responseStatus = await args.Request.SendResponseAsync(returnMessage);
                System.Diagnostics.Debug.WriteLine("New note posted: " + message["newNote"].ToString());
            }
            else if (operation.Equals("getMessages"))
            {
                var notes = notePad.GetNotes();
                ValueSet returnMessage = new ValueSet();
                returnMessage.Add("notes", notes.ToArray());
                var responseStatus = await args.Request.SendResponseAsync(returnMessage);
            }

            messageDefferal.Complete();
        }
    }


}
