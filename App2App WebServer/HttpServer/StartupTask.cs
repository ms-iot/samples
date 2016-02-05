// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Text;
using Windows.Foundation.Collections;
using Windows.ApplicationModel.Background;
using Windows.ApplicationModel.AppService;
using Windows.Networking.Sockets;
using System.IO;
using Windows.Storage.Streams;
using System.Threading.Tasks;
using System.Runtime.InteropServices.WindowsRuntime;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace HttpServer
{
    public sealed class StartupTask : IBackgroundTask
    {
        BackgroundTaskDeferral serviceDeferral;
        HttpServer httpServer;

        public void Run(IBackgroundTaskInstance taskInstance)
        {
            // Get the deferral object from the task instance
            serviceDeferral = taskInstance.GetDeferral();

            httpServer = new HttpServer(8000);
            httpServer.StartServer();
        }

    }

    public sealed class HttpServer : IDisposable
    {
        private const string offHtmlString = "<html><head><title>Blinky App</title></head><body><form action=\"blinky.html\" method=\"GET\"><input type=\"radio\" name=\"state\" value=\"on\" onclick=\"this.form.submit()\"> On<br><input type=\"radio\" name=\"state\" value=\"off\" checked onclick=\"this.form.submit()\"> Off</form></body></html>";
        private const string onHtmlString = "<html><head><title>Blinky App</title></head><body><form action=\"blinky.html\" method=\"GET\"><input type=\"radio\" name=\"state\" value=\"on\" checked onclick=\"this.form.submit()\"> On<br><input type=\"radio\" name=\"state\" value=\"off\" onclick=\"this.form.submit()\"> Off</form></body></html>";
        private const uint BufferSize = 8192;
        private int port = 8000;
        private StreamSocketListener listener;
        private AppServiceConnection appServiceConnection;

        public HttpServer(int serverPort)
        {
            listener = new StreamSocketListener();
            listener.Control.KeepAlive = true;
            listener.Control.NoDelay = true;

            port = serverPort;
            listener.ConnectionReceived += async (s, e) => { await ProcessRequestAsync(e.Socket); };
        }

        public void StartServer()
        {
            Task.Run(async () =>
            {
                await listener.BindServiceNameAsync(port.ToString());

                // Initialize the AppServiceConnection
                appServiceConnection = new AppServiceConnection();
                appServiceConnection.PackageFamilyName = "BlinkyWebService_1w720vyc4ccym";
                appServiceConnection.AppServiceName = "App2AppComService";

                // Send a initialize request 
                var res = await appServiceConnection.OpenAsync();
                if (res != AppServiceConnectionStatus.Success)
                {
                    throw new Exception("Failed to connect to the AppService");
                }
            });
        }


        public void Dispose()
        {
            listener.Dispose();
        }

        private async Task ProcessRequestAsync(StreamSocket socket)
        {
            // this works for text only
            StringBuilder request = new StringBuilder();
            byte[] data = new byte[BufferSize];
            IBuffer buffer = data.AsBuffer();
            uint dataRead = BufferSize;
            using (IInputStream input = socket.InputStream)
            {
                while (dataRead == BufferSize)
                {
                    await input.ReadAsync(buffer, BufferSize, InputStreamOptions.Partial);
                    request.Append(Encoding.UTF8.GetString(data, 0, data.Length));
                    dataRead = buffer.Length;
                }
            }

            string requestAsString = request.ToString();
            string[] splitRequestAsString = requestAsString.Split('\n');
            if (splitRequestAsString.Length != 0)
            {
                string requestMethod = splitRequestAsString[0];
                string[] requestParts = requestMethod.Split(' ');
                if (requestParts.Length > 1)
                {
                    if (requestParts[0] == "GET")
                        WriteResponse(requestParts[1], socket);
                    else
                        throw new InvalidDataException("HTTP method not supported: "
                            + requestParts[0]);
                }
            }
        }

        private void WriteResponse(string request, StreamSocket socket)
        {
            // See if the request is for blinky.html, if yes get the new state
            string state = "Unspecified";
            bool stateChanged = false;
            if (request.Contains("blinky.html?state=on"))
            {
                state = "On";
                stateChanged = true;
            }
            else if (request.Contains("blinky.html?state=off"))
            {
                state = "Off";
                stateChanged = true;
            }

            if (stateChanged)
            {
                var updateMessage = new ValueSet();
                updateMessage.Add("Command", state);
#pragma warning disable CS4014
                appServiceConnection.SendMessageAsync(updateMessage);
#pragma warning restore CS4014
            }

            string html = state == "On" ? onHtmlString : offHtmlString;
            byte[] bodyArray = Encoding.UTF8.GetBytes(html);
            // Show the html 
            using (var outputStream = socket.OutputStream)
            {
                using (Stream resp = outputStream.AsStreamForWrite())
                {
                    using (MemoryStream stream = new MemoryStream(bodyArray))
                    {
                        string header = String.Format("HTTP/1.1 200 OK\r\n" +
                                            "Content-Length: {0}\r\n" +
                                            "Connection: close\r\n\r\n",
                                            stream.Length);
                        byte[] headerArray = Encoding.UTF8.GetBytes(header);
                        resp.Write(headerArray, 0, headerArray.Length);
                        stream.CopyTo(resp);
                        resp.Flush();
                    }
                }
            }
        }
    }
}
