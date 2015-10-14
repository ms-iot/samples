// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Threading.Tasks;
using System.Net;
using System.IO;

namespace SpeechTranslator
{
    public class TranslatorManager
    {
        private string originalString;
        private string translated;

        public TranslatorManager()
        {
        }

        public async Task<string> Translate( string text, string from, string to )
        {
            this.originalString = text;
            AdmAuthentication admAuth = new AdmAuthentication( ConstantParam.clientid, ConstantParam.clientsecret );
            AdmAccessToken admToken = await admAuth.GetAccessToken();
            this.translated = await TranslateMethod( "Bearer" + " " + admToken.access_token, this.originalString, from, to );
            return this.translated;
        }

        public string GetTranslatedString()
        {
            return this.translated;
        }

        public async static Task<string> TranslateMethod(string authToken, string originalS, string from, string to)
        {
            string text = originalS; 
            string transuri = ConstantParam.ApiUri + System.Net.WebUtility.UrlEncode(text) + "&from=" + from + "&to=" + to;

            HttpWebRequest httpWebRequest = (HttpWebRequest)WebRequest.Create(transuri);

            httpWebRequest.Headers["Authorization"] = authToken;
            string trans;

            WebResponse response = await httpWebRequest.GetResponseAsync();

            using (Stream stream = response.GetResponseStream())
            {
                System.Runtime.Serialization.DataContractSerializer dcs = new System.Runtime.Serialization.DataContractSerializer(Type.GetType("System.String"));

                trans = (string)dcs.ReadObject(stream);
                return trans;
            }
        }
    }
}
