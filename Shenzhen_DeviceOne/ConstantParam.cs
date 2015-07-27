using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Shenzhen_DeviceOne
{
    class ConstantParam
    {
        public static readonly string DatamarketAccessUri = "https://datamarket.accesscontrol.windows.net/v2/OAuth2-13";
        public const int RefreshTokenDuration = 9;


        //Token Property
        public static string clientid = "magicbunny888";
        public static string clientsecret = "FVw2RUvCvP//CaBomIQliFxqWIVM8B0angjeLOZvvDE=";

       // public static string clientid = "iotmaker";
       // public static string clientsecret = "nA/M181Yi4PF08c0n8IjEz1b9SWSkzj0+wukNQBwcf4=";

        ////// Speak-Into Language
        //public static Windows.Globalization.Language lang = new Windows.Globalization.Language("zh-CN");

        ////Translator Language 
        //public static string from = "zh-CHS";
        //public static string to = "en";


        //public static Windows.Globalization.Language lang = new Windows.Globalization.Language("zh-CN");
        //public static string from = "zh-CN";
        //public static string to = "en";

        //public static string from1 = "zh-CHS";
        //public static string to1 = "en";

        public static string ApiUri = "http://api.microsofttranslator.com/v2/Http.svc/Translate?text=";




        /// <summary>
        ///  Uncomment if you are deploying to Device 1
        /// </summary>
        /// 

        // Stream Socket parameters
        public static string SelfPort = "8083";
        public static string ClientPort = "8082";
        // public static string ServerHostname = "shenzhen_mbm2";
        //public static string ServerHostname = "Shenzhenrpi1";
        public static string ServerHostname = "mbmdemo4";


        /// <summary>
        ///  Uncomment if you are deploying to Device 2
        /// </summary>
        /// 
        //public static string SelfPort = "8082";
        //public static string ClientPort = "8083";
        ////public static string ServerHostname = "shenzhen_mbm1";
        ////public static string ServerHostname = "Shenzhen_Rpi2";
        //public static string ServerHostname = "mbmdemo3";
    }
}
