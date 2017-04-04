// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Threading.Tasks;
using System.Net;
using System.IO;
using Windows.Storage.Streams;

// See http://docs.microsofttranslator.com/text-translate.html

namespace SpeechTranslator
{
    public struct TranslationResult
    {
        public string text;
        public IRandomAccessStream voice;
    };

    public class TranslatorManager
    {
        private AzureAuthentication azureAuth = null;
        public TranslatorManager()
        {
        }

        public async Task<TranslationResult> Translate( string text, string from, string to )
        {
            TranslationResult result;
            if (azureAuth == null)
            {
                azureAuth = new AzureAuthentication(ConstantParam.subscriptionKey);
            }
            string azureAccessToken = await azureAuth.GetAccessTokenAsync();
            result.text = await TranslateText( azureAccessToken, text, from, to );
            result.voice = await TranslateTextToSpeech(azureAccessToken, result.text, to);
            return result;
        }
        
        public async static Task<string> TranslateText(string authToken, string originalS, string from, string to)
        {
            const string TranslateURI = "http://api.microsofttranslator.com/v2/Http.svc/Translate?text=";
            
            string transuri = TranslateURI + System.Net.WebUtility.UrlEncode(originalS) + "&from=" + from + "&to=" + to;
            HttpWebRequest httpWebRequest = (HttpWebRequest)WebRequest.Create(transuri);
            httpWebRequest.Headers["Authorization"] = "Bearer "+authToken;

            string trans;
            using (WebResponse response = await httpWebRequest.GetResponseAsync())
            {
                using (Stream stream = response.GetResponseStream())
                {
                    System.Runtime.Serialization.DataContractSerializer dcs = new System.Runtime.Serialization.DataContractSerializer(Type.GetType("System.String"));
                    trans = (string)dcs.ReadObject(stream);                   
                }
            }

            return trans;
        }
        
        public async static Task<IRandomAccessStream> TranslateTextToSpeech(string authToken, string stringToSpeak, string speechLanguage)
        {
            const string SpeakURI = "http://api.microsofttranslator.com/v2/Http.svc/Speak?text=";

            string speakuri = SpeakURI + System.Net.WebUtility.UrlEncode(stringToSpeak) + "&language=" + speechLanguage;
            HttpWebRequest httpWebRequest = (HttpWebRequest)WebRequest.Create(speakuri);
            httpWebRequest.Headers["Authorization"] = "Bearer " + authToken;

            MemoryStream voiceMemoryStream = new MemoryStream();
            using (WebResponse response = await httpWebRequest.GetResponseAsync())
            {
                Stream voiceStream = response.GetResponseStream();
                await voiceStream.CopyToAsync(voiceMemoryStream);
            }
            
            var result = System.IO.WindowsRuntimeStreamExtensions.AsRandomAccessStream(voiceMemoryStream);
            result.Seek(0);
            return result;
        }        
    }
}
