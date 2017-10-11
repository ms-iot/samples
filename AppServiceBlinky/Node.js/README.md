# Using an app service to blink an LED
We’ll create a simple Blinky [app service](https://docs.microsoft.com/en-us/windows/uwp/launch-resume/how-to-create-and-consume-an-app-service) and connect a LED to your Windows IoT Core device (Raspberry Pi 2 or 3, MinnowBoard Max or DragonBoard). We'll also create a simple app service client that blinks the LED. Be aware that the GPIO APIs are only available on Windows IoT Core, so this sample cannot run on your desktop.

## Set up your hardware
___
The hardware setup for this sample is the same as the [C# ‘Blinky’ sample]({{site.baseurl}}/{{page.lang}}/Samples/helloblinky).
Note that the app will not run successfully if it cannot find any available GPIO ports.

## Load the projects in Visual Studio
___

You can find the source code for this sample by downloading a zip of all of our samples [here](https://github.com/ms-iot/samples/archive/develop.zip) and navigating to the `samples-develop\AppServiceBlinky`.  Make a copy of the folder on your disk and open the projects from Visual Studio.  BlinkyService.sln implements the app service and must be started first.  BlinkyClient.sln implements the app service client.

{% include samples/AppDeploymentCS.md %}

When everything is set up, you should be able to press F5 from each instance of Visual Studio.  The BlinkyService app will deploy and start on the Windows IoT device, and you should see the package family name printed the debug output window on Visual Studio.  Before pressing F5 for BlinkyClient app verify that the value of connection.PackageFamilyName matches the value output in the output window by BlinkyService.  When you press F5 for BlinkyClient you should see the attached LED blink.

## Let's look at the code
___
The code is in 2 projects BlinkyService and BlinkyClient.  First we'll look at BlinkyService.

## Adding an app service
___
To add an appservice to our background application first we need to open appxmanifest.xml in a text editor and add an extension with Category="windows.AppService"

```XML
<Extensions>
    <uap:Extension Category="windows.appService" EntryPoint="BlinkyService.StartupTask">
        <uap:AppService Name="BlinkyService" />
    </uap:Extension>
    <Extension Category="windows.backgroundTasks" EntryPoint="BlinkyService.StartupTask">
        <BackgroundTasks>
        <iot:Task Type="startup" />
        </BackgroundTasks>
    </Extension>
</Extensions>
```

Next we'll add a check in the StartupTask::Run method to see if the application is being started as an appservice

```C#
//Check to determine whether this activation was caused by an incoming app service connection
var appServiceTrigger = taskInstance.TriggerDetails as AppServiceTriggerDetails;
if (appServiceTrigger != null)
{
    //Verify that the app service connection is requesting the "BlinkyService" that this class provides
    if (appServiceTrigger.Name.Equals("BlinkyService"))
    {
        //Store the connection and subscribe to the "RequestRecieved" event to be notified when clients send messages
        connection = appServiceTrigger.AppServiceConnection;
        connection.RequestReceived += Connection_RequestReceived;
    }
    else
    {
        deferral.Complete();
    }
}
```

At the beginning of BlinkyService's StartupTask::Run get the deferral object and set up a Canceled event handler to clean up the deferral on exit.

```C#
deferral = taskInstance.GetDeferral();
taskInstance.Canceled += TaskInstance_Canceled;
```

When the Canceled event handler is called Complete the deferral for this instance of the app service if one exists.  If the deferral is not completed then the app service process will be killed by the operating system even if other clients still have connections open to the app service.

```C#
private void TaskInstance_Canceled(IBackgroundTaskInstance sender, BackgroundTaskCancellationReason reason)
{
    if (deferral != null)
    {
        deferral.Complete();
        deferral = null;
    }
}
```

Finally we need to handle service requests:

```C#
private void Connection_RequestReceived(AppServiceConnection sender, AppServiceRequestReceivedEventArgs args)
{
    var messageDeferral = args.GetDeferral();

    //The message is provided as a ValueSet (IDictionary<String,Object)
    //The only message this server understands is with the name "requestedPinValue" and values of "Low" and "High"
    ValueSet message = args.Request.Message;
    string requestedPinValue = (string)message["requestedPinValue"];


    if (message.ContainsKey("requestedPinValue"))
    {

        if (requestedPinValue.Equals("High"))
        {
            pin.Write(GpioPinValue.High);
        }
        else if (requestedPinValue.Equals("Low"))
        {
            pin.Write(GpioPinValue.Low);
        }
        else
        {
            System.Diagnostics.Debug.WriteLine("Reqested pin value is not understood: " + requestedPinValue);
            System.Diagnostics.Debug.WriteLine("Valid values are 'High' and 'Low'");
        }

    }
    else
    {
        System.Diagnostics.Debug.WriteLine("Message not understood");
        System.Diagnostics.Debug.WriteLine("Valid command is: requestedPinValue");
    }

    messageDeferral.Complete();
}
```

## Connect to the app service in BlinkyClient
___
When the client starts it opens a connection to the client.  The string assigned to blinkyService.packageFamilyName uniquely identifies the service we want to connect to.

```Node.js
function connectAppService() {
    if (blinkyService === undefined) {
        blinkyService = new Windows.ApplicationModel.AppService.AppServiceConnection();

        blinkyService.appServiceName = "BlinkyService";
        blinkyService.packageFamilyName = "BlinkyService-uwp_gpek5j0d8wyr0";

        blinkyService.openAsync().done((status) => {
            if (status !== Windows.ApplicationModel.AppService.AppServiceConnectionStatus.success) {
                console.log("connection unsuccessful. status: " + status);
            } else {
                blinkyService.onserviceclosed = onAppServiceClosed;
            }
        }, (err) => {
            if (err) {
                console.log("openAsync error: " + err);
            }
        });
    }
}
```

If everything connects without an error then the timer callback will toggle the value of the LED each time the timer event handler is called.

```Node.js
setInterval(function () {
    if ("High" === requestedPinValue) {
        requestedPinValue = "Low";
    } else {
        requestedPinValue = "High";
    }
    callAppService(requestedPinValue);
}, interval);
```

Remember that we connected the other end of the LED to the 3.3 Volts power supply, so we need to drive the pin to low to have current flow into the LED.
