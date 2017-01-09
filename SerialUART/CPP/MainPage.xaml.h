// Copyright (c) Microsoft. All rights reserved.

//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace SerialSampleCpp
{

    /// <summary>
    /// A wrapper class for holding DeviceInformation while being bindable to XAML. 
    /// </summary>
    [Windows::UI::Xaml::Data::Bindable] // in c++, adding this attribute to ref classes enables data binding for more info search for 'Bindable' on the page http://go.microsoft.com/fwlink/?LinkId=254639 
    public ref class Device sealed
    {
    public:
        Device(Platform::String^ id, Windows::Devices::Enumeration::DeviceInformation^ deviceInfo);

        property Platform::String^ Id
        {
            Platform::String^ get()
            {
                return _id;
            }
        }
        property Windows::Devices::Enumeration::DeviceInformation^ DeviceInfo
        {
            Windows::Devices::Enumeration::DeviceInformation^ get()
            {
                return _deviceInformation;
            }
        }

    private:
        Platform::String^ _id;
        Windows::Devices::Enumeration::DeviceInformation^ _deviceInformation;
    };

    /// <summary>
    /// The MainPage class
    /// </summary>
    public ref class MainPage sealed
    {
    public:
        static Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^ListAvailableSerialDevicesAsync(void);

        // For XAML binding purposes, use the IObservableVector interface containing Object^ objects. 
        // This wraps the real implementation of _availableDevices which is implemented as a Vector.
        // See "Data Binding Overview (XAML)" https://msdn.microsoft.com/en-us/library/windows/apps/xaml/hh758320.aspx
        property Windows::Foundation::Collections::IObservableVector<Platform::Object^>^ AvailableDevices
        {
            Windows::Foundation::Collections::IObservableVector<Platform::Object^>^ get()
            {
                return _availableDevices;
            }
        }
        MainPage();
    private:
        Platform::Collections::Vector<Platform::Object^>^ _availableDevices;
        Windows::Devices::SerialCommunication::SerialDevice ^_serialPort;
        Windows::Storage::Streams::DataWriter^ _dataWriterObject;
        Windows::Storage::Streams::DataReader^ _dataReaderObject;
        Concurrency::cancellation_token_source* cancellationTokenSource;

        // event handlers
        void comPortInput_Click(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void sendTextButton_Click(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void closeDevice_Click(Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void Listen();

        void ListAvailablePorts(void);
        void CancelReadTask(void);
        void CloseDevice(void);
        Concurrency::task<void> WriteAsync(Concurrency::cancellation_token cancellationToken);
        Concurrency::task<void> ReadAsync(Concurrency::cancellation_token cancellationToken);
        Concurrency::task<void> ConnectToSerialDeviceAsync(Windows::Devices::Enumeration::DeviceInformation ^device, Concurrency::cancellation_token cancellationToken);
    };
}
