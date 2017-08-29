using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IoTCoreDefaultApp
{
    public static class Common
    {
        public static string GetLocalizedText(string keyText )
        {
            return new Windows.ApplicationModel.Resources.ResourceLoader().GetString(keyText);
        }
        
    }
}
