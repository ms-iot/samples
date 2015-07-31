// Copyright (c) Microsoft. All rights reserved.

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




