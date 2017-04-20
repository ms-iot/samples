#pragma once

#include "pch.h"

namespace MyBackgroundApplicationCpp
{
    using namespace Windows::System::Threading;
    using namespace Windows::Devices::Gpio;
    using namespace Windows::ApplicationModel::Background;

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class StartupTask sealed : public Windows::ApplicationModel::Background::IBackgroundTask
    {
    public:
        virtual void Run(Windows::ApplicationModel::Background::IBackgroundTaskInstance^ taskInstance);
        void Timer_Tick(ThreadPoolTimer^ timer);
        void InitGPIO();

    private:

        Platform::Agile<BackgroundTaskDeferral> deferral;
        ThreadPoolTimer^ timer;
        GpioPin^ pin;
        GpioPinValue pinValue;
    };
}
