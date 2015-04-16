//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Utility to manipulate GPIO resources from an interactive
// command shell.
//
// This utility requires the gpiokmdfdemo.sys driver to be
// installed. The driver carries out all operations on behalf
// of this usermode executable.
//

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory>

// Device driver name and path
#define GPOT_DEVICE_NAME L"GpioKmdfDemo"
#define GPOT_DEVICE_PATH L"\\\\.\\" GPOT_DEVICE_NAME

// CTL_CODE device type
#define FILE_DEVICE_GPOT 0xffffUL

// Device driver IOCTL codes
#define IOCTL_GPOT_OPEN_OUTPUT CTL_CODE(FILE_DEVICE_GPOT, 0x101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GPOT_CLOSE       CTL_CODE(FILE_DEVICE_GPOT, 0x102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GPOT_WRITE_LOW   CTL_CODE(FILE_DEVICE_GPOT, 0x103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GPOT_WRITE_HIGH  CTL_CODE(FILE_DEVICE_GPOT, 0x104, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Global handle to the device driver
static HANDLE hDev = INVALID_HANDLE_VALUE;

// Help text
static const WCHAR HelpText[] =
L"\n"
L"BlinkyApp: Interactive GPIO app demo tool\n"
L"Commands:\n"
L"\n"
L"  help                            Prints this help message.\n"
L"  (l)ow 2                         Writes GPIO 5 LOW.\n"
L"  (h)igh 2                        Writes GPIO 5 HIGH.\n"
L"\n"
L"Example:\n"
L"\n"
L"  BlinkyApp.exe low 2\n"
L"  BlinkyApp.exe high 2\n"
L"\n"
L"\n"
L"------------------------------------------------------------\n"
L"\n"
L"Pin mapping for RPi2 from rpi2.asl\n"
L"\n"
L"  GPIO    #   | Parameter |  Example (GPIO low)   |  Example (GPIO high)\n"
L"  GPIO [  0 ]    =  0        BlinkyApp.exe l  0   |  BlinkyApp.exe h  0\n"
L"  GPIO [  1 ]    =  1        BlinkyApp.exe l  1   |  BlinkyApp.exe h  1\n"
L"  GPIO [  5 ]    =  2        BlinkyApp.exe l  2   |  BlinkyApp.exe h  2\n"
L"  GPIO [  6 ]    =  3        BlinkyApp.exe l  3   |  BlinkyApp.exe h  3\n"
L"  GPIO [ 12 ]    =  4        BlinkyApp.exe l  4   |  BlinkyApp.exe h  4\n"
L"  GPIO [ 13 ]    =  5        BlinkyApp.exe l  5   |  BlinkyApp.exe h  5\n"
L"  GPIO [ 16 ]    =  6        BlinkyApp.exe l  6   |  BlinkyApp.exe h  6\n"
L"  GPIO [ 18 ]    =  7        BlinkyApp.exe l  7   |  BlinkyApp.exe h  7\n"
L"  GPIO [ 22 ]    =  8        BlinkyApp.exe l  8   |  BlinkyApp.exe h  8\n"
L"  GPIO [ 23 ]    =  9        BlinkyApp.exe l  9   |  BlinkyApp.exe h  9\n"
L"  GPIO [ 24 ]    = 10        BlinkyApp.exe l 10   |  BlinkyApp.exe h 10\n"
L"  GPIO [ 25 ]    = 11        BlinkyApp.exe l 11   |  BlinkyApp.exe h 11\n"
L"  GPIO [ 26 ]    = 12        BlinkyApp.exe l 12   |  BlinkyApp.exe h 12\n"
L"  GPIO [ 27 ]    = 13        BlinkyApp.exe l 13   |  BlinkyApp.exe h 13\n"
L"  GPIO [ 35 ]    = 14        BlinkyApp.exe l 14   |  BlinkyApp.exe h 14\n"
L"  GPIO [ 47 ]    = 15        BlinkyApp.exe l 15   |  BlinkyApp.exe h 15\n"
L"\n"
;

static
HRESULT
StringToULong(_In_ PCWSTR Str,
              _Out_ ULONG *Value)
{
    PWSTR endPtr;
    PCWSTR end = Str + wcslen(Str);

    *Value = wcstoul(Str, &endPtr, 0);
    return (endPtr == end) ? S_OK : E_INVALIDARG;
}

// Help display
static
HRESULT
Help(_In_ int Argc, 
     _In_reads_(Argc) wchar_t *Argv[])
{
	UNREFERENCED_PARAMETER(Argc);
	UNREFERENCED_PARAMETER(Argv);

	wprintf(HelpText);

	return S_OK;
}

