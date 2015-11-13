//
// Copyright (c) Microsoft. All rights reserved.

// Utility to manipulate GPIO resources from a command shell.
//
// This utility requires the gpiokmdfdemo driver to be
// installed. The driver carries out all operations on behalf
// of this usermode executable.
//

#pragma once

#define MBM_GPIO_PIN_COUNT 10

// Pin Mapping for the MBM
static const WCHAR MBM_PinMappingText[] =
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
