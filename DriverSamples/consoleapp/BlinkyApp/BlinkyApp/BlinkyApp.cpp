//
// Copyright (c) Microsoft. All rights reserved.

// Utility to manipulate GPIO resources from a command shell.
//
// This utility requires the gpiokmdfdemo driver to be
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

// Supported platforms used for GPIO pin mappings
#define PLATFORM_UNDEFINED 0
#define PLATFORM_ARM 1
#define PLATFORM_X86 2

// GPIO pin count for supported platforms
#define MBM_GPIO_PIN_COUNT 10
#define RPI2_GPIO_PIN_COUNT 15

// Global handle to the device driver
static HANDLE hDev = INVALID_HANDLE_VALUE;

// Pin Mapping for the MBM
static const WCHAR MBM_PinMapping[] =
L"\n"
L"Minnow Board Max (MBM) [x86]: GPIO Pin Mapping and Examples \n"
L"\n"
L"  GPIO No. |      Example      |      Example      | Header  \n"
L"           |     (GPIO low)    |     (GPIO high)   | Pin No. \n"
L"  GPIO  0  | BlinkyApp.exe l 0 | BlinkyApp.exe h 0 |   21    \n"
L"  GPIO  1  | BlinkyApp.exe l 1 | BlinkyApp.exe h 1 |   23    \n"
L"  GPIO  2  | BlinkyApp.exe l 2 | BlinkyApp.exe h 2 |   25    \n"
L"  GPIO  3  | BlinkyApp.exe l 3 | BlinkyApp.exe h 3 |   14    \n"
L"  GPIO  4  | BlinkyApp.exe l 4 | BlinkyApp.exe h 4 |   16    \n"
L"  GPIO  5  | BlinkyApp.exe l 5 | BlinkyApp.exe h 5 |   18    \n"
L"  GPIO  6  | BlinkyApp.exe l 6 | BlinkyApp.exe h 6 |   20    \n"
L"  GPIO  7  | BlinkyApp.exe l 7 | BlinkyApp.exe h 7 |   22    \n"
L"  GPIO  8  | BlinkyApp.exe l 8 | BlinkyApp.exe h 8 |   24    \n"
L"  GPIO  9  | BlinkyApp.exe l 9 | BlinkyApp.exe h 9 |   26    \n"
L"\n"
;

// Pin matrix matching MBM GPIO pins to ACPI table index
static const INT MBM_PIN_MATRIX[MBM_GPIO_PIN_COUNT] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }; 

// Pin Mapping for the RPi2
static const WCHAR RPi2_PinMapping[] =
L"\n"
L"Raspberry Pi 2 (RPi2) [ARM]: GPIO Pin Mapping and Examples \n"
L"\n"
L"  GPIO No. |      Example       |      Example       | Header  \n"
L"           |     (GPIO low)     |     (GPIO high)    | Pin No. \n"
L"  GPIO  4  | BlinkyApp.exe l  4 | BlinkyApp.exe h  4 |    7    \n"
L"  GPIO  5  | BlinkyApp.exe l  5 | BlinkyApp.exe h  5 |   29    \n"
L"  GPIO  6  | BlinkyApp.exe l  6 | BlinkyApp.exe h  6 |   31    \n"
L"  GPIO 12  | BlinkyApp.exe l 12 | BlinkyApp.exe h 12 |   32    \n"
L"  GPIO 13  | BlinkyApp.exe l 13 | BlinkyApp.exe h 13 |   33    \n"
L"  GPIO 16  | BlinkyApp.exe l 16 | BlinkyApp.exe h 16 |   36    \n"
L"  GPIO 18  | BlinkyApp.exe l 18 | BlinkyApp.exe h 18 |   12    \n"
L"  GPIO 22  | BlinkyApp.exe l 22 | BlinkyApp.exe h 22 |   15    \n"
L"  GPIO 23  | BlinkyApp.exe l 23 | BlinkyApp.exe h 23 |   16    \n"
L"  GPIO 24  | BlinkyApp.exe l 24 | BlinkyApp.exe h 24 |   18    \n"
L"  GPIO 25  | BlinkyApp.exe l 25 | BlinkyApp.exe h 25 |   22    \n"
L"  GPIO 26  | BlinkyApp.exe l 26 | BlinkyApp.exe h 26 |   37    \n"
L"  GPIO 27  | BlinkyApp.exe l 27 | BlinkyApp.exe h 27 |   13    \n"
L"\n"
;

