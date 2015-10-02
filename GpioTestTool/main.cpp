// Copyright (c) Microsoft. All rights reserved.


//
// GpioTestTool
//
//   Utility to manipulate GPIO pins from the command line.
//   Demonstrates how to use the GPIO WinRT APIs from standard
//   C++ with WRL.
//

#include <windows.h>
#include <strsafe.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cwctype>

#include <windows.foundation.h>
#include <windows.devices.gpio.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Devices::Gpio;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

<<<<<<< HEAD
class wexception
{
public:
    explicit wexception (const std::wstring &msg) : msg_(msg) { }
    virtual ~wexception () { }

    virtual const wchar_t *wwhat () const
    {
        return msg_.c_str();
    }

private:
    std::wstring msg_;
};

ComPtr<IGpioPin> MakePin (int PinNumber)
{
    ComPtr<IGpioPin> pin;
=======
typedef ITypedEventHandler<GpioPin *, GpioPinValueChangedEventArgs *>
    IGpioPinValueChangedHandler;

class wexception
{
public:
    explicit wexception (const std::wstring& Msg) : msg(Msg) { }

    wexception (const wchar_t* Msg, ...) {
        HRESULT hr;
        WCHAR buf[512];

        va_list argList;
        va_start(argList, Msg);
        hr = StringCchVPrintfW(
                buf,
                ARRAYSIZE(buf),
                Msg,
                argList);
        va_end(argList);

        this->msg = SUCCEEDED(hr) ? buf : Msg;
    }

    virtual ~wexception () { /*empty*/ }

    virtual const wchar_t *wwhat () const
    {
        return this->msg.c_str();
    }

private:
    std::wstring msg;
};

ComPtr<IGpioController> MakeDefaultController ()
{
    ComPtr<IGpioController> controller;
>>>>>>> upstream/develop

    // get the activation factory
    ComPtr<IGpioControllerStatics> controllerStatics;
    HRESULT hr = GetActivationFactory(
        HStringReference(RuntimeClass_Windows_Devices_Gpio_GpioController).Get(),
        &controllerStatics);
    if (FAILED(hr)) {
<<<<<<< HEAD
        std::wostringstream msg;
        msg << L"Failed to get activation factory for GpioController. hr = 0x" <<
            std::hex << hr;
        throw wexception(msg.str());
    }

    ComPtr<IGpioController> controller;
    hr = controllerStatics->GetDefault(controller.GetAddressOf());
    if (FAILED(hr)) {
        throw wexception(L"Failed to get instance of default GPIO controller");
=======
        throw wexception(
            L"Failed to get activation factory for %s. Please ensure the type "
            L"is present on the system. (hr = 0x%x)",
            RuntimeClass_Windows_Devices_Gpio_GpioController,
            hr);
    }

    hr = controllerStatics->GetDefault(controller.GetAddressOf());
    if (FAILED(hr)) {
        throw wexception(
            L"Failed to get instance of default GPIO controller. (hr = 0x%x)",
            hr);
>>>>>>> upstream/develop
    }

    if (!controller) {
        throw wexception(L"GPIO is not available on this system");
    }

<<<<<<< HEAD
    hr = controller->OpenPin(PinNumber, pin.GetAddressOf());
    if (FAILED(hr)) {
        std::wostringstream msg;
        msg << L"Failed to open pin. hr = 0x" << std::hex << hr;
        throw wexception(msg.str());
=======
    return controller;
}

ComPtr<IGpioPin> MakePin (int PinNumber)
{
    auto controller = MakeDefaultController();

    ComPtr<IGpioPin> pin;
    HRESULT hr = controller->OpenPin(PinNumber, pin.GetAddressOf());
    if (FAILED(hr)) {
        throw wexception(L"Failed to open pin. (hr = 0x%x)", hr);
>>>>>>> upstream/develop
    }

    return pin;
}

<<<<<<< HEAD
=======
void ListPins ()
{
    auto controller = MakeDefaultController();

    INT32 pinCount;
    HRESULT hr = controller->get_PinCount(&pinCount);
    if (FAILED(hr)) {
        throw wexception(L"Failed to get pin count. (hr = 0x%x)", hr);
    }

    wprintf(L"The default GPIO controller has %d pins:\n", pinCount);
    for (INT32 pinNumber = 0; pinNumber < pinCount; ++pinNumber) {
        ComPtr<IGpioPin> pin;
        hr = controller->OpenPin(pinNumber, pin.GetAddressOf());
        switch (hr) {
        case S_OK:
            wprintf(L"  Pin %d is available\n", pinNumber);
            break;
        case __HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION):
            wprintf(L"  Pin %d is currently in use\n", pinNumber);
            break;
        case __HRESULT_FROM_WIN32(ERROR_NOT_FOUND):
            // pin is not available to applications; don't print anything
            break;
        default:
            wprintf(
                L"  Pin %d: unexpected error occurred attempting to open. (hr = 0x%x)\n",
                pinNumber,
                hr);
        }
    }
}

>>>>>>> upstream/develop
GpioPinValue operator! (GpioPinValue Value)
{
    return (Value == GpioPinValue_Low) ? GpioPinValue_High : GpioPinValue_Low;
}

std::wistream& operator>> (std::wistream& is, GpioPinValue& value)
{
    int raw;
    is >> raw;
    if (is.fail()) {
        return is;
    }
    switch (raw) {
    case GpioPinValue_Low:
    case GpioPinValue_High:
        value = GpioPinValue(raw);
        break;
    default:
        is.clear(is.failbit);
    }
    return is;
}

<<<<<<< HEAD
std::wostream& operator<< (std::wostream& os, GpioPinValue value)
{
    switch (value) {
    case GpioPinValue_Low:
        return os << L"Low";
    case GpioPinValue_High:
        return os << L"High";
    default:
        return os << L"[undefined]";
=======
PCWSTR StringFromGpioPinValue (GpioPinValue value)
{
    switch (value) {
    case GpioPinValue_Low: return L"Low";
    case GpioPinValue_High: return L"High";
    default: return L"[invalid GpioPinValue]";
>>>>>>> upstream/develop
    }
}

std::wistream& operator>> (std::wistream& is, GpioPinDriveMode& value)
{
    is.clear();
    std::wstring driveMode;
    is >> driveMode;
    if (!_wcsicmp(driveMode.c_str(), L"input")) {
        value = GpioPinDriveMode_Input;
    } else if (!_wcsicmp(driveMode.c_str(), L"output")) {
        value = GpioPinDriveMode_Output;
    } else if (!_wcsicmp(driveMode.c_str(), L"inputPullUp")) {
        value = GpioPinDriveMode_InputPullUp;
    } else if (!_wcsicmp(driveMode.c_str(), L"inputPullDown")) {
        value = GpioPinDriveMode_InputPullDown;
    } else {
        is.clear(is.failbit);
    }
    return is;
}

<<<<<<< HEAD
std::wostream& operator<< (std::wostream& os, GpioPinDriveMode value)
{
    switch (value) {
    case GpioPinDriveMode_Input:
        return os << L"input";
    case GpioPinDriveMode_Output:
        return os << L"output";
    case GpioPinDriveMode_InputPullUp:
        return os << L"inputPullUp";
    case GpioPinDriveMode_InputPullDown:
        return os << L"inputPullDown";
    default:
        return os << L"[undefined]";
    }
}

std::wostream& operator<< (std::wostream& os, GpioSharingMode value)
{
    switch (value) {
    case GpioSharingMode_Exclusive:
        return os << L"Exclusive";
    case GpioSharingMode_SharedReadOnly:
        return os << L"SharedReadOnly";
    default:
        return os << L"[undefined]";
=======
PCWSTR StringFromGpioPinDriveMode (GpioPinDriveMode value)
{
    switch (value) {
    case GpioPinDriveMode_Input: return L"input";
    case GpioPinDriveMode_Output: return L"output";
    case GpioPinDriveMode_InputPullUp: return L"inputPullUp";
    case GpioPinDriveMode_InputPullDown: return L"inputPullDown";
    default: return L"[invalid GpioPinDriveMode]";
    }
}

PCWSTR StringFromGpioSharingMode (GpioSharingMode value)
{
    switch (value) {
    case GpioSharingMode_Exclusive: return L"Exclusive";
    case GpioSharingMode_SharedReadOnly: return L"SharedReadOnly";
    default: return L"[invalid GpioSharingMode]";
    }
}

PCWSTR StringFromGpioPinEdge (GpioPinEdge value)
{
    switch (value) {
    case GpioPinEdge_FallingEdge: return L"Falling";
    case GpioPinEdge_RisingEdge: return L"Rising";
    default: return L"[Invalid GpioPinEdge value]";
>>>>>>> upstream/develop
    }
}

PCWSTR Help =
    L"Commands:\n"
    L" > write 0|1                        Write pin low (0) or high (1)\n"
<<<<<<< HEAD
=======
    L" > high                             Alias for 'write 1'\n"
    L" > low                              Alias for 'write 0'\n"
>>>>>>> upstream/develop
    L" > toggle                           Toggle the pin from its current state\n"
    L" > read                             Read pin\n"
    L" > setdrivemode drive_mode          Set the pins's drive mode\n"
    L"     where drive_mode = input|output|\n"
    L"                        inputPullUp|inputPullDown\n"
<<<<<<< HEAD
    L" > info                             Dump information about the pin\n"
    L" > help                             Display this help message\n"
    L" > quit                             Quit\n\n";

void ShowPrompt (_In_ IGpioPin* pin)
{
    GpioPinValue outputLatch = GpioPinValue_High;
    while (std::wcin) {
        std::wcout << L"> ";
=======
    L" > interrupt on|off                 Register or unregister for pin value\n"
    L"                                    change events.\n"
    L" > info                             Dump information about the pin\n"
    L" > help                             Display this help message\n"
    L" > quit                             Quit\n";

void ShowPrompt (_In_ IGpioPin* pin)
{
    auto listener = Callback<IGpioPinValueChangedHandler>([] (
        _In_ IGpioPin* /*Pin*/,
        _In_ IGpioPinValueChangedEventArgs* Args
        ) -> HRESULT
    {
        GpioPinEdge edge;
        Args->get_Edge(&edge);
        wprintf(L"%s edge occurred.\n", StringFromGpioPinEdge(edge));
        return S_OK;
    });

    auto token = EventRegistrationToken();
    bool listenerRegistered = false;

    GpioPinValue outputLatch = GpioPinValue_High;
    while (std::wcin) {
        wprintf(L"> ");
>>>>>>> upstream/develop

        std::wstring line;
        if (!std::getline(std::wcin, line)) {
            return;
        }
        std::wistringstream linestream(line);

        std::wstring command;
        linestream >> command;
        if ((command == L"q") || (command == L"quit")) {
            return;
        } else if ((command == L"h") || (command == L"help")) {
<<<<<<< HEAD
            std::wcout << Help;
=======
            wprintf(L"%s\n", Help);
>>>>>>> upstream/develop
        } else if ((command == L"w") || (command == L"write")) {
            GpioPinValue value;
            linestream >> value;
            if (linestream.fail()) {
<<<<<<< HEAD
                std::wcout << L"Syntax error: expecting 0 or 1\n";
                std::wcout << L"Usage: write 0|1\n";
=======
                wprintf(L"Syntax error: expecting 0 or 1\nUsage: write 0|1\n");
>>>>>>> upstream/develop
                continue;
            }

            HRESULT hr = pin->Write(value);
            if (FAILED(hr)) {
<<<<<<< HEAD
                std::wcout << L"Failed to write pin. hr = 0x" << std::hex << hr << "\n";
=======
                wprintf(L"Failed to write pin. (hr = 0x%x)\n", hr);
                continue;
            }
        } else if (command == L"high") {
            HRESULT hr = pin->Write(GpioPinValue_High);
            if (FAILED(hr)) {
                wprintf(L"Failed to write pin. (hr = 0x%x)\n", hr);
                continue;
            }
        } else if (command == L"low") {
            HRESULT hr = pin->Write(GpioPinValue_Low);
            if (FAILED(hr)) {
                wprintf(L"Failed to write pin. (hr = 0x%x)\n", hr);
>>>>>>> upstream/develop
                continue;
            }
        } else if ((command == L"t") || (command == L"toggle")) {
            outputLatch = !outputLatch;
            HRESULT hr = pin->Write(outputLatch);
            if (FAILED(hr)) {
<<<<<<< HEAD
                std::wcout << L"Failed to write pin. hr = 0x" << std::hex << hr << "\n";
=======
                wprintf(L"Failed to write pin. (hr = 0x%x)\n", hr);
>>>>>>> upstream/develop
                continue;
            }
        } else if ((command == L"r") || (command == L"read")) {
            GpioPinValue value;
            HRESULT hr = pin->Read(&value);
            if (FAILED(hr)) {
<<<<<<< HEAD
                std::wcout << L"Failed to read pin. hr = 0x" << std::hex << hr << "\n";
                continue;
            }
            std::wcout << value << L"\n";
        } else if (command == L"setdrivemode") {
            GpioPinDriveMode driveMode;
            linestream >> driveMode;
            if (linestream.fail()) {
                std::wcout << L"Syntax error: expecting valid drive mode\n";
=======
                wprintf(L"Failed to read pin. (hr = 0x%x)\n", hr);
                continue;
            }
            wprintf(L"%s\n", StringFromGpioPinValue(value));
        } else if (command == L"setdrivemode") {
            GpioPinDriveMode driveMode;
            linestream >> driveMode;
            if (!linestream) {
                wprintf(L"Syntax error: expecting valid drive mode. Type 'help' for usage.\n");
>>>>>>> upstream/develop
                continue;
            }

            HRESULT hr = pin->SetDriveMode(driveMode);
            if (FAILED(hr)) {
<<<<<<< HEAD
                std::wcout << L"Failed to set drive mode. hr = 0x" << std::hex << hr << "\n";
                continue;
            }
=======
                wprintf(L"Failed to set drive mode. (hr = 0x%x)\n", hr);
                continue;
            }
        } else if ((command == L"int") || (command == L"interrupt")) {
            std::wstring onOrOff;
            linestream >> onOrOff;
            if (onOrOff == L"on") {
                if (listenerRegistered) {
                    wprintf(L"Interrupt listener already registered.\n");
                    continue;
                }

                HRESULT hr = pin->add_ValueChanged(listener.Get(), &token);
                if (FAILED(hr)) {
                    wprintf(
                        L"Failed to add event listener to ValueChanged event. (hr = 0x%x)\n",
                        hr);
                    continue;
                }
                listenerRegistered = true;
            } else if (onOrOff == L"off") {
                if (!listenerRegistered) {
                    wprintf(L"No interrupt listener is currently registered.\n");
                    continue;
                }

                HRESULT hr = pin->remove_ValueChanged(token);
                if (FAILED(hr)) {
                    wprintf(
                        L"Failed to remove ValueChanged event. (hr = 0x%x)\n",
                        hr);
                    continue;
                }
                listenerRegistered = false;
            } else {
                wprintf(
                    L"Expecting 'on' or 'off': %s. Type 'help' for usage.\n",
                    onOrOff.c_str());
            }
>>>>>>> upstream/develop
        } else if ((command == L"i") || (command == L"info")) {
            int pinNumber;
            pin->get_PinNumber(&pinNumber);
            GpioSharingMode sharingMode;
            pin->get_SharingMode(&sharingMode);
            TimeSpan debounceTimeout;
            pin->get_DebounceTimeout(&debounceTimeout);
            GpioPinDriveMode driveMode;
            pin->GetDriveMode(&driveMode);
<<<<<<< HEAD

            std::wcout << L"        Pin Number: " << std::dec << pinNumber << L"\n";
            std::wcout << L"      Sharing Mode: " << sharingMode << L"\n";
            std::wcout << L"  Debounce Timeout: " << debounceTimeout.Duration << L"\n";
            std::wcout << L"        Drive Mode: " << driveMode << L"\n";
        } else if (command.empty()) {
            // ignore
        } else {
            std::wcout << L"Unrecognized command: " << command <<
                L". Type 'help' for command usage.\n";
=======
            GpioPinValue value;
            pin->Read(&value);

            wprintf(
                L"        Pin Number: %d\n"
                L"      Sharing Mode: %s\n"
                L"  Debounce Timeout: %d ms\n"
                L"        Drive Mode: %s\n"
                L"             Value: %s\n",
                pinNumber,
                StringFromGpioSharingMode(sharingMode),
                int(debounceTimeout.Duration / 10000LL),
                StringFromGpioPinDriveMode(driveMode),
                StringFromGpioPinValue(value));
        } else if (command.empty()) {
            // ignore
        } else {
            wprintf(
                L"Unrecognized command: %s. Type 'help' for command usage.\n",
                command.c_str());
>>>>>>> upstream/develop
        }
    }
}

<<<<<<< HEAD
void PrintUsage (PCWSTR name)
{
    wprintf(
        L"%s: Command line GPIO testing utility\n"
        L"Usage: %s PinNumber\n"
        L"\n"
        L"  PinNumber     The pin number with which you wish to interact. This\n"
        L"                parameter is required.\n"
        L"\n"
        L"Example:\n"
        L"  %s 47\n",
        name,
        name,
        name);
=======
PCWSTR Usage =
L"GpioTestTool: Command line GPIO testing utility\n"
L"Usage: %s [-list] PinNumber\n"
L"\n"
L"  -list         List the available pins on the default GPIO controller.\n"
L"  PinNumber     The pin number with which you wish to interact. This\n"
L"                parameter is required.\n"
L"\n"
L"Example:\n"
L"  %s -list\n"
L"  %s 47\n";

void PrintUsage (PCWSTR name)
{
    wprintf(Usage, name, name, name);
>>>>>>> upstream/develop
}

int __cdecl wmain (_In_ int argc, _In_reads_(argc) wchar_t *argv[])
{
<<<<<<< HEAD
    int optind = 1;
    if (optind < argc) {
        if (!_wcsicmp(argv[optind], L"-h") || !wcscmp(argv[optind], L"-?")) {
            PrintUsage(argv[0]);
            return 0;
        }
    } else {
        std::wcerr << L"Missing required command line parameter PinNumber\n\n";
        PrintUsage(argv[0]);
        return 1;
    }

    INT32 pinNumber;
    {
        PCWSTR arg = argv[optind++];
        wchar_t *endptr;
        pinNumber = INT32(wcstoul(arg, &endptr, 0));
        if (endptr != (arg + wcslen(arg))) {
            std::wcerr << L"Expecting integer: " << arg << L"\n";
            std::wcerr << L"Type '" << argv[0] << " -h' for usage\n";
            return 1;
        }
=======
    enum MainReturnValue { Success, Failure };

    if (argc < 2) {
        fwprintf(
            stderr,
            L"Missing required command line parameter PinNumber\n\n");
        PrintUsage(argv[0]);
        return Success;
    }

    PCWSTR arg = argv[1];
    if (!_wcsicmp(arg, L"-h") || !_wcsicmp(arg, L"/h") ||
        !_wcsicmp(arg, L"-?") || !_wcsicmp(arg, L"/?")) {

        PrintUsage(argv[0]);
        return Success;
>>>>>>> upstream/develop
    }

    RoInitializeWrapper roInit(RO_INIT_MULTITHREADED);
    try {
<<<<<<< HEAD
        auto pin = MakePin(pinNumber);
        std::wcout << L"Type 'help' for a list of commands\n";
        ShowPrompt(pin.Get());
    } catch (const wexception& ex) {
        std::wcerr << L"Error: " << ex.wwhat() << L"\n";
        return 1;
    }

    return 0;
=======
        if (!_wcsicmp(arg, L"-list") || !_wcsicmp(arg, L"/list")) {
            ListPins();
            return Success;
        } else {
            std::wistringstream is(arg);
            INT32 pinNumber;
            is >> pinNumber;
            if (!is) {
                fwprintf(
                    stderr,
                    L"Expecting integer: %s\nType %s /? for usage.\n",
                    arg,
                    argv[0]);
                return Failure;
            }

            auto pin = MakePin(pinNumber);
            wprintf(L"Type 'help' for a list of commands\n");
            ShowPrompt(pin.Get());
            return Success;
        }
    } catch (const wexception& ex) {
        fwprintf(stderr, L"Error: %s\n", ex.wwhat());
        return Failure;
    }
>>>>>>> upstream/develop
}
