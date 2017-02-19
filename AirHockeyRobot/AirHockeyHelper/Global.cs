// Copyright (c) Microsoft. All rights reserved.

using System.Diagnostics;

namespace AirHockeyHelper
{
    public class Global
    {
        private static Stopwatch stopwatch;
        public static Stopwatch Stopwatch
        {
            get
            {
                if (stopwatch == null)
                {
                    stopwatch = new Stopwatch();
                }

                return stopwatch;
            }
        }
    }
}
