// Copyright (c) Microsoft. All rights reserved.

#include <windows.h>
#include <cfgmgr32.h>
#include <propkey.h>
#include <strsafe.h>
#include <wrl.h>

#include <vector>
#include <string>
#include <iostream>

using namespace Microsoft::WRL::Wrappers;

typedef HandleT<HandleTraits::HANDLENullTraits> ThreadHandle;

enum class SerialParity {
    None,
    Odd,
    Even,
    Mark,
    Space,
};

enum class SerialStopBits {
    One,
    OnePointFive,
    Two,
};

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

PCWSTR StringFromSerialParity (SerialParity Parity)
{
    switch (Parity) {
    case SerialParity::None: return L"none";
    case SerialParity::Odd: return L"odd";
    case SerialParity::Even: return L"even";
    case SerialParity::Mark: return L"mark";
    case SerialParity::Space: return L"space";
    default: return L"[invalid parity]";
    }
}

PCWSTR StringFromSerialStopBits (SerialStopBits StopBits)
{
    switch (StopBits) {
    case SerialStopBits::One: return L"1";
    case SerialStopBits::OnePointFive: return L"1.5";
    case SerialStopBits::Two: return L"2";
    default: return L"[invalid serial stop bits]";
    }
}

std::wstring GetFirstDevice ()
{
    ULONG length;
    CONFIGRET cr = CM_Get_Device_Interface_List_SizeW(
            &length,
            const_cast<GUID*>(&GUID_DEVINTERFACE_COMPORT),
            nullptr,        // pDeviceID
            CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

    if (cr != CR_SUCCESS) {
        throw wexception(
            L"Failed to get size of device interface list. (cr = 0x%x)",
            cr);
    }

    std::vector<WCHAR> buf(length);
    cr = CM_Get_Device_Interface_ListW(
            const_cast<GUID*>(&GUID_DEVINTERFACE_COMPORT),
            nullptr,        // pDeviceID
            buf.data(),
            buf.capacity(),
            CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

    if (cr != CR_SUCCESS) {
        throw wexception(
            L"Failed to get device interface list. (cr = 0x%x)",
            cr);
    }

    return std::wstring(buf.data());
}

void ListDevices ()
{
    ULONG length;
    CONFIGRET cr = CM_Get_Device_Interface_List_SizeW(
            &length,
            const_cast<GUID*>(&GUID_DEVINTERFACE_COMPORT),
            nullptr,        // pDeviceID
            CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

    if ((cr != CR_SUCCESS) || (length == 0)) {
        throw wexception(
            L"Failed to get size of device interface list. "
            L"(length = %lu, cr = 0x%x)",
            length,
            cr);
    }

    std::vector<WCHAR> buf(length);
    cr = CM_Get_Device_Interface_ListW(
            const_cast<GUID*>(&GUID_DEVINTERFACE_COMPORT),
            nullptr,        // pDeviceID
            buf.data(),
            buf.size(),
            CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

    if ((cr != CR_SUCCESS) || (length == 0)) {
        throw wexception(
            L"Failed to get device interface list. (length = %lu, cr = 0x%x)",
            length,
            cr);
    }

    if (!buf[0]) {
        wprintf(L"No serial devices were found.\n");
        return;
    }

    *buf.rbegin() = UNICODE_NULL;

    ULONG index = 0;
    for (PCWSTR deviceInterface = buf.data();
        *deviceInterface;
        deviceInterface += wcslen(deviceInterface) + 1) {

        const DEVPROPKEY propkey = {
            PKEY_DeviceInterface_Serial_PortName.fmtid,
            PKEY_DeviceInterface_Serial_PortName.pid
        };
        DEVPROPTYPE propertyType;
        WCHAR portName[512];
        ULONG propertyBufferSize = sizeof(portName);
        cr = CM_Get_Device_Interface_PropertyW(
                deviceInterface,
                &propkey,
                &propertyType,
                reinterpret_cast<BYTE*>(&portName),
                &propertyBufferSize,
                0); // ulFlags

        if ((cr == CR_SUCCESS) && (propertyType == DEVPROP_TYPE_STRING)) {
            wprintf(L" %lu (%s): %s\n", index, portName, deviceInterface);
        } else {
            wprintf(L" %lu: %s\n", index, deviceInterface);
        }

        ++index;
    }
}

void ReadConsoleWriteSerial (HANDLE serialHandle)
{
    HANDLE stdIn = GetStdHandle(STD_INPUT_HANDLE);
    if (!SetConsoleMode(stdIn, ENABLE_PROCESSED_INPUT)) {
        throw wexception(
            L"Failed to set console mode for stdin. (GetLastError() = 0x%x)",
            GetLastError());
    }

    Event overlappedEvent(CreateEventW(NULL, TRUE, FALSE, NULL));
    if (!overlappedEvent.IsValid()) {
        throw wexception(
            L"Failed to create event for overlapped IO. "
            L"(GetLastError() = 0x%x)",
            GetLastError());
    }

    for (;;) {
        WCHAR wbuf[512];
        DWORD charsRead;
        if (!ReadConsole(
                stdIn,
                wbuf,
                ARRAYSIZE(wbuf),
                &charsRead,
                NULL)) {

            throw wexception(
                L"Failed to read console. (GetLastError = 0x%x)",
                GetLastError());
        }

        // convert to ANSI
        CHAR buf[ARRAYSIZE(wbuf)];
        int bytesConverted = WideCharToMultiByte(
                CP_ACP,
                0,          // dwFlags
                wbuf,
                charsRead,
                buf,
                sizeof(buf),
                NULL,       // lpDefaultChar
                NULL);      // lpUsedDefaultChar
        if (!bytesConverted) {
            throw wexception(
                L"Failed to convert wide string to ANSI. "
                L"(GetLastError() = 0x%x)",
                GetLastError());
        }


        auto overlapped = OVERLAPPED();
        overlapped.hEvent = overlappedEvent.Get();

        DWORD bytesWritten;
        if (!WriteFile(
                serialHandle,
                buf,
                bytesConverted,
                &bytesWritten,
                &overlapped) && (GetLastError() != ERROR_IO_PENDING)) {

            throw wexception(
                L"Write to serial device failed. (GetLastError() = 0x%x)",
                GetLastError());
        }

        if (!GetOverlappedResult(
                serialHandle,
                &overlapped,
                &bytesWritten,
                TRUE)) {

            throw wexception(
                L"GetOverlappedResult() for SetCommTimeouts() failed. "
                L"(GetLastError() = 0x%x)",
                GetLastError());
        }
    }
}

const WCHAR* FindSpecialChar (
    _In_reads_(Length) const WCHAR* Buf,
    ULONG LengthInChars,
    _Out_ bool *Found
    )
{
    const WCHAR* ch = Buf;
    while (ch != (Buf + LengthInChars)) {
        switch (*ch) {
        case L'\r':
        case L'\b':
            *Found = true;
            return ++ch;
        }
        ++ch;
    }

    *Found = false;
    return ch;
}

void ReadSerialWriteConsole (HANDLE serialHandle)
{
    HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!SetConsoleMode(
        stdOut,
        ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT)) {

        throw wexception(
            L"Failed to set console mode for stdout. GetLastError() = 0x%x",
            GetLastError());
    }

    Event overlappedEvent(CreateEventW(NULL, TRUE, FALSE, NULL));
    if (!overlappedEvent.IsValid()) {
        throw wexception(
            L"Failed to create event for overlapped IO. "
            L"(GetLastError() = 0x%x)",
            GetLastError());
    }

    for (;;) {
        auto overlapped = OVERLAPPED();
        overlapped.hEvent = overlappedEvent.Get();

        CHAR buf[512];
        DWORD bytesRead = 0;
        if (!ReadFile(
                serialHandle,
                reinterpret_cast<BYTE*>(buf),
                sizeof(buf),
                &bytesRead,
                &overlapped) && (GetLastError() != ERROR_IO_PENDING)) {

            throw wexception(
                L"Failed to read from serial device. (GetLastError = 0x%x)",
                GetLastError());
        }

        if (!GetOverlappedResult(
                serialHandle,
                &overlapped,
                &bytesRead,
                TRUE)) {

            throw wexception(
                L"GetOverlappedResult() for ReadFile() failed. "
                L"(GetLastError() = 0x%x)",
                GetLastError());
        }

        WCHAR wbuf[ARRAYSIZE(buf)];
        int charsConverted = MultiByteToWideChar(
                CP_ACP,
                MB_PRECOMPOSED,
                buf,
                bytesRead,
                wbuf,
                ARRAYSIZE(wbuf));
        if (!charsConverted) {
            throw wexception(
                L"Failed to convert string to unicode. (GetLastError() = 0x%x)",
                GetLastError());
        }

        const WCHAR* chunkBegin = wbuf;
        const WCHAR* const bufEnd = wbuf + charsConverted;
        do {
            // scan for cairrage returns and backspaces
            bool specialCharFound;
            const WCHAR* chunkEnd = FindSpecialChar(
                    chunkBegin,
                    bufEnd - chunkBegin,
                    &specialCharFound);

            DWORD charsWritten;
            if (!WriteConsole(
                    stdOut,
                    chunkBegin,
                    chunkEnd - chunkBegin,
                    &charsWritten,
                    NULL)) {

                throw wexception(
                    L"WriteConsole failed. (GetLastError() = 0x%x)",
                    GetLastError());
            }

            // Handle special character if one was found. The special
            // character is the last character of the chunk.
            if (specialCharFound)
            {
                switch (chunkEnd[-1]) {
                case '\r':
                {
                    const WCHAR lf = L'\n';
                    if (!WriteConsole(stdOut, &lf, 1, &charsWritten, NULL)) {
                        throw wexception(
                            L"WriteConsole failed. (GetLastError() = 0x%x)",
                            GetLastError());
                    }
                    break;
                }
                case '\b':
                {
                    const WCHAR backspace[] = {L' ', L'\b'};
                    if (!WriteConsole(
                            stdOut,
                            &backspace,
                            ARRAYSIZE(backspace),
                            &charsWritten,
                            NULL)) {

                        throw wexception(
                            L"WriteConsole failed. (GetLastError() = 0x%x)",
                            GetLastError());
                    }
                    break;
                }
                } // switch
            }

            chunkBegin = chunkEnd;
        } while (chunkBegin != bufEnd);
    }
}

DWORD CALLBACK ReadSerialThreadProc (PVOID Param)
{
    try {
        ReadSerialWriteConsole(static_cast<HANDLE>(Param));
    } catch (const wexception& ex) {
        std::wcerr << L"Error: " << ex.wwhat() << L"\n";
        return 1;
    }

    return 0;
}

void RunSerialConsole (PCWSTR DevicePath, _In_opt_ PCWSTR DcbString)
{
    DCB dcb;
    if (DcbString) {
        if (!BuildCommDCB(DcbString, &dcb)) {
            throw wexception(
                L"Failed to build device control block. "
                L"(DcbString = '%s', GetLastError() = 0x%x)",
                DcbString,
                GetLastError());
        }
    }

    wprintf(L"Opening port '%s'\n", DevicePath);

    FileHandle serialHandle(CreateFileW(
        DevicePath,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL));
    if (!serialHandle.IsValid()) {
        throw wexception(
            L"Failed to open handle to serial device. "
            L"(DevicePath = %s, GetLastError() = 0x%x)",
            DevicePath,
            GetLastError());
    }

    if (!SetupComm(serialHandle.Get(), 512, 512)) {
        throw wexception(
            L"SetupComm() failed. (GetLastError() = 0x%x)",
            GetLastError());
    }

    if (DcbString) {
        if (!SetCommState(serialHandle.Get(), &dcb)) {
            throw wexception(
                L"SetCommState() failed. Failed to set device to desired "
                L"configuration. (GetLastError() = 0x%x)",
                GetLastError());
        }
    }

    auto commTimeouts = COMMTIMEOUTS();
    {
        commTimeouts.ReadIntervalTimeout = MAXDWORD;
        commTimeouts.ReadTotalTimeoutConstant = MAXDWORD - 1;
        commTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
        commTimeouts.WriteTotalTimeoutConstant = 0;
        commTimeouts.WriteTotalTimeoutMultiplier = 0;
    }

    if (!SetCommTimeouts(serialHandle.Get(), &commTimeouts)) {
        throw wexception(
            L"SetCommTimeouts failed. (GetLastError() = 0x%x)",
            GetLastError());
    }

    DCB actualDcb;
    if (!GetCommState(serialHandle.Get(), &actualDcb)) {
        throw wexception(
            L"Failed to get current comm state. (GetLastError() = 0x%x)",
            GetLastError());
    }

    wprintf(
        L"        baud = %d\n"
        L"      parity = %s\n"
        L"   data bits = %d\n"
        L"   stop bits = %s\n",
        actualDcb.BaudRate,
        StringFromSerialParity(SerialParity(actualDcb.Parity)),
        actualDcb.ByteSize,
        StringFromSerialStopBits(SerialStopBits(actualDcb.StopBits)));

    wprintf(L" =====================   Connected - hit ctrl+c to exit   =====================\n");

    ThreadHandle thread(CreateThread(
        NULL,
        0,
        ReadSerialThreadProc,
        serialHandle.Get(),
        0,
        NULL));
    if (!thread.IsValid()) {
        throw wexception(
            L"Failed to create ReadSerialThreadProc thread. "
            L"(GetLastError() = 0x%x)",
            GetLastError());
    }

    ReadConsoleWriteSerial(serialHandle.Get());
}

PCWSTR Help =
L"MinComm: Serial Terminal for OneCore\n"
L"\n"
L"Usage: %s [-list] device_path [baud=<B>] [parity=<P>] [data=<D>] [stop=<S>] [xon={on|off}] [odsr={on|off}] [octs={on|off}] [dtr={on|off|hs}] [rts={on|off|hs|tg}] [idsr={on|off}]\n"
L"\n"
L"  -list                List all available serial ports on the system and exit.\n"
L"  device_path          Device path or COM port to open (e.g. COM1)\n"
L"  baud=<B>             Specifies the transmission rate in bits per second.\n"
L"  parity={n|e|o|m|s}   Specifies how the system uses the parity bit to check\n"
L"                       for transmission errors. The abbreviations stand for\n"
L"                       none, even, odd, mark, and space.\n"
L"                       The default value is e.\n"
L"  data={5|6|7|8}       Specifies the number of data bits in a character.\n"
L"                       The default value is 7.\n"
L"  stop={1|1.5|2}       Specifies the number of stop bits that define the end of\n"
L"                       a character. If the baud rate is 110, the default value\n"
L"                       is 2. Otherwise, the default value is 1.\n"
L"  xon={on|off}         Specifies whether the xon or xoff protocol for data-flow\n"
L"                       control is on or off.\n"
L"  odsr={on|off}        Specifies whether output handshaking that uses the\n"
L"                       Data Set Ready (DSR) circuit is on or off.\n"
L"  octs={on|off}        Specifies whether output handshaking that uses the\n"
L"                       Clear To Send (CTS) circuit is on or off.\n"
L"  dtr={on|off|hs}      Specifies whether the Data Terminal Ready (DTR) circuit\n"
L"                       is on or off or set to handshake.\n"
L"  rts={on|off|hs|tg}   Specifies whether the Request To Send (RTS) circuit is\n"
L"                       set to on, off, handshake, or toggle.\n"
L"  idsr={on|off}        Specifies whether the DSR circuit sensitivity is on\n"
L"                       or off.\n"
L"\n"
L"See documentation for the Mode command on Technet for more information on the\n"
L"connection parameters:\n"
L"  https://technet.microsoft.com/en-us/library/cc732236.aspx)\n"
L"\n"
L"Examples:\n"
L"  Connect to the first serial port found in the port's default configuration:\n"
L"    %s\n"
L"\n"
L"  List all serial ports on the system:\n"
L"    %s -list\n"
L"\n"
L"  Open COM1 in 115200 8N1 configuration:\n"
L"    %s COM1 baud=115200 parity=n data=8 stop=1\n"
L"\n"
L"  Open COM1 in 115200 8N1 configuration:\n"
L"    %s \\\\.\\COM1 baud=115200 parity=n data=8 stop=1\n"
L"\n"
L"  Open device interface in 115200 8N1 configuration:\n"
L"    %s \\\\?\\USB#VID_FFFF&PID_0005#{86e0d1e0-8089-11d0-9ce4-08003e301f73} baud=115200 parity=n data=8 stop=1\n";

int __cdecl wmain (int argc, _In_reads_(argc) const wchar_t* argv[])
{
    std::wstring devicePath;
    std::wstring dcbString;

    if (argc < 2) {
        // connect to the first serial port found
        devicePath = GetFirstDevice();
        if (devicePath.empty()) {
            fwprintf(
                stderr,
                L"No serial devices were found.\n");
            return 1;
        }
    } else if (!_wcsicmp(argv[1], L"-?") || !_wcsicmp(argv[1], L"/?") ||
        !_wcsicmp(argv[1], L"-h") || !_wcsicmp(argv[1], L"/h") ||
        !_wcsicmp(argv[1], L"-help") || !_wcsicmp(argv[1], L"/help")) {

        wprintf(Help, argv[0], argv[0], argv[0], argv[0], argv[0], argv[0]);
        return 0;
    } else if (!_wcsicmp(argv[1], L"-list") || !_wcsicmp(argv[1], L"/list")) {
        try {
            ListDevices();
        } catch (const wexception& ex) {
            std::wcerr << L"Error: " << ex.wwhat() << L"\n";
            return 1;
        }
        return 0;
    } else {
        // first positional parameter is device path
        devicePath = argv[1];

        // combine the rest of the parameters into a DCB string
        for (int i = 2; i < argc; ++i) {
            dcbString += argv[i];
            dcbString += L" ";
        }
    }

    try {
        RunSerialConsole(
            devicePath.c_str(),
            dcbString.empty() ? nullptr : dcbString.c_str());
    } catch (const wexception& ex) {
        std::wcerr << L"Error: " << ex.wwhat() << L"\n";
        return 1;
    }

    return 0;
}
