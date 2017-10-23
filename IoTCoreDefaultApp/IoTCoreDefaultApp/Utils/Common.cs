using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IoTCoreDefaultApp
{
    public static class Common
    {
        /// <summary>
        /// Set to Yes if System need to reboot for Applying Lang Changes
        /// </summary>
        internal static bool LangApplyRebootRequired;

        public static string GetResourceText(string keyText)
        {
            var loader = Windows.ApplicationModel.Resources.ResourceLoader.GetForCurrentView();
            string strLoc = loader.GetString(keyText);

            //No Localized String, Check for en-US
            if ( strLoc.Trim().Length == 0 )
            {
                var newloader = Windows.ApplicationModel.Resources.ResourceLoader.GetForCurrentView(Constants.FallbackLang) ;
                strLoc = loader.GetString(keyText);
            }

            return strLoc ;
        }
        
    }
}
