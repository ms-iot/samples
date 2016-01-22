//
// Copyright (c) Microsoft. All rights reserved.

// Utility to manipulate GPIO resources from a command shell.
//
// This utility requires the gpiokmdfdemo driver to be
// installed. The driver carries out all operations on behalf
// of this usermode executable.
//

#pragma once

#define SBC_GPIO_PIN_COUNT 14

// Pin Mapping for the RPi2
static const WCHAR SBC_PinMappingText[] =
L"\n"
L"Dragon [ARM]: GPIO Pin Mapping and Examples \n"
L"\n"
L"  GPIO No. |      Example        |      Example        | Header  \n"
L"           |     (GPIO low)      |     (GPIO high)     | Pin No. \n"
L"  GPIO  36 | BlinkyApp.exe l  36 | BlinkyApp.exe h  36 |   23    \n"
L"  GPIO  12 | BlinkyApp.exe l  12 | BlinkyApp.exe h  12 |   24    \n"
L"  GPIO  13 | BlinkyApp.exe l  13 | BlinkyApp.exe h  13 |   25    \n"
L"  GPIO  69 | BlinkyApp.exe l  69 | BlinkyApp.exe h  69 |   26    \n"
L"  GPIO 115 | BlinkyApp.exe l 115 | BlinkyApp.exe h 115 |   27    \n"
L"  GPIO  25 | BlinkyApp.exe l  25 | BlinkyApp.exe h  25 |   30    \n"
L"  GPIO  35 | BlinkyApp.exe l  35 | BlinkyApp.exe h  35 |   31    \n"
L"  GPIO  34 | BlinkyApp.exe l  34 | BlinkyApp.exe h  34 |   32    \n"
L"  GPIO  28 | BlinkyApp.exe l  28 | BlinkyApp.exe h  28 |   33    \n"
L"  GPIO  33 | BlinkyApp.exe l  33 | BlinkyApp.exe h  33 |   34    \n"
L"  GPIO  21 | BlinkyApp.exe l  21 | BlinkyApp.exe h  21 |   LED 1 \n"
L"  GPIO 120 | BlinkyApp.exe l 120 | BlinkyApp.exe h 120 |   LED 2 \n"
L"  GPIO   1 | BlinkyApp.exe l   1 | BlinkyApp.exe h   1 |   LED 3 \n"
L"  GPIO   2 | BlinkyApp.exe l   2 | BlinkyApp.exe h   2 |   LED 4 \n"
L"\n"
;

// Pin matrix matching RPi2 GPIO pin to ACPI table index
static const INT SBC_PIN_MATRIX[SBC_GPIO_PIN_COUNT] = { 36, 12, 13, 69, 115, 25, 35, 34, 28, 33, 21, 120, 1, 2 };
