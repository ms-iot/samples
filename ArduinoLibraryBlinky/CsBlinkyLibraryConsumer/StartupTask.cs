using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.System.Threading;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace CsBlinkyLibraryConsumer
{
    public sealed class StartupTask : IBackgroundTask
    {

        BackgroundTaskDeferral deferral;
        ArduinoLedLibrary.LedController ledController;
        ThreadPoolTimer timer;
        bool isLedOn;
        public void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();
            ledController = new ArduinoLedLibrary.LedController();
            isLedOn = false;

            timer = ThreadPoolTimer.CreatePeriodicTimer(this.Tick, TimeSpan.FromMilliseconds(500));            
        }
        

        public void Tick(ThreadPoolTimer source)
        {
            if (isLedOn)
            {
                ledController.LedOff();
            }
            else
            {
                ledController.LedOn();
            }
            isLedOn = !isLedOn;
        }
    }
}
