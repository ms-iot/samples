// You can learn more about AllJoyn.JS scripting here
// https://wiki.allseenalliance.org/_media/training/programming_alljoyn.js.pdf


// The AJ object contains methods to interact with AllJoyn
var AJ = require('AllJoyn');


// AllJoyn.JS calls this function when it's done connecting to the AllJoyn bus.
AJ.onAttach = function () {
    // You can insert print statements, which are shown on the AllJoyn.JS console if it's connected.
    print("Attached to the AllJoyn bus");

    // This will be called for any AllJoyn service that implements the sought interface.
    // Make sure that if there are multiple devices in the network this may be called multiple
    // times for different services.
    AJ.findService('com.microsoft.ZWaveBridge.Meter.Power', function (svc) {
        print("Connected to a service with the Power Meter interface");

        var oldValue = undefined;

        // The "One-shot and Interval Timers" section of the PDF at the top details how to set up timers.
        // This will call the monitoring function every 350 milliseconds.
        setInterval(function () {
            // Reading properties is done through callbacks.
            svc.getProp('Value').onReply = function (newValue) {
                print('Power: ' + newValue);

                if (newValue > 0 && oldValue == 0) {
                    print("Notifying that something was connected...");

                    // You can read more about notifications in the "Notifications" section of the PDF at the top.
                    // AllJoyn.JS will read out loud the English message text, which is the default if nothing is specified.
                    // Any notifications will be read out loud regardless of their urgency.
                    AJ.notification(AJ.notification.Info, "Something was connected to the switch!").send(100);
                } else if (newValue == 0 && oldValue > 0) {
                    print("Notifying that something was disconnected...");
                    AJ.notification(AJ.notification.Info, "The device connected to the switch was unplugged!").send(100);
                }

                oldValue = newValue;
            };
        }, 350);
    });

    AJ.findService('com.microsoft.ZWaveBridge.SensorMultilevel.Luminance', function (svc) {
        print("Connected to a service with the Luminance Sensor interface");

        var oldValue = undefined;

        // The timer object that setInterval returns can be used to stop it or change the interval.
        var timer = setInterval(function () {
            svc.getProp('Value').onReply = function (newValue) {
                print('Luminance: ' + newValue);

                // You can watch for any changes you're interested in,
                // AllJoyn.JS is a complete JavaScript runtime.
                if (newValue - oldValue >= 500) {
                    print("Notifying that the luminance went up...");
                    AJ.notification(AJ.notification.Info, "The room got brighter!").send(100);
                } else if (newValue - oldValue <= -500) {
                    print("Notifying that the luminance went down...");
                    AJ.notification(AJ.notification.Info, "The room got darker!").send(100);
                }

                // The following line stops the periodic calling of this function.
                // clearInterval(timer);

                oldValue = newValue;
            };
        }, 350);
    });

    AJ.findService('org.alljoyn.ControlPanel.Property', function (svc) {
        print(JSON.stringify(svc));
        if (true) return;

        var oldValue = undefined;

        print("Connected to the LG Refrigerator door!");

        setInterval(function () {
            svc.getProp('Values').onReply = function (newValue) {
                print('Door: ' + newValue);

                if (oldValue != undefined && oldValue != newValue) {
                    if (newValue == 1) {
                        AJ.notification(AJ.notification.Info, "The refrigerator door was opened.").send(100);
                    } else if (newValue - oldValue <= -500) {
                        AJ.notification(AJ.notification.Info, "The refrigerator door was closed.").send(100);
                    }
                }

                oldValue = newValue;
            };
        }, 350);
    });

    AJ.findService('org.alljoyn.ControlPanel.Property', function (svc) {
        if (true) return;

        var oldValue = undefined;

        print("Connected to the LG AC Temperature sensor!");

        setInterval(function () {
            svc.getProp('Values').onReply = function (newValue) {
                print('Temperature: ' + newValue);

                if (oldValue < 30 && newValue >= 30){
                    AJ.notification(AJ.notification.Info, "It's getting hot... Do you want to turn on the AC?").send(100);
                } else if (oldValue > 5 && newValue <= 5){
                    AJ.notification(AJ.notification.Info, "It's getting cold... Do you want to turn on the heat?").send(100);
                }

                oldValue = newValue;
            };
        }, 350);
    });
}