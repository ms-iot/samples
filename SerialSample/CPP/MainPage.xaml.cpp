//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <ppltasks.h>

using namespace Cpp_Serial;

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

    _listOfDevices = ref new Platform::Collections::Vector<Windows::Devices::Enumeration::DeviceInformation ^>();
    _devicesVector = ref new Platform::Collections::Vector<Windows::Devices::Enumeration::DeviceInformation^>();
//    _items = ref new Platform::Collections::Vector<Windows::Devices::Enumeration::DeviceInformation^>();
    _items = ref new Platform::Collections::Vector<Object^>();
    
    comPortInput->IsEnabled = false;
    sendTextButton->IsEnabled = false;
    ListAvailablePorts();
//    TestGetDevices();
}

/// <summary>
/// ListAvailablePorts
/// </summary>
void MainPage::ListAvailablePorts(void)
{
    //using asynchronous operation, get a list of serial devices available on this device
    Concurrency::create_task(ListAvailableSerialDevicesAsync()).then([this](Windows::Devices::Enumeration::DeviceInformationCollection ^serialDeviceCollection)
    {
        Windows::Devices::Enumeration::DeviceInformationCollection ^_deviceCollection = serialDeviceCollection;
        status->Text = "Select a device and connect";

        // start with an empty list
        _listOfDevices->Clear();
        _devicesVector->Clear();

        Platform::String ^test1;
        Windows::Devices::Enumeration::DeviceInformation ^test2;
        Windows::Devices::Enumeration::DeviceInformation ^deviceData;

        //for (auto &&device : _deviceCollection)
        //{
        //    test1 = device->Id;
        //    test2 = device;

        //    _devicesVector->Append(device);
//            _listOfDevices->Append(device);
        //}

        auto deviceCount = _deviceCollection->Size;
        for (int i=0; i<deviceCount; i++)
        {
            deviceData = _deviceCollection->GetAt(i);
            _items->Append(deviceData);
            _listOfDevices->Append(deviceData);
            _devicesVector->Append(deviceData);
        }

//        DeviceListSource->Source = _listOfDevices;

//        DeviceListSource->Source = _devicesVector;
        DeviceListSource->Source = Items;
//        DeviceListSource->Source = DevicesIVector;
        

        comPortInput->IsEnabled = true;
        ConnectDevices->SelectedIndex = -1;


    });
}

/// <summary>
/// ListAvailableSerialDevicesAsync
/// An asynchronous operation that returns a collection of DeviceInformation for all serial devices detected on 
/// the device.
/// - Uses SerialDevice.GetDeviceSelector to enumerate all serial devices on the device
/// </summary>
Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^MainPage::ListAvailableSerialDevicesAsync(void)
{
    // Construct AQS String for all serial devices on system
    Platform::String ^serialDevices_aqs = Windows::Devices::SerialCommunication::SerialDevice::GetDeviceSelector();

    // Identify all paired devices satisfying query
    return Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(serialDevices_aqs);
}

void MainPage::comPortInput_Click(Object^ sender, RoutedEventArgs^ e)
{
    auto selection = ConnectDevices->SelectedItems;

    if (selection->Size < 1)
    {
        status->Text = L"Select a device and connect";
        return;
    }

    Windows::Devices::Enumeration::DeviceInformation ^entry = static_cast<Windows::Devices::Enumeration::DeviceInformation^>(selection->GetAt(0));

    concurrency::create_task(ConnectToSerialDeviceAsync(entry));
    //.then([this](Concurrency::task<void> t)
    //{
    //    try
    //    {
    //        t.get();
    //    }
    //    catch (Platform::Exception ^e)
    //    {
    //        status->Text = L"UsbSerial::connectToDeviceAsync failed with a Platform::Exception type. (message: " + e->Message + L")";
    //    }
    //    catch (...)
    //    {
    //        status->Text = L"UsbSerial::connectToDeviceAsync failed with a non-Platform::Exception type. (vid: " + _vid + L" pid: " + _pid + L")";
    //    }
    //});
}

Concurrency::task<void> MainPage::ConnectToSerialDeviceAsync(Windows::Devices::Enumeration::DeviceInformation ^device)
{
    return Concurrency::create_task(Windows::Devices::SerialCommunication::SerialDevice::FromIdAsync(device->Id))
        .then([this](Windows::Devices::SerialCommunication::SerialDevice ^serial_device)
    {        
        _serialPort = serial_device;

        // Disable the 'Connect' button 
        comPortInput->IsEnabled = false;
        Windows::Foundation::TimeSpan _timeOut;// = ref new Windows::Foundation::TimeSpan();
        _timeOut.Duration = 10000000L;

        // Configure serial settings
        _serialPort->WriteTimeout = _timeOut;
        _serialPort->ReadTimeout = _timeOut;
        _serialPort->BaudRate = 9600;
//        _serialPort->BaudRate = _baud;
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

        // The callback launches an async Read task to wait for data
        rcvdText->Text = "Waiting for data...";

        //cancellationTokenSource

        sendTextButton->IsEnabled = true;

        // Enable RX
        _dataReaderObject = ref new Windows::Storage::Streams::DataReader(_serialPort->InputStream);
        _dataReaderObject->InputStreamOptions = Windows::Storage::Streams::InputStreamOptions::Partial;

        //_rx = ref new Windows::Storage::Streams::DataReader(_serial_device->InputStream);
        //_rx->InputStreamOptions = Windows::Storage::Streams::InputStreamOptions::Partial;  // Partial mode will allow for better async reads
        //_current_load_operation = _rx->LoadAsync(100);

        // Enable TX
        _dataWriteObject = ref new Windows::Storage::Streams::DataWriter(_serialPort->OutputStream);
        //_tx = ref new Windows::Storage::Streams::DataWriter(_serial_device->OutputStream);
        //_current_store_operation = nullptr;

        // Set connection ready flag
        //_connection_ready = true;
        //ConnectionEstablished();
    });
}

void MainPage::sendTextButton_Click(Object^ sender, RoutedEventArgs^ e)
{
    try
    {
        if (_serialPort != nullptr)
        {
            if (sendText->Text->Length() > 0)
            {
//                _dataWriteObject = ref new Windows::Storage::Streams::DataWriter(_serialPort->OutputStream);
                WriteAsync();
            }
            else
            {
                status->Text = "Enter the text you want to write and then click on 'WRITE'";
            }
        }
        else
        {
            status->Text = "Select a device and connect";
        }
    }
    catch (Platform::Exception ^ex)
    {
        status->Text = "sendTextButton_Click: " + ex->Message;
    }

    //if (_dataWriteObject != nullptr)
    //{
    //    _dataWriteObject->DetachStream();
    //    _dataWriteObject = nullptr;
    //}
}

Concurrency::task<void> MainPage::WriteAsync(void)
{
    _dataWriteObject->WriteString(sendText->Text);

    return concurrency::create_task(_dataWriteObject->StoreAsync()).then([this](unsigned int bytesWritten)
    {
        if (bytesWritten > 0)
        {
            status->Text = sendText->Text + "\n";
            status->Text += "Bytes written successfully!";
        }
        sendText->Text = "";

        //return _dataWriteObject->FlushAsync();
    });
}

void MainPage::rcvdText_TextChanged(Object^ sender, TextChangedEventArgs^ e)
{
    try
    {
        if (_serialPort != nullptr)
        {
//            _dataReaderObject = ref new Windows::Storage::Streams::DataReader(_serialPort->InputStream);
            concurrency::create_task(ReadAsync(cancellationTokenSource));
//            ReadAsync(cancellationTokenSource);
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

    // Cleanup once complete
    //if (_dataReaderObject != nullptr)
    //{
    //    _dataReaderObject->DetachStream();
    //    _dataReaderObject = nullptr;
    //}
}

Concurrency::task<void> MainPage::ReadAsync(concurrency::cancellation_token_source cts)
{
//    _dataReaderObject->InputStreamOptions = Windows::Storage::Streams::InputStreamOptions::Partial;
    unsigned int _readBufferLength = 1024;

    return concurrency::create_task(_dataReaderObject->LoadAsync(_readBufferLength), cts.get_token()).then([this](unsigned int bytesRead)
    {
        if (bytesRead > 0)
        {
            rcvdText->Text = _dataReaderObject->ReadString(bytesRead);
            status->Text = "\nBytes read successfully!";
        }
    });
}

void MainPage::CancelReadTask(void)
{    
    cancellationTokenSource.cancel();
}

void MainPage::CloseDevice(void)
{
    //if (_serialPort != nullptr)
    //{
    //    _serialPort->Dispose;//Dispose();
    //}
    delete(_serialPort);
    _serialPort = nullptr;

    comPortInput->IsEnabled = true;
    sendTextButton->IsEnabled = false;
    rcvdText->Text = "";
    _listOfDevices->Clear();
}

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



//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
void MainPage::TestGetDevices(void)
{
    Concurrency::create_task(listAvailableDevicesAsync()).then([this](Windows::Devices::Enumeration::DeviceInformationCollection ^device_collection_)
    {
        // If a friendly name was specified, then identify the associated device
        if (_vid) {
            // Store parameter as a member to ensure the duration of object allocation
            _device_collection = device_collection_;
            if (!_device_collection->Size)
            {
                throw ref new Platform::Exception(E_UNEXPECTED, L"No USB devices found.");
            }

            _device = identifyDeviceFromCollection(_device_collection);
        }


        if (!_device) {
            throw ref new Platform::Exception(E_UNEXPECTED, L"ERROR! Hacking too much time!");
        }

        return connectToDeviceAsync(_device);
    }).then([this](Concurrency::task<void> t)
    {
        try
        {
            t.get();
        }
        catch (Platform::Exception ^e)
        {
            _errorMessage = L"UsbSerial::connectToDeviceAsync failed with a Platform::Exception type. (message: " + e->Message + L")";
        }
        catch (...)
        {
//            _errorMessage = L"UsbSerial::connectToDeviceAsync failed with a non-Platform::Exception type. (vid: " + _vid + L" pid: " + _pid + L")";
            _errorMessage = L"UsbSerial::connectToDeviceAsync failed with a non-Platform::Exception type.";

        }
    });
}


//-------------------------------------------
// listAvailableDevicesAsync
Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^MainPage::listAvailableDevicesAsync(void)
{
    // Construct AQS String from service id of desired device
    Platform::String ^device_aqs = Windows::Devices::SerialCommunication::SerialDevice::GetDeviceSelector();

    // Identify all paired devices satisfying query
    return Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(device_aqs);
}

// connectToDeviceAsync
Concurrency::task<void> MainPage::connectToDeviceAsync(Windows::Devices::Enumeration::DeviceInformation ^device_)
{
    return Concurrency::create_task(Windows::Devices::SerialCommunication::SerialDevice::FromIdAsync(device_->Id))
        .then([this](Windows::Devices::SerialCommunication::SerialDevice ^serial_device_)
    {
        // Store parameter as a member to ensure the duration of object allocation
        _serial_device = serial_device_;

        // Configure the device properties
        _serial_device->Handshake = Windows::Devices::SerialCommunication::SerialHandshake::None;
        _serial_device->BaudRate = _baud;

//        case SerialConfig::SERIAL_8N1:
            _serial_device->DataBits = 8;
            _serial_device->Parity = Windows::Devices::SerialCommunication::SerialParity::None;
            _serial_device->StopBits = Windows::Devices::SerialCommunication::SerialStopBitCount::One;

        // Enable RX
        //_rx = ref new Windows::Storage::Streams::DataReader(_serial_device->InputStream);
        //_rx->InputStreamOptions = Windows::Storage::Streams::InputStreamOptions::Partial;  // Partial mode will allow for better async reads
        //_current_load_operation = _rx->LoadAsync(100);

        // Enable TX
        //_tx = ref new Windows::Storage::Streams::DataWriter(_serial_device->OutputStream);
        //_current_store_operation = nullptr;

        // Set connection ready flag
        //_connection_ready = true;
        //ConnectionEstablished();
    });
}

Windows::Devices::Enumeration::DeviceInformation ^MainPage::identifyDeviceFromCollection( Windows::Devices::Enumeration::DeviceInformationCollection ^devices_ )
{
    for (auto &&device : devices_)
    {
        // If the vid doesn't match, move to the next device.
        if (std::string::npos == std::wstring(device->Id->Data()).find(_vid->Data()))
        {
            continue;
        }

        // If the pid doesn't match, move to the next device.
        if (_pid && std::string::npos == std::wstring(device->Id->Data()).find(_pid->Data()))
        {
            continue;
        }

        // If the supplied values match, we've identified the device!
        return device;
    }

    // If we searched and found nothing that matches the identifier, we've failed to connect and cannot recover.
    throw ref new Platform::Exception(E_INVALIDARG, L"No USB devices found matching the specified identifier.");
}
