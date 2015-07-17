// Copyright (c) Microsoft. All rights reserved.

using System.Diagnostics;

namespace AirHockeyHelper2
{
    public class Global
    {
        private static Stopwatch _stopwatch;
        public static Stopwatch Stopwatch
        {
            get
            {
                if (_stopwatch == null)
                {
                    _stopwatch = new Stopwatch();
                }

                return _stopwatch;
            }
        }
    }
}
