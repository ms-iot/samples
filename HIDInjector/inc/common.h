/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    common.h

Environment:

    User mode

--*/

#pragma once

#include <pshpack1.h>

//
// input from device to system - DO NOT modify this structure without changing the HID descriptor in the driver!
//
typedef struct _HIDINJECTOR_INPUT_REPORT {
    unsigned char ReportId;   

	union {
		struct {
			UCHAR Modifiers;
			UCHAR Key1;
			UCHAR Key2;
			UCHAR Key3;
			UCHAR Key4;
			UCHAR Padding[2]; // Padding to keep the reports the same size
		} KeyReport;
		struct {
			UCHAR Buttons;
			SHORT AbsoluteX;
			SHORT AbsoluteY;
			UCHAR Padding[2]; // Padding to keep the reports the same size
		} MouseReport;
		struct {
			UCHAR Flags;  // Bit 0 = Tip Switch, Bit 1 = In Range, Bit 2-7 = unused
			UCHAR ContactIndentifier;
			USHORT AbsoluteX;
			USHORT AbsoluteY;
			UCHAR ContactCount;
		} TouchReport;
	} Report;
} HIDINJECTOR_INPUT_REPORT, *PHIDINJECTOR_INPUT_REPORT;

// Values for ReportId
#define KEYBOARD_REPORT_ID		1
#define MOUSE_REPORT_ID			2
#define TOUCH_REPORT_ID			3
#define MAX_COUNT_REPORT_ID		4

// Values for KeyReport.Modifiers from the HID Spec
#define KEBBOARD_LEFT_CONTROL	0x01
#define KEYBOARD_LEFT_SHIFT		0x02
#define KEYBOARD_LEFT_ALT		0x04
#define KEYBOARD_LEFT_GUI		0x08
#define KEBBOARD_RIGHT_CONTROL	0x10
#define KEYBOARD_RIGHT_SHIFT	0x20
#define KEYBOARD_RIGHT_ALT		0x40
#define KEYBOARD_RIGHT_GUI		0x80

// Values for MouseReport.Buttons from the HID Spec
#define MOUSE_BUTTON_1			0x01
#define MOUSE_BUTTON_2			0x02
#define MOUSE_BUTTON_3			0x04

// Values for TouchReport.Flags from the HID Spec
#define TOUCH_TIP_SWITCH		0x01
#define TOUCH_IN_RANGE			0x02
#define TOUCH_MAX_FINGER		0x0a // 10
#define TOUCH_PHYSICAL_MAX		0x7FFF	//Physical maximum given for the X and Y usage at the touch part of the HID_REPORT_DESCRIPTOR at the driver.

#include <poppack.h>