// Pin matrix matching RPi2 GPIO pin to ACPI table index
static const INT RPI2_PIN_MATRIX[RPI2_GPIO_PIN_COUNT] = { 4, 5, 6, 12, 13, 16, 18, 22, 23, 24, 25, 26, 27, 35, 47 };

// Help text
static const WCHAR HelpText[] =
L"\n"
L"BlinkyApp: GPIO app tool\n"
L"\n"
L"Commands:\n"
L"\n"
L"  help      : Prints this help message. \n"
L"  (l)ow  5  : Sets GPIO #5 LOW.  \n"
L"  (h)igh 5  : Sets GPIO #5 HIGH. \n"
L"\n"
;

static DWORD dwPlatformArch = PLATFORM_UNDEFINED;

static
HRESULT
GetPlatformArch(_In_ int Argc,
    _In_reads_(Argc) wchar_t *Argv[])
{
    UNREFERENCED_PARAMETER(Argc);
    UNREFERENCED_PARAMETER(Argv);

    WCHAR wProcArch[6] = { 0 };
    DWORD dwRet = 0;

    // Get the device architecture string
    dwRet = GetEnvironmentVariableW(L"PROCESSOR_ARCHITECTURE", wProcArch, 6);

    // Check if the environment variable is found
    if (dwRet == 0)
    {
        wprintf(L"\n");
        wprintf(L"Unable to determine device architecture based on environment variable 'PROCESSOR_ARCHITECTURE' \n");
        wprintf(L"GetLastError() = %d\n", GetLastError());
        wprintf(L"\n");
        return S_FALSE;
    }

    // Check which architecture
    if (dwRet == 3)
    {
        // Compare the architectures
        if (0 == wcscmp(wProcArch, L"ARM"))
        {
            dwPlatformArch = PLATFORM_ARM;
            return S_OK;
        }

        if (0 == wcscmp(wProcArch, L"x86"))
        {
            dwPlatformArch = PLATFORM_X86;
            return S_OK;
        }
    }

    wprintf(L"\n");
    wprintf(L"Platform Architecture Not Supported Yet \n");
    wprintf(L"No Gpio Pin Mapping May Be Available \n");
    wprintf(L"\n");

    return S_FALSE;
}

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

    // Print the help text
    wprintf(HelpText);

    if (dwPlatformArch == PLATFORM_ARM)
        wprintf(RPi2_PinMapping);

    if (dwPlatformArch == PLATFORM_X86)
        wprintf(MBM_PinMapping);

    if (dwPlatformArch == PLATFORM_UNDEFINED)
        wprintf(L"No Pin Mapping Available \n");

    return S_OK;
}

static
ULONG
GetPinIndex(ULONG GPIO_NUM)
{
    INT iIndex;

    if (dwPlatformArch == PLATFORM_X86)
    {
        for (iIndex = 0; iIndex < MBM_GPIO_PIN_COUNT; iIndex++)
        {
            if (MBM_PIN_MATRIX[iIndex] == GPIO_NUM)
                break;
        }

        if (iIndex >= MBM_GPIO_PIN_COUNT)
            iIndex = -1;
    }

    if (dwPlatformArch == PLATFORM_ARM)
    {
        for (iIndex = 0; iIndex < RPI2_GPIO_PIN_COUNT; iIndex++)
        {
            if (RPI2_PIN_MATRIX[iIndex] == GPIO_NUM)
                break;
        }

        if (iIndex >= RPI2_GPIO_PIN_COUNT)
            iIndex = -1;
    }

    if (dwPlatformArch == PLATFORM_UNDEFINED)
        iIndex = -1;

    return iIndex;
}

