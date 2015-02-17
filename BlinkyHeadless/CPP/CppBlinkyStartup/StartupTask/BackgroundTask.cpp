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

BackgroundTask::BackgroundTask()
{
}

void BackgroundTask::Run(IBackgroundTaskInstance^ taskInstance)
{
    Deferral = taskInstance->GetDeferral();

    TimerElapsedHandler ^handler = ref new TimerElapsedHandler(
        [this](ThreadPoolTimer ^timer) 
        {
            if (pin != nullptr)
            {
                if (LEDStatus == 0)
                {
                    LEDStatus = 1;
					pin->Write(GpioPinValue::High);
                }
                else
                {
                    LEDStatus = 0;
					pin->Write(GpioPinValue::Low);
                }
            }
        }
    );
    TimeSpan interval;
    interval.Duration = 500 * 1000 * 10;
    Timer = ThreadPoolTimer::CreatePeriodicTimer(handler, interval);

    InitGpio();
}

void BackgroundTask::InitGpio()
{
	create_task(GpioController::GetDefaultAsync()).then([this](task<GpioController ^> controllerOp) {
		auto gpio = controllerOp.get();
		pin = gpio->OpenPin(LED_PIN);

		if (pin != nullptr)
		{
			pin->Write(GpioPinValue::High);
			pin->SetDriveMode(GpioPinDriveMode::Output);
		}
	});
}
