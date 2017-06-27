// Copyright (c) Microsoft. All rights reserved.
//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace GpioOneWire;

using namespace Platform;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
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

void GpioOneWire::DhtSensor::Init (GpioPin^ InputPin, GpioPin^ OutputPin)
{
    // Use InputPullUp if supported, otherwise fall back to Input (floating)
    this->inputDriveMode =
        InputPin->IsDriveModeSupported(GpioPinDriveMode::InputPullUp) ?
        GpioPinDriveMode::InputPullUp : GpioPinDriveMode::Input;

    InputPin->SetDriveMode(this->inputDriveMode);
    auto reader = ref new GpioChangeReader(InputPin);

    OutputPin->Write(GpioPinValue::Low);
    OutputPin->SetDriveMode(GpioPinDriveMode::Output);

    this->inputPin = InputPin;
    this->changeReader = reader;
    this->outputPin = OutputPin;
}

_Use_decl_annotations_
HRESULT GpioOneWire::DhtSensor::SampleInterrupts (GpioOneWire::DhtSensorReading& Reading)
{
    Reading = DhtSensorReading();

    LARGE_INTEGER qpf;
    QueryPerformanceFrequency(&qpf);
    
    this->changeReader->Polarity = GpioChangePolarity::Falling;
    this->changeReader->Clear();
    this->changeReader->Start();

    // Ensure change reader always gets stopped when this scope exits
    struct _StopChangeReader {
        ~_StopChangeReader ()
        {
            this->changeReader->Stop();
        }

        GpioChangeReader^ changeReader;
    } stopChangeReader = {this->changeReader};

    HRESULT hr = this->SendInitialPulse();
    if (FAILED(hr)) {
        return hr;
    }

    // Wait for 43 falling edges to show up
    Event completedEvent(CreateEvent(nullptr, TRUE, FALSE, nullptr));
    if (!completedEvent.IsValid()) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    auto operation = this->changeReader->WaitForItemsAsync(43);
    operation->Completed = ref new AsyncActionCompletedHandler([&] (
        IAsyncAction^ /*AsyncInfo*/,
        AsyncStatus Status
        )
    {
        SetEvent(completedEvent.Get());
    });

    if (WaitForSingleObject(completedEvent.Get(), 100) != WAIT_OBJECT_0) {
        // Request cancellation
        operation->Cancel();

        // Wait for operation to complete with status of cancelled
        WaitForSingleObject(completedEvent.Get(), INFINITE);
        return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
    }

    // discard first two falling edges
    this->changeReader->GetNextItem();
    this->changeReader->GetNextItem();

    // pulse widths greater than 110us are considered 1's
    const TimeSpan oneThreshold = {110LL * 10000000LL / 1000000LL};
    TimeSpan first = this->changeReader->GetNextItem().RelativeTime;
    for (int i = 0; i < 40; ++i)
    {
        TimeSpan second = this->changeReader->GetNextItem().RelativeTime;
        TimeSpan pulseWidth = {second.Duration - first.Duration};

        if (pulseWidth.Duration > oneThreshold.Duration) {
            Reading.bits[40 - i - 1] = true;
        }

        first = second;
    }

    if (!Reading.IsValid()) {
        // checksum mismatch
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    return S_OK;
}

_Use_decl_annotations_
HRESULT GpioOneWire::DhtSensor::SamplePolling (GpioOneWire::DhtSensorReading& Reading)
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

    HRESULT hr = this->SendInitialPulse();
    if (FAILED(hr)) {
        return hr;
    }

    GpioPinValue previousValue = this->inputPin->Read();

    // catch the first rising edge
    const ULONG initialRisingEdgeTimeoutMillis = 1;
    ULONGLONG endTickCount = GetTickCount64() + initialRisingEdgeTimeoutMillis;
    for (;;) {
        if (GetTickCount64() > endTickCount) {
            return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
        }

        GpioPinValue value = this->inputPin->Read();
        if (value != previousValue) {
            // rising edge?
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

        GpioPinValue value = this->inputPin->Read();
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

HRESULT GpioOneWire::DhtSensor::SendInitialPulse ()
{
    LARGE_INTEGER qpf;
    QueryPerformanceFrequency(&qpf);

    //
    // Bring the DHT data line low for 18ms. The output pin is driving the
    // gate of a transistor, so we bring the pin high to pull the DHT22
    // signal low.
    //
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    this->outputPin->Write(GpioPinValue::High);

    LARGE_INTEGER deadline;
    deadline.QuadPart = now.QuadPart + 18LL * qpf.QuadPart / 1000LL;
    while (now.QuadPart < deadline.QuadPart) {
        QueryPerformanceCounter(&now);
    }

    this->outputPin->Write(GpioPinValue::Low);

    if ((now.QuadPart - deadline.QuadPart) > (10LL * qpf.QuadPart / 1000LL)) {
        // Initial pulse must be less than about 30ms or DHT malfunctions
        return HRESULT_FROM_WIN32(ERROR_RETRY);
    }

    return S_OK;
}

MainPage::MainPage() :
    mode(Mode::Interrupts),
    previousMode(Mode::Interrupts),
    stats()
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

    GpioPin^ inputPin = this->OpenPin(controller, DHT_INPUT_PIN_NUMBER);
    if (!inputPin) {
        return;
    }

    GpioPin^ outputPin = this->OpenPin(controller, DHT_OUTPUT_PIN_NUMBER);
    if (!outputPin) {
        return;
    }

    this->dhtSensor.Init(inputPin, outputPin);
    this->pullResistorText->Text = this->dhtSensor.PullResistorRequired() ?
        L"10k pull-up resistor required." : L"Pull-up resistor not required.";

    // Create a periodic timer to sample from the DHT22 every second
    TimeSpan period = { 1 * 10000000LL };
    this->timer = ThreadPoolTimer::CreatePeriodicTimer(
        ref new TimerElapsedHandler(this, &MainPage::timerElapsed),
        period);

    this->statusText->Text = L"Status: Initialized Successfully";
}

void GpioOneWire::MainPage::radioButton_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    RadioButton^ rb = (RadioButton^)sender;
    if (rb == this->interruptButton) {
        this->mode = Mode::Interrupts;
    } else if (rb == this->pollingButton) {
        this->mode = Mode::Polling;
    } else if (rb == this->pauseButton) {
        this->mode = Mode::Paused;
    }
}

void GpioOneWire::MainPage::timerElapsed (
    Windows::System::Threading::ThreadPoolTimer^ Timer
    )
{
    HRESULT sensorHr;
    DhtSensorReading reading;

    switch (this->mode) {
    case Mode::Interrupts:
        sensorHr = this->dhtSensor.SampleInterrupts(reading);
        break;
    case Mode::Polling:
        sensorHr = this->dhtSensor.SamplePolling(reading);
        break;
    case Mode::Paused:
    default:
        return;
    }

    if (this->mode != this->previousMode) {
        this->stats = _Stats();
        this->previousMode = this->mode;
    }

    ++this->stats.TotalSampleCount;

    String^ statusString;
    String^ humidityString;
    String^ temperatureString;

    if (FAILED(sensorHr)) {
        humidityString = L"Humidity: (failed)";
        temperatureString = L"Temperature: (failed)";

        switch (sensorHr) {
        case __HRESULT_FROM_WIN32(ERROR_RETRY):
            statusString = L"Initial pulse timing error";
            ++this->stats.PulseTimingErrors;
            break;
        case __HRESULT_FROM_WIN32(ERROR_TIMEOUT):
            statusString = L"Timed out waiting for sample";
            ++this->stats.TimeoutErrors;
            break;
        case __HRESULT_FROM_WIN32(ERROR_INVALID_DATA):
            statusString = L"Checksum validation failed";
            ++this->stats.ChecksumErrors;
            break;
        default:
            statusString = L"Failed to get reading";
        }
    }
    else
    {
        ++this->stats.SuccessfulSampleCount;

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
            throw Exception::CreateException(hr, L"Failed to print string");
        }

        humidityString = ref new String(buf);

        hr = StringCchPrintfW(
            buf,
            ARRAYSIZE(buf),
            L"Temperature: %.1f \u00B0C",
            temperature);
        if (FAILED(hr)) {
            throw Exception::CreateException(hr, L"Failed to print string");
        }

        temperatureString = ref new String(buf);

        hr = StringCchPrintfW(
            buf,
            ARRAYSIZE(buf),
            L"Succeeded");

        if (FAILED(hr)) {
            throw Exception::CreateException(hr, L"Failed to print string");
        }

        statusString = ref new String(buf);
    }

    String^ reliabilityString;
    {
        WCHAR buf[512];

#ifdef _DEBUG
        HRESULT hr = StringCchPrintfW(
            buf,
            ARRAYSIZE(buf),
            L"Successful Samples: %d/%d (%2.2f%%). PulseTiming: %d, Timeout: %d, Checksum: %d",
            this->stats.SuccessfulSampleCount,
            this->stats.TotalSampleCount,
            100.0 * this->stats.SuccessfulSampleCount / this->stats.TotalSampleCount,
            this->stats.PulseTimingErrors,
            this->stats.TimeoutErrors,
            this->stats.ChecksumErrors);
#else
        HRESULT hr = StringCchPrintfW(
            buf,
            ARRAYSIZE(buf),
            L"Successful Samples: %d/%d (%2.2f%%)",
            this->stats.SuccessfulSampleCount,
            this->stats.TotalSampleCount,
            100.0 * this->stats.SuccessfulSampleCount / this->stats.TotalSampleCount);
#endif // DEBUG

        if (FAILED(hr)) {
            throw Exception::CreateException(hr, L"Failed to print string");
        }

        reliabilityString = ref new String(buf);
    }

    this->Dispatcher->RunAsync(
        CoreDispatcherPriority::Normal,
        ref new DispatchedHandler([=] ()
    {
        this->statusText->Text = statusString;
        this->humidityText->Text = humidityString;
        this->temperatureText->Text = temperatureString;
        this->reliabilityText->Text = reliabilityString;
    }));
}

GpioPin^ GpioOneWire::MainPage::OpenPin (
    GpioController^ Controller,
    int PinNumber
    )
{
    try {
        return Controller->OpenPin(PinNumber);
    } catch (Exception^ ex) {
        WCHAR buf[512];
        HRESULT hr = StringCchPrintfW(
            buf,
            ARRAYSIZE(buf),
            L"Failed to open GPIO pin %d: %s (0x%x)",
            PinNumber,
            ex->Message->Data(),
            ex->HResult);

        if (FAILED(hr)) {
            throw Exception::CreateException(hr, L"Failed to print string");
        }

        this->statusText->Text = ref new String(buf);
        return nullptr;
    }
}
