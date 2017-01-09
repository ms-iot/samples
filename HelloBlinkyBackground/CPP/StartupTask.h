/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

    The MIT License(MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/
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
		Windows::Devices::Gpio::GpioPinValue pinValue;
		const int LED_PIN = 5;
		Windows::Devices::Gpio::GpioPin ^pin;
    };
}
