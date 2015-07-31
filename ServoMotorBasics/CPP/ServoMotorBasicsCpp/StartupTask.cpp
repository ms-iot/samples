#include "pch.h"
#include "StartupTask.h"
#include <Windows.h>
using namespace ServoMotorBasicsCpp;

using namespace Platform;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Devices::Gpio;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;


void StartupTask::Run(IBackgroundTaskInstance^ taskInstance)
{
	Deferral = taskInstance->GetDeferral();

	LARGE_INTEGER ticks;
	QueryPerformanceFrequency(&ticks);
	TicksASecond = ticks.QuadPart;

	GpioController^ controller = GpioController::GetDefault();

	MotorPin = controller->OpenPin(MOTOR_PIN);
	MotorPin->SetDriveMode(GpioPinDriveMode::Output);

	TimerElapsedHandler ^handler = ref new TimerElapsedHandler(
		[this](ThreadPoolTimer ^timer)
	{
		if (currentPulseLength == RESTING_PULSE_LENGTH)
		{
			currentPulseLength = CLOCKWISE_PULSE_LENGTH;
		}
		else if (currentPulseLength == CLOCKWISE_PULSE_LENGTH) {
			currentPulseLength = COUNTER_CLOCKWISE_PULSE_LENGTH;
		}
		else {
			currentPulseLength = RESTING_PULSE_LENGTH;
		}
	});

	TimeSpan interval;
	//Change motor direction every 2 seconds
	interval.Duration = 2000 * 1000 * 10;
	Timer = ThreadPoolTimer::CreatePeriodicTimer(handler, interval);

	while (true) {
		if (currentPulseLength != 0) {
			MotorPin->Write(GpioPinValue::High);
			Wait(currentPulseLength);
			MotorPin->Write(GpioPinValue::Low);
		}
		Wait(PULSE_FREQUENCY - currentPulseLength);
	}
}


void StartupTask::Wait(double milliseconds) {
	LARGE_INTEGER currentTicks;
	QueryPerformanceCounter(&currentTicks);
	double targetTicks = currentTicks.QuadPart + milliseconds*TicksASecond / 1000.0;
	while (currentTicks.QuadPart < targetTicks)
	{
		QueryPerformanceCounter(&currentTicks);
	}

}