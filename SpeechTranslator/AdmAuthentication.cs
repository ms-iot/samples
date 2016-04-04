// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Text;
using System.Threading.Tasks;


using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Net;
using System.IO;


namespace SpeechTranslator
{
    class AdmAuthentication
    {   

        private string clientId;
        private string clientSecret;
        private string request;
        private Task<AdmAccessToken> token;
        private static System.Threading.Timer accessTokenRenewer;

        public AdmAuthentication(string clientid, string clientsecret)
        {
            this.clientId = clientid;
            this.clientSecret = clientsecret;

            this.request = string.Format("grant_type=client_credentials&client_id={0}&client_secret={1}&scope=http://api.microsofttranslator.com", WebUtility.UrlEncode(clientId), WebUtility.UrlEncode(clientSecret));

            this.token = HttpPost(ConstantParam.DatamarketAccessUri, request);
            accessTokenRenewer = new System.Threading.Timer(new System.Threading.TimerCallback(OnTokenExpiredCallback), this, TimeSpan.FromMinutes(ConstantParam.RefreshTokenDuration), TimeSpan.FromMilliseconds(-1));

        }

        public Task<AdmAccessToken> GetAccessToken()
        {
            return this.token;
        }

        private void RenewAccessToken()
        {
            Task<AdmAccessToken> newAccessToken = HttpPost(ConstantParam.DatamarketAccessUri, this.request);
            
            //swap the new token with the old one
            this.token = newAccessToken;

        }
        private void OnTokenExpiredCallback(object stateInfo)
        {
            try
            {
                RenewAccessToken();
            }
            catch (Exception ex)
            {

            }
            finally
            {
                try
                {
                    accessTokenRenewer.Change(TimeSpan.FromMinutes(ConstantParam.RefreshTokenDuration), TimeSpan.FromMilliseconds(-1));
                }
                catch (Exception ex)
                {

                }
            }

        }

        private async Task<AdmAccessToken> HttpPost(string datamarketAccessUri, string requestDetails)
        {
            HttpWebRequest webRequest = (HttpWebRequest)HttpWebRequest.Create(datamarketAccessUri);
            webRequest.ContentType = "application/x-www-form-urlencoded";
            webRequest.Method = "POST";
            byte[] bytes = Encoding.UTF8.GetBytes(requestDetails);
            Task<Stream> requestTask = webRequest.GetRequestStreamAsync();
            using (Stream requestStream = requestTask.Result)
            {
                requestStream.Write(bytes, 0, bytes.Length);
            }

            Task<WebResponse> responseTask = webRequest.GetResponseAsync();

            DataContractJsonSerializer serializer = new DataContractJsonSerializer(typeof(AdmAccessToken));
            AdmAccessToken token = (AdmAccessToken)serializer.ReadObject(responseTask.Result.GetResponseStream());
            return token;

        }

    }

    [DataContract]
    public class AdmAccessToken
    {
        [DataMember]
        public string access_token { get; set; }
        [DataMember]
        public string token_type { get; set; }
        [DataMember]
        public string expires_in { get; set; }
        [DataMember]
        public string scope { get; set; }
    }
}

