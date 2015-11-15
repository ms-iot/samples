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
#include "board.h"
#include "ioport.h"
#include "timer.h"
#include "led.h"

#ifdef CONF_BOARD_ENABLE_RS485_XPLAINED
#define RS485_XPLAINED_LD1  IOPORT_CREATE_PIN(PORTC, 6)
#define RS485_XPLAINED_LD2  IOPORT_CREATE_PIN(PORTC, 7)
#define RS485_XPLAINED_LD3  IOPORT_CREATE_PIN(PORTC, 4)
#define RS485_XPLAINED_LD4  IOPORT_CREATE_PIN(PORTC, 5)

static struct itimer Off_Delay_Timer[LEDS_MAX];

/*************************************************************************
* Description: Turn on an LED
* Returns: none
* Notes: none
*************************************************************************/
void led_on(uint8_t index)
{
    switch (index) {
        case 0:
            ioport_set_value(RS485_XPLAINED_LD1, 1);
            break;
        case 1:
            ioport_set_value(RS485_XPLAINED_LD2, 1);
            break;
        case 2:
            ioport_set_value(RS485_XPLAINED_LD3, 1);
            break;
        case 3:
            ioport_set_value(RS485_XPLAINED_LD4, 1);
            break;
        default:
            break;
    }
    if (index < LEDS_MAX) {
        timer_interval_infinity(&Off_Delay_Timer[index]);
    }
}

/*************************************************************************
* Description: Turn off an LED
* Returns: none
* Notes: none
*************************************************************************/
void led_off(uint8_t index)
{
    switch (index) {
        case 0:
            ioport_set_value(RS485_XPLAINED_LD1, 0);
            break;
        case 1:
            ioport_set_value(RS485_XPLAINED_LD2, 0);
            break;
        case 2:
            ioport_set_value(RS485_XPLAINED_LD3, 0);
            break;
        case 3:
            ioport_set_value(RS485_XPLAINED_LD4, 0);
            break;
        default:
            break;
    }
    if (index < LEDS_MAX) {
        timer_interval_infinity(&Off_Delay_Timer[index]);
    }
}

/*************************************************************************
* Description: Get the state of the LED
* Returns: true if on, false if off.
* Notes: none
*************************************************************************/
bool led_state(uint8_t index)
{
    switch (index) {
        case 0:
            return ioport_pin_is_high(RS485_XPLAINED_LD1);
        case 1:
            return ioport_pin_is_high(RS485_XPLAINED_LD2);
        case 2:
            return ioport_pin_is_high(RS485_XPLAINED_LD3);
        case 3:
            return ioport_pin_is_high(RS485_XPLAINED_LD4);
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
void led_toggle(uint8_t index)
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
void led_off_delay(uint8_t index,
    uint32_t delay_ms)
{
    if (index < LEDS_MAX) {
        timer_interval_start(&Off_Delay_Timer[index], delay_ms);
    }
}

/*************************************************************************
* Description: Turn on, and delay before going off.
* Returns: none
* Notes: none
*************************************************************************/
void led_on_interval(uint8_t index,
    uint16_t interval_ms)
{
    if (index < LEDS_MAX) {
        led_on(index);
        timer_interval_start(&Off_Delay_Timer[index], interval_ms);
    }
}

/*************************************************************************
* Description: Task for blinking LED
* Returns: none
* Notes: none
*************************************************************************/
void led_task(void)
{
    uint8_t i;  /* loop counter */

    for (i = 0; i < LEDS_MAX; i++) {
        if (timer_interval_expired(&Off_Delay_Timer[i])) {
            timer_interval_infinity(&Off_Delay_Timer[i]);
            led_off(i);
        }
    }
}

/*************************************************************************
* Description: Initialize the LED hardware
* Returns: none
* Notes: none
*************************************************************************/
void led_init(void)
{
    uint8_t i;  /* loop counter */

    /* configure the LEDs for Rx and Tx indication */
    ioport_configure_pin(RS485_XPLAINED_LD1,
        IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
    ioport_configure_pin(RS485_XPLAINED_LD2,
        IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
    ioport_configure_pin(RS485_XPLAINED_LD3,
        IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
    ioport_configure_pin(RS485_XPLAINED_LD4,
        IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
    /* initialize the timers, while giving LEDs a brief test */
    for (i = 0; i < LEDS_MAX; i++) {
        led_on(i);
        led_off_delay(i,500);
    }        
}
#endif
