// Copyright (c) Microsoft. All rights reserved.
//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace GpioOneWire
{
    struct DhtSensorReading {
        bool IsValid ( ) const
        {
            unsigned long long value = this->bits.to_ullong();
            unsigned int checksum =
                ((value >> 32) & 0xff) +
                ((value >> 24) & 0xff) +
                ((value >> 16) & 0xff) +
                ((value >> 8) & 0xff);

            return (checksum & 0xff) == (value & 0xff);
        }

        double Humidity ( ) const
        {
            unsigned long long value = this->bits.to_ullong();
            return ((value >> 24) & 0xffff) * 0.1;
        }

        double Temperature ( ) const
        {
            unsigned long long value = this->bits.to_ullong();
            double temp = ((value >> 8)  & 0x7FFF) * 0.1;
            if ((value >> 8) & 0x8000)
                temp = -temp;
            return temp;
        }

        std::bitset<40> bits;
    };

    class DhtSensor
    {
        enum { SAMPLE_HOLD_LOW_MILLIS = 18 };

    public:

        DhtSensor ( ) :
            inputDriveMode(Windows::Devices::Gpio::GpioPinDriveMode::Input)
        { }

        void Init (
            Windows::Devices::Gpio::GpioPin^ InputPin,
            Windows::Devices::Gpio::GpioPin^ OnputPin
            );

        HRESULT SampleInterrupts (_Out_ DhtSensorReading& Reading);
        HRESULT SamplePolling (_Out_ DhtSensorReading& Reading);

        bool PullResistorRequired ( ) const
        {
            return inputDriveMode != Windows::Devices::Gpio::GpioPinDriveMode::InputPullUp;
        }

    private:

        HRESULT SendInitialPulse ();

        Windows::Devices::Gpio::GpioPin^ inputPin;
        Windows::Devices::Gpio::GpioChangeReader^ changeReader;
        Windows::Devices::Gpio::GpioPin^ outputPin;
        Windows::Devices::Gpio::GpioPinDriveMode inputDriveMode;
    };

    /// <summary>
    /// The main page of the application - used to show samples from the DHT22.
    /// </summary>
    public ref class MainPage sealed
    {
        enum {
            DHT_INPUT_PIN_NUMBER = 4,
            DHT_OUTPUT_PIN_NUMBER = 5,
        };

    public:
        MainPage();

    private:
        void Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void timerElapsed (Windows::System::Threading::ThreadPoolTimer^ Timer);
        void radioButton_Checked (Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        Windows::Devices::Gpio::GpioPin^ OpenPin (
            Windows::Devices::Gpio::GpioController^ Controller,
            int PinNumber
            );

        enum class Mode { Interrupts, Polling, Paused } mode, previousMode;
        struct _Stats {
            int TotalSampleCount;
            int SuccessfulSampleCount;
            int PulseTimingErrors;
            int TimeoutErrors;
            int ChecksumErrors;
        } stats;

        DhtSensor dhtSensor;
        Windows::System::Threading::ThreadPoolTimer^ timer;

    };
}
