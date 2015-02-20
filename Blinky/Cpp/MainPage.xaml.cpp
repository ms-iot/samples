 /********************************************************
*                                                        *
*   © Copyright (C) Microsoft. All rights reserved.      *
*                                                        *
*********************************************************/


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

    timer_ = ref new DispatcherTimer();
    TimeSpan interval;
    interval.Duration = 500 * 1000 * 10;
    timer_->Interval = interval;
    timer_->Tick += ref new EventHandler<Object ^>(this, &MainPage::OnTick);
    timer_->Start();
}

void MainPage::InitGPIO()
{
    create_task(GpioController::GetDefaultAsync()).then([this](task<GpioController ^> controllerOp) {
        auto gpio = controllerOp.get();

        if (gpio == nullptr)
        {
            pin_ = nullptr;
            GpioStatus->Text = "There is no GPIO controller on this device.";
            return;
        }

        pin_ = gpio->OpenPin(LED_PIN);

        if (pin_ == nullptr)
        {
            GpioStatus->Text = "There were problems initializing the GPIO pin.";
            return;
        }

        pin_->Write(GpioPinValue::High);
        pin_->SetDriveMode(GpioPinDriveMode::Output);

        GpioStatus->Text = "GPIO pin initialized correctly.";
    });
}

void MainPage::FlipLED()
{
    if (LEDStatus_ == 0)
    {
        LEDStatus_ = 1;
        if (pin_ != nullptr)
        {
            pin_->Write(GpioPinValue::High);
        }
        LED->Fill = redBrush_;
    }
    else
    {
        LEDStatus_ = 0;
        if (pin_ != nullptr)
        {
            pin_->Write(GpioPinValue::Low);
        }
        LED->Fill = grayBrush_;
    }
}

void MainPage::TurnOffLED()
{
    if (LEDStatus_ == 1)
    {
        FlipLED();
    }
}

void MainPage::OnTick(Object ^sender, Object ^args)
{
    FlipLED();
}


void MainPage::Delay_ValueChanged(Object^ sender, RangeBaseValueChangedEventArgs^ e)
{
    if (timer_ == nullptr)
    {
        return;
    }
    if (e->NewValue == Delay->Minimum)
    {
        DelayText->Text = "Stopped";
        timer_->Stop();
        TurnOffLED();
    }
    else
    {
        long delay = static_cast<long>(e->NewValue);
        auto txt = std::to_wstring(delay) + L"ms";
        DelayText->Text = ref new String(txt.c_str());
        TimeSpan interval;
        interval.Duration = delay * 1000 * 10;
        timer_->Interval = interval;
        timer_->Start();
    }

}
