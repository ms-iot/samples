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

namespace IoTConnectorClient
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
        private Dictionary<string, string> originalPages;
        private bool sendMsg;
        private string devloc;
        private string status = "Ready to send messages";
        private string loginStatus = "#status#";
        private const string  AccountContainer ="AccountContainer";
        private const string HostKey = "HostName";
        private const string DeviceKey = "DeviceID";
        private const string SharedKey = "SharedKey";
        private AzureIoTHub msgHub;

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="serverPort">Port to start server on</param>
        internal HttpInterfaceManager(int serverPort)
        {
            sendMsg = true;
            helper = new WebHelper();
            listener = new StreamSocketListener();
            msgHub = new AzureIoTHub();
            lp = new LocationProvider();
            htmlPages = new Dictionary<string, string>();
            originalPages = new Dictionary<string, string>();
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
                    // Convert the request bytes to a string
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
        /// <summary>
        /// sends a number of messages to the cloud
        /// </summary>
        /// <param name="numMsg">The number of messages to send</param>
        /// <returns></returns>
        private async Task SendMessages(int numMsg)
        {
            sendMsg = true;
            string listElements = "";
            string postStatus = Constants.READY_AZURE;
            for (int i = 0; i < numMsg; i++)
            {
                if (sendMsg)
                {
                    string msg = await lp.GetLocation();
                    string[] parsedmsg = msg.Split(',');
                    var coords = new
                    {
                        type = "coordinates",
                        latitude = parsedmsg[0],
                        longitude = parsedmsg[1],
                        deviceName = msgHub.GetDeviceId(),
                        version = Constants.VERSION
                    };
                    var str = new
                    {
                        message = coords,
                        time = DateTime.Now.ToString()
                    };
                    var fullMsg = JsonConvert.SerializeObject(str);
                    try
                    {
                        await msgHub.SendDeviceToCloudMessageAsync(fullMsg);
                    } catch (Exception)
                    {
                        postStatus = Constants.ERROR_AZURE;
                        break;
                    }
                    
                    await Task.Delay(TimeSpan.FromSeconds(1));
                    string newElement = "<li class='msg'>" + fullMsg + "</li>\n";
                    listElements = newElement + listElements;
                }
                else
                {
                    Debug.WriteLine("MESSAGES STOPPED");
                    break;
                }


            }

            listElements = Constants.LIST_HEADER + "\n" + listElements;
            string html = await LoadandUpdateStatus(Constants.LIST_HEADER, listElements, NavConstants.DEFAULT_PAGE);
            html = await LoadandUpdateStatus(status, postStatus, NavConstants.DEFAULT_PAGE); 
            status = postStatus;

        }

        /// <summary>
        /// Load an html page, if not already cached then read from file
        /// </summary>
        /// <param name="page">page file name</param>
        /// <returns>the entire page as a string</returns>
        private async Task<string> LoadPage(string page)
        {
            string html = "";
            if (htmlPages.ContainsKey(page))
            {
                html = htmlPages[page];
            }
            else
            {
                string fullpath = @"\views\" + page;
                html = await GeneratePageHtml(fullpath);
                htmlPages.Add(page, html);
                originalPages.Add(page, html);
            }
            return html;
        }
        /// <summary>
        /// Load a specific page and then update any variables in the page
        /// </summary>
        /// <param name="currentStatus">variable in html page to replace </param>
        /// <param name="postStatus">value that will be inserted into html page</param>
        /// <param name="page">page to load</param>
        /// <returns></returns>
        private async Task<string> LoadandUpdateStatus(string currentStatus, string postStatus, string page)
        {
            string html = await LoadPage(page);
            html = html.Replace(currentStatus, postStatus);
            htmlPages[page] = html;
            return html;
        }
        /// <summary>
        /// Attempt to create a deviceclient/instance of AzureIoTHub with credentials given.
        /// If instantiation fails, then the login page will be shown, with an error message
        /// </summary>
        /// <returns>the page to load</returns>
        private async Task<string> LoginWithCredentials(string host, string device, string shared)
        {
            string html = "";
            string connectionString = DeviceConnectionStringBuilder.CreateWithDeviceInfo(host, device, shared);
            if (msgHub.Connect(connectionString))
            {
                html = await LoadPage(NavConstants.DEFAULT_PAGE);
                string loc = await lp.GetLocation();
                devloc = "Device Location: " + loc;
                html = await LoadandUpdateStatus("Device Location: #location#", devloc, NavConstants.DEFAULT_PAGE);
                originalPages[NavConstants.DEFAULT_PAGE] = html;
            } else
            {
                html = await LoadandUpdateStatus(loginStatus, Constants.LOGIN_ERROR, NavConstants.LOGIN);
                loginStatus = Constants.LOGIN_ERROR;
            }
            return html;
        }

        /// <summary>
        /// Refresh or clear the log by reloading the default page
        /// </summary>
        /// <param name="pages"></param>
        /// <returns></returns>
        private async Task UpdateLog(IDictionary<string, string> pages)
        {
            string loc = await lp.GetLocation();
            loc = "Device Location: " + loc;
            string html = pages[NavConstants.DEFAULT_PAGE].Replace(devloc, loc);
            devloc = loc;
            htmlPages[NavConstants.DEFAULT_PAGE] = html;
        }
        /// <summary>
        /// Turn post request parameters into a dictionary of strings
        /// </summary>
        /// <param name="request"></param>
        /// <returns></returns>
        private IDictionary<string, string> ParseParametersFromRequest(string request)
        {
            if (string.IsNullOrEmpty(request))
            {
                throw new System.Exception("Request is null");
            }

            IDictionary<string, string> parameters = WebHelper.ParseGetParametersFromUrl(new Uri(string.Format("http://0.0.0.0/{0}", request)));
            return parameters;
        }
        /// <summary>
        /// Basic validation of POST request with login data
        /// </summary>
        /// <param name="request">POST request</param>
        /// <returns></returns>
        private bool ValidateLoginRequest(IDictionary<string, string> parameters)
        {
            if(parameters == null)
            {
                return false;
            }
            //make sure the necessary components exist
            if(!parameters.ContainsKey(HostKey) || !parameters.ContainsKey(DeviceKey) || !parameters.ContainsKey(SharedKey))
            {
                return false;
            }
            //make sure keys are not null or empty
            List<string> keys = parameters.Keys.ToList<string>();
            foreach(string key in keys)
            {
                if(string.IsNullOrEmpty(parameters[key]))
                {
                    return false;
                }
            }
            return true;
        }
        /// <summary>
        /// respond to GET/POST request
        /// </summary>
        /// <param name="request"></param>
        /// <param name="os"></param>
        /// <param name="socketInfo"></param>
        /// <returns></returns>
        private async Task writeResponseAsync(string request, IOutputStream os, StreamSocketInformation socketInfo)
        {
            try
            {
                request = request.TrimEnd('\0'); //remove possible null from request

                string[] requestParts = request.Split('/');

                //home page
                if (request.Contains(NavConstants.DEFAULT_PAGE))
                {

                    string html = "";
                    try
                    {
                        IDictionary<string, string> parameters = ParseParametersFromRequest(request);
                        if (!ValidateLoginRequest(parameters))
                        {
                            html = await LoadandUpdateStatus(loginStatus, Constants.LOGIN_ERROR, NavConstants.LOGIN);
                            loginStatus = Constants.LOGIN_ERROR;
                        }
                        else
                        {
                            html = await LoginWithCredentials(parameters[HostKey], parameters[DeviceKey], parameters[SharedKey]);
                        }
                        await WebHelper.WriteToStream(html, os);
                    } catch (System.Exception ex)
                    {
                        try
                        {
                            // Try to send an error page back if there was a problem servicing the request
                            html = helper.GenerateErrorPage("There's been an error: " + ex.Message + "<br><br>" + ex.StackTrace);
                            await WebHelper.WriteToStream(html, os);
                        }
                        catch (Exception e)
                        {
                            System.Diagnostics.Debug.WriteLine(e.StackTrace.ToString());
                        }

                    }
                    

                } else if (request.Equals("/") || request.Contains(NavConstants.LOGIN))
                {
                    string html = "";
                    //silent login- if the data exists, then login wiht the device otherwise show the login page
                    if(msgHub.Connect())
                    {
                        html = await LoadPage(NavConstants.DEFAULT_PAGE);
                        string loc = await lp.GetLocation();
                        devloc = "Device Location: " + loc;
                        html = await LoadandUpdateStatus("Device Location: #location#", devloc, NavConstants.DEFAULT_PAGE);
                        originalPages[NavConstants.DEFAULT_PAGE] = html;
                    } else
                    {
                        html = await LoadandUpdateStatus(loginStatus, Constants.LOGIN_STD_MSG, NavConstants.LOGIN);
                        loginStatus = Constants.LOGIN_STD_MSG;
                    }
                    
                    await WebHelper.WriteToStream(html, os);
                } else if (request.Contains(NavConstants.SEND_MESSAGE))
                {
                    IDictionary<string, string> parameters = ParseParametersFromRequest(request);
                    string html = "";
                    if (parameters.ContainsKey("msgs"))
                    {
                        int numMsg = Int32.Parse(parameters["msgs"]);
                        SendMessages(numMsg);
                        html = await LoadandUpdateStatus(status, Constants.SENDING_AZURE, NavConstants.DEFAULT_PAGE);
                        status = Constants.SENDING_AZURE;
                    } else
                    {
                        html = await LoadandUpdateStatus(status, Constants.ERROR_AZURE, NavConstants.DEFAULT_PAGE);
                    }
                    
                    await WebHelper.WriteToStream(html, os);

                } else if(request.Contains(NavConstants.CANCEL_MESSAGE))
                {
                    sendMsg = false;
                    string html = await LoadandUpdateStatus(status, Constants.READY_AZURE, NavConstants.DEFAULT_PAGE);
                    status = Constants.READY_AZURE;
                    await WebHelper.WriteToStream(html, os);
                } else if (request.Contains(NavConstants.CLEAR_LOG))
                {
                    await UpdateLog(originalPages);
                    await WebHelper.WriteToStream(htmlPages[NavConstants.DEFAULT_PAGE], os);
                } else if (request.Contains(NavConstants.REFRESH_LOG))
                {
                    await UpdateLog(htmlPages);
                    await WebHelper.WriteToStream(htmlPages[NavConstants.DEFAULT_PAGE], os);
                } else
                {
                    using (Stream resp = os.AsStreamForWrite())
                    {
                        bool exists = await loadCSSPage(request, resp);
                        

                        // Send 404 not found if can't find file
                        if (!exists)
                        {
                            byte[] headerArray = Encoding.UTF8.GetBytes(
                                                  "HTTP/1.1 404 Not Found\r\n" +
                                                  "Content-Length:0\r\n" +
                                                  "Connection: close\r\n\r\n");
                            await resp.WriteAsync(headerArray, 0, headerArray.Length);
                        } else
                        {
                            Debug.WriteLine("LOADED: " + request);
                        }

                        await resp.FlushAsync();
                    }
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Exception in writeResponseAsync(): " + ex.Message);
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
        private async Task<bool> loadCSSPage(string request, Stream resp)
        {

            bool exists = true;
            try
            {
                var folder = Windows.ApplicationModel.Package.Current.InstalledLocation;

                // Map the requested path to Assets\Web folder
                string filePath = NavConstants.ASSETSWEB + request.Replace('/', '\\');

                // Open the file and write it to the stream
                using (Stream fs = await folder.OpenStreamForReadAsync(filePath))
                {
                    string contentType = "";
                    if (request.Contains("css"))
                    {
                        contentType = "Content-Type: text/css\r\n";
                    }
                    if (request.Contains("htm"))
                    {
                        contentType = "Content-Type: text/html\r\n";
                    }
                    string header = String.Format("HTTP/1.1 200 OK\r\n" +
                                    "Content-Length: {0}\r\n{1}" +
                                    "Connection: close\r\n\r\n",
                                    fs.Length,
                                    contentType);
                    byte[] headerArray = Encoding.UTF8.GetBytes(header);
                    await resp.WriteAsync(headerArray, 0, headerArray.Length);
                    await fs.CopyToAsync(resp);
                }
            }
            catch (FileNotFoundException)
            {
                exists = false;
            }
            return exists;

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
