using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel.AppService;
using Windows.ApplicationModel.Core;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Networking.Sockets;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.System.Threading;

namespace IoTSender
{
    /// <summary>
    /// HttpServer class that services the content for the Security System web interface
    /// </summary>
    internal class HttpInterfaceManager : IDisposable
    {
        private const uint BufferSize = 8192;
        private int port = 8000;
        private readonly StreamSocketListener listener;
        private WebHelper helper;
        private LocationProvider lp;
        private Dictionary<string, string> htmlPages;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="serverPort">Port to start server on</param>
        internal HttpInterfaceManager(int serverPort)
        {

            helper = new WebHelper();
            listener = new StreamSocketListener();
            lp = new LocationProvider();
            htmlPages = new Dictionary<string, string>();
            port = serverPort;
            listener.ConnectionReceived += (s, e) =>
            {
                try
                {
                    // Process incoming request
                    processRequestAsync(e.Socket);
                }
                catch (Exception ex)
                {
                    Debug.WriteLine("Exception in StreamSocketListener.ConnectionReceived(): " + ex.Message);
                }
            };
        }

        public async void StartServer()
        {
            await helper.InitializeAsync();

#pragma warning disable CS4014
            listener.BindServiceNameAsync(port.ToString());
#pragma warning restore CS4014
        }

        public void Dispose()
        {
            listener.Dispose();
        }

        /// <summary>
        /// Process the incoming request
        /// </summary>
        /// <param name="socket"></param>
        private async void processRequestAsync(StreamSocket socket)
        {
            try
            {
                StringBuilder request = new StringBuilder();
                using (IInputStream input = socket.InputStream)
                {
                    // Convert the request bytes to a string that we understand
                    byte[] data = new byte[BufferSize];
                    IBuffer buffer = data.AsBuffer();
                    uint dataRead = BufferSize;
                    while (dataRead == BufferSize)
                    {
                        await input.ReadAsync(buffer, BufferSize, InputStreamOptions.Partial);
                        request.Append(Encoding.UTF8.GetString(data, 0, data.Length));
                        dataRead = buffer.Length;
                    }
                }

                using (IOutputStream output = socket.OutputStream)
                {
                    // Parse the request
                    string[] requestParts = request.ToString().Split('\n');
                    string requestMethod = requestParts[0];
                    string[] requestMethodParts = requestMethod.Split(' ');

                    // Process the request and write a response to send back to the browser
                    if (requestMethodParts[0].ToUpper() == "GET")
                    {
                        Debug.WriteLine("request for: {0}", requestMethodParts[1]);
                        await writeResponseAsync(requestMethodParts[1], output, socket.Information);
                    }
                    else if (requestMethodParts[0].ToUpper() == "POST")
                    {
                        string requestUri = string.Format("{0}?{1}", requestMethodParts[1], requestParts[requestParts.Length - 1]);
                        Debug.WriteLine("POST request for: {0} ", requestUri);
                        await writeResponseAsync(requestUri, output, socket.Information);
                    }
                    else
                    {
                        throw new InvalidDataException("HTTP method not supported: "
                                                       + requestMethodParts[0]);
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Exception in processRequestAsync(): " + ex.Message);
            }
        }

        private async Task writeResponseAsync(string request, IOutputStream os, StreamSocketInformation socketInfo)
        {
            try
            {
                request = request.TrimEnd('\0'); //remove possible null from POST request

                string[] requestParts = request.Split('/');

                // Request for the root page, so redirect to home page
                if (request.Equals("/") || request.Contains(NavConstants.DEFAULT_PAGE))
                {
                    string html = "";
                    if (htmlPages.ContainsKey(NavConstants.DEFAULT_PAGE))
                    {
                        html = htmlPages[NavConstants.DEFAULT_PAGE];
                    } else
                    {
                        // Generate the default config page
                        html = await GeneratePageHtml(NavConstants.DEFAULT_PAGE);
                        htmlPages.Add(NavConstants.DEFAULT_PAGE, html);
                    } 
                    await WebHelper.WriteToStream(html, os);

                } else if (request.Contains(NavConstants.SEND_MESSAGE))
                {
                    string[] messages = request.Split('/');
                    int numMsg = Int32.Parse(messages[messages.Length - 1]);
                    string listElements = "";
                    for(int i = 0; i < numMsg; i++)
                    {
                        string msg = await lp.GetLocation();
                        var str = new
                        {
                            message = msg,
                            time = DateTime.Now.ToString()
                        };
                        var fullMsg = JsonConvert.SerializeObject(str);
                        await AzureIoTHub.SendDeviceToCloudMessageAsync(fullMsg);
                        await Task.Delay(TimeSpan.FromSeconds(1));
                        string newElement = "<li>" + fullMsg + "</li>\n";
                        listElements= newElement+listElements;
                        
                    }
                    if (htmlPages.ContainsKey(NavConstants.DEFAULT_PAGE))
                    {
                        string html = htmlPages[NavConstants.DEFAULT_PAGE];
                        listElements = "#msgList#\n" + listElements;
                        html = html.Replace("#msgList#", listElements);
                        htmlPages[NavConstants.DEFAULT_PAGE] = html;
                        await WebHelper.WriteToStream(html, os);
                    }

                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Exception in writeResponseAsync(): " + ex.Message);
                Debug.WriteLine(ex.StackTrace);

                // Log telemetry event about this exception
                var events = new Dictionary<string, string> { { "WebServer", ex.Message } };
                Debug.WriteLine(ex.StackTrace.ToString());
                try
                {
                    // Try to send an error page back if there was a problem servicing the request
                    string html = helper.GenerateErrorPage("There's been an error: " + ex.Message + "<br><br>" + ex.StackTrace);
                    await WebHelper.WriteToStream(html, os);
                }
                catch (Exception e)
                {
                    System.Diagnostics.Debug.WriteLine(e.StackTrace.ToString());
                }
            }
        }

        /// <summary>
        /// Get basic html for requested page, with list of stations populated
        /// </summary>
        /// <param name="requestedPage">nav enum ex: home.htm</param>
        /// <returns>string with full HTML, ready to have items replaced. ex: #onState#</returns>
        private async Task<string> GeneratePageHtml(string requestedPage)
        {
            string html = await helper.GeneratePage(requestedPage);
            return html;
        }

        /// <summary>
        /// Redirect to a page
        /// </summary>
        /// <param name="path">Relative path to page</param>
        /// <param name="os"></param>
        /// <returns></returns>
        private async Task redirectToPage(string path, IOutputStream os)
        {
            using (Stream resp = os.AsStreamForWrite())
            {
                byte[] headerArray = Encoding.UTF8.GetBytes(
                                  "HTTP/1.1 302 Found\r\n" +
                                  "Content-Length:0\r\n" +
                                  "Location: /" + path + "\r\n" +
                                  "Connection: close\r\n\r\n");
                await resp.WriteAsync(headerArray, 0, headerArray.Length);
                await resp.FlushAsync();
            }
        }
    }
}
