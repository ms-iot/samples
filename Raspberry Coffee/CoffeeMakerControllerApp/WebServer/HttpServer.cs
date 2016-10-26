using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;

namespace CortonaCoffee.WebServer
{
    public sealed class HttpServer : IDisposable
    {
        private const uint BufferSize = 8192;

        private readonly StreamSocketListener listener;

        private int port = 8000;

        public delegate void HttpRequestReceivedEvent(HTTPRequest request);
        public event HttpRequestReceivedEvent OnRequestReceived;

        public HttpServer(int serverPort)
        {
            if (listener == null)
            {
                listener = new StreamSocketListener();
            }
            port = serverPort;

            listener.ConnectionReceived += (s, e) => ProcessRequestAsync(e.Socket);
        }

        public void Dispose()
        {
            if (listener != null)
            {
                listener.Dispose();
            }
        }

        internal async void StartServer()
        {
            //#pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
            try
            {
                await listener.BindServiceNameAsync(port.ToString());
            }
            catch (Exception ex)
            {
                SocketErrorStatus status = SocketError.GetStatus(ex.HResult);
                if (status == SocketErrorStatus.AddressAlreadyInUse)
                {
                    await Task.Delay(1000 * 30);
                    StartServer();
                }
                System.Diagnostics.Debug.WriteLine("Http Server could not bind to service {0}. Error {1}", port, ex.Message);
            }
            //#pragma warning restore CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed
        }

        private async void ProcessRequestAsync(StreamSocket socket)
        {
            HTTPRequest request;
            using (IInputStream stream = socket.InputStream)
            {
                HttpRequestParser parser = new HttpRequestParser();
                request = await parser.GetHttpRequestForStream(stream);
                OnRequestReceived.Invoke(request);
            }

            using (IOutputStream output = socket.OutputStream)
            {
                if (request.Method == "GET")
                {
                    await WriteResponseAync(request.URL, output);
                }
                else
                {
                    throw new InvalidDataException("HTTP method not supported: " + request.Method);
                }
            }
        }

        private async Task WriteResponseAync(string request, IOutputStream output)
        {
            using (Stream resp = output.AsStreamForWrite())
            {
                byte[] bodyArray = Encoding.UTF8.GetBytes("<html><head><title>Black and Decker Coffee Maker</title></head><body>Provision your coffee maker.</body></html>");
                MemoryStream stream = new MemoryStream(bodyArray);
                string header = String.Format("HTTP/1.1 200 OK\r\n" +
                                                "Content-Length: {0}\r\n" +
                                                "Connection: close\r\n\r\n",
                                                stream.Length);
                byte[] headerArray = Encoding.UTF8.GetBytes(header);
                await resp.WriteAsync(headerArray, 0, headerArray.Length);
                await stream.CopyToAsync(resp);
                await resp.FlushAsync();
            }
        }
    }
}
