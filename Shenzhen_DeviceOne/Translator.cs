using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


using System.Net;
using System.IO;

namespace Shenzhen_DeviceOne
{
    class Translator
    {
        private string originalString;
        private string translated;

        public Translator(string s, string from, string to)
        {
            this.originalString = s;
            AdmAuthentication admAuth = new AdmAuthentication(ConstantParam.clientid, ConstantParam.clientsecret);
            Task<AdmAccessToken> admToken = admAuth.GetAccessToken();
            this.translated = TranslateMethod("Bearer" + " " + admToken.Result.access_token, this.originalString, from, to);
        }

        public string GetTranslatedString()
        {
            return this.translated;
        }

        public static string TranslateMethod(string authToken, string originalS, string from, string to)
        {
            string text = originalS; //"你能听见我";
            //string from = "en";
            //string to = "zh-CHS";
            //string from = Constants.from;// "zh-CHS";
            //string to = Constants.to; // "en";

            string transuri = ConstantParam.ApiUri + System.Net.WebUtility.UrlEncode(text) + "&from=" + from + "&to=" + to;

            HttpWebRequest httpWebRequest = (HttpWebRequest)WebRequest.Create(transuri);
            //  httpWebRequest.ContentType = "application/x-www-form-urlencoded";
            httpWebRequest.Headers["Authorization"] = authToken;
            //httpWebRequest.Method = "GET";
            string trans;

            Task<WebResponse> response = httpWebRequest.GetResponseAsync();

            using (Stream stream = response.Result.GetResponseStream())
            {
                System.Runtime.Serialization.DataContractSerializer dcs = new System.Runtime.Serialization.DataContractSerializer(Type.GetType("System.String"));
                //DataContractJsonSerializer dcs = new DataContractJsonSerializer(Type.GetType("System.String"));
                trans = (string)dcs.ReadObject(stream);
                return trans;

            }
        }
    }
}
