// Copyright (c) Microsoft. All rights reserved.


//
// spitesttool
//
//   Utility to read and write SPI devices from the command line.
//

#include <ppltasks.h>
#include <collection.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cwctype>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::Spi;

class wexception
{
public:
    explicit wexception (const std::wstring &msg) : msg_(msg) { }
    virtual ~wexception () { /*empty*/ }

    virtual const wchar_t *wwhat () const
    {
        return msg_.c_str();
    }

private:
    std::wstring msg_;
};

void ListSpiControllers ()
{
    using namespace Windows::Devices::Enumeration;
    using namespace Platform::Collections;

    String^ friendlyNameProperty =
        L"System.DeviceInterface.Spb.ControllerFriendlyName";
    auto properties = ref new Vector<String^>();
    properties->Append(friendlyNameProperty);
    auto dis = concurrency::create_task(DeviceInformation::FindAllAsync(
            SpiDevice::GetDeviceSelector(),
            properties)).get();
    if (dis->Size < 1) {
        std::wcout << L"There are no SPI controllers on this system.\n";
        return;
    }

    wprintf(L"  FriendlyName DeviceId\n");
    for (const auto& di : dis) {
        wprintf(
            L"  %12s %s\n",
            ((String^)di->Properties->Lookup(friendlyNameProperty))->Data(),
            di->Id->Data());
    }
}

SpiDevice^ MakeDevice (
    String^ friendlyName,
    int chipSelectLine,
    SpiMode mode,
    int dataBitLength,
    int clockFrequency
    )
{
    using namespace Windows::Devices::Enumeration;

    String^ aqs;
    if (friendlyName)
        aqs = SpiDevice::GetDeviceSelector(friendlyName);
    else
        aqs = SpiDevice::GetDeviceSelector();

    auto dis = concurrency::create_task(
        DeviceInformation::FindAllAsync(aqs)).get();
    if (dis->Size < 1)
        throw wexception(L"SPI bus not found.");

    String^ id = dis->GetAt(0)->Id;

    auto settings = ref new SpiConnectionSettings(chipSelectLine);

    if (int(mode) != -1) {
        settings->Mode = mode;
    }

    if (dataBitLength != 0) {
        settings->DataBitLength = dataBitLength;
    }

    if (clockFrequency != 0) {
        settings->ClockFrequency = clockFrequency;
    }

    auto device = concurrency::create_task(
        SpiDevice::FromIdAsync(id, settings)).get();

    if (!device) {
        std::wostringstream msg;
        msg << L"Chip select line " << std::dec << chipSelectLine << L" on bus " << id->Data() <<
            L" is in use. Please ensure that no other applications are using SPI.";
        throw wexception(msg.str());
    }

    return device;
}

std::wistream& expect (std::wistream& is, wchar_t delim)
{
    wchar_t ch;
    while (is.get(ch)) {
        if (ch == delim) return is;
        if (!isspace(ch)) {
            is.clear(is.failbit);
            break;
        }
    }
    return is;
}

std::wistream& operator>> (std::wistream& is, std::vector<BYTE>& bytes)
{
    bytes.clear();

    if (!expect(is, L'{')) {
        std::wcout << L"Syntax error: expecting '{'\n";
        return is;
    }

    // get a sequence of bytes, e.g.
    //   write { 0 1 2 3 4 aa bb cc dd }
    unsigned int byte;
    while (is >> std::hex >> byte) {
        if (byte > 0xff) {
            std::wcout << L"Out of range [0, 0xff]: " << std::hex << byte << L"\n";
            is.clear(is.failbit);
            return is;
        }
        bytes.push_back(static_cast<BYTE>(byte));
    }

    if (bytes.empty()) {
        std::wcout << L"Zero-length buffers are not allowed\n";
        is.clear(is.failbit);
        return is;
    }

    is.clear();
    if (!expect(is, L'}')) {
        std::wcout << L"Syntax error: expecting '}'\n";
        return is;
    }
    return is;
}

std::wostream& operator<< (std::wostream& os, Array<BYTE>^& bytes)
{
    for (auto byte : bytes)
        os << L" " << std::hex << byte;
    return os;
}

PCWSTR Help =
    L"Commands:\n"
    L" > write { 00 11 22 .. FF }         Write bytes to device\n"
    L" > read N                           Read N bytes\n"
    L" > writeread { 00 11 .. FF } N      Write bytes then read N bytes\n"
    L" > fullduplex { 00 11 .. FF }       Perform full duplex transfer\n"
    L" > info                             Display device information\n"
    L" > help                             Display this help message\n"
    L" > quit                             Quit\n\n";

