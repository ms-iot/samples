// Copyright (c) Microsoft. All rights reserved.
//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace GpioOneWire;

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
using namespace Windows::UI::Core;
using namespace Windows::System::Threading;
using namespace Windows::Devices::Gpio;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

void GpioOneWire::DhtSensor::Init (GpioPin^ Pin)
{
    // Use InputPullUp if supported, otherwise fall back to Input (floating)
    this->inputDriveMode =
        Pin->IsDriveModeSupported(GpioPinDriveMode::InputPullUp) ?
        GpioPinDriveMode::InputPullUp : GpioPinDriveMode::Input;

    Pin->SetDriveMode(this->inputDriveMode);
    this->pin = Pin;
}

_Use_decl_annotations_
HRESULT GpioOneWire::DhtSensor::Sample (GpioOneWire::DhtSensorReading& Reading)
{
    Reading = DhtSensorReading();

    LARGE_INTEGER qpf;
    QueryPerformanceFrequency(&qpf);

    // This is the threshold used to determine whether a bit is a '0' or a '1'.
    // A '0' has a pulse time of 76 microseconds, while a '1' has a
    // pulse time of 120 microseconds. 110 is chosen as a reasonable threshold.
    // We convert the value to QPF units for later use.
    const unsigned int oneThreshold = static_cast<unsigned int>(
        110LL * qpf.QuadPart / 1000000LL);

    // Latch low value onto pin
    this->pin->Write(GpioPinValue::Low);

    // Set pin as output
    this->pin->SetDriveMode(GpioPinDriveMode::Output);

    // Wait for at least 18 ms
    Sleep(SAMPLE_HOLD_LOW_MILLIS);

    // Set pin back to input
    this->pin->SetDriveMode(this->inputDriveMode);

    GpioPinValue previousValue = this->pin->Read();

    // catch the first rising edge
    const ULONG initialRisingEdgeTimeoutMillis = 1;
    ULONGLONG endTickCount = GetTickCount64() + initialRisingEdgeTimeoutMillis;
    for (;;) {
        if (GetTickCount64() > endTickCount) {
            return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
        }

        GpioPinValue value = this->pin->Read();
        if (value != previousValue) {
            // rising edgue?
            if (value == GpioPinValue::High) {
                break;
            }
            previousValue = value;
        }
    }

    LARGE_INTEGER prevTime = { 0 };

    const ULONG sampleTimeoutMillis = 10;
    endTickCount = GetTickCount64() + sampleTimeoutMillis;

    // capture every falling edge until all bits are received or
    // timeout occurs
    for (unsigned int i = 0; i < (Reading.bits.size() + 1);) {
        if (GetTickCount64() > endTickCount) {
            return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
        }

        GpioPinValue value = this->pin->Read();
        if ((previousValue == GpioPinValue::High) && (value == GpioPinValue::Low)) {
            // A falling edge was detected
            LARGE_INTEGER now;
            QueryPerformanceCounter(&now);

            if (i != 0) {
                unsigned int difference = static_cast<unsigned int>(
                    now.QuadPart - prevTime.QuadPart);
                Reading.bits[Reading.bits.size() - i] =
                    difference > oneThreshold;
            }

            prevTime = now;
            ++i;
        }

        previousValue = value;
    }

    if (!Reading.IsValid()) {
        // checksum mismatch
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    return S_OK;
}

MainPage::MainPage()
{
    InitializeComponent();
}

void GpioOneWire::MainPage::Page_Loaded(
    Platform::Object^ sender,
    Windows::UI::Xaml::RoutedEventArgs^ e
    )
{
    GpioController^ controller = GpioController::GetDefault();
    if (!controller) {
        this->statusText->Text = L"GPIO is not available on this system";
        return;
    }

    GpioPin^ pin;
    try {
        pin = controller->OpenPin(DHT_PIN_NUMBER);
    } catch (Exception^ ex) {
        this->statusText->Text = L"Failed to open GPIO pin: " + ex->Message;
        return;
    }

    this->dhtSensor.Init(pin);
    this->pullResistorText->Text = this->dhtSensor.PullResistorRequired() ?
        L"10k pull-up resistor required." : L"Pull-up resistor not required.";

    // Create a periodic timer to sample from the DHT11 every 2 seconds
    TimeSpan period = { 2 * 10000000LL };
    this->timer = ThreadPoolTimer::CreatePeriodicTimer(
        ref new TimerElapsedHandler(this, &MainPage::timerElapsed),
        period);

    this->statusText->Text = L"Status: Initialized Successfully";
}

void GpioOneWire::MainPage::timerElapsed (
    Windows::System::Threading::ThreadPoolTimer^ Timer
    )
{
    HRESULT sensorHr;
    DhtSensorReading reading;

    int retryCount = 0;
    do {
        sensorHr = this->dhtSensor.Sample(reading);
    } while (FAILED(sensorHr) && (++retryCount < 20));

    String^ statusString;
    String^ humidityString;
    String^ temperatureString;

    if (FAILED(sensorHr)) {
        humidityString = L"Humidity: (failed)";
        temperatureString = L"Temperature: (failed)";

        switch (sensorHr) {
        case __HRESULT_FROM_WIN32(ERROR_IO_DEVICE):
            statusString = L"Did not catch all falling edges";
            break;
        case __HRESULT_FROM_WIN32(ERROR_TIMEOUT):
            statusString = L"Timed out waiting for sample";
            break;
        case __HRESULT_FROM_WIN32(ERROR_INVALID_DATA):
            statusString = L"Checksum validation failed";
            break;
        default:
            statusString = L"Failed to get reading";
        }
    }
    else
    {
        double humidity = reading.Humidity();
        double temperature = reading.Temperature();
        
        HRESULT hr;
        wchar_t buf[128];

        hr = StringCchPrintfW(
            buf,
            ARRAYSIZE(buf),
            L"Humidity: %.1f%% RH",
            humidity);
        if (FAILED(hr)) {
            throw ref new Exception(hr, L"Failed to print string");
        }

        humidityString = ref new String(buf);

        hr = StringCchPrintfW(
            buf,
            ARRAYSIZE(buf),
            L"Temperature: %.1f \u00B0C",
            temperature);
        if (FAILED(hr)) {
            throw ref new Exception(hr, L"Failed to print string");
        }

        temperatureString = ref new String(buf);

        hr = StringCchPrintfW(
            buf,
            ARRAYSIZE(buf),
            L"Succeeded (%d %s)",
            retryCount,
            (retryCount == 1) ? L"retry" : L"retries");
        if (FAILED(hr)) {
            throw ref new Exception(hr, L"Failed to print string");
        }

        statusString = ref new String(buf);
    }

    this->Dispatcher->RunAsync(
        CoreDispatcherPriority::Normal,
        ref new DispatchedHandler([=] ()
    {
        this->statusText->Text = statusString;
        this->humidityText->Text = humidityString;
        this->temperatureText->Text = temperatureString;
    }));
}

