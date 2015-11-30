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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void led_ld3_on(
        void);
    void led_ld4_on(
        void);
    void led_ld3_off(
        void);
    void led_ld4_off(
        void);
    bool led_ld3_state(
        void);
    void led_ld3_toggle(
        void);

    void led_tx_on(
        void);
    void led_rx_on(
        void);

    void led_tx_on_interval(
        uint16_t interval_ms);
    void led_rx_on_interval(
        uint16_t interval_ms);

    void led_tx_off(
        void);
    void led_rx_off(
        void);

    void led_tx_off_delay(
        uint32_t delay_ms);
    void led_rx_off_delay(
        uint32_t delay_ms);

    void led_tx_toggle(
        void);
    void led_rx_toggle(
        void);

    bool led_tx_state(
        void);
    bool led_rx_state(
        void);

    void led_task(
        void);
    void led_init(
        void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
