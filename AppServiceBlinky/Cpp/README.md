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

```C++
//Check to determine whether this activation was caused by an incoming app service connection
auto appServiceTrigger = dynamic_cast<AppServiceTriggerDetails^>(taskInstance->TriggerDetails);

if (appServiceTrigger != nullptr)
{
    if (String::operator==(appServiceTrigger->Name, L"BlinkyService"))
    {
        controller = GpioController::GetDefault();
        serviceConnection = appServiceTrigger->AppServiceConnection;
        serviceConnection->RequestReceived += ref new Windows::Foundation::TypedEventHandler<Windows::ApplicationModel::AppService::AppServiceConnection ^, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs ^>(this, &BlinkyService::BlinkyServiceProvider::OnRequestReceived);
    }
    else
    {
        serviceDeferral->Complete();
    }
}
```

At the beginning of BlinkyService's StartupTask::Run get the deferral object and set up a Canceled event handler to clean up the deferral on exit.

```C++
serviceDeferral = taskInstance->GetDeferral();
taskInstance->Canceled += ref new Windows::ApplicationModel::Background::BackgroundTaskCanceledEventHandler(this, &BlinkyService::BlinkyServiceProvider::OnCanceled);
```

When the Canceled event handler is called Complete the deferral for this instance of the app service if one exists.  If the deferral is not completed then the app service process will be killed by the operating system even if other clients still have connections open to the app service.

```C++
void BlinkyService::BlinkyServiceProvider::OnCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason)
{
    if (serviceDeferral != nullptr)
    {
        serviceDeferral->Complete();
    }
}
```

Finally we need to handle service requests:

```C++
void BlinkyService::BlinkyServiceProvider::OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection ^sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs ^args)
{
    auto messageDeferral = args->GetDeferral();

    auto responseMessage = ref new ValueSet();

    if (args->Request->Message != nullptr)
    {
        if ((args->Request->Message->HasKey(L"SetLedState")) && (args->Request->Message->HasKey(L"PinNumber")))
        {
            try
            {
                auto commandObject = args->Request->Message->Lookup(L"SetLedState");
                String^ command = safe_cast<String^>(commandObject);
                auto pinNumberObject = args->Request->Message->Lookup(L"PinNumber");
                int pinNumber = safe_cast<int>(pinNumberObject);
                auto pin = controller->OpenPin(pinNumber);
                pin->SetDriveMode(GpioPinDriveMode::Output);

                if (String::operator==(command, L"SetLedStateOn"))
                {
                    pin->Write(GpioPinValue::High);
                    responseMessage->Insert(L"Response", L"Success");
                }
                else if (String::operator==(command, L"SetLedStateOff"))
                {
                    pin->Write(GpioPinValue::Low);
                    responseMessage->Insert(L"Response", L"Succes");
                }
                else
                {
                    responseMessage->Insert(L"Response", L"Request Failed:Invalid SetLedState parameter value.");
                }
            }
            catch (InvalidCastException^ ic)
            {
                responseMessage->Insert(L"Response", L"Request Failed:Invalid cast exception occurred.");
            }
            catch (Exception^ e)
            {
                responseMessage->Insert(L"Response", L"Request Failed:Unknown exception occurred.");
            }
        }
        else
        {
            responseMessage->Insert(L"Response", L"Request Failed:Invalid request.");
        }
    }
    else
    {
        responseMessage->Insert(L"Response", L"Failed: Request message is empty.");
    }

    args->Request->SendResponseAsync(responseMessage);

    messageDeferral->Complete();
}
```

## Connect to the app service in BlinkyClient
___
When the client starts it opens a connection to the client.  The string assigned to connection.PackageFamilyName uniquely identifies the service we want to connect to.

```C++
void StartupTask::Run(IBackgroundTaskInstance^ taskInstance)
{
    serviceDeferral = taskInstance->GetDeferral();

    serviceConnection = ref new AppServiceConnection();
    serviceConnection->PackageFamilyName = L"BlinkyService-uwp_2yx4q2bk84nj4";
    serviceConnection->AppServiceName = L"BlinkyService";

    auto connectTask = Concurrency::create_task(serviceConnection->OpenAsync());
    connectTask.then([this](AppServiceConnectionStatus connectStatus)
    {
        if (AppServiceConnectionStatus::Success == connectStatus)
        {
            auto message = ref new ValueSet();
            command = L"SetLedStateOn";
            message->Insert(L"SetLedState", command);
            message->Insert(L"PinNumber", 5);
            auto messageTask = Concurrency::create_task(serviceConnection->SendMessageAsync(message));

            messageTask.then([this](AppServiceResponse ^appServiceResponse)
            {
                if ((appServiceResponse->Status == AppServiceResponseStatus::Success) &&
                    (appServiceResponse->Message != nullptr) &&
                    (appServiceResponse->Message->HasKey(L"Response")))
                {
                    auto response = appServiceResponse->Message->Lookup(L"Response");
                    String^ responseMessage = safe_cast<String^>(response);
                    if (String::operator==(responseMessage, L"Success"))
                    {
                        TimerElapsedHandler ^handler = ref new TimerElapsedHandler(
                            [this](ThreadPoolTimer ^timer)
                        {
                            RequestBlinkService();
                        });

                        TimeSpan interval;
                        interval.Duration = 500 * 1000 * 10;
                        timer = ThreadPoolTimer::CreatePeriodicTimer(handler, interval);
                    }
                }
            });
        }
        else
        {
            serviceDeferral->Complete();
        }
    });
}
```

If everything connects without an error then the timer callback will toggle the value of the LED each time the timer event handler is called.

```C++
command = String::operator==(command, L"SetLedStateOn") ? L"SetLedStateOff" : L"SetLedStateOn";
auto message = ref new ValueSet();
message->Insert(L"SetLedState", command);
message->Insert(L"PinNumber", 5);
auto messageTask = Concurrency::create_task(serviceConnection->SendMessageAsync(message));
```

Remember that we connected the other end of the LED to the 3.3 Volts power supply, so we need to drive the pin to low to have current flow into the LED.
