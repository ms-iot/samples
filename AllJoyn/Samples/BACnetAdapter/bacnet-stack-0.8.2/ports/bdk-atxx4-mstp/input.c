/**************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
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
#include <stdint.h>
#include <stdbool.h>
#include "hardware.h"
#include "timer.h"
/* me */
#include "input.h"

#ifndef BDK_VERSION
#define BDK_VERSION 4
#endif

static uint8_t Address_Switch;
static uint8_t Buttons;
static struct itimer Debounce_Timer;

#ifndef BDK_V1_HACK
#define BDK_V1_HACK 0
#endif

#if BDK_V1_HACK
/* version 1 BDK workaournd for floating inputs */
static void input_switch_workaround(
    void)
{
    /* configure the port pins for the switch - as outputs */
    BIT_SET(DDRA, DDA0);
    BIT_SET(DDRA, DDA1);
    BIT_SET(DDRA, DDA2);
    BIT_SET(DDRA, DDA3);
    BIT_SET(DDRA, DDA4);
    BIT_SET(DDRA, DDA5);
    BIT_SET(DDRA, DDA6);
    /* turn off the outputs */
    BIT_CLEAR(PORTA, PORTA0);
    BIT_CLEAR(PORTA, PORTA1);
    BIT_CLEAR(PORTA, PORTA2);
    BIT_CLEAR(PORTA, PORTA3);
    BIT_CLEAR(PORTA, PORTA4);
    BIT_CLEAR(PORTA, PORTA5);
    BIT_CLEAR(PORTA, PORTA6);
    /* configure the port pins for the switch - as inputs */
    BIT_CLEAR(DDRA, DDA0);
    BIT_CLEAR(DDRA, DDA1);
    BIT_CLEAR(DDRA, DDA2);
    BIT_CLEAR(DDRA, DDA3);
    BIT_CLEAR(DDRA, DDA4);
    BIT_CLEAR(DDRA, DDA5);
    BIT_CLEAR(DDRA, DDA6);

    return;
}
#endif

/* debounce the inputs */
void input_task(
    void)
{
    uint8_t value;
    static uint8_t old_address = 0;
    static uint8_t old_buttons = 0;

    /* only check the inputs every debounce time */
    if (timer_interval_expired(&Debounce_Timer)) {
        timer_interval_reset(&Debounce_Timer);
        /* pins used are PA6, PA5, PA4, PA3, PA2, PA1, PA0 */
#if BDK_V1_HACK
        /* version 1 BDK - workaround */
        value = (PINA & 0x7F);
#else
        /* version 2 BDK - has inverted inputs */
        value = ~PINA;
        value &= 0x7F;
#endif
        if (value == old_address) {
            /* stable value */
            Address_Switch = old_address;
        }
        old_address = value;
#if (BDK_VERSION==4)
        /* pins used are PB3, PB2, PB1 */
        value = BITMASK_CHECK(PINB, 0x0E);
        value >>= 1;
#else
        /* pins used are PB4, PB3, PB2, PB1, PB0 */
        value = BITMASK_CHECK(PINB, 0x1F);
#endif
        if (value == old_buttons) {
            /* stable value */
            Buttons = old_buttons;
        }
        old_buttons = value;
    }
#if BDK_V1_HACK
    input_switch_workaround();
#endif
}

uint8_t input_address(
    void)
{
    return Address_Switch;
}

uint8_t input_rotary_value(
    uint8_t index)
{
    return Buttons;
}

bool input_button_value(
    uint8_t index)
{
    bool value = false;

    switch (index) {
        case 0:
            value = BIT_CHECK(Buttons, 0);
            break;
        case 1:
            value = BIT_CHECK(Buttons, 1);
            break;
        case 2:
            value = BIT_CHECK(Buttons, 2);
            break;
        case 3:
            value = BIT_CHECK(Buttons, 3);
            break;
        case 4:
            value = BIT_CHECK(Buttons, 4);
            break;
        default:
            break;
    }

    return value;
}


void input_init(
    void)
{
    /* configure the port pins for the switch */
    BIT_CLEAR(DDRA, DDA0);
    BIT_CLEAR(DDRA, DDA1);
    BIT_CLEAR(DDRA, DDA2);
    BIT_CLEAR(DDRA, DDA3);
    BIT_CLEAR(DDRA, DDA4);
    BIT_CLEAR(DDRA, DDA5);
    BIT_CLEAR(DDRA, DDA6);
    /* activate the internal pull up resistors */
    BIT_SET(PORTA, PORTA0);
    BIT_SET(PORTA, PORTA1);
    BIT_SET(PORTA, PORTA2);
    BIT_SET(PORTA, PORTA3);
    BIT_SET(PORTA, PORTA4);
    BIT_SET(PORTA, PORTA5);
    BIT_SET(PORTA, PORTA6);
    /* configure the port pins for rotary switch inputs */
#if (BDK_VERSION==4)
    BIT_CLEAR(DDRB, DDB1);
    BIT_CLEAR(DDRB, DDB2);
    BIT_CLEAR(DDRB, DDB3);
    /* activate the internal pull up resistors */
    BIT_SET(PORTB, PORTB1);
    BIT_SET(PORTB, PORTB2);
    BIT_SET(PORTB, PORTB3);
#else
    BIT_CLEAR(DDRB, DDB1);
    BIT_CLEAR(DDRB, DDB2);
    BIT_CLEAR(DDRB, DDB3);
    BIT_CLEAR(DDRB, DDB4);
    /* activate the internal pull up resistors */
    BIT_SET(PORTB, PORTB1);
    BIT_SET(PORTB, PORTB2);
    BIT_SET(PORTB, PORTB3);
    BIT_SET(PORTB, PORTB4);
#endif
    timer_interval_start(&Debounce_Timer, 30);
}
