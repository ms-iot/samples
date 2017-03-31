// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Net;
using System.Net.Http;
using System.Threading.Tasks;

namespace SpeechTranslator
{
    class AzureAuthentication
    {
        /// URL of the token service
        private static readonly Uri ServiceUrl = new Uri("https://api.cognitive.microsoft.com/sts/v1.0/issueToken");
        
        /// Name of header used to pass the subscription key to the token service
        private const string OcpApimSubscriptionKeyHeader = "Ocp-Apim-Subscription-Key";

        /// Gets the subscription key.
        private string subscriptionKey;

        /// Cache the value of the last valid token obtained from the token service.
        private string storedTokenValue;

        /// When the last valid token was obtained.
        private DateTime storedTokenTime = DateTime.MinValue;

        /// After obtaining a valid token, this class will cache it for this duration.
        /// Use a duration of 9 minutes, which is less than the actual token lifetime of 10 minutes.
        private static readonly TimeSpan TokenCacheDuration = new TimeSpan(0, 9, 0);

        public HttpStatusCode RequestStatusCode { get; private set; }

        public AzureAuthentication(string subscriptionKey)
        {
            this.subscriptionKey = subscriptionKey;
            this.RequestStatusCode = HttpStatusCode.InternalServerError;
        }

        public async Task<string> GetAccessTokenAsync()
        {
            if (subscriptionKey == string.Empty)
            {
                return string.Empty;
            }

            // Re-use the cached token if there is one.
            if ((DateTime.Now - storedTokenTime) < TokenCacheDuration)
            {
                return storedTokenValue;
            }

            using (var client = new HttpClient())
            {
                using (var request = new HttpRequestMessage())
                {
                    request.Method = HttpMethod.Post;
                    request.RequestUri = ServiceUrl;
                    request.Content = new StringContent(string.Empty);
                    request.Headers.TryAddWithoutValidation(OcpApimSubscriptionKeyHeader, this.subscriptionKey);
                    client.Timeout = TimeSpan.FromSeconds(5);
                    var response = await client.SendAsync(request);
                    this.RequestStatusCode = response.StatusCode;
                    response.EnsureSuccessStatusCode();
                    var token = await response.Content.ReadAsStringAsync();
                    storedTokenTime = DateTime.Now;
                    storedTokenValue = token;
                    return storedTokenValue;
                }
            }
        }
    }   
}

