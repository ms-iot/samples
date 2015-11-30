// Copyright (c) Microsoft. All rights reserved.

#include <windows.h>
#include <cfgmgr32.h>
#include <propkey.h>
#include <strsafe.h>
#include <wrl.h>

#include <vector>
#include <string>
#include <sstream>
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

struct SerialParamMask {
    ULONG BaudSet : 1;
    ULONG ParitySet : 1;
    ULONG DataLengthSet : 1;
    ULONG StopBitsSet : 1;
    ULONG XonSet : 1;
    ULONG OdsrSet : 1;
    ULONG OctsSet : 1;
    ULONG DtrSet : 1;
    ULONG RtsSet : 1;
    ULONG IdsrSet : 1;
};

class wexception
{
public:
    explicit wexception (const std::wstring& Msg) : msg(Msg) { }

    wexception (const wchar_t* Msg, ...)
    {
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

PCWSTR StringFromDtrControl (DWORD DtrControl)
{
    switch (DtrControl) {
    case DTR_CONTROL_ENABLE: return L"on";
    case DTR_CONTROL_DISABLE: return L"off";
    case DTR_CONTROL_HANDSHAKE: return L"handshake";
    default: return L"[invalid DtrControl value]";
    }
}

PCWSTR StringFromRtsControl (DWORD RtsControl)
{
    switch (RtsControl) {
    case RTS_CONTROL_ENABLE: return L"on";
    case RTS_CONTROL_DISABLE: return L"off";
    case RTS_CONTROL_HANDSHAKE: return L"handshake";
    case RTS_CONTROL_TOGGLE: return L"toggle";
    default: return L"[invalid RtsControl value]";
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
            static_cast<ULONG>(buf.capacity()),
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
            static_cast<ULONG>(buf.size()),
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

            DWORD error = GetLastError();
            if (error == ERROR_OPERATION_ABORTED) {
                return; // error is due to application exit
            }

            throw wexception(
                L"Failed to read from serial device. (GetLastError = 0x%x)",
                error);
        }

        if (!GetOverlappedResult(
                serialHandle,
                &overlapped,
                &bytesRead,
                TRUE)) {

            DWORD error = GetLastError();
            if (error == ERROR_OPERATION_ABORTED) {
                return; // error is due to application exit
            }

            throw wexception(
                L"GetOverlappedResult() for ReadFile() failed. "
                L"(GetLastError() = 0x%x)",
                error);
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
                    static_cast<ULONG>(bufEnd - chunkBegin),
                    &specialCharFound);

            DWORD charsWritten;
            if (!WriteConsole(
                    stdOut,
                    chunkBegin,
                    static_cast<ULONG>(chunkEnd - chunkBegin),
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


//
// Parse serial connection parameters. BuildCommDCB is not available on onecore.
//
bool
ParseConnectionParams (
    int argc,
    _In_reads_(argc) const wchar_t* argv[],
    _Out_ DCB* DcbPtr,
    _Out_ SerialParamMask* MaskPtr
    )
{
    *MaskPtr = SerialParamMask();
    DcbPtr->DCBlength = sizeof(*DcbPtr);

    for (int i = 0; i < argc; ++i) {
        std::wstring param(argv[i]);
        std::wstring::size_type found = param.find_first_of(L'=');
        if (found == param.npos) {
            fwprintf(stderr, L"Expecting '=': %s", argv[i]);
            return false;
        }

        if ((found + 1) >= param.length()) {
            fwprintf(stderr, L"Expecting value after '=': %s", argv[i]);
            return false;
        }

        std::wstring name = param.substr(0, found);
        std::wstring value = param.substr(found + 1);

        if (name == L"baud") {
            // baud = <B>
            std::wistringstream stream(value);
            stream >> DcbPtr->BaudRate;
            if (stream.fail()) {
                fwprintf(stderr, L"Expecting integer value for baud: %s", param.c_str());
                return false;
            }

            MaskPtr->BaudSet = true;
        } else if (name == L"parity") {
            // parity={n|e|o|m|s}
            if (value.length() != 1) {
                fwprintf(stderr, L"Expecting n|e|o|m|s for parity: %s", param.c_str());
                return false;
            }

            SerialParity parity;
            switch (value[0]) {
            case L'n':
                parity = SerialParity::None;
                break;
            case L'e':
                parity = SerialParity::Even;
                break;
            case L'o':
                parity = SerialParity::Odd;
                break;
            case L'm':
                parity = SerialParity::Mark;
                break;
            case L's':
                parity = SerialParity::Space;
                break;
            default:
                fwprintf(stderr, L"Expecting n|e|o|m|s for parity: %s", param.c_str());
                return false;
            }

            DcbPtr->Parity = BYTE(parity);
            DcbPtr->fParity = TRUE;
            MaskPtr->ParitySet = true;
        } else if (name == L"data") {
            // data={5|6|7|8}
            if (value.length() != 1) {
                fwprintf(stderr, L"Expecting 5|6|7|8 for data length: %s", param.c_str());
                return false;
            }

            switch (value[0]) {
            case L'5':
            case L'6':
            case L'7':
            case L'8':
                break;
            default:
                fwprintf(stderr, L"Expecting 5|6|7|8 for data length: %s", param.c_str());
                return false;
            }

            DcbPtr->ByteSize = BYTE(value[0] - L'0');
            MaskPtr->DataLengthSet = true;
        } else if (name == L"stop") {
            // stop={1|1.5|2}
            SerialStopBits stopBits;
            if (value == L"1") {
                stopBits = SerialStopBits::One;
            } else if (value == L"1.5") {
                stopBits = SerialStopBits::OnePointFive;
            } else if (value == L"2") {
                stopBits = SerialStopBits::Two;
            } else {
                fwprintf(stderr, L"Expecting 1|1.5|2 for stop bits: %s", param.c_str());
                return false;
            }

            DcbPtr->StopBits = BYTE(stopBits);
            MaskPtr->StopBitsSet = true;
        } else if (name == L"xon") {
            // xon={on|off}
            if (value == L"on") {
                DcbPtr->fInX = TRUE;
                DcbPtr->fOutX = TRUE;
            } else if (value == L"off") {
                DcbPtr->fInX = FALSE;
                DcbPtr->fOutX = FALSE;
            } else {
                fwprintf(stderr, L"Expecting on|off for xon: %s", param.c_str());
                return false;
            }

            MaskPtr->XonSet = true;
        } else if (name == L"odsr") {
            // odsr={on|off}
            if (value == L"on") {
                DcbPtr->fOutxDsrFlow = TRUE;
            } else if (value == L"off") {
                DcbPtr->fOutxDsrFlow = FALSE;
            } else {
                fwprintf(stderr, L"Expecting on|off for odsr: %s", param.c_str());
                return false;
            }

            MaskPtr->OdsrSet = true;
        } else if (name == L"octs") {
            // octs={on|off}
            if (value == L"on") {
                DcbPtr->fOutxCtsFlow = TRUE;
            } else if (value == L"off") {
                DcbPtr->fOutxCtsFlow = FALSE;
            } else {
                fwprintf(stderr, L"Expecting on|off for octs: %s", param.c_str());
                return false;
            }

            MaskPtr->OctsSet = true;
        } else if (name == L"dtr") {
            // dtr={on|off|hs}
            if (value == L"on") {
                DcbPtr->fDtrControl = DTR_CONTROL_ENABLE;
            } else if (value == L"off") {
                DcbPtr->fDtrControl = DTR_CONTROL_DISABLE;
            } else if (value == L"hs") {
                DcbPtr->fDtrControl = DTR_CONTROL_HANDSHAKE;
            } else {
                fwprintf(stderr, L"Expecting on|off|hs for dtr: %s", param.c_str());
                return false;
            }

            MaskPtr->DtrSet = true;
        } else if (name == L"rts") {
            // rts={on|off|hs|tg}
            if (value == L"on") {
                DcbPtr->fRtsControl = RTS_CONTROL_ENABLE;
            } else if (value == L"off") {
                DcbPtr->fRtsControl = RTS_CONTROL_DISABLE;
            } else if (value == L"hs") {
                DcbPtr->fRtsControl = RTS_CONTROL_HANDSHAKE;
            } else if (value == L"tg") {
                DcbPtr->fRtsControl = RTS_CONTROL_TOGGLE;
            } else {
                fwprintf(stderr, L"Expecting on|off|hs|tg for rts: %s", param.c_str());
                return false;
            }

            MaskPtr->RtsSet = true;
        } else if (name == L"idsr") {
            // idsr={on|off}
            if (value == L"on") {
                DcbPtr->fDsrSensitivity = TRUE;
            } else if (value == L"off") {
                DcbPtr->fDsrSensitivity = FALSE;
            } else {
                fwprintf(stderr, L"Expecting on|off for idsr: %s", param.c_str());
                return false;
            }

            MaskPtr->IdsrSet = true;
        } else {
            fwprintf(stderr, L"Unrecognized parameter: %s", param.c_str());
            return false;
        }
    }

    return true;
}

void RunSerialConsole (PCWSTR DevicePath, _In_ DCB* DcbPtr, SerialParamMask ParamMask)
{
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

    DCB dcb;
    if (!GetCommState(serialHandle.Get(), &dcb)) {
        throw wexception(
                L"GetCommState() failed. Failed to get current device "
                L"configuration. (GetLastError() = 0x%x)",
                GetLastError());
    }

    if (ParamMask.BaudSet) {
        dcb.BaudRate = DcbPtr->BaudRate;
    }

    if (ParamMask.ParitySet) {
        dcb.Parity = DcbPtr->Parity;
        dcb.fParity = DcbPtr->fParity;
    }

    if (ParamMask.DataLengthSet) {
        dcb.ByteSize = DcbPtr->ByteSize;
    }

    if (ParamMask.StopBitsSet) {
        dcb.StopBits = DcbPtr->StopBits;
    }

    if (ParamMask.XonSet) {
        dcb.fInX = DcbPtr->fInX;
        dcb.fOutX = DcbPtr->fOutX;
    }

    if (ParamMask.OdsrSet) {
        dcb.fOutxDsrFlow = DcbPtr->fOutxDsrFlow;
    }

    if (ParamMask.OctsSet) {
        dcb.fOutxCtsFlow = DcbPtr->fOutxCtsFlow;
    }

    if (ParamMask.DtrSet) {
        dcb.fDtrControl = DcbPtr->fDtrControl;
    }

    if (ParamMask.RtsSet) {
        dcb.fRtsControl = DcbPtr->fRtsControl;
    }

    if (ParamMask.IdsrSet) {
        dcb.fDsrSensitivity = DcbPtr->fDsrSensitivity;
    }

    if (!SetCommState(serialHandle.Get(), &dcb)) {
        throw wexception(
            L"SetCommState() failed. Failed to set device to desired "
            L"configuration. (GetLastError() = 0x%x)",
            GetLastError());
    }

    auto commTimeouts = COMMTIMEOUTS();
    {
        commTimeouts.ReadIntervalTimeout = 10;
        commTimeouts.ReadTotalTimeoutConstant = 0;
        commTimeouts.ReadTotalTimeoutMultiplier = 0;
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
        L"                    baud = %d\n"
        L"                  parity = %s\n"
        L"               data bits = %d\n"
        L"               stop bits = %s\n"
        L"   XON/XOFF flow control = %s\n"
        L" output DSR flow control = %s\n"
        L" output CTS flow control = %s\n"
        L"             DTR control = %s\n"
        L"             RTS control = %s\n"
        L" DSR circuit sensitivity = %s\n",
        actualDcb.BaudRate,
        StringFromSerialParity(SerialParity(actualDcb.Parity)),
        actualDcb.ByteSize,
        StringFromSerialStopBits(SerialStopBits(actualDcb.StopBits)),
        (actualDcb.fInX && actualDcb.fOutX) ? L"on" : L"off",
        actualDcb.fOutxDsrFlow ? L"on" : L"off",
        actualDcb.fOutxCtsFlow ? L"on" : L"off",
        StringFromDtrControl(actualDcb.fDtrControl),
        StringFromRtsControl(actualDcb.fRtsControl),
        actualDcb.fDsrSensitivity ? L"on" : L"off");

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
L"  data={5|6|7|8}       Specifies the number of data bits in a character.\n"
L"  stop={1|1.5|2}       Specifies the number of stop bits that define the end of\n"
L"                       a character.\n"
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
L"Parameters that are not specified will default to the port's current\n"
L"configuration. For more information on the connection parameters, see the\n"
L"Technet documentation for the Mode command:\n"
L"  https://technet.microsoft.com/en-us/library/cc732236.aspx\n"
L"\n"
L"Examples:\n"
L"  Connect to the first serial port found in the port's current configuration:\n"
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

    auto paramMask = SerialParamMask();
    auto dcb = DCB();
    dcb.DCBlength = sizeof(DCB);

    if (argc < 2) {
        // connect to the first serial port found in the port's
        // current configuration
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
        // if the first positional parameter contains an '=',
        // take the first enumerated device.
        int optind;
        devicePath = argv[1];
        if (devicePath.find_first_of(L'=') == devicePath.npos) {
            optind = 2;
        } else {
            optind = 1;
            devicePath = GetFirstDevice();
        }

        if (!ParseConnectionParams(
                argc - optind,
                argv + optind,
                &dcb,
                &paramMask)) {

            return 1;
        }
    }

    try {
        RunSerialConsole(devicePath.c_str(), &dcb, paramMask);
    } catch (const wexception& ex) {
        std::wcerr << L"Error: " << ex.wwhat() << L"\n";
        return 1;
    }

    return 0;
}
