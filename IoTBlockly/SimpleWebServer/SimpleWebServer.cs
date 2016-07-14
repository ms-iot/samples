using System;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Networking.Sockets;
using Windows.Storage;
using Windows.Storage.Streams;
using System.Collections.Generic;

// The SimpleWebServer helper class helps you create a basic web server from a UWP application.
// It leverages Windows.Networking.Sockets.StreamSocketListener.
// You can add handlers for GET and POST methods and specific paths using SimpleWebServer.Get() and SimpleWebServer.Post().
// To serve static files, you use SimpleWebServer.UseStatic().
// Each handler is a SimpleWebServer.RouteCallback async delegate, which takes in a SimpleWebServer.Request object and uses
// SimpleWebServer.Response to send the response back. Request and Response have some handy helper functions inside.
// The overall mechanism is (very) loosely modeleded on Node.JS Express package.
//
// This helper class is not complete (a few TODOs are sprinkled around the code) but it works well for the IoTBlockly needs.
// Please feel free to send us comments, reuse and expand it.

namespace IoTUtilities
{
    public class SimpleWebServer : IDisposable
    {
        public class Request
        {
            public string RawRequestString { get; private set; }
            public string RawRequestMethod { get; private set; }
            public string[] RequestPart { get; private set; }
            public string Method
            {
                get
                {
                    try
                    {
                        return RequestPart[0];
                    }
                    catch
                    {
                        return null;
                    }
                }
            }
            public string Path
            {
                get
                {
                    try
                    {
                        return RequestPart[1];
                    }
                    catch
                    {
                        return null;
                    }
                }
            }

            private string[] splitRawRequest;
            private string[] values;

            public Request(string rawRequestString)
            {
                RawRequestString = rawRequestString;

                splitRawRequest = rawRequestString.Split('\n');
                if (splitRawRequest.Length != 0)
                {
                    RawRequestMethod = splitRawRequest[0];
                    RequestPart = RawRequestMethod.Split(' ');
                    var valuePart = splitRawRequest[splitRawRequest.Length - 1];
                    if (!valuePart.StartsWith("\0"))
                    {
                        values = valuePart.Split('&');
                    }
                }
            }

            public string GetValue(string key)
            {
                if (values == null) { return null; }
                string result = null;
                key += "=";
                foreach (var v in values)
                {
                    if (v.StartsWith(key))
                    {
                        var value = v.Substring(key.Length);
                        result = System.Net.WebUtility.UrlDecode(value);
                        break;
                    }
                }
                if (result != null)
                {
                    // REVIEW: weird... tons of /0 in the string at the end...
                    result.Replace("\0", "");
                }
                return result;
            }
        }

        public class Response
        {
            private StreamSocket socket;

            public Response(StreamSocket socket)
            {
                this.socket = socket;
            }

            public async Task SendFileAsync(Stream file)
            {
                using (var outputStream = socket.OutputStream)
                {
                    using (Stream resp = outputStream.AsStreamForWrite())
                    {
                        await WriteHeaderNoFlushAsync(resp, 200, "OK", file.Length);
                        await file.CopyToAsync(resp);
                        await resp.FlushAsync();
                    }
                }
            }

            public async Task SendFileContentAsync(string fileContent)
            {
                using (var outputStream = socket.OutputStream)
                {
                    using (Stream resp = outputStream.AsStreamForWrite())
                    {
                        byte[] fileContentArray = Encoding.UTF8.GetBytes(fileContent);
                        await WriteHeaderNoFlushAsync(resp, 200, "OK", fileContentArray.Length);
                        await resp.WriteAsync(fileContentArray, 0, fileContentArray.Length);
                        await resp.FlushAsync();
                    }
                }
            }

            public async Task SendStatusAsync(int statusCode)
            {
                using (var outputStream = socket.OutputStream)
                {
                    using (Stream resp = outputStream.AsStreamForWrite())
                    {
                        var message = statusCode.ToString();
                        // TODO: add more status code mappings
                        switch (statusCode)
                        {
                            case 200:
                                message = "OK";
                                break;
                            case 302:
                                message = "Found";
                                break;
                            case 404:
                                message = "Not Found";
                                break;
                            case 500:
                                message = "Internal Server Error";
                                break;
                        }

                        await WriteHeaderNoFlushAsync(resp, statusCode, message);
                        await resp.FlushAsync();
                    }
                }
            }

            public async Task RedirectAsync(string path)
            {
                using (var outputStream = socket.OutputStream)
                {
                    using (Stream resp = outputStream.AsStreamForWrite())
                    {
                        await WriteHeaderNoFlushAsync(resp, 302, "Found", 0, path);
                        await resp.FlushAsync();
                    }
                }
            }