// Sets GPIO High 
static
HRESULT
GpioHigh(_In_ int Argc,
         _In_reads_(Argc) wchar_t *Argv[])
{
	HRESULT hr;
	DWORD   bytesReturned;
    ULONG   pinIndex;
    ULONG   GpioPin;


    // Get the ulong value for the gpio pin from the user
    hr = StringToULong(Argv[2], &GpioPin);
    if (FAILED(hr))
    {
        wprintf(L"Invalid argument %s\n", Argv[2]);
        goto end;
    }

    // Get the corresponding acpi table index based on the gpio pin number
    if (-1 == (pinIndex = GetPinIndex(GpioPin)))
    {
        wprintf(L"\n");
        wprintf(L"Invalid Gpio Pin: %s\n", Argv[2]);
        wprintf(L"\n");
        goto end;
    }

    // Open resource for output
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_OPEN_OUTPUT, 
                         &pinIndex,
                         sizeof(pinIndex),
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"\n");
		wprintf(L"Failed to open pin as output: 0x%x\n", hr);
        wprintf(L"\n");
		goto end;
	}

	// Send IOCTL command to turn GPIO high
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_WRITE_HIGH, 
                         &pinIndex,
                         sizeof(pinIndex),
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"\n");
		wprintf(L"Failed to write pin high: 0x%x\n", hr);
        wprintf(L"\n");
		goto end;
	}
	
	// Close resource
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_CLOSE, 
                         &pinIndex,
                         sizeof(pinIndex),
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"\n");
		wprintf(L"Failed to close pin: 0x%x\n", hr);
        wprintf(L"\n");
		goto end;
	}

	hr = S_OK;

end:
	return hr;
}

// Sets GPIO low
static
HRESULT
GpioLow(_In_ int Argc,
        _In_reads_(Argc) wchar_t *Argv[])
{
	HRESULT hr;
	DWORD   bytesReturned;
    ULONG   pinIndex;
    ULONG   GpioPin;

    // Get the integer value for the pin
    hr = StringToULong(Argv[2], &GpioPin);
    if (FAILED(hr))
    {
        wprintf(L"\n");
        wprintf(L"Invalid argument %s\n", Argv[2]);
        wprintf(L"\n");
        goto end;
    }

    // Get the corresponding acpi table index based on the gpio pin number
    if (-1 == (pinIndex = GetPinIndex(GpioPin)))
    {
        wprintf(L"\n");
        wprintf(L"Invalid Gpio Pin: %s\n", Argv[2]);
        wprintf(L"\n");
        goto end;
    }

    // Open resource for output
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_OPEN_OUTPUT, 
                         &pinIndex,
                         sizeof(pinIndex),
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"\n");
		wprintf(L"Failed to open pin as output: 0x%x\n", hr);
        wprintf(L"\n");
		goto end;
	}
	
	// Send IOCTL command to turn GPIO low
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_WRITE_LOW, 
                         &pinIndex,
                         sizeof(pinIndex),
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"\n");
		wprintf(L"Failed to write pin high: 0x%x\n", hr);
        wprintf(L"\n");
		goto end;
	}
	
    // Close resource
	if (!DeviceIoControl(hDev, 
                         IOCTL_GPOT_CLOSE, 
                         &pinIndex,
                         sizeof(pinIndex),
                         nullptr, 
                         0, 
                         &bytesReturned, 
                         nullptr))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"\n");
		wprintf(L"Failed to close pin: 0x%x\n", hr);
        wprintf(L"\n");
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

    // Get the platform architecture
    GetPlatformArch(argc, argv);

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
        wprintf(L"\n");
		wprintf(L"Error: Failed to open handle to driver (%s): 0x%x\n", GPOT_DEVICE_PATH, GetLastError());
        wprintf(L"       Verify that the sample driver gpiokmdfdemo.sys is installed and running without issues.\n");
        wprintf(L"       To verify, type: devcon status ACPI\\GPOT0001\n");
        wprintf(L"\n");
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
        wprintf(L"\n");
		wprintf(L"Command not recognized: %s\n", cwCommand);
        wprintf(L"\n");
		Help(argc, argv);
	}
	
	CloseHandle(hDev);

	return 0;
}

