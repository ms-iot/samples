// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.System.Threading;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace CsLcdLibraryConsumer
{
    public sealed class StartupTask : IBackgroundTask
    {
        BackgroundTaskDeferral deferral;
        ArduinoLcdLibrary.LcdController lcdController;
        ThreadPoolTimer timer;
        int tickCount = 0;
        public void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();
      
            lcdController = new ArduinoLcdLibrary.LcdController();
            lcdController.PrintLine(0, "Lcd Initialized");

            timer = ThreadPoolTimer.CreatePeriodicTimer(this.Tick, TimeSpan.FromMilliseconds(1000));
        }

        public void Tick(ThreadPoolTimer timer)
        {
            tickCount++;
            lcdController.PrintLine(1, "Tick Count: " + tickCount.ToString());
        }
    }
}
