//
// Copyright (c) Microsoft. All rights reserved.

// Utility to manipulate GPIO resources from a command shell.
//
// This utility requires the gpiokmdfdemo driver to be
// installed. The driver carries out all operations on behalf
// of this usermode executable.
//

#include <stdio.h>
#include "utils.h"
#include "mbm.h"
#include "rpi2.h"
#include "sbc.h"

// Platform code
DWORD g_dwPlatformNameCode = PLATFORM_NAME_UNDEFINED;

// Platform name
WCHAR *g_platformName = NULL;

// Converts a string to unsigned long 
HRESULT StringToULong(_In_ PCWSTR Str, _Out_ ULONG *Value)
{
    PWSTR endPtr;
    PCWSTR end = Str + wcslen(Str);

    *Value = wcstoul(Str, &endPtr, 0);
    return (endPtr == end) ? S_OK : E_INVALIDARG;
}

// Shows the help
HRESULT Help(_In_ int Argc, _In_reads_(Argc) wchar_t *Argv[])
{
    UNREFERENCED_PARAMETER(Argc);
    UNREFERENCED_PARAMETER(Argv);

    // Print the help text
    wprintf(HelpText);

    if (NULL != g_platformName)
        wprintf(L"PATFORM: %s\n", g_platformName);

    if (g_dwPlatformNameCode == PLATFORM_NAME_RPI2)
        wprintf(RPi2_PinMappingText);

    if (g_dwPlatformNameCode == PLATFORM_NAME_MBM)
        wprintf(MBM_PinMappingText);

    if (g_dwPlatformNameCode == PLATFORM_NAME_SBC)
        wprintf(SBC_PinMappingText);

    if (g_dwPlatformNameCode == PLATFORM_NAME_UNDEFINED)
        wprintf(L"No Pin Mapping Available For This Platform\n"); 

    return S_OK;
}

// Gets the platform name
bool GetPlatformName()
{
    bool fRet = false;
    HKEY key = NULL;
    WCHAR biosRegKey[34] = L"hardware\\description\\system\\bios\\";
    WCHAR subKey[18] = L"SystemProductName";
    DWORD size = 0;

    // Try and open the registry key
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, biosRegKey, 0, KEY_READ, &key))
    {
        wprintf(L"Unable to open reg key: %s\n", biosRegKey);
        RegCloseKey(key);
        return fRet;
    }

    // Try and get the size of the key
    if (ERROR_SUCCESS != RegQueryValueEx(key, subKey, NULL, NULL, NULL, &size))
    {
        wprintf(L"Unable to query reg sub-key %s for its size in bytes\n", subKey);
        RegCloseKey(key);
        return fRet;
    }

    // Create a array big enough for the name
    if (NULL == (g_platformName = (WCHAR*)malloc(size)))
    {
        wprintf(L"Unable to allocate memory to store the platform name in memory\n");
        RegCloseKey(key);
        return fRet;
    }

    // Retrieve the name key from the registry
    if (ERROR_SUCCESS != RegQueryValueEx(key, subKey, NULL, NULL, (LPBYTE)g_platformName, &size))
    {
        wprintf(L"Unable to read value of reg sub-key %s in the registry\n", subKey);
        RegCloseKey(key);
        return fRet;
    }

    fRet = true;

    // Assign the platform name 
    if (0 == wcscmp(g_platformName, L"Raspberry Pi 2 Model B"))
    {
        g_dwPlatformNameCode = PLATFORM_NAME_RPI2;
    }

    if (0 == wcscmp(g_platformName, L"Minnowboard Max B3 PLATFORM"))
    {
        g_dwPlatformNameCode = PLATFORM_NAME_MBM;
    }

    if (0 == wcscmp(g_platformName, L"SBC"))
    {
        g_dwPlatformNameCode = PLATFORM_NAME_SBC;
    }

    // Close the registry key
    RegCloseKey(key);
    return fRet;
}
