//
// Copyright (c) Microsoft. All rights reserved.

// Utility to manipulate GPIO resources from a command shell.
//
// This utility requires the gpiokmdfdemo driver to be
// installed. The driver carries out all operations on behalf
// of this usermode executable.
//

#pragma once

#include <Windows.h>

// Platforms
#define PLATFORM_NAME_UNDEFINED 0
#define PLATFORM_NAME_RPI2 1
#define PLATFORM_NAME_MBM 2
#define PLATFORM_NAME_SBC 3

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

HRESULT Help           (_In_ int Argc, _In_reads_(Argc) wchar_t *Argv[]);
HRESULT GetPlatformArch(_In_ int Argc, _In_reads_(Argc) wchar_t *Argv[]);
HRESULT StringToULong  (_In_ PCWSTR Str, _Out_ ULONG *Value);
bool    GetPlatformName();
