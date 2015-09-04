//
// Copyright (c) Microsoft. All rights reserved.

// Utility to manipulate GPIO resources from a command shell.
//
// This utility requires the gpiokmdfdemo driver to be
// installed. The driver carries out all operations on behalf
// of this usermode executable.
//

#pragma once

#define RPI2_GPIO_PIN_COUNT 15

// Pin Mapping for the RPi2
static const WCHAR RPi2_PinMappingText[] =
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
