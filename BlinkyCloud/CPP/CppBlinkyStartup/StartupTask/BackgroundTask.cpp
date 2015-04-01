 /********************************************************
*                                                        *
*   © Copyright (C) Microsoft. All rights reserved.      *
*                                                        *
*********************************************************/


#include "pch.h"
#include "BackgroundTask.h"

using namespace Windows::Foundation;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Gpio;
using namespace concurrency;

using namespace StartupTask;
using namespace Platform;

using namespace Microsoft::IoT::Maker;

BackgroundTask::BackgroundTask()
{
}

void BackgroundTask::Run(IBackgroundTaskInstance^ taskInstance)
{
    svc = ref new MakerClient();
    Deferral = taskInstance->GetDeferral();

    TimerElapsedHandler ^handler = ref new TimerElapsedHandler(
        [this](ThreadPoolTimer ^timer) 
        {
            if (LEDStatus == 0)
            {
                LEDStatus = 1;
                if (pin != nullptr)
                    pin->Write(GpioPinValue::High);
            }
            else
            {
                LEDStatus = 0;
                if (pin != nullptr)
                    pin->Write(GpioPinValue::Low);
            }
            svc->SendData("LEDStatus", LEDStatus);    // Upload data to cloud
        }
    );
    TimeSpan interval;
    interval.Duration = 1000 * 1000 * 10;
    Timer = ThreadPoolTimer::CreatePeriodicTimer(handler, interval);

    InitGpio();

    svc->CommandReceived += ref new CommandReceivedEventHandler(
        [this, handler, interval](Object^ sender, CommandReceivedEventArgs^ e)
        {
            // handle commands from cloud
            if (e->CommandName == L"BlinkOn")
            {
                if (Timer == nullptr)
                {
                    Timer = ThreadPoolTimer::CreatePeriodicTimer(handler, interval);
                }
            }
            else if (e->CommandName == L"BlinkOff")
            {
                if (Timer != nullptr)
                    Timer->Cancel();
                Timer = nullptr;
            }
        }
    );

    svc->SendEvent(L"BlinkyStart", L"");        // Upload event to cloud
}

void BackgroundTask::InitGpio()
{
    create_task(GpioController::GetDefaultAsync()).then([this](task<GpioController ^> controllerOp) {
        auto gpio = controllerOp.get();

        if (gpio == nullptr)
        {
            pin = nullptr;
            return;
        }

        pin = gpio->OpenPin(LED_PIN);

        if (pin == nullptr)
        {
            return;
        }

        pin->Write(GpioPinValue::High);
        pin->SetDriveMode(GpioPinDriveMode::Output);
    });
}
