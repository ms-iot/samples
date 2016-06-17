using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IoTViewer
{
    static class SimulatedAzureIoT
    {
        public static Message GetResults()
        {
            return new Message("47.6141", "-120.1441", DateTime.Now.ToString());
        }
    }
}
