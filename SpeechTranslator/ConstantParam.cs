namespace SpeechTranslator
{
    class ConstantParam
    {
        public static readonly string DatamarketAccessUri = "https://datamarket.accesscontrol.windows.net/v2/OAuth2-13";
        public const int RefreshTokenDuration = 9;


        //Token Property
        public static string clientid = null;
        public static string clientsecret = null;
        public static string ApiUri = "http://api.microsofttranslator.com/v2/Http.svc/Translate?text=";

        // Stream Socket parameters
        public static string SelfPort = null;
        public static string ClientPort = null;
        public static string ServerHostname = null;

    }
}