            private Task WriteHeaderNoFlushAsync(Stream resp, int statusCode, string message, long contentLength = 0, string location = "")
            {
                if (!String.IsNullOrEmpty(location))
                {
                    location = "Location: " + location + "\r\n";
                }
                else
                {
                    location = "";
                }
                string header = String.Format(
                    "HTTP/1.1 {0} {1}\r\n" +
                    location +
                    "Content-Length: {2}\r\n" +
                    "Connection: close\r\n\r\n",
                    statusCode, message,
                    contentLength);
                byte[] headerArray = Encoding.UTF8.GetBytes(header);
                return resp.WriteAsync(headerArray, 0, headerArray.Length);
            }
        }

        public delegate Task RouteCallback(Request req, Response res);

        public delegate Task<string> Filter(Stream input);

        const uint BufferSize = 8192;
        StreamSocketListener listener;
        List<Tuple<string, RouteCallback>> getRoutes = new List<Tuple<string, RouteCallback>>();
        List<Tuple<string, RouteCallback>> getStaticRoutes = new List<Tuple<string, RouteCallback>>();
        List<Tuple<string, RouteCallback>> postRoutes = new List<Tuple<string, RouteCallback>>();

        public SimpleWebServer()
        {
            listener = new StreamSocketListener();
            listener.Control.KeepAlive = true;
            listener.Control.NoDelay = true;
            listener.ConnectionReceived += (s, e) => { ProcessRequestAsync(e.Socket); };
        }

        public async void Listen(int port)
        {
            await listener.BindServiceNameAsync(port.ToString());
        }

        public void Dispose()
        {
            listener.Dispose();
        }

        public void UseStatic(StorageFolder root)
        {
            getStaticRoutes.Add(Tuple.Create<string, RouteCallback>(
                "/",
                async (req, res) => { await WriteStaticResponse(req, res, root); }));
        }

        public void Get(string path, RouteCallback callback)
        {
            getRoutes.Add(Tuple.Create<string, RouteCallback>(path, callback));
        }

        public void Post(string path, RouteCallback callback)
        {
            postRoutes.Add(Tuple.Create<string, RouteCallback>(path, callback));
        }

        private async void ProcessRequestAsync(StreamSocket socket)
        {
            // very rudimentary web server request processing

            // grab the request
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

            // build some handy wrapper objects for request and response
            var req = new Request(request.ToString());
            var res = new Response(socket);

            if (req.Method == null)
            {
                Debug.WriteLine("Cannot retrieve HTTP method");
                return;
            }

            // go through the defined routes to handle the request
            // TODO: implement route matching based on string patterns
            var path = req.Path;
            bool handled = false;
            switch (req.Method)
            {
                case "GET":
                    foreach (var t in getRoutes)
                    {
                        if (t.Item1 == path)
                        {
                            await t.Item2(req, res);
                            handled = true;
                            break;
                        }
                    }
                    if (!handled)
                    {
                        foreach (var t in getStaticRoutes)
                        {
                            if (path.StartsWith(t.Item1))
                            {
                                await t.Item2(req, res);
                                handled = true;
                                break;
                            }
                        }
                    }
                    if (!handled)
                    {
                        // REVIEW: is 404 the right status code to return?
                        await res.SendStatusAsync(404);
                    }
                    break;

                case "POST":
                    foreach (var t in postRoutes)
                    {
                        if (t.Item1 == path)
                        {
                            await t.Item2(req, res);
                            handled = true;
                            break;
                        }
                    }
                    if (!handled)
                    {
                        // REVIEW: is 404 the right status code to return?
                        await res.SendStatusAsync(404);
                    }
                    break;

                default:
                    Debug.WriteLine("HTTP method not supported: " + req.Method);
                    break;
            }
        }

        public static async Task WriteStaticResponse(Request req, Response res, StorageFolder root)
        {
            try
            {
                string requestedFile = req.Path;
                if (requestedFile == "/")
                {
                    requestedFile += "index.html";
                }
                string filePath = requestedFile.Replace('/', '\\');
                using (Stream fs = await root.OpenStreamForReadAsync(filePath))
                {
                    await res.SendFileAsync(fs);
                }
            }
            catch (FileNotFoundException)
            {
                await res.SendStatusAsync(404);
            }
            catch (Exception)
            {
                await res.SendStatusAsync(500);
            }
        }

        public static async Task WriteStaticResponseFilter(Request req, Response res, StorageFolder root, Filter filter)
        {
            // very rudimentary filtering for static web pages
            try
            {
                string requestedFile = req.Path;
                if (requestedFile == "/")
                {
                    requestedFile += "index.html";
                }
                string filePath = requestedFile.Replace('/', '\\');
                using (Stream fs = await root.OpenStreamForReadAsync(filePath))
                {
                    // TODO: we should not await on the full content, but just filter as we read the stream...
                    var content = await filter(fs);
                    await res.SendFileContentAsync(content);
                }
            }
            catch (FileNotFoundException)
            {
                await res.SendStatusAsync(404);
            }
            catch (Exception)
            {
                await res.SendStatusAsync(500);
            }
        }

    }
}
