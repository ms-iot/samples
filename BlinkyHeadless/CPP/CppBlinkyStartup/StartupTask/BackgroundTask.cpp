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
            if (outPin != nullptr) 
            {
                if (LEDStatus == 0)
                {
                    LEDStatus = 1;
                    outPin->Value = GpioPinValue::High;
                }
                else
                {
                    LEDStatus = 0;
                    outPin->Value = GpioPinValue::Low;
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
    auto deviceId = GpioController::GetDeviceSelector("GPIO_S5");
    create_task(DeviceInformation::FindAllAsync(deviceId, nullptr)).then(
        [this](task<DeviceInformationCollection ^> collectionOp) 
        {
            try 
            {
                auto deviceInfos = collectionOp.get();
                auto firstDeviceId = deviceInfos->GetAt(0)->Id;
                create_task(GpioController::FromIdAsync(firstDeviceId)).then(
                    [this](task<GpioController^> controllerOp) 
                    {
                        try
                        {
                            auto controller = controllerOp.get();
                            auto pinInfo = controller->Pins->Lookup(0);
                            pinInfo->TryOpenOutput(GpioPinValue::Low, GpioSharingMode::Exclusive, &outPin);
                        }
                        catch (Exception ^)
                        {
                            // TODO (alecont): we need to marshal this back...
                        }
                    });
            }
            catch (Exception ^)
            {
                // TODO (alecont): we need to marshal this back...
            }
        });
}
