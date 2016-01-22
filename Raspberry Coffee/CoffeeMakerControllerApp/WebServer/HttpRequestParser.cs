using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Storage.Streams;

namespace CortonaCoffee.WebServer
{
    enum HttpParserState
    {
        METHOD,
        URL,
        URLPARM,
        URLVALUE,
        VERSION,
        HEADERKEY,
        HEADERVALUE,
        BODY,
        OK
    };

    public enum HttpResponseState
    {
        OK = 200,
        BAD_REQUEST = 400,
        NOT_FOUND = 404
    }

    public sealed class HTTPRequest
    {
        public string Method { get; set; }

        public string URL { get; set; }

        public string Version { get; set; }

        public IEnumerable<KeyValuePair<string, string>> URLParametes { get; set; }

        public bool Execute { get; set; }

        public IEnumerable<KeyValuePair<string, string>> Headers { get; set; }

        public int BodySize { get; set; }

        public string BodyContent { get; set; }

        public HTTPRequest()
        {

        }
    }

    public sealed class HttpRequestParser
    {
        private HttpParserState ParserState;

        private HTTPRequest HTTPRequest;

        private uint BufferSize = 8192;

        public HttpRequestParser(uint readBuffer)
        {
            this.HTTPRequest = new HTTPRequest();
            this.BufferSize = readBuffer;
        }

        public HttpRequestParser() :
            this(8192)
        {

        }

        public IAsyncOperation<HTTPRequest> GetHttpRequestForStream(IInputStream stream)
        {
            return ProcessStream(stream).AsAsyncOperation();
        }

        private async Task<HTTPRequest> ProcessStream(IInputStream stream)
        {
            Dictionary<string, string> _httpHeaders = null;
            Dictionary<string, string> _urlParameters = null;

            byte[] data = new byte[BufferSize];
            StringBuilder requestString = new StringBuilder();
            uint dataRead = BufferSize;

            IBuffer buffer = data.AsBuffer();

            string hValue = "";
            string hKey = "";

            try
            {
                // binary data buffer index
                uint bfndx = 0;

                // Incoming message may be larger than the buffer size.
                while (dataRead == BufferSize)
                {
                    await stream.ReadAsync(buffer, BufferSize, InputStreamOptions.Partial);
                    requestString.Append(Encoding.UTF8.GetString(data, 0, data.Length));
                    dataRead = buffer.Length;

                    // read buffer index
                    uint ndx = 0;
                    do
                    {
                        switch (ParserState)
                        {
                            case HttpParserState.METHOD:
                                if (data[ndx] != ' ')
                                    HTTPRequest.Method += (char)buffer.GetByte(ndx++);
                                else
                                {
                                    ndx++;
                                    ParserState = HttpParserState.URL;
                                }
                                break;
                            case HttpParserState.URL:
                                if (data[ndx] == '?')
                                {
                                    ndx++;
                                    hKey = "";
                                    HTTPRequest.Execute = true;
                                    _urlParameters = new Dictionary<string, string>();
                                    ParserState = HttpParserState.URLPARM;
                                }
                                else if (data[ndx] != ' ')
                                    HTTPRequest.URL += (char)buffer.GetByte(ndx++);
                                else
                                {
                                    ndx++;
                                    HTTPRequest.URL = WebUtility.UrlDecode(HTTPRequest.URL);
                                    ParserState = HttpParserState.VERSION;
                                }
                                break;
                            case HttpParserState.URLPARM:
                                if (data[ndx] == '=')
                                {
                                    ndx++;
                                    hValue = "";
                                    ParserState = HttpParserState.URLVALUE;
                                }
                                else if (data[ndx] == ' ')
                                {
                                    ndx++;

                                    HTTPRequest.URL = WebUtility.UrlDecode(HTTPRequest.URL);
                                    ParserState = HttpParserState.VERSION;
                                }
                                else
                                {
                                    hKey += (char)buffer.GetByte(ndx++);
                                }
                                break;
                            case HttpParserState.URLVALUE:
                                if (data[ndx] == '&')
                                {
                                    ndx++;
                                    hKey = WebUtility.UrlDecode(hKey);
                                    hValue = WebUtility.UrlDecode(hValue);
                                    _urlParameters[hKey] = _urlParameters.ContainsKey(hKey) ? _urlParameters[hKey] + ", " + hValue : hValue;
                                    hKey = "";
                                    ParserState = HttpParserState.URLPARM;
                                }
                                else if (data[ndx] == ' ')
                                {
                                    ndx++;
                                    hKey = WebUtility.UrlDecode(hKey);
                                    hValue = WebUtility.UrlDecode(hValue);
                                    _urlParameters[hKey] = _urlParameters.ContainsKey(hKey) ? _urlParameters[hKey] + ", " + hValue : hValue;
                                    HTTPRequest.URL = WebUtility.UrlDecode(HTTPRequest.URL);
                                    ParserState = HttpParserState.VERSION;
                                }
                                else
                                {
                                    hValue += (char)buffer.GetByte(ndx++);
                                }
                                break;
                            case HttpParserState.VERSION:
                                if (data[ndx] == '\r')
                                    ndx++;
                                else if (data[ndx] != '\n')
                                    HTTPRequest.Version += (char)buffer.GetByte(ndx++);
                                else
                                {
                                    ndx++;
                                    hKey = "";
                                    _httpHeaders = new Dictionary<string, string>();
                                    ParserState = HttpParserState.HEADERKEY;
                                }
                                break;
                            case HttpParserState.HEADERKEY:
                                if (data[ndx] == '\r')
                                    ndx++;
                                else if (data[ndx] == '\n')
                                {
                                    ndx++;
                                    if (_httpHeaders.ContainsKey("Content-Length"))
                                    {
                                        HTTPRequest.BodySize = Convert.ToInt32(_httpHeaders["Content-Length"]);
                                        ParserState = HttpParserState.BODY;
                                    }
                                    else
                                        ParserState = HttpParserState.OK;
                                }
                                else if (data[ndx] == ':')
                                    ndx++;
                                else if (data[ndx] != ' ')
                                    hKey += (char)buffer.GetByte(ndx++);
                                else
                                {
                                    ndx++;
                                    hValue = "";
                                    ParserState = HttpParserState.HEADERVALUE;
                                }
                                break;
                            case HttpParserState.HEADERVALUE:
                                if (data[ndx] == '\r')
                                    ndx++;
                                else if (data[ndx] != '\n')
                                    hValue += (char)buffer.GetByte(ndx++);
                                else
                                {
                                    ndx++;
                                    _httpHeaders.Add(hKey, hValue);
                                    hKey = "";
                                    ParserState = HttpParserState.HEADERKEY;
                                }
                                break;
                            case HttpParserState.BODY:
                                // Append to request BodyData
                                this.HTTPRequest.BodyContent = Encoding.UTF8.GetString(data, 0, this.HTTPRequest.BodySize);
                                bfndx += dataRead - ndx;
                                ndx = dataRead;
                                if (this.HTTPRequest.BodySize <= bfndx)
                                {
                                    ParserState = HttpParserState.OK;
                                }
                                break;
                                //default:
                                //   ndx++;
                                //   break;

                        }
                    }
                    while (ndx < dataRead);
                };

                // Print out the received message to the console.
                Debug.WriteLine("You received the following message : \n" + requestString);
                if (_httpHeaders != null)
                    HTTPRequest.Headers = _httpHeaders.AsEnumerable();
                if (_urlParameters != null)
                    HTTPRequest.URLParametes = _urlParameters.AsEnumerable();

                return HTTPRequest;
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.ToString());
            }

            return null;
        }

    }
}
