#define RPI1

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SpeechTranslator
{
    class ConstantParam
    {
        public static readonly string DatamarketAccessUri = "https://datamarket.accesscontrol.windows.net/v2/OAuth2-13";
        public const int RefreshTokenDuration = 9;

        //Token Property
        public static string clientid = "magicbunny888";
        public static string clientsecret = "FVw2RUvCvP//CaBomIQliFxqWIVM8B0angjeLOZvvDE=";
        public static string ApiUri = "http://api.microsofttranslator.com/v2/Http.svc/Translate?text=";

#if RPI1
        /// <summary>
        ///  Uncomment if you are deploying to device with the name "rpi1"
        /// </summary>
        /// 

        // Stream Socket parameters
        public static string SelfPort = "8082";
        public static string ClientPort = "8083";
        public static string ServerHostname = "speechtransrpi2";
#else
        /// <summary>
        ///  Uncomment if you are deploying to device with the name "rpi2"
        /// </summary>
        /// 

        // Stream Socket parameterss
        public static string SelfPort = "8083";
        public static string ClientPort = "8082";
        public static string ServerHostname = "speechtransrpi1";
#endif
    }
}
