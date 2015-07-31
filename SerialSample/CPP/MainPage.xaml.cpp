// Copyright (c) Microsoft. All rights reserved.

//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace SerialSampleCpp;

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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
    InitializeComponent();

    comPortInput->IsEnabled = false;
    sendTextButton->IsEnabled = false;
    _availableDevices = ref new Platform::Collections::Vector<Platform::Object^>();
    
    ListAvailablePorts();
}

/// <summary>
/// Finds all serial devices available on the device and populates a ListBox with the Ids of each device.
/// </summary>
void MainPage::ListAvailablePorts(void)
{
    cancellationTokenSource = new Concurrency::cancellation_token_source();

    //using asynchronous operation, get a list of serial devices available on this device
    Concurrency::create_task(ListAvailableSerialDevicesAsync()).then([this](Windows::Devices::Enumeration::DeviceInformationCollection ^serialDeviceCollection)
    {
        Windows::Devices::Enumeration::DeviceInformationCollection ^_deviceCollection = serialDeviceCollection;

        // start with an empty list
        _availableDevices->Clear();

        status->Text = "Select a device and connect";

        for (auto &&device : _deviceCollection)
        {
            _availableDevices->Append(ref new Device(device->Id, device));
        }

        // this will populate the ListBox with our available device Ids.
        DeviceListSource->Source = AvailableDevices;

        comPortInput->IsEnabled = true;
        ConnectDevices->SelectedIndex = -1;
    });
}

/// <summary>
/// An asynchronous operation that returns a collection of DeviceInformation objects for all serial devices detected on the device.
/// </summary>
Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^MainPage::ListAvailableSerialDevicesAsync(void)
{
    // Construct AQS String for all serial devices on system
    Platform::String ^serialDevices_aqs = Windows::Devices::SerialCommunication::SerialDevice::GetDeviceSelector();

    // Identify all paired devices satisfying query
    return Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(serialDevices_aqs);
}

/// <summary>
/// Creates a task chain that attempts connect to a serial device asynchronously. 
/// </summary
Concurrency::task<void> MainPage::ConnectToSerialDeviceAsync(Windows::Devices::Enumeration::DeviceInformation ^device, Concurrency::cancellation_token cancellationToken)
{
    return Concurrency::create_task(Windows::Devices::SerialCommunication::SerialDevice::FromIdAsync(device->Id), cancellationToken)
        .then([this](Windows::Devices::SerialCommunication::SerialDevice ^serial_device)
    {
        try
        {
            _serialPort = serial_device;

            // Disable the 'Connect' button 
            comPortInput->IsEnabled = false;
            Windows::Foundation::TimeSpan _timeOut;
            _timeOut.Duration = 10000000L;

            // Configure serial settings
            _serialPort->WriteTimeout = _timeOut;
            _serialPort->ReadTimeout = _timeOut;
            _serialPort->BaudRate = 9600;     
            _serialPort->Parity = Windows::Devices::SerialCommunication::SerialParity::None;
            _serialPort->StopBits = Windows::Devices::SerialCommunication::SerialStopBitCount::One;
            _serialPort->DataBits = 8;
            _serialPort->Handshake = Windows::Devices::SerialCommunication::SerialHandshake::None;

            // Display configured settings
            status->Text = "Serial port configured successfully!\n ----- Properties ----- \n";
            status->Text += "BaudRate: " + _serialPort->BaudRate.ToString() + "\n";
            status->Text += "DataBits: " + _serialPort->DataBits.ToString() + "\n";
            status->Text += "Handshake: " + _serialPort->Handshake.ToString() + "\n";
            status->Text += "Parity: " + _serialPort->Parity.ToString() + "\n";
            status->Text += "StopBits: " + _serialPort->StopBits.ToString() + "\n";

            // setup our data reader for handling incoming data
            _dataReaderObject = ref new Windows::Storage::Streams::DataReader(_serialPort->InputStream);
            _dataReaderObject->InputStreamOptions = Windows::Storage::Streams::InputStreamOptions::Partial;

            // setup our data writer for handling outgoing data
            _dataWriterObject = ref new Windows::Storage::Streams::DataWriter(_serialPort->OutputStream);

            // Setting this text will trigger the event handler that runs asynchronously for reading data from the input stream
            rcvdText->Text = "Waiting for data...";

            sendTextButton->IsEnabled = true;
        }
        catch (Platform::Exception ^ex)
        {
            status->Text = "Error connecting to device!\nsendTextButton_Click: " + ex->Message;
            // perform any cleanup needed
            CloseDevice();
        }
    });
}

