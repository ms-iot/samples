// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

using DeviceProviders;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;

namespace AllJoynVoice
{
    /// <summary>
    /// Listens to notifications on the AllJoyn bus and raises OnNotification with
    /// the English message text when a notification arrives.
    /// </summary>
    class AllJoynNotifier
    {
        public delegate void OnNotificationHandler(string message);

        public event OnNotificationHandler OnNotification;

        /// Reference: https://allseenalliance.org/developers/learn/base-services/notification/interface
        private readonly IReadOnlyList<string> NOTIFICATION_PATHS = new List<string>
        {
            @"/emergency",
            @"/warning",
            @"/info",
        };

        private readonly string NOTIFICATION_INTERFACE = @"org.alljoyn.Notification";

        /// Reference: https://git.allseenalliance.org/cgit/services/notification.git/tree/cpp/src/PayloadAdapter.cc#n337
        private readonly int NOTIFICATION_TEXT_ARGUMENT_INDEX = 9;

        private ICollection<ISignal> subcribedSignals = new Collection<ISignal>();

        public AllJoynNotifier()
        {
            Config.ServiceJoined += ServiceJoined;
        }

        private void ServiceJoined(IService service)
        {
            if (!service.ImplementsInterface(NOTIFICATION_INTERFACE)) return;

            foreach (string path in NOTIFICATION_PATHS)
            {
                IBusObject busObject;
                IInterface @interface;
                ISignal signal;

                try
                {
                    busObject = service.Objects.First(x => x.Path == path);
                }
                catch (InvalidOperationException)
                {
                    Logger.LogError("Couldn't find bus object: " + path);
                    return;
                }

                try
                {
                    @interface = busObject.Interfaces.First(x => x.Name == NOTIFICATION_INTERFACE);
                }
                catch (InvalidOperationException)
                {
                    Logger.LogError("Couldn't find Interface: " + NOTIFICATION_INTERFACE);
                    return;
                }

                try
                {
                    signal = @interface.Signals.Single(x => x.Name.ToLower() == "notify");
                    signal.SignalRaised += AllJoynNotifier_SignalRaised;
                    subcribedSignals.Add(signal);
                }
                catch (InvalidOperationException)
                {
                    Logger.LogError("The Notification interface is not implemented correctly.");
                    return;
                }

                Logger.LogInfo(string.Format("Subscribed to {0} notifications on {1}.", path, service.Name));
            }
        }

        private void AllJoynNotifier_SignalRaised(ISignal sender, IList<object> args)
        {
            Logger.LogInfo("Received Notification signal from " + sender.Name);

            if (OnNotification == null) return;

            string message = null;

            try
            {
                var textStructs = ((IList<object>)args[NOTIFICATION_TEXT_ARGUMENT_INDEX]).Cast<AllJoynMessageArgStructure>();

                /// textStructs has type a(ss), where the first element of the struct (i.e. x[0]) is the
                /// language as per RFC 5646, while the second element (x[1]) is the message text.
                var english = textStructs.First(x => ((string)x[0]).StartsWith("en"));

                message = (string)english[1];
            }
            catch (Exception ex)
            {
                Logger.LogException(this.GetType().Name, ex);
                return;
            }

            Logger.LogInfo("Notification text: " + message);

            OnNotification(message);
        }
    }
}