using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Networking.Sockets;
using Windows.Storage;
using Windows.Storage.Search;
using Windows.Storage.Streams;

namespace IoTConnectorClient
{
    internal class WebHelper
    {
        //template for all pages
        private Dictionary<string, string> htmlTemplates = new Dictionary<string, string>();
        private StorageFolder InstallFolder;


        /// <summary>
        /// Initializes the WebHelper with the default.htm template
        /// </summary>
        /// <returns></returns>
        /// 
        internal async Task InitializeAsync()
        {
            this.InstallFolder = Windows.ApplicationModel.Package.Current.InstalledLocation;
            await LoadHTMLTemplate(NavConstants.DEFAULT_PAGE);
        }
        /// <summary>
        /// load page from cache or file
        /// </summary>
        /// <param name="page"></param>
        /// <returns></returns>
        public IAsyncOperation<string> GeneratePage(string page)
        {
            return GeneratePageHelper(page).AsAsyncOperation<string>();
        }
        /// <summary>
        /// async operation to load page from cache/file
        /// </summary>
        /// <param name="page"></param>
        /// <returns></returns>
        private async Task<string> GeneratePageHelper(string page)
        {
            if (!this.htmlTemplates.ContainsKey(page))
            {
                await LoadHTMLTemplate(page);
            }
            return this.htmlTemplates[page];
        }
        /// <summary>
        /// load html page from file, store in cache
        /// </summary>
        /// <param name="page"></param>
        /// <returns></returns>
        private async Task LoadHTMLTemplate(string page)
        {
            string htmlBody = "";
            var filePath = string.Format("{0}\\{1}", NavConstants.ASSETSWEB, page);
            var file = (IStorageFile)await this.InstallFolder.TryGetItemAsync(filePath);
            if (file != null)
            {
                htmlBody = await FileIO.ReadTextAsync(file);
                this.htmlTemplates.Add(page, htmlBody);
            }
        }

        /// <summary>
        /// Helper function to generate page
        /// </summary>
        /// <param name="title">Title that appears on the window</param>
        /// <param name="titleBar">Title that appears on the header bar of the page</param>
        /// <param name="content">Content for the body of the page</param>
        /// <returns></returns>
        internal string GenerateErrorPage(string errorMessage)
        {
            return GeneratePage("Error", "Error", errorMessage, "");
        }

        /// <summary>
        /// Helper function to generate page
        /// </summary>
        /// <param name="title">Title that appears on the window</param>
        /// <param name="titleBar">Title that appears on the header bar of the page</param>
        /// <param name="content">Content for the body of the page</param>
        /// <param name="message">A status message that will appear above the content</param>
        /// <returns></returns>
        string GeneratePage(string title, string titleBar, string content, string head = "", string message = "")
        {
            string html = this.htmlTemplates[NavConstants.DEFAULT_PAGE];

            return html;
        }

        /// <summary>
        /// Parses the GET parameters from the URL and returns the parameters and values in a Dictionary
        /// </summary>
        /// <param name="uri"></param>
        /// <returns></returns>
        public static IDictionary<string, string> ParseGetParametersFromUrl(Uri uri)
        {
            Dictionary<string, string> parameters = new Dictionary<string, string>();
            if (!string.IsNullOrEmpty(uri.Query))
            {
                var decoder = new WwwFormUrlDecoder(uri.Query);
                foreach (WwwFormUrlDecoderEntry entry in decoder)
                {
                    parameters.Add(entry.Name, entry.Value);
                }
            }
            return parameters;
        }

        /// <summary>
        /// Writes html data to the stream
        /// </summary>
        /// <param name="data"></param>
        /// <param name="os"></param>
        /// <returns></returns>
        /// 
        public static IAsyncAction WriteToStream(string data, IOutputStream os)
        {
            return WriteToStreamHelper(data, os).AsAsyncAction();
        }

        private static async Task WriteToStreamHelper(string data, IOutputStream os)
        {
            using (Stream resp = os.AsStreamForWrite())
            {
                // Look in the Data subdirectory of the app package
                byte[] bodyArray = Encoding.UTF8.GetBytes(data);
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

        /// <summary>
        /// Writes a file to the stream
        /// </summary>
        /// <param name="file"></param>
        /// <param name="os"></param>
        /// <returns></returns>
        /// 
        public IAsyncAction WriteFileToStream(StorageFile file, IOutputStream os)
        {
            return WriteFileToStreamHelper(file, os).AsAsyncAction();
        }
        /// <summary>
        /// Async helper to write file to stream
        /// </summary>
        /// <param name="file"></param>
        /// <param name="os"></param>
        /// <returns></returns>
        private static async Task WriteFileToStreamHelper(StorageFile file, IOutputStream os)
        {
            using (Stream resp = os.AsStreamForWrite())
            {
                try
                {
                    using (Stream fs = await file.OpenStreamForReadAsync())
                    {
                        string header = String.Format("HTTP/1.1 200 OK\r\n" +
                                        "Content-Length: {0}\r\n" +
                                        "Connection: close\r\n\r\n",
                                        fs.Length);
                        byte[] headerArray = Encoding.UTF8.GetBytes(header);
                        await resp.WriteAsync(headerArray, 0, headerArray.Length);
                        await fs.CopyToAsync(resp);

                    }
                }
                catch (FileNotFoundException)
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
}