/// <summary>
/// Returns a task that sends the outgoing data from the sendText textbox to the output stream. 
/// </summary
Concurrency::task<void> MainPage::WriteAsync(Concurrency::cancellation_token cancellationToken)
{
    _dataWriterObject->WriteString(sendText->Text);

    return concurrency::create_task(_dataWriterObject->StoreAsync(), cancellationToken).then([this](unsigned int bytesWritten)
    {
        if (bytesWritten > 0)
        {
            status->Text = sendText->Text + "\n";
            status->Text += "Bytes written successfully!";
        }
        sendText->Text = "";
    });
}

/// <summary>
/// Returns a task that reads in the data from the input stream
/// </summary
Concurrency::task<void> MainPage::ReadAsync(Concurrency::cancellation_token cancellationToken)
{
    unsigned int _readBufferLength = 1024;

    return concurrency::create_task(_dataReaderObject->LoadAsync(_readBufferLength), cancellationToken).then([this](unsigned int bytesRead)
    {
        if (bytesRead > 0)
        {
            rcvdText->Text = _dataReaderObject->ReadString(bytesRead);
            status->Text = "\nBytes read successfully!";
        }
    });
}

/// <summary>
/// Initiates task cancellation
/// </summary
void MainPage::CancelReadTask(void)
{
    cancellationTokenSource->cancel();
}

/// <summary>
/// Closes the comport currently connected
/// </summary
void MainPage::CloseDevice(void)
{
    delete(_dataReaderObject);
    _dataReaderObject = nullptr;

    delete(_dataWriterObject);
    _dataWriterObject = nullptr;

    delete(_serialPort);
    _serialPort = nullptr;

    comPortInput->IsEnabled = true;
    sendTextButton->IsEnabled = false;
    rcvdText->Text = "";
}

/// <summary>
/// Event handler that is triggered when the user clicks on the "Connect" button. 
//  Attempts to connect to the serial device that the user selected.
/// </summary
void MainPage::comPortInput_Click(Object^ sender, RoutedEventArgs^ e)
{
    auto selectionIndex = ConnectDevices->SelectedIndex;

    if (selectionIndex < 0)
    {
        status->Text = L"Select a device and connect";
        return;
    }

    Device^ selectedDevice = static_cast<Device^>(_availableDevices->GetAt(selectionIndex));
    Windows::Devices::Enumeration::DeviceInformation ^entry = selectedDevice->DeviceInfo;

    concurrency::create_task(ConnectToSerialDeviceAsync(entry, cancellationTokenSource->get_token()));
}

/// <summary>
/// Event handler that is triggered when the user clicks the "WRITE" button.
/// Sends the characters located in the sendText TextBox.
/// </summary
void MainPage::sendTextButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    if (_serialPort != nullptr)
    {
        try
        {
            if (sendText->Text->Length() > 0)
            {
                WriteAsync(cancellationTokenSource->get_token());
            }
            else
            {
                status->Text = "Enter the text you want to write and then click on 'WRITE'";
            }
        }
        catch (Platform::Exception ^ex)
        {
            status->Text = "sendTextButton_Click: " + ex->Message;
        }
    }
    else
    {
        status->Text = "Select a device and connect";
    }
}

/// <summary>
/// Event handler that is triggered when text is written to the Read Data window
/// </summary
void MainPage::rcvdText_TextChanged(Object^ sender, TextChangedEventArgs^ e)
{
    try
    {
        if (_serialPort != nullptr)
        {
            concurrency::create_task(ReadAsync(cancellationTokenSource->get_token()));
        }
    }
    catch (Platform::Exception ^ex)
    {
        if (ex->GetType()->FullName == "TaskCanceledException")
        {
            status->Text = "Reading task was cancelled, closing device and cleaning up";
            CloseDevice();
        }
        else
        {
            status->Text = ex->Message;
        }
    }
}

/// <summary>
/// Event handler closing the currently connected serial device
/// </summary
void MainPage::closeDevice_Click(Object^ sender, RoutedEventArgs^ e)
{
    try
    {
        status->Text = "";
        CancelReadTask();
        CloseDevice();
        ListAvailablePorts();
    }
    catch (Platform::Exception ^ex)
    {
        status->Text = ex->Message;
    }
}

/// <summary>
/// Constructor for the Device class
/// </summary
Device::Device(Platform::String^ id, Windows::Devices::Enumeration::DeviceInformation^ deviceInfo)
{
    _id = id;
    _deviceInformation = deviceInfo;
}