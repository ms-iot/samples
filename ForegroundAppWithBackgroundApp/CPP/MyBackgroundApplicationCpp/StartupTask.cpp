#include "pch.h"
#include "StartupTask.h"

using namespace MyBackgroundApplicationCpp;

using namespace Platform;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Foundation;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

const int LED_PIN = 5;
const int ticksPerMillisecond = 10000;

void StartupTask::Run(IBackgroundTaskInstance^ taskInstance)
{
    deferral = taskInstance->GetDeferral();
    InitGPIO();
    if (pin != nullptr)
    {
        TimeSpan ts;
        ts.Duration = 500 * ticksPerMillisecond;
        timer = ThreadPoolTimer::CreatePeriodicTimer(ref new TimerElapsedHandler(this, &StartupTask::Timer_Tick), ts);
    }
}

void StartupTask::Timer_Tick(ThreadPoolTimer^ timer)
{
    if (pinValue == GpioPinValue::High)
    {
        pinValue = GpioPinValue::Low;
        pin->Write(pinValue);
    }
    else
    {
        pinValue = GpioPinValue::High;
        pin->Write(pinValue);
    }
}


void StartupTask::InitGPIO()
{
    auto gpio = GpioController::GetDefault();

    // Show an error if there is no GPIO controller
    if (gpio == nullptr)
    {
        pin = nullptr;
        return;
    }

    pin = gpio->OpenPin(LED_PIN);
    pinValue = GpioPinValue::High;
    pin->Write(pinValue);
    pin->SetDriveMode(GpioPinDriveMode::Output);
}