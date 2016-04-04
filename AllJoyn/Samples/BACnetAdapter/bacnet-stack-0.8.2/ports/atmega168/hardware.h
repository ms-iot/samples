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
#define F_CPU 7372800UL
#endif

#if defined(__IAR_SYSTEMS_ICC__) || defined(__IAR_SYSTEMS_ASM__)
#include <iom168.h>
#else
#if !defined(__AVR_ATmega168__)
#error Firmware is configured for ATmega168 only (-mmcu=atmega168)
#endif
#endif
#include "iar2gcc.h"
#include "avr035.h"

#define LED_NPDU_INIT() BIT_SET(DDRD, DDD5)
#define LED_NPDU_ON() BIT_CLEAR(PORTD, PD5)
#define LED_NPDU_OFF() BIT_SET(PORTD, PD5)
/* #define LED_NPDU PORTD_Bit5 */
/* #define LED_NPDU_OFF() {LED_NPDU = false;} */
/* #define LED_NPDU_ON() {LED_NPDU = true;} */

#define LED_GREEN_INIT() BIT_SET(DDRD, DDD4)
#define LED_GREEN_ON() BIT_CLEAR(PORTD, PD4)
#define LED_GREEN_OFF() BIT_SET(PORTD, PD4)

#endif
