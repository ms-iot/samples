#pragma once

#include "pch.h"

namespace BlinkyHeadlessCpp
{
	[Windows::Foundation::Metadata::WebHostHidden]
    public ref class StartupTask sealed : public Windows::ApplicationModel::Background::IBackgroundTask
    {
    public:
		StartupTask();
        virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance);

	private:
		void InitGpio();
	private:
		Platform::Agile<Windows::ApplicationModel::Background::BackgroundTaskDeferral> Deferral;
		Windows::ApplicationModel::Background::IBackgroundTaskInstance^ TaskInstance;
		Windows::System::Threading::ThreadPoolTimer ^Timer;
		int LEDStatus = 0;
		const int LED_PIN = 5;
		Windows::Devices::Gpio::GpioPin ^pin;
    };
}
