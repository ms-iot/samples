using System;
using System.Threading;

namespace IoTBlocklyHelper
{
    public sealed class Basic
    {
        private readonly ManualResetEventSlim waitEvent = new ManualResetEventSlim(false);
        private readonly DateTime startTime = DateTime.Now;

        public Basic()
        {
        }

        public void Pause(double milliseconds)
        {
            waitEvent.Wait(TimeSpan.FromMilliseconds(milliseconds));
        }

        public double RunningTime()
        {
            return (DateTime.Now - startTime).TotalMilliseconds;
        }
    }
}
