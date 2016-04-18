using System;
using System.Threading;

namespace IoTBlocklyHelper
{
    public sealed class Basic
    {
        private readonly ManualResetEventSlim waitEvent = new ManualResetEventSlim(false);

        public Basic()
        {
        }

        public void Pause(double milliseconds)
        {
            waitEvent.Wait(TimeSpan.FromMilliseconds(milliseconds));
        }
    }
}
