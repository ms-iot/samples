//
// Copyright (c) Microsoft. All rights reserved.

// Utility to manipulate GPIO resources from a command shell.
//
// This utility requires the gpiokmdfdemo driver to be
// installed. The driver carries out all operations on behalf
// of this usermode executable.
//

#pragma once

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

static HRESULT GpioHigh   (_In_ int Argc, _In_reads_(Argc) wchar_t *Argv[]);
static HRESULT GpioLow    (_In_ int Argc, _In_reads_(Argc) wchar_t *Argv[]);
extern HRESULT Help       (_In_ int Argc, _In_reads_(Argc) wchar_t *Argv[]);
static ULONG   GetPinIndex(ULONG GPIO_NUM);

typedef HRESULT(*CommandHandler)(int Argc, wchar_t *Argv[]);

struct Command
{
    PCWSTR CmdName;
    CommandHandler CmdFunction;
};

Command Commands[]
{
    { L"help",   Help },
    { L"high",   GpioHigh },
    { L"h",      GpioHigh },
    { L"low",    GpioLow },
    { L"l",      GpioLow }
};
