//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// Module Name:
//
//    gpiokmdfdemo.h
//
// Abstract:
//
//    IOCTL interface to the gpiokmdf driver.
//
// Environment:
//
//    kernel
//

#ifndef _GPIOKMDFDEMO_H_
#define _GPIOKMDFDEMO_H_

#define GPOT_MAX_RESOURCES 128

// Device name and path
#define GPOT_DEVICE_NAME L"GpioKmdfDemo"
#define GPOT_DEVICE_PATH L"\\\\.\\" GPOT_DEVICE_NAME

#define FILE_DEVICE_GPOT 0xffffUL

// Driver IOCTL Codes
#define IOCTL_GPOT_OPEN_OUTPUT             CTL_CODE(FILE_DEVICE_GPOT, 0x101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GPOT_CLOSE                   CTL_CODE(FILE_DEVICE_GPOT, 0x102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GPOT_WRITE_LOW               CTL_CODE(FILE_DEVICE_GPOT, 0x103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_GPOT_WRITE_HIGH              CTL_CODE(FILE_DEVICE_GPOT, 0x104, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Struct to represent open state
typedef enum _GPOT_OPEN_STATE {
	OpenStateNotOpened = 0,
	OpenStateInput,
	OpenStateOutput
} GPOT_OPEN_STATE;

// Struct to represent status of an entry
typedef struct _GPOT_STATUS_ENTRY {
	LARGE_INTEGER ConnectionId;
	GPOT_OPEN_STATE OpenState;
} GPOT_STATUS_ENTRY;

#endif // _GPIOKMDFDEMO_H_

