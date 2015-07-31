#pragma once

#include "pch.h"

namespace ServoMotorBasicsCpp
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class StartupTask sealed : public Windows::ApplicationModel::Background::IBackgroundTask
	{
	public:
		virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance);
	private:
		Platform::Agile<Windows::ApplicationModel::Background::BackgroundTaskDeferral> Deferral;
		Windows::Devices::Gpio::GpioPin ^MotorPin;
		int64 TicksASecond;
		double currentPulseLength;
		int MOTOR_PIN = 13;
		double CLOCKWISE_PULSE_LENGTH = 1;
		double COUNTER_CLOCKWISE_PULSE_LENGTH = 2;
		double RESTING_PULSE_LENGTH = 0;
		double PULSE_FREQUENCY = 20;
		void Wait(double milliseconds);
		Windows::System::Threading::ThreadPoolTimer ^Timer;

	};
}
