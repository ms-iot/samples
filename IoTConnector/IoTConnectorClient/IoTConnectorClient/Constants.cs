using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IoTConnectorClient
{
    static class Constants
    {
        public const string LOGIN_ERROR = "An error occurred. Please enter the correct credentials below";
        public const string LOGIN_STD_MSG = "Please enter your credentials below";
        public const string SENDING_AZURE = "Sending to Azure...";
        public const string READY_AZURE = "Ready to send messages";
        public const string ERROR_AZURE = "An error occurred. Please log out and log in with the correct credentials.";
        public const string LIST_HEADER = "<p id='start-list'>#msgList#</p>";
        public const int PORT = 8001;
        public const string VERSION = "1.0";
    }
}
