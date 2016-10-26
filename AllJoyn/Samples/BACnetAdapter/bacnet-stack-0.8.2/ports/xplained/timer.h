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
*********************************************************************/
#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

/* interval and elapsed millisecond timer */
/* interval not to exceed 49.7 days */
#define TIMER_INTERVAL_MAX UINT32_MAX

/* structure for elapsed timer */
struct etimer {
    uint32_t start;
};

/* structure for interval timer */
struct itimer {
    uint32_t start;
    uint32_t interval;
};

typedef void (*timer_callback_function) (void);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /* -- Interval Timer library -- */
    /* defined in generic timer module */
    uint32_t timer_interval_start(struct itimer *t, uint32_t interval);
    uint32_t timer_interval_start_seconds(struct itimer *t, uint32_t interval);
    uint32_t timer_interval_start_minutes(struct itimer *t, uint32_t interval);
    /* adjust interval without restarting */
    uint32_t timer_interval_adjust(struct itimer *t, uint32_t interval);
    /* adds interval to start - good for cyclic timers */
    void timer_interval_reset(struct itimer *t);
    /* sets interval to zero - always expired */
    void timer_interval_none(struct itimer *t);
    /* sets interval to max - never expires */
    void timer_interval_infinity(struct itimer *t);
    /* syncs the start time to the next interval */
    uint32_t timer_interval_resync(struct itimer *t);
    /* restarts the interval timer */
    uint32_t timer_interval_restart(struct itimer *t);
    bool timer_interval_expired(struct itimer *t);
    uint32_t timer_interval(struct itimer *t);
    bool timer_interval_active(struct itimer *t);
    uint32_t timer_interval_seconds(struct itimer *t);
    uint32_t timer_interval_elapsed(struct itimer *t);
    /* -- Elapsed Timer library - lower RAM usage or alternate functional usage -- */
    uint32_t timer_elapsed_start(struct etimer *t);
    uint32_t timer_elapsed_start_offset(struct etimer *t, uint32_t offset);
    bool timer_elapsed_milliseconds(struct etimer *t, uint32_t interval);
    bool timer_elapsed_seconds(struct etimer *t, uint32_t interval);
    bool timer_elapsed_minutes(struct etimer *t, uint32_t interval);
    uint32_t timer_elapsed_time(struct etimer *t);

    /* define these functions in hardware specific timer module */
    void timer_init(void);
    /* Raw API is used only by the elapsed and interval timer library.
       Do not use it directly in your code. */
    uint32_t timer_milliseconds(void);
    bool timer_callback(
        timer_callback_function callback,
        uint32_t milliseconds);
    bool timer_callback_oneshot(
        timer_callback_function callback,
        uint32_t milliseconds);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
