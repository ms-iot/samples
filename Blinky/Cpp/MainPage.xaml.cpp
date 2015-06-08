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

//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <string>

using namespace BlinkyCpp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Gpio;
using namespace concurrency;

MainPage::MainPage()
{
    InitializeComponent();

    InitGPIO();
	if (pin_ != nullptr)
	{
		timer_ = ref new DispatcherTimer();
		TimeSpan interval;
		interval.Duration = 500 * 1000 * 10;
		timer_->Interval = interval;
		timer_->Tick += ref new EventHandler<Object ^>(this, &MainPage::OnTick);
		timer_->Start();
	}
}

void MainPage::InitGPIO()
{
	auto gpio = GpioController::GetDefault();

	if (gpio == nullptr)
	{
		pin_ = nullptr;
		GpioStatus->Text = "There is no GPIO controller on this device.";
		return;
	}

	pin_ = gpio->OpenPin(LED_PIN);
	pin_->Write(pinValue_);
	pin_->SetDriveMode(GpioPinDriveMode::Output);

	GpioStatus->Text = "GPIO pin initialized correctly.";
}


void MainPage::OnTick(Object ^sender, Object ^args)
{
	if (pinValue_ == GpioPinValue::High)
	{
		pinValue_ = GpioPinValue::Low;
		pin_->Write(pinValue_);
		LED->Fill = redBrush_;
	}
	else
	{
		pinValue_ = GpioPinValue::High;
		pin_->Write(pinValue_);
		LED->Fill = grayBrush_;
	}
}




