#pragma once

#include "pch.h"
#include <agile.h>

using namespace Windows::ApplicationModel::Background;
using namespace Windows::System::Threading;

namespace StartupTask
{
    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class BackgroundTask sealed : public IBackgroundTask
    {
    public:
		BackgroundTask();
        virtual void Run(IBackgroundTaskInstance^ taskInstance);

    private:
        void InitGpio();

    private:
        Platform::Agile<Windows::ApplicationModel::Background::BackgroundTaskDeferral> Deferral;
        IBackgroundTaskInstance^ TaskInstance;
        ThreadPoolTimer ^Timer;
        int LEDStatus = 0; 
        Windows::Devices::Gpio::GpioOutputPin ^outPin;
    };
}
