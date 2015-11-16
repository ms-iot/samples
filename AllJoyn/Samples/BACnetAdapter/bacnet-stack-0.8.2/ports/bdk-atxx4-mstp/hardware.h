/**************************************************************************
*
* Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/
#ifndef HARDWARE_H
#define HARDWARE_H

#if !defined(F_CPU)
    /* The processor clock frequency */
#define F_CPU 18432000UL
#endif

/* IAR compiler specific configuration */
#if defined(__ICCAVR__)
#if defined(__ATmega644P__)
#include <iom644p.h>
#endif
#if defined(__ATmega1284P__)
#include <iom1284p.h>
#endif
#endif

/* AVR-GCC compiler specific configuration */
#if defined(__GNUC__)
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#if defined(__AVR_ATmega644P__)
/* defined for ATmega644p */
#elif defined(__AVR_ATmega1284P__)
/* defined for ATmega1284p */
#else
#error For ATmega644P or ATmega1284p only (-mmcu=atmega644p -mmcu=atmega1284p)
#endif
#endif

#if defined (__CROSSWORKS_AVR)
#include <avr.h>
#if (__TARGET_PROCESSOR != ATmega644P)
#error Firmware is configured for ATmega644P only
#endif
#endif

#include "iar2gcc.h"
#include "bits.h"

/* SEEPROM is 24LC128 */
/*#define SEEPROM_PAGE_SIZE 64 */
/*#define SEEPROM_WORD_ADDRESS_16BIT 1 */
/* SEEPROM is 24C16 */
#ifndef SEEPROM_PAGE_SIZE
#define SEEPROM_PAGE_SIZE 16
#endif
#ifndef SEEPROM_WORD_ADDRESS_16BIT
#define SEEPROM_WORD_ADDRESS_16BIT 0
#endif

/* Serial EEPROM address */
#define SEEPROM_I2C_ADDRESS 0xA0
/* Serial EEPROM clocking speed - usually 100000 or 400000 */
#define SEEPROM_I2C_CLOCK 400000L
/* Serial EEPROM max write cycle in milliseconds as defined by datasheet */
#define SEEPROM_WRITE_CYCLE 5

#define LED_2 2
#define LED_3 3
#define LED_4 1
#define LED_5 0
#define MAX_LEDS 4

#endif
