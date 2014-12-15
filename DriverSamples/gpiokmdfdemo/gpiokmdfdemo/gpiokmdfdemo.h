/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

    The MIT License(MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

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

