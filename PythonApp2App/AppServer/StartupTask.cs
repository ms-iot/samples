using System;
using Windows.ApplicationModel.AppService;
using Windows.ApplicationModel.Background;
using Windows.Foundation.Collections;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace AppServer
{
    public sealed class StartupTask : IBackgroundTask
    {
        public StartupTask()
        {
            System.Diagnostics.Debug.WriteLine("StartUp");
        }

        BackgroundTaskDeferral deferral;

        public void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();
            System.Diagnostics.Debug.WriteLine(Windows.ApplicationModel.Package.Current.Id.FamilyName);

            var appService = taskInstance.TriggerDetails as AppServiceTriggerDetails;

            if (appService != null)
            {
                if (appService.Name.Equals("App2AppService"))
                {
                    appService.AppServiceConnection.RequestReceived += AppServiceConnection_RequestReceived;

                    var message = new ValueSet();
                    message["Greetings"] = appService.CallerPackageFamilyName;
                    appService.AppServiceConnection.SendMessageAsync(message).AsTask().Wait();
                }
                else
                {
                    deferral.Complete();
                }
            }
        }

        private async void AppServiceConnection_RequestReceived(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
        {
            var messageDefferal = args.GetDeferral();
            var message = args.Request.Message;

            var testValues = new ValueSet()
            {
                { "Null", null},

                { "BoolMin", false },
                { "BoolMax", true },

                { "CharMin", System.Char.MinValue},
                { "CharMax", System.Char.MaxValue},

                { "ByteMin", System.Byte.MinValue },
                { "ByteMax", System.Byte.MaxValue },

                { "Int16Min", System.Int16.MinValue},
                { "Int16Max", System.Int16.MaxValue},
                { "Int32Min", System.Int32.MinValue},
                { "Int32Max", System.Int32.MaxValue},
                { "Int64Min", System.Int64.MinValue},
                { "Int64Max", System.Int64.MaxValue},

                { "UInt16Min", System.UInt16.MinValue},
                { "UInt16Max", System.UInt16.MaxValue},
                { "UInt32Min", System.UInt32.MinValue},
                { "UInt32Max", System.UInt32.MaxValue},
                //{ "UInt64Min", System.UInt64.MinValue},
                //{ "UInt64Max", System.UInt64.MaxValue},

                { "SingleMin", System.Single.MinValue},
                { "SingleMax", System.Single.MaxValue},
                { "DoubleMin", System.Double.MinValue},
                { "DoubleMax", System.Double.MaxValue},

                { "StringNull", null},
                { "StringEmpty", String.Empty},
                { "String", "string"},

                { "Array<bool>", new System.Boolean[] {false, true } },

                { "Array<Char>", new System.Char[] {System.Char.MinValue, System.Char.MaxValue } },
                { "Array<byte>", new System.Byte[] {System.Byte.MinValue, System.Byte.MaxValue } },

                { "Array<Int16>", new System.Int16[] { System.Int16.MinValue, System.Int16.MaxValue } },
                { "Array<Int32>", new System.Int32[] { System.Int32.MinValue, System.Int32.MaxValue } },
                { "Array<Int64>", new System.Int64[] { System.Int64.MinValue, System.Int64.MaxValue } },

                { "Array<UInt16>", new System.UInt16[] { System.UInt16.MinValue, System.UInt16.MaxValue } },
                { "Array<UInt32>", new System.UInt32[] { System.UInt32.MinValue, System.UInt32.MaxValue } },
                //{ "Array<UInt64>", new System.UInt64[] { System.UInt64.MinValue, System.UInt64.MaxValue } },

                { "Array<Single>", new System.Single[] { System.Single.MinValue, System.Single.MaxValue } },
                { "Array<Double>", new System.Double[] { System.Double.MinValue, System.Double.MaxValue } },

                { "Array<String>", new System.String[] {String.Empty, "string", "foobar"} },
            };

            object action;
            if (message.TryGetValue("Action", out action))
            {
                var str = action as string;
                if (String.Compare(str, "TestValues") == 0)
                {
                    await args.Request.SendResponseAsync(testValues);
                }
                else if (String.Compare(str, "Echo") == 0)
                {
                    await args.Request.SendResponseAsync(message);
                }
                else if (String.Compare(str, "Disconnect") == 0)
                {
                    Windows.UI.Xaml.Application.Current.Exit();
                }
            }

            messageDefferal.Complete();
        }
    }
}
