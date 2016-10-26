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

#include "BlinkyApp.h"
#include "utils.h"
#include "rpi2.h"
#include "mbm.h"
#include "sbc.h"

// Platform name code, used to dynamically set up the correct pin-mapping
extern DWORD g_dwPlatformNameCode;

// Global handle to the device driver
static HANDLE hDev = INVALID_HANDLE_VALUE;

// Main entry function
int wmain(_In_ int argc, _In_reads_(argc) wchar_t *argv[])
{
    // Local variables
    bool fCommandRecognized  = false; 
    const wchar_t *cwCommand = nullptr;

    // Get the platform name
    // Used to set up the correct GPIO pin mapping
    GetPlatformName();

    // Get the input from the user
    if (argc < 2)
    {
        //Show help
        Help(argc, argv);

        // Exit
        return 1;
    }

	// Open handle to the device driver
	hDev = CreateFileW(GPOT_DEVICE_PATH, GENERIC_ALL, 0,  nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    // Check if we got a good handle to the device driver
	if (INVALID_HANDLE_VALUE == hDev)
	{
        wprintf(L"\n");
		wprintf(L"Error: Failed to open handle to driver (%s): 0x%x\n", GPOT_DEVICE_PATH, GetLastError());
        wprintf(L"       Verify that the sample driver gpiokmdfdemo.sys is installed and running without issues.\n");
        wprintf(L"       To verify, type: devcon status ACPI\\GPOT0001\n");
        wprintf(L"\n");

        // Exit
		return 1;
	}

    // Get the command from the user input (high/low/help)
    cwCommand = argv[1];
	for (int i = 0; i < ARRAYSIZE(Commands); ++i)
	{
		if (0 == _wcsicmp(cwCommand, Commands[i].CmdName))
		{
            fCommandRecognized = true;
			Commands[i].CmdFunction(argc, argv);
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

// Sets GPIO High 
static HRESULT GpioHigh(_In_ int Argc, _In_reads_(Argc) wchar_t *Argv[])
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

    // Open the resource for output
    if (!DeviceIoControl(hDev, IOCTL_GPOT_OPEN_OUTPUT, &pinIndex, sizeof(pinIndex), nullptr, 0, &bytesReturned, nullptr))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"\n");
        wprintf(L"Failed to open pin as output: 0x%x\n", hr);
        wprintf(L"\n");
        goto end;
    }

    // Send IOCTL command to turn GPIO high
    if (!DeviceIoControl(hDev, IOCTL_GPOT_WRITE_HIGH, &pinIndex, sizeof(pinIndex), nullptr, 0, &bytesReturned, nullptr))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"\n");
        wprintf(L"Failed to write pin high: 0x%x\n", hr);
        wprintf(L"\n");
        goto end;
    }

    // Close resource
    if (!DeviceIoControl(hDev, IOCTL_GPOT_CLOSE, &pinIndex, sizeof(pinIndex), nullptr, 0, &bytesReturned, nullptr))
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
static HRESULT GpioLow(_In_ int Argc, _In_reads_(Argc) wchar_t *Argv[])
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

    // Open the resource for output
    if (!DeviceIoControl(hDev, IOCTL_GPOT_OPEN_OUTPUT, &pinIndex, sizeof(pinIndex), nullptr, 0, &bytesReturned, nullptr))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"\n");
        wprintf(L"Failed to open pin as output: 0x%x\n", hr);
        wprintf(L"\n");
        goto end;
    }

    // Send IOCTL command to turn GPIO low
    if (!DeviceIoControl(hDev, IOCTL_GPOT_WRITE_LOW, &pinIndex, sizeof(pinIndex), nullptr, 0, &bytesReturned, nullptr))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        wprintf(L"\n");
        wprintf(L"Failed to write pin high: 0x%x\n", hr);
        wprintf(L"\n");
        goto end;
    }

    // Close resource
    if (!DeviceIoControl(hDev, IOCTL_GPOT_CLOSE, &pinIndex, sizeof(pinIndex), nullptr, 0, &bytesReturned, nullptr))
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

// Gets the driver pin index based on the GPIO number
static ULONG GetPinIndex(ULONG GPIO_NUM)
{
    INT iIndex = 0;

    // MBM
    if (g_dwPlatformNameCode == PLATFORM_NAME_MBM)
    {
        for (iIndex = 0; iIndex < MBM_GPIO_PIN_COUNT; iIndex++)
        {
            if (MBM_PIN_MATRIX[iIndex] == GPIO_NUM)
                break;
        }

        if (iIndex >= MBM_GPIO_PIN_COUNT)
            iIndex = -1;
    }

    // RPi2
    if (g_dwPlatformNameCode == PLATFORM_NAME_RPI2)
    {
        for (iIndex = 0; iIndex < RPI2_GPIO_PIN_COUNT; iIndex++)
        {
            if (RPI2_PIN_MATRIX[iIndex] == GPIO_NUM)
                break;
        }

        if (iIndex >= RPI2_GPIO_PIN_COUNT)
            iIndex = -1;
    }

    // SBC
    if (g_dwPlatformNameCode == PLATFORM_NAME_SBC)
    {
        for (iIndex = 0; iIndex < SBC_GPIO_PIN_COUNT; iIndex++)
        {
            if (SBC_PIN_MATRIX[iIndex] == GPIO_NUM)
                break;
        }

        if (iIndex >= SBC_GPIO_PIN_COUNT)
            iIndex = -1;
    }

    // Undefined
    if (g_dwPlatformNameCode == PLATFORM_NAME_UNDEFINED)
        iIndex = -1;

    return iIndex;
}
