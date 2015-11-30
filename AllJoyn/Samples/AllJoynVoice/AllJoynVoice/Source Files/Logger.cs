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

using System;
using System.Diagnostics;
using Windows.Foundation.Diagnostics;
using Windows.Storage;

namespace AllJoynVoice
{
    class Logger
    {
        private static Logger theInstance;

        public static Logger Instance()
        {
            if (theInstance == null)
            {
                theInstance = new Logger();
            }

            return theInstance;
        }

        private LoggingSession session;
        private LoggingChannel channel;

        private static readonly string filename = "AllJoynVoice.etl";

        private Logger()
        {
            session = new LoggingSession("AllJoynVoiceSession");
            channel = new LoggingChannel("AllJoynVoiceChannel", null);

            session.AddLoggingChannel(channel);

            Windows.UI.Xaml.Application.Current.UnhandledException += Current_UnhandledException;
        }

        public static void Current_UnhandledException(object sender, Windows.UI.Xaml.UnhandledExceptionEventArgs e)
        {
            LogException("Unhandled exception.", e.Exception);
            Flush();
        }

        /// <summary>
        /// Saves the current log to a file and discards the previous log.
        /// </summary>
        public static void Flush()
        {
            Instance().session.SaveToFileAsync(ApplicationData.Current.LocalFolder, filename).AsTask().Wait();
        }

        /// <summary>
        /// Logs an exception and returns it.
        /// </summary>
        /// <param name="msg">A message associated with the exception.</param>
        /// <param name="exception">The exception.</param>
        /// <returns>The exception.</returns>
        public static Exception LogException(string msg, Exception exception)
        {
            Debug.WriteLine("Exception: " + msg);
            Debug.WriteLine("Message: " + exception.Message);

            LoggingFields fields = new LoggingFields();

            fields.AddString("Error message", msg);
            fields.AddString("Exception message", exception.Message);
            fields.AddInt32("HResult", exception.HResult);

            Instance().channel.LogEvent("Exception", fields, LoggingLevel.Error);

            return exception;
        }

        public static void LogInfo(string msg)
        {
            Debug.WriteLine("Info: " + msg);
            Instance().channel.LogMessage(msg, LoggingLevel.Information);
        }

        public static void LogError(string msg)
        {
            Debug.WriteLine("Error: " + msg);
            Instance().channel.LogMessage(msg, LoggingLevel.Error);
        }
    }
}
