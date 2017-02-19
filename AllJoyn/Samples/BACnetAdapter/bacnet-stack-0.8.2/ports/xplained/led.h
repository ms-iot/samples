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
#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <stdbool.h>

#define LED_RS485_RX 0
#define LED_RS485_TX 1
#define LED_APDU 2
#define LED_DEBUG 3

#define LEDS_MAX 4


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CONF_BOARD_ENABLE_RS485_XPLAINED
    void led_on(
        uint8_t index);
    void led_on_interval(
        uint8_t index,
        uint16_t interval_ms);
    void led_off(
        uint8_t index);
    void led_off_delay(
        uint8_t index,
        uint32_t delay_ms);
    void led_toggle(
        uint8_t index);
    bool led_state(
        uint8_t index);
    void led_task(
        void);
    void led_init(
        void);
#else
    /* dummy stubs */
	#define led_on(x)
    #define led_on_interval(x, ms)
    #define led_off(x)
    #define led_off_delay(x, ms)
    #define led_toggle(x)
    #define led_state(x)
    #define led_task()
    #define led_init()
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