void ShowPrompt (SpiDevice^ device)
{
    while (std::wcin) {
        std::wcout << L"> ";

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
            std::wcout << Help;
        } else if (command == L"write") {
            std::vector<BYTE> writeBuf;
            if (!(linestream >> writeBuf)) {
                std::wcout << L"Usage: write { 55 a0 ... ff }\n";
                continue;
            }

            device->Write(ArrayReference<BYTE>(
                writeBuf.data(),
                static_cast<unsigned int>(writeBuf.size())));
        } else if (command == L"read") {
            // expecting a single int, number of bytes to read
            unsigned int bytesToRead;
            if (!(linestream >> std::dec >> bytesToRead)) {
                std::wcout << L"Expecting integer. e.g: read 4\n";
                continue;
            }

            auto readBuf = ref new Array<BYTE>(bytesToRead);
            device->Read(readBuf);
            std::wcout << readBuf << L"\n";
        } else if (command == L"writeread") {
            // get a sequence of bytes, e.g.
            //   write 0 1 2 3 4 aa bb cc dd
            std::vector<BYTE> writeBuf;
            if (!(linestream >> writeBuf)) {
                std::wcout << L"Usage: writeread { 55 a0 ... ff } 4\n";
                continue;
            }

            unsigned int bytesToRead;
            if (!(linestream >> std::dec >> bytesToRead)) {
                std::wcout << L"Syntax error: expecting integer\n";
                std::wcout << L"Usage: writeread { 55 a0 ... ff } 4\n";
                continue;
            }
            auto readBuf = ref new Array<BYTE>(bytesToRead);

            device->TransferSequential(
                ArrayReference<BYTE>(
                    writeBuf.data(),
                    static_cast<unsigned int>(writeBuf.size())),
                readBuf);

            std::wcout << readBuf << L"\n";
        } else if (command == L"fullduplex") {
            std::vector<BYTE> writeBuf;
            if (!(linestream >> writeBuf)) {
                std::wcout << L"Usage: fullduplex { 55 a0 ... ff }\n";
                continue;
            }
            auto readBuf = ref new Array<BYTE>(
                static_cast<unsigned int>(writeBuf.size()));

            device->TransferFullDuplex(
                ArrayReference<BYTE>(
                    writeBuf.data(),
                    static_cast<unsigned int>(writeBuf.size())),
                readBuf);

            std::wcout << readBuf << L"\n";
        } else if (command == L"info") {
            auto settings = device->ConnectionSettings;

            std::wcout << L"        DeviceId: " << device->DeviceId->Data() << "\n";
            std::wcout << L"  ChipSelectLine: " << std::dec << settings->ChipSelectLine << L"\n";
            std::wcout << L"            Mode: " << std::dec << int(settings->Mode) << L"\n";
            std::wcout << L"   DataBitLength: " << std::dec << settings->DataBitLength << L"\n";
            std::wcout << L"  ClockFrequency: " << std::dec << settings->ClockFrequency << L" Hz\n";

        } else if (command.empty()) {
            // ignore
        } else {
            std::wcout << L"Unrecognized command: " << command <<
                L". Type 'help' for command usage.\n";
        }
    }
}

void PrintUsage (PCWSTR name)
{
    wprintf(
        L"SpiTestTool: Command line SPI testing utility\n"
        L"Usage: %s [-list] [-n FriendlyName] [-c ChipSelectLine] [-m Mode] "
        L"[-d DataBitLength] [-f ClockFrequency]\n"
        L"\n"
        L"  -list           List available SPI controllers and exit.\n"
        L"  FriendlyName    The friendly name of the SPI controller over which\n"
        L"                  you wish to communicate. This parameter is\n"
        L"                  optional and defaults to the first enumerated SPI\n"
        L"                  controller.\n"
        L"  ChipSelectLine  The chip select line to use. This parameter is\n"
        L"                  optional and defaults to 0.\n"
        L"  Mode            The SPI mode to use (0-3). This parameter is\n"
        L"                  optional and defaults to Mode 0.\n"
        L"  DataBitLength   The data bit length to use. This parameter is optional\n"
        L"                  and defaults to 8.\n"
        L"  ClockFrequency  The SPI clock frequency to use. This parameter is\n"
        L"                  optional and defaults to 4Mhz.\n"
        L"\n"
        L"Examples:\n"
        L"  Connect to the first SPI controller found with default settings\n"
        L"  (ChipSelectLine=0, Mode=0, DataBitLength=8, Frequency=4Mhz):\n"
        L"    %s\n"
        L"\n"
        L"  List available SPI controllers and exit:\n"
        L"    %s -list\n"
        L"\n"
        L"  Connect to SPI1 in mode 2, with default speed (4Mhz) and chip\n"
        L"  select line (0):\n"
        L"    %s -n SPI1 -m 2\n"
        L"\n"
        L"  Connect to chip select 1 on SPI1 in mode 2 at 1Mhz:\n"
        L"    %s -c 1 -n SPI1 -m 2 -f 1000000\n",
        name,
        name,
        name,
        name,
        name);
}

