using System;
using Windows.ApplicationModel.Background;
using Windows.Storage;

namespace IoTBlocklyBackgroundApp
{
    public sealed class StartupTask : IBackgroundTask
    {
        BackgroundTaskDeferral deferral;
        StorageFolder localFolder = Windows.ApplicationModel.Package.Current.InstalledLocation;
        ChakraHost.ChakraHostAsync host = new ChakraHost.ChakraHostAsync();

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

            var server = new IoTUtilities.SimpleWebServer();

            server.UseStatic(publicFolder);

            server.Post("/runcode", async (req, res) =>
            {
                var code = req.GetValue("code");
                if (!String.IsNullOrEmpty(code))
                {
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


            //            var code =
            //@"
            //var basic = new IoTBlocklyHelper.Basic();
            //var gpio = new IoTBlocklyHelper.Gpio();
            //var senseHat = new IoTBlocklyHelper.SenseHat();
            //var adc = new IoTBlocklyHelper.Adc();
            //var eventsQueue = [];

            //function pauseHelper(ms) {
            //  if (eventsQueue.length == 0) {
            //    basic.pause(ms);
            //  }
            //  else {
            //    // senseHat.print(eventsQueue.length, Windows.UI.Colors.black, Windows.UI.Colors.blue);
            //    var pauseUntilTarget = basic.runningTime() + ms;
            //    while (basic.runningTime() < pauseUntilTarget) {
            //      for (var i = 0; i < eventsQueue.length; ++i) {
            //        //senseHat.print(i, Windows.UI.Colors.black, Windows.UI.Colors.blue);
            //        eventsQueue[i]();
            //      }
            //    }
            //  }
            //}

            //// CODE START

            //while (true) {
            //  senseHat.clear(true);
            //  pauseHelper(300);
            //  senseHat.drawLedMatrix('00000000|01100110|11111111|11111111|01111110|00111100|00011000|00000000|', Windows.UI.Colors.red, 0, 0, true);
            //  pauseHelper(300);
            //}

            //// EVENT START
            //eventsQueue.push(function() {
            //  //senseHat.print('q', Windows.UI.Colors.black, Windows.UI.Colors.blue);
            //  if (senseHat.getJoystickState(IoTBlocklyHelper.SenseHatJoystickButton.enter) > 0)
            //            {
            //                senseHat.print('Welcome to Windows 10 IoT Core', Windows.UI.Colors.black, Windows.UI.Colors.blue);
            //            }
            //        });
            //// EVENT END

            //";

            //            var code = @"
            //var basic = new IoTBlocklyHelper.Basic();
            //var gpio = new IoTBlocklyHelper.Gpio();
            //var senseHat = new IoTBlocklyHelper.SenseHat();
            //var adc = new IoTBlocklyHelper.Adc();
            //var eventsQueue = [];

            //function pauseHelper(ms) {
            //    if (eventsQueue.length == 0) {
            //        basic.pause(ms);
            //    }
            //    else {
            //        var pauseUntilTarget = basic.runningTime() + ms;
            //        while (basic.runningTime() < pauseUntilTarget) {
            //            for (var i = 0; i < eventsQueue.length; ++i) {
            //                eventsQueue[i]();
            //            }
            //        }
            //    }
            //}

            //eventsQueue.push(function () {
            //    if (senseHat.getJoystickState(IoTBlocklyHelper.SenseHatJoystickButton.enter) > 0) {
            //        senseHat.print('IoT Core', Windows.UI.Colors.black, Windows.UI.Colors.blue);
            //    }
            //});


            //while (true) {
            //    senseHat.clear(true);
            //    pauseHelper(300);
            //    senseHat.drawLedMatrix('00000000|01100110|11111111|11111111|01111110|00111100|00011000|00000000|', Windows.UI.Colors.red, 0, 0, true);
            //    pauseHelper(300);
            //}
            //";

            //            host.runScriptAsync(code);
        }
    }
}

