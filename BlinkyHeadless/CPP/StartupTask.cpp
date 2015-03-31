#include "pch.h"
#include "StartupTask.h"

using namespace BlinkyHeadlessCpp;

using namespace Platform;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Foundation;
using namespace Windows::Devices::Gpio;
using namespace Windows::System::Threading;
using namespace concurrency;

StartupTask::StartupTask()
{
}

void StartupTask::Run(IBackgroundTaskInstance^ taskInstance)
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
	});

	TimeSpan interval;
	interval.Duration = 500 * 1000 * 10;
	Timer = ThreadPoolTimer::CreatePeriodicTimer(handler, interval);

	InitGpio();
}

void StartupTask::InitGpio()
{

	auto gpio = GpioController::GetDefault();

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
}

