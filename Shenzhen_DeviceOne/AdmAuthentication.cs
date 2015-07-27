using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


using System.Runtime.Serialization;
using System.Runtime.Serialization.Json;
using System.Net;
using System.IO;


namespace Shenzhen_DeviceOne
{
    class AdmAuthentication
    {      // public static readonly string DatamarketAccessUri = "https://datamarket.accesscontrol.windows.net/v2/OAuth2-13";
        private string clientId;
        private string clientSecret;
        private string request;
        private Task<AdmAccessToken> token;
        private static System.Threading.Timer accessTokenRenewer;

        //Access token expires every 10 minutes. Renew it every 9 minutes only.
        // private const int RefreshTokenDuration = 9;


        public AdmAuthentication(string clientid, string clientsecret)
        {
            this.clientId = clientid;
            this.clientSecret = clientsecret;

            this.request = string.Format("grant_type=client_credentials&client_id={0}&client_secret={1}&scope=http://api.microsofttranslator.com", WebUtility.UrlEncode(clientId), WebUtility.UrlEncode(clientSecret));


            // this.request = "grant_type=client_credentials&client_id=magicbunny888&client_secret=FVw2RUvCvP%2f%2fCaBomIQliFxqWIVM8B0angjeLOZvvDE%3d&scope=http://api.microsofttranslator.com";
            this.token = HttpPost(ConstantParam.DatamarketAccessUri, request);
            accessTokenRenewer = new System.Threading.Timer(new System.Threading.TimerCallback(OnTokenExpiredCallback), this, TimeSpan.FromMinutes(ConstantParam.RefreshTokenDuration), TimeSpan.FromMilliseconds(-1));

        }
        // accessTokenRenewer = new DispatcherTimer(new DispatcherTimer.)
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
            //HttpWebRequest webRequest =(HttpWebRequest)WebRequest.Create(datamarketAccessUri);
            HttpWebRequest webRequest = (HttpWebRequest)HttpWebRequest.Create(datamarketAccessUri);
            webRequest.ContentType = "application/x-www-form-urlencoded";
            webRequest.Method = "POST";
            string webResponse = "URI test";
            byte[] bytes = Encoding.UTF8.GetBytes(requestDetails);
            Task<Stream> requestTask = webRequest.GetRequestStreamAsync();
            using (Stream requestStream = requestTask.Result)
            {
                requestStream.Write(bytes, 0, bytes.Length);
            }

            Task<WebResponse> responseTask = webRequest.GetResponseAsync();
            /*
            using (StreamReader requestReader = new StreamReader(responseTask.Result.GetResponseStream()))
            {
                webResponse = requestReader.ReadToEnd();
            }*/


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

