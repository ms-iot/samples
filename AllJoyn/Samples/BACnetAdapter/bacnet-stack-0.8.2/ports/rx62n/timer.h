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
#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

/* Timer Module */

/* elapsed timer structure */
struct etimer {
    uint32_t start;
};
/* interval timer structure */
struct itimer {
    uint32_t start;
    uint32_t interval;
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /* these 3 functions are created in the hardware specific module */
    void timer_init(
        void);
    uint32_t timer_milliseconds(
        void);
    uint8_t timer_milliseconds_byte(
        void);

    /* these functions are in the generic timer.c module */

    /* elapsed timer */
    void timer_elapsed_start(
        struct etimer *t);
    void timer_elapsed_start_offset(
        struct etimer *t,
        uint32_t offset);
    uint32_t timer_elapsed_time(
        struct etimer *t);
    bool timer_elapsed_milliseconds(
        struct etimer *t,
        uint32_t value);
    bool timer_elapsed_seconds(
        struct etimer *t,
        uint32_t value);
    bool timer_elapsed_minutes(
        struct etimer *t,
        uint32_t value);
    bool timer_elapsed_milliseconds_short(
        struct etimer *t,
        uint16_t value);
    bool timer_elapsed_seconds_short(
        struct etimer *t,
        uint16_t value);
    bool timer_elapsed_minutes_short(
        struct etimer *t,
        uint16_t value);

    /* interval timer */
    void timer_interval_start(
        struct itimer *t,
        uint32_t interval);
    void timer_interval_start_seconds(
        struct itimer *t,
        uint32_t interval);
    void timer_interval_start_minutes(
        struct itimer *t,
        uint32_t interval);
    bool timer_interval_expired(
        struct itimer *t);
    uint32_t timer_interval(
        struct itimer *t);
    uint32_t timer_interval_elapsed(
        struct itimer *t);
    void timer_interval_no_expire(
        struct itimer *t);
    void timer_interval_reset(
        struct itimer *t);
    void timer_interval_restart(
        struct itimer *t);

    /* special for 8-bit microcontrollers - limited to 255ms */
    uint8_t timer_milliseconds_delta(
        uint8_t start);
    uint8_t timer_milliseconds_mark(
        void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