int main (Platform::Array<Platform::String^>^ args)
{
    String^ friendlyName;
    int chipSelectLine = 0;
    SpiMode mode = SpiMode(-1);
    int dataBitLength = 0;
    int clockFrequency = 0;

    for (unsigned int optind = 1; optind < args->Length; optind += 2) {
        PCWSTR arg1 = args->get(optind)->Data();
        if (!_wcsicmp(arg1, L"-h") || !_wcsicmp(arg1, L"/h") ||
            !_wcsicmp(arg1, L"-?") || !_wcsicmp(arg1, L"/?")) {

            PrintUsage(args->get(0)->Data());
            return 0;
        }

        if (!_wcsicmp(arg1, L"-l") || !_wcsicmp(arg1, L"-list")) {
            ListSpiControllers();
            return 0;
        }

        if (arg1[0] != L'-') {
            std::wcerr << L"Unexpected positional parameter: " << arg1 << L"\n";
            return 1;
        }

        if (args->get(optind)->Length() != 2) {
            std::wcerr << L"Invalid option format: " << arg1 << L"\n";
            return 1;
        }

        if ((optind + 1) >= args->Length) {
            std::wcerr << L"Missing required parameter for option: " << arg1 << L"\n";
            return 1;
        }
        PCWSTR arg2 = args->get(optind + 1)->Data();

        wchar_t *endptr;
        switch (towlower(arg1[1])) {
        case L'n':
            friendlyName = args->get(optind + 1);
            break;
        case L'c':
            chipSelectLine = static_cast<int>(
                wcstoul(arg2, &endptr, 0));
            break;
        case L'm':
        {
            unsigned long modeUlong = wcstoul(arg2, &endptr, 0);
            if ((modeUlong < 0) || (modeUlong > 3)) {
                std::wcerr << L"Invalid mode value: " << modeUlong <<
                    L". Mode must be in the range [0, 3]\n";
                return 1;
            }
            mode = static_cast<SpiMode>(modeUlong);
            static_assert(int(SpiMode::Mode0) == 0, "Verifying SpiMode::Mode0 is 0");
            static_assert(int(SpiMode::Mode1) == 1, "Verifying SpiMode::Mode1 is 1");
            static_assert(int(SpiMode::Mode2) == 2, "Verifying SpiMode::Mode2 is 2");
            static_assert(int(SpiMode::Mode3) == 3, "Verifying SpiMode::Mode3 is 3");

            break;
        }
        case L'd':
            dataBitLength = static_cast<int>(wcstoul(arg2, &endptr, 0));
            if (dataBitLength == 0) {
                std::wcerr << L"Invalid data bit length\n";
                return 1;
            }
            break;
        case L'f':
            clockFrequency = static_cast<int>(wcstoul(arg2, &endptr, 0));
            if (clockFrequency == 0) {
                std::wcerr << L"Invalid clock frequency\n";
                return 1;
            }
            break;
        default:
            std::wcerr << L"Unrecognized option: " << arg1 << L"\n";
            return 1;
        }
    }

    try {
        auto device = MakeDevice(
            friendlyName,
            chipSelectLine,
            mode,
            dataBitLength,
            clockFrequency);

        std::wcout << L"  Type 'help' for a list of commands\n";
        ShowPrompt(device);
    } catch (const wexception& ex) {
        std::wcerr << L"Error: " << ex.wwhat() << L"\n";
        return 1;
    } catch (Platform::Exception^ ex) {
        std::wcerr << L"Error: " << ex->Message->Data() << L"\n";
        return 1;
    }

    return 0;
}
