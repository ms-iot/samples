// Copyright (c) Microsoft. All rights reserved.

#define RPI1


namespace SpeechTranslator
{
    class ConstantParam
    {
        public static readonly string DatamarketAccessUri = "https://datamarket.accesscontrol.windows.net/v2/OAuth2-13";

        //Access token expires every 10 minutes. Renew it every 9 minutes only.
        public const int RefreshTokenDuration = 9;

        //Token Property
        public static string clientid = "YourAzureAccount";
        public static string clientsecret = "YourAzureAccountClientSectet";
        public static string ApiUri = "http://api.microsofttranslator.com/v2/Http.svc/Translate?text=";

#if RPI1

        // Stream Socket parameters
        public static string SelfPort = "8082";
        public static string ClientPort = "8083";
        public static string ServerHostname = "speechtransrpi2";
#else

        // Stream Socket parameterss
        public static string SelfPort = "8083";
        public static string ClientPort = "8082";
        public static string ServerHostname = "speechtransrpi1";
#endif
    }
}
