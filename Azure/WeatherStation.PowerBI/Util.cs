using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using Microsoft.IdentityModel.Clients.ActiveDirectory;
using Newtonsoft.Json;
using Windows.UI.Xaml.Controls;

namespace WeatherStation
{
    class Util
    {
        static public String LocalIPAddress()
        {
            if (!System.Net.NetworkInformation.NetworkInterface.GetIsNetworkAvailable())
            {
                return "not connected";
            }

            var hostNames = Windows.Networking.Connectivity.NetworkInformation.GetHostNames();

            var firstIPv4HostName = (from hostName in hostNames
                                     where hostName.IPInformation != null && hostName.Type == Windows.Networking.HostNameType.Ipv4
                                     select hostName).FirstOrDefault();

            if (firstIPv4HostName == null)
            {
                return "unkown";
            }
            else
            {
                return firstIPv4HostName.DisplayName;
            }
        }
    }
}