//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace GpioOneWire
{
    struct Dht11Reading {
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
            return ((value >> 32) & 0xff) + ((value >> 24) & 0xff) / 10.0;
        }
    
        double Temperature ( ) const
        {
            unsigned long long value = this->bits.to_ullong();
            return ((value >> 16) & 0xff) + ((value >> 8) & 0xff) / 10.0;
        }

        std::bitset<40> bits;
    };

    class Dht11
    {
    public:
        
        Dht11 ( ) : 
            pin(nullptr),
            inputDriveMode(Windows::Devices::Gpio::GpioPinDriveMode::Input)
        { }

        void Init (Windows::Devices::Gpio::GpioPin^ Pin);

        HRESULT Sample (_Out_ Dht11Reading& Reading);

        bool PullResistorRequired ( ) const
        {
            return inputDriveMode != Windows::Devices::Gpio::GpioPinDriveMode::InputPullUp;
        }

    private:
        Windows::Devices::Gpio::GpioPin^ pin;
        Windows::Devices::Gpio::GpioPinDriveMode inputDriveMode;
    };

	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

    private:
        void Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void timerElapsed (Windows::System::Threading::ThreadPoolTimer^ Timer);

        Dht11 dht11;
        Windows::System::Threading::ThreadPoolTimer^ timer;
    };
}
