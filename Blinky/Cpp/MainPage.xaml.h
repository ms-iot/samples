 /********************************************************
*                                                        *
*   © Copyright (C) Microsoft. All rights reserved.      *
*                                                        *
*********************************************************/


//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace BlinkyCpp
{
    public ref class MainPage sealed
    {
    public:
        MainPage();

    private:
        void InitGPIO();
        void FlipLED();
        void TurnOffLED();
        void OnTick(Platform::Object ^sender, Platform::Object ^args);
        void Delay_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e);

        Windows::UI::Xaml::DispatcherTimer ^timer_;
        int LEDStatus_ = 0;
        const int LED_PIN = 0;
        Windows::Devices::Gpio::GpioPin ^pin_;
        Windows::UI::Xaml::Media::SolidColorBrush ^redBrush_ = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::Red);
        Windows::UI::Xaml::Media::SolidColorBrush ^grayBrush_ = ref new Windows::UI::Xaml::Media::SolidColorBrush(Windows::UI::Colors::LightGray);
    };
}
