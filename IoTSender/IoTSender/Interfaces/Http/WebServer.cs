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
        private Dictionary<string, string> originalPages;
        private bool sendMsg;
        private string devloc;
        private string status = "Ready to send messages";
        private string loginStatus = "#status#";
        private const string  AccountContainer ="AccountContainer";
        private const string HostKey = "HostName";
        private const string DeviceKey = "DeviceID";
        private const string SharedKey = "SharedKey";
        private const string ProvisionKey = "DeviceProvision";
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
        private async Task SendMessages(string request)
        {
            sendMsg = true;
            string[] messages = request.Split('/');
            int numMsg = Int32.Parse(messages[messages.Length - 1]);
            string listElements = "";
            string postStatus = "Ready to send messages";
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
                        longitude = parsedmsg[1]
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
                    } catch (System.Exception e )
                    {
                        postStatus = "An error occurred when sending your message. Please sign in with your correct credentials";
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
            string html = await LoadPage(NavConstants.DEFAULT_PAGE);
            listElements = "<p id='start-list'>#msgList#</p> \n" + listElements;
            html = html.Replace("<p id='start-list'>#msgList#</p>", listElements);
            html = html.Replace(status, postStatus);
            status = postStatus;
            htmlPages[NavConstants.DEFAULT_PAGE] = html;

        }
        private void SignOutDevice()
        {
            if(ApplicationData.Current.LocalSettings.Containers.ContainsKey(AccountContainer))
            {
                ApplicationDataContainer appcontainer = ApplicationData.Current.LocalSettings.Containers[AccountContainer];
                if (appcontainer.Values.ContainsKey(HostKey))
                {
                    appcontainer.Values.Remove(HostKey);

                }
                if (appcontainer.Values.ContainsKey(DeviceKey))
                {
                    appcontainer.Values.Remove(DeviceKey);

                }
                if (appcontainer.Values.ContainsKey(SharedKey))
                {
                    appcontainer.Values.Remove(SharedKey);

                }
                if (appcontainer.Values.ContainsKey(ProvisionKey))
                {
                    appcontainer.Values.Remove(ProvisionKey);

                }
                ApplicationData.Current.LocalSettings.DeleteContainer(AccountContainer);
            }
            
        }
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
        private async Task<string> LoadandUpdateStatus(string currentStatus, string postStatus, string page)
        {
            string html = await LoadPage(page);
            html = html.Replace(currentStatus, postStatus);
            htmlPages[page] = html;
            return html;
        }
        private async Task<string> LoginWithDevice()
        {
            string html = "";
            ApplicationDataContainer appcontainer = ApplicationData.Current.LocalSettings.Containers[AccountContainer];
            try
            {
                if(appcontainer.Values.ContainsKey(ProvisionKey))
                {
                    string provKey = appcontainer.Values[ProvisionKey] as string;
                    if (provKey.ToLower() == "on")
                    {
                        msgHub = new AzureIoTHub();
                    }
                } else
                {
                    string connectionString = DeviceConnectionStringBuilder.CreateWithDeviceInfo(appcontainer.Values[HostKey] as string, appcontainer.Values[DeviceKey] as string, appcontainer.Values[SharedKey] as string);

                    msgHub = new AzureIoTHub(connectionString);

                }
                html = await LoadPage(NavConstants.DEFAULT_PAGE);
                string loc = await lp.GetLocation();
                devloc = "Device Location: " + loc;
                html = await LoadandUpdateStatus("Device Location: #location#", devloc, NavConstants.DEFAULT_PAGE);
                originalPages[NavConstants.DEFAULT_PAGE] = html;
            }
            catch (System.Exception e)
            {
                html = await LoadandUpdateStatus(loginStatus, Constants.LOGIN_ERROR, NavConstants.LOGIN);
                loginStatus = Constants.LOGIN_ERROR;
            }
            return html;
        }
        private bool ParseLoginData(IDictionary<string, string> parameters)
        {
            ApplicationDataContainer appcontainer = ApplicationData.Current.LocalSettings.Containers[AccountContainer];
            bool isValid = false;
            if(parameters.ContainsKey(ProvisionKey))
            {
                appcontainer.Values[ProvisionKey] = parameters[ProvisionKey];
                isValid = true;
            } else if (parameters.ContainsKey(HostKey) && parameters.ContainsKey(DeviceKey) && parameters.ContainsKey(SharedKey))
            {
                appcontainer.Values[HostKey] = parameters[HostKey];
                appcontainer.Values[DeviceKey] = parameters[DeviceKey];
                appcontainer.Values[SharedKey] = parameters[SharedKey];
                isValid = true;

            }
            return isValid;

        }
        private async Task UpdateLog(IDictionary<string, string> pages)
        {
            string html = await LoadPage(NavConstants.DEFAULT_PAGE);
            string loc = await lp.GetLocation();
            loc = "Device Location: " + loc;
            html = pages[NavConstants.DEFAULT_PAGE].Replace(devloc, loc);
            devloc = loc;
            htmlPages[NavConstants.DEFAULT_PAGE] = html;
        }

        private bool ValidateLoginRequest(string request)
        {
            if (string.IsNullOrEmpty(request))
            {
                return false;
            }
            IDictionary<string, string> parameters = WebHelper.ParseGetParametersFromUrl(new Uri(string.Format("http://0.0.0.0/{0}", request)));
            if(!ParseLoginData(parameters))
            {
                return false;
            }
            return true;
        }
        private void CreateContainerIfNotCreated()
        {
            if(!ApplicationData.Current.LocalSettings.Containers.ContainsKey(AccountContainer))
            {
                ApplicationData.Current.LocalSettings.CreateContainer(AccountContainer, ApplicationDataCreateDisposition.Always);
            }
        }
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
                    CreateContainerIfNotCreated();
                    if (!ValidateLoginRequest(request))
                    {
                        html = await LoadandUpdateStatus(loginStatus, Constants.LOGIN_ERROR, NavConstants.LOGIN);
                        loginStatus = Constants.LOGIN_ERROR;
                    } else
                    {
                        html = await LoginWithDevice();   
                    }
                    await WebHelper.WriteToStream(html, os);

                } else if (request.Equals("/") || request.Contains(NavConstants.LOGIN))
                {
                    string html = "";
                    //silent login- if the data exists, then login wiht the device otherwise show the login page
                    if (!ApplicationData.Current.LocalSettings.Containers.ContainsKey(AccountContainer))
                    {
                        html = await LoadandUpdateStatus(loginStatus, Constants.LOGIN_STD_MSG, NavConstants.LOGIN);
                        loginStatus = Constants.LOGIN_STD_MSG;
                    } else
                    {
                        html = await LoginWithDevice();
                    }
                    await WebHelper.WriteToStream(html, os);
                } else if (request.Contains(NavConstants.SEND_MESSAGE))
                {
                    SendMessages(request);
                    string html = await LoadandUpdateStatus(status, Constants.SENDING_AZURE, NavConstants.DEFAULT_PAGE);
                    status = Constants.SENDING_AZURE;
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
                } else if (request.Contains(NavConstants.SIGN_OUT))
                {
                    this.SignOutDevice();
                    string html = await LoadandUpdateStatus(loginStatus, Constants.LOGIN_STD_MSG, NavConstants.LOGIN);
                    loginStatus = Constants.LOGIN_STD_MSG;
                    await WebHelper.WriteToStream(html, os);
                }
                else
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
            catch (FileNotFoundException ex)
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
