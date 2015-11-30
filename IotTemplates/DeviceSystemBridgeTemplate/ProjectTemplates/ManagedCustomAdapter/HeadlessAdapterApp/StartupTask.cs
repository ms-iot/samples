using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using BridgeRT;
using AdapterLib;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace HeadlessAdapterApp
{
    public sealed class StartupTask : IBackgroundTask
    {
        public void Run(IBackgroundTaskInstance taskInstance)
        {

            Adapter adapter = null;
            deferral = taskInstance.GetDeferral();

            try
            {
                adapter = new Adapter();
                dsbBridge = new DsbBridge(adapter);

                var initResult = dsbBridge.Initialize();
                if (initResult != 0)
                {
                    throw new Exception("DSB Bridge initialization failed!");
                }
            }
            catch (Exception ex)
            {
                if (dsbBridge != null)
                {
                    dsbBridge.Shutdown();
                }

                if (adapter != null)
                {
                    adapter.Shutdown();
                }

                throw;
            }
        }

        private DsbBridge dsbBridge;
        private BackgroundTaskDeferral deferral;
    }
}
