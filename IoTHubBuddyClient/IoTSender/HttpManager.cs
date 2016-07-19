using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;

namespace IoTHubBuddyClient
{
    
    class HttpManager
    {
        private HttpInterfaceManager httpInterfaceManager;

        public IAsyncAction Initialize()
        {
            return Task.Run(async () => {
                this.httpInterfaceManager = new HttpInterfaceManager(8001);
                this.httpInterfaceManager.StartServer();
            }).AsAsyncAction();
        }
    }
}
