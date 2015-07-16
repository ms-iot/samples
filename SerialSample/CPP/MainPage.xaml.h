//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace Cpp_Serial
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();
        void ListAvailablePorts(void);
        
        static Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^ListAvailableSerialDevicesAsync(void);

        property Windows::Foundation::Collections::IVector<Windows::Devices::Enumeration::DeviceInformation^>^ DevicesIVector
        {
            Windows::Foundation::Collections::IVector<Windows::Devices::Enumeration::DeviceInformation^>^ get() { return _devicesVector; }
        }

        property Windows::Foundation::Collections::IObservableVector<Platform::Object^>^ Items
        {
            Windows::Foundation::Collections::IObservableVector<Platform::Object^>^ get()
            {
                return _items;
            }
        }

        //property Windows::Foundation::Collections::IObservableVector<Windows::Devices::Enumeration::DeviceInformation^>^ Items
        //{
        //    Windows::Foundation::Collections::IObservableVector<Windows::Devices::Enumeration::DeviceInformation^>^ get()
        //    {
        //        return _items;
        //    }
        //}

        //--------------------------------------------------
        void TestGetDevices(void);        
        static Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^listAvailableDevicesAsync(void);
        
    private:
        Platform::Collections::Vector<Windows::Devices::Enumeration::DeviceInformation^>^ _devicesVector;
//        Platform::Collections::Vector<Windows::Devices::Enumeration::DeviceInformation^>^ _items;
        Platform::Collections::Vector<Platform::Object^>^ _items;

        Windows::Devices::SerialCommunication::SerialDevice ^_serialPort;
        Windows::Foundation::Collections::IVector<Windows::Devices::Enumeration::DeviceInformation ^> ^_listOfDevices;
        Concurrency::cancellation_token_source cancellationTokenSource;
        Windows::Storage::Streams::DataWriter^ _dataWriteObject;
        Windows::Storage::Streams::DataReader^ _dataReaderObject;

        void CancelReadTask(void);
        void CloseDevice(void);
                
        void comPortInput_Click(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void sendTextButton_Click(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void closeDevice_Click(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void rcvdText_TextChanged(Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);

        Concurrency::task<void> WriteAsync(void);
        Concurrency::task<void> ReadAsync(concurrency::cancellation_token_source cancellationTokenSource);
        Concurrency::task<void> ConnectToSerialDeviceAsync(Windows::Devices::Enumeration::DeviceInformation ^device);


        //----------------
        uint32_t _baud = 9600;
        Platform::String ^_errorMessage;
        Windows::Devices::Enumeration::DeviceInformation ^_device;
        Windows::Devices::Enumeration::DeviceInformationCollection ^_device_collection;
        Windows::Devices::SerialCommunication::SerialDevice ^_serial_device;

        // USB\VID_067B&PID_2303&REV_0300
        Platform::String ^_pid = "2303";
        Platform::String ^_vid = "067B";

        Concurrency::task<void> connectToDeviceAsync(Windows::Devices::Enumeration::DeviceInformation ^device_);
        Windows::Devices::Enumeration::DeviceInformation ^identifyDeviceFromCollection(Windows::Devices::Enumeration::DeviceInformationCollection ^devices_);

	};
}
