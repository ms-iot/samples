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
*********************************************************************/
#include <stdint.h>
#include "hardware.h"
#include "timer.h"
#include "led.h"

#ifndef BDK_VERSION
#define BDK_VERSION 4
#endif

static struct itimer Off_Delay_Timer[MAX_LEDS];

/*************************************************************************
* Description: Turn on an LED
* Returns: none
* Notes: none
*************************************************************************/
void led_on(
    uint8_t index)
{
    switch (index) {
        case 0:
            BIT_SET(PORTD, PD7);
            break;
        case 1:
            BIT_SET(PORTD, PD6);
            break;
        case 2:
#if (BDK_VERSION==4)
            BIT_SET(PORTB, PB0);
#else
            BIT_SET(PORTC, PC7);
#endif
            break;
        case 3:
#if (BDK_VERSION==4)
            BIT_SET(PORTB, PB4);
#else
            BIT_SET(PORTC, PC6);
#endif
            break;
        default:
            break;
    }
    if (index < MAX_LEDS) {
        timer_interval_no_expire(&Off_Delay_Timer[index]);
    }
}

/*************************************************************************
* Description: Turn off an LED
* Returns: none
* Notes: none
*************************************************************************/
void led_off(
    uint8_t index)
{
    switch (index) {
        case 0:
            BIT_CLEAR(PORTD, PD7);
            break;
        case 1:
            BIT_CLEAR(PORTD, PD6);
            break;
        case 2:
#if (BDK_VERSION==4)
            BIT_CLEAR(PORTB, PB0);
#else
            BIT_CLEAR(PORTC, PC7);
#endif
            break;
        case 3:
#if (BDK_VERSION==4)
            BIT_CLEAR(PORTB, PB4);
#else
            BIT_CLEAR(PORTC, PC6);
#endif
            break;
        default:
            break;
    }
    if (index < MAX_LEDS) {
        timer_interval_no_expire(&Off_Delay_Timer[index]);
    }
}

/*************************************************************************
* Description: Get the state of the LED
* Returns: true if on, false if off.
* Notes: none
*************************************************************************/
bool led_state(
    uint8_t index)
{
    switch (index) {
        case 0:
            return (BIT_CHECK(PIND, PIND7));
        case 1:
            return (BIT_CHECK(PIND, PIND6));
        case 2:
#if (BDK_VERSION==4)
            return (BIT_CHECK(PINB, PINC0));
#else
            return (BIT_CHECK(PINC, PINC7));
#endif
        case 3:
#if (BDK_VERSION==4)
            return (BIT_CHECK(PINB, PINC4));
#else
            return (BIT_CHECK(PINC, PINC6));
#endif
        default:
            break;
    }

    return false;
}

/*************************************************************************
* Description: Toggle the state of the setup LED
* Returns: none
* Notes: none
*************************************************************************/
void led_toggle(
    uint8_t index)
{
    if (led_state(index)) {
        led_off(index);
    } else {
        led_on(index);
    }
}

/*************************************************************************
* Description: Delay before going off to give minimum brightness.
* Returns: none
* Notes: none
*************************************************************************/
void led_off_delay(
    uint8_t index,
    uint32_t delay_ms)
{
    if (index < MAX_LEDS) {
        timer_interval_start(&Off_Delay_Timer[index], delay_ms);
    }
}

/*************************************************************************
* Description: Turn on, and delay before going off.
* Returns: none
* Notes: none
*************************************************************************/
void led_on_interval(
    uint8_t index,
    uint16_t interval_ms)
{
    if (index < MAX_LEDS) {
        led_on(index);
        timer_interval_start(&Off_Delay_Timer[index], interval_ms);
    }
}

/*************************************************************************
* Description: Task for blinking LED
* Returns: none
* Notes: none
*************************************************************************/
void led_task(
    void)
{
    uint8_t i;  /* loop counter */

    for (i = 0; i < MAX_LEDS; i++) {
        if (timer_interval_expired(&Off_Delay_Timer[i])) {
            timer_interval_no_expire(&Off_Delay_Timer[i]);
            led_off(i);
        }
    }
}

/*************************************************************************
* Description: Initialize the LED hardware
* Returns: none
* Notes: none
*************************************************************************/
void led_init(
    void)
{
    uint8_t i;  /* loop counter */

    /* configure the port pins as outputs */
    BIT_SET(DDRD, DDD7);
    BIT_SET(DDRD, DDD6);
#if (BDK_VERSION==4)
    BIT_SET(DDRB, DDB0);
    BIT_SET(DDRB, DDB4);
#else
    BIT_SET(DDRC, DDC7);
    BIT_SET(DDRC, DDC6);
#endif
    for (i = 0; i < MAX_LEDS; i++) {
        led_on_interval(i, 500);
    }
}
