using System;
using System.Threading.Tasks;
using Windows.ApplicationModel.Background;
using Windows.Storage;
using IoTUtilities;
using System.IO;
using System.Text;

namespace IoTBlocklyBackgroundApp
{
    public sealed class StartupTask : IBackgroundTask
    {
        BackgroundTaskDeferral deferral;
        StorageFolder localFolder = Windows.ApplicationModel.Package.Current.InstalledLocation;
        ChakraHost.ChakraHostAsync host = new ChakraHost.ChakraHostAsync();
        const string mostRecentScriptCacheBlocks = "last-script-blocks.xml";
        const string mostRecentScriptCacheJSCode = "last-script-jscode.js";
        string mostRecentBlocks = "";

        // First, we set up the webserver with the html/css/js files we have in the public folder of this project.
        // We used blockly from https://developers.google.com/blockly/ (amazing frameworks and super easy to use).
        // We created a few custom blocks (inspired by the ones used by the BBC micro:bit, check it out at https://www.microbit.co.uk/).
        // You can go and modify the blocks and the JavaScript translations. Check out public\index.html, public\blockly\blocks-custom.js
        // and public\blockly\javascript-custom.js.
        // Then we add handlers for the "runcode" and "stopcode" POSTs, which leverage the Chakra JavaScript engine to execute
        // the JavaScript code translated by blockly.
        // The JavaScript code uses a few helper functions from the IoTBlocklyHelper WinRT library (which is added applocal to
        // this UWP Background Application).
        // Notice that most of the blocks require the Raspberry Pi Sense Hat (https://www.raspberrypi.org/products/sense-hat/).
        // And then we start listening on port 8000 and start serving pages: Use your favorite browser to browse to your
        // Raspberry Pi IP address:8000 and you can start coding.
        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();
            var publicFolder = await localFolder.GetFolderAsync("public");

            await StartMostRecentScript();

            var server = new SimpleWebServer();

            // include last saved script
            server.Get(
                "/",
                async (req, res) => { await SimpleWebServer.WriteStaticResponseFilter(req, res, publicFolder, IncludeLastScript); });
            server.Get(
                "/index.html",
                async (req, res) => { await SimpleWebServer.WriteStaticResponseFilter(req, res, publicFolder, IncludeLastScript); });

            server.UseStatic(publicFolder);

            server.Post("/runcode", async (req, res) =>
            {
                var code = req.GetValue("code");
                var blocks = req.GetValue("blocks");
                if (!String.IsNullOrEmpty(code))
                {
                    await SaveMostRecentScript(code, blocks);
                    host.runScriptAsync(code);
                }
                await res.RedirectAsync("..");
            });

            server.Post("/stopcode", async (req, res) =>
            {
                host.haltScript();
                await res.RedirectAsync("..");
            });

            server.Listen(8000);
        }

        private async Task StartMostRecentScript()
        {
            var storageFolder = ApplicationData.Current.LocalFolder;

            try
            {
                var lastBlocks = await storageFolder.GetFileAsync(mostRecentScriptCacheBlocks);
                mostRecentBlocks = await FileIO.ReadTextAsync(lastBlocks);
            }
            catch (Exception)
            {
                // do nothing if we cannot open or read the file
            }

            try
            {
                var lastScript = await storageFolder.GetFileAsync(mostRecentScriptCacheJSCode);
                string jscode = await FileIO.ReadTextAsync(lastScript);
                host.runScriptAsync(jscode);
            }
            catch (Exception)
            {
                // do nothing if we cannot open or read the file
            }
        }

        private async Task SaveMostRecentScript(string jscode, string blocks)
        {
            var storageFolder = ApplicationData.Current.LocalFolder;
            try
            {
                var lastScript = await storageFolder.CreateFileAsync(
                    mostRecentScriptCacheJSCode,
                    CreationCollisionOption.ReplaceExisting);
                await FileIO.WriteTextAsync(lastScript, jscode);
            }
            catch (Exception)
            {
            }
            try
            {
                var lastScript = await storageFolder.CreateFileAsync(
                    mostRecentScriptCacheBlocks,
                    CreationCollisionOption.ReplaceExisting);
                // TODO: we should not hardcode this so much, but for now it's ok...
                var newBlocks = blocks.Replace(@"<xml xmlns=""http://www.w3.org/1999/xhtml"">", @"<xml id=""last-script"" style=""display: none"">");
                await FileIO.WriteTextAsync(lastScript, newBlocks);
            }
            catch (Exception)
            {
            }
        }

        private async Task<string> IncludeLastScript(Stream input)
        {
            var inputStream = input.AsInputStream();
            string result;
            using (var dataReader = new Windows.Storage.Streams.DataReader(input.AsInputStream()))
            {
                uint numBytesLoaded = await dataReader.LoadAsync((uint)input.Length);
                string text = dataReader.ReadString(numBytesLoaded);
                result = text.Replace(@"<!-- INCLUDE xml for last-script -->", mostRecentBlocks);
            }
            return result;
        }
    }
}