// Sets GPIO_5 high
static
HRESULT
GpioHigh(_In_ int Argc,
         _In_reads_(Argc) wchar_t *Argv[])
{
	HRESULT hr;
	DWORD   bytesReturned;
	ULONG   currentIndex = 2; // GPIO 5 on RPi2


    // Get the integer value for the pin
    hr = StringToULong(Argv[2], &currentIndex);
    if (FAILED(hr))
    {
        wprintf(L"Invalid argument %s\n", Argv[2]);
        goto end;
    }


    // Open resource for output
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_OPEN_OUTPUT, 
                         &currentIndex, 
                         sizeof(currentIndex), 
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		wprintf(L"Failed to open pin as output: 0x%x\n", hr);
		goto end;
	}

	// Send IOCTL command to turn GPIO high
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_WRITE_HIGH, 
                         &currentIndex, 
                         sizeof(currentIndex), 
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		wprintf(L"Failed to write pin high: 0x%x\n", hr);
		goto end;
	}
	
	// Close resource
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_CLOSE, 
                         &currentIndex, 
                         sizeof(currentIndex), 
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		wprintf(L"Failed to close pin: 0x%x\n", hr);
		goto end;
	}

	hr = S_OK;

end:
	return hr;
}

// Sets GPIO 5 low
static
HRESULT
GpioLow(_In_ int Argc,
        _In_reads_(Argc) wchar_t *Argv[])
{
	HRESULT hr;
	DWORD   bytesReturned;
	ULONG   currentIndex = 2; // GPIO 5 on RPi2

    // Get the integer value for the pin
    hr = StringToULong(Argv[2], &currentIndex);
    if (FAILED(hr))
    {
        wprintf(L"Invalid argument %s\n", Argv[2]);
        goto end;
    }

    // Open resource for output
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_OPEN_OUTPUT, 
                         &currentIndex, 
                         sizeof(currentIndex), 
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		wprintf(L"Failed to open pin as output: 0x%x\n", hr);
		goto end;
	}
	
	// Send IOCTL command to turn GPIO low
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_WRITE_LOW, 
                         &currentIndex, 
                         sizeof(currentIndex), 
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		wprintf(L"Failed to write pin high: 0x%x\n", hr);
		goto end;
	}
	
    // Close resource
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_CLOSE, 
                         &currentIndex, 
                         sizeof(currentIndex), 
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		wprintf(L"Failed to close pin: 0x%x\n", hr);
		goto end;
	}

	hr = S_OK;

end:
	return hr;
}

typedef HRESULT(*NewCommandHandler)(int Argc, wchar_t *Argv[]);

struct NewCommand
{
	PCWSTR CmdName;
	NewCommandHandler NewCmdFunction;
};

static const NewCommand NewCommands[]
{
	{ L"help",   Help },
	{ L"high",   GpioHigh },
	{ L"h",      GpioHigh },
	{ L"low",    GpioLow },
	{ L"l",      GpioLow }
};

int
__cdecl
wmain(_In_ int argc,
_In_reads_(argc) wchar_t *argv[]
)
{
    // Local variables
    bool fCommandRecognized  = false; 
    const wchar_t *cwCommand = nullptr;

    // Get the input from the user
    if (argc < 2)
    {
        //Show help
        Help(argc, argv);
        return 1;
    }

	// Open handle to the device driver
	hDev = CreateFileW(GPOT_DEVICE_PATH,
					   GENERIC_ALL,
					   0, 
					   nullptr,
					   OPEN_EXISTING,
					   FILE_ATTRIBUTE_NORMAL,
					   nullptr);

    // Check if we got a good handle to the device driver
	if (hDev == INVALID_HANDLE_VALUE)
	{
		wprintf(L"Error: Failed to open handle to driver (%s): 0x%x\n", GPOT_DEVICE_PATH, GetLastError());
        wprintf(L"       Verify that the sample driver gpiokmdfdemo.sys is installed and running without issues.\n");
        wprintf(L"       To verify, type: devcon status ACPI\\GPOT0001\n");
		return 1;
	}

    // Get command from the user input
    cwCommand = argv[1];
	for (int i = 0; i < ARRAYSIZE(NewCommands); ++i)
	{
		if (0 == _wcsicmp(cwCommand, NewCommands[i].CmdName))
		{
            fCommandRecognized = true;
			NewCommands[i].NewCmdFunction(argc, argv);
		}
	}

    // Print error message if command is not recognized
	if (!fCommandRecognized)
	{
		wprintf(L"Command not recognized: %s\n", cwCommand);
		Help(argc, argv);
	}
	
	CloseHandle(hDev);

	return 0;
}

