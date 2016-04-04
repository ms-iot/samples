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
#include <stdbool.h>
#include <stdint.h>
#include "timer.h"

/* interval and elapsed millisecond timer */
/* interval not to exceed 49.7 days */
/* minimum interval of 1ms may be 0 to 1ms */

/*************************************************************************
* Description: Start a timer from now.
* Returns: elapsed milliseconds since last set
* Notes: none
**************************************************************************/
uint32_t timer_interval_start(struct itimer *t, uint32_t interval)
{
    uint32_t now = 0;
    uint32_t elapsed = 0;

    now = timer_milliseconds();
    elapsed = now - t->start;
    t->start = now;
    t->interval = interval;

    return elapsed;
}

/*************************************************************************
* Description: Start a timer from now.
* Returns: elapsed seconds since last set
* Notes: none
**************************************************************************/
uint32_t timer_interval_start_seconds(struct itimer *t, uint32_t interval)
{
    uint32_t elapsed = 0;

    interval *= 1000L;
    elapsed = timer_interval_start(t, interval);
    elapsed /= 1000L;

    return elapsed;
}

/*************************************************************************
* Description: Start a timer from now.
* Returns: elapsed minutes since last set
* Notes: none
**************************************************************************/
uint32_t timer_interval_start_minutes(struct itimer *t, uint32_t interval)
{
    uint32_t elapsed = 0;

    interval *= 60L;
    interval *= 1000L;

    elapsed = timer_interval_start(t, interval);
    elapsed /= 1000L;
    elapsed /= 60L;

    return elapsed;
}

/*************************************************************************
* Description: Change the timer interval without restart
* Returns: previous interval value
* Notes: none
**************************************************************************/
uint32_t timer_interval_adjust(struct itimer *t, uint32_t interval)
{
    uint32_t previous_interval = t->interval;

    t->interval = interval;

    return previous_interval;
}

/*************************************************************************
* Description: Reset the timer with the same interval
* Returns: none
* Notes: none
**************************************************************************/
void timer_interval_reset(struct itimer *t)
{
    t->start += t->interval;
}

/*************************************************************************
* Description: Restart the timer from now, syncing on existing interval
* Returns: elapsed milliseconds since last reset
* Notes: none
**************************************************************************/
uint32_t timer_interval_resync(struct itimer *t)
{
    uint32_t elapsed;
    uint32_t intervals;
    uint32_t gap;
    uint32_t now;

    now = timer_milliseconds();
    elapsed = now - t->start;
    if ((t->interval != TIMER_INTERVAL_MAX) && (t->interval != 0)) {
        if (elapsed >= t->interval) {
            /* catch up to now */
            intervals = elapsed / t->interval;
            gap = intervals * t->interval;
            t->start += gap;
        }
    }

    return elapsed;
}

/*************************************************************************
* Description: Restart the timer from now
* Returns: elapsed milliseconds since last set
* Notes: none
**************************************************************************/
uint32_t timer_interval_restart(struct itimer *t)
{
    uint32_t now;
    uint32_t elapsed;

    now = timer_milliseconds();
    elapsed = now - t->start;
    t->start = now;

    return elapsed;
}

/*************************************************************************
* Description: Reset the timer with the zero interval - always expired
* Returns: none
* Notes: none
**************************************************************************/
void timer_interval_none(struct itimer *t)
{
    t->interval = 0;
}

/*************************************************************************
* Description: Reset the timer with the max interval - never expires
* Returns: none
* Notes: none
**************************************************************************/
void timer_interval_infinity(struct itimer *t)
{
    t->interval = TIMER_INTERVAL_MAX;
}

/*************************************************************************
* Description: Determines if the timer has an active interval
* Returns: true if active
* Notes: none
**************************************************************************/
bool timer_interval_active(struct itimer *t)
{
    return ((t->interval != TIMER_INTERVAL_MAX) && (t->interval != 0));
}

/*************************************************************************
* Description: Check to see if the time interval has elapsed
* Returns: true if expired
* Notes: Setting the interval to max never expires, to zero always expires
**************************************************************************/
bool timer_interval_expired(struct itimer *t)
{
    uint32_t elapsed = 0;
    bool status = false;
    uint32_t now;

    if (t->interval == 0) {
        status = true;
    } else if (t->interval == TIMER_INTERVAL_MAX) {
        status = false;
    } else {
        now = timer_milliseconds();
        elapsed = now - t->start;
        if (elapsed >= t->interval) {
            status = true;
        }
    }

    return status;
}

/*************************************************************************
* Description: Return the elapsed time
* Returns: number of milliseconds elapsed
* Notes: none
**************************************************************************/
uint32_t timer_interval_elapsed(struct itimer *t)
{
    uint32_t now;
    uint32_t elapsed;

    now = timer_milliseconds();
    elapsed = now - t->start;

    return elapsed;
}

/*************************************************************************
* Description: Return the interval time
* Returns: number of milliseconds for which the interval is set
* Notes: none
**************************************************************************/
uint32_t timer_interval(struct itimer *t)
{
    return t->interval;
}

/*************************************************************************
* Description: Return the interval time
* Returns: number of seconds for which the interval is set
* Notes: none
**************************************************************************/
uint32_t timer_interval_seconds(struct itimer *t)
{
    return (t->interval/1000);
}

/* Elapsed Timer */

/*************************************************************************
* Description: Restart the timer from now
* Returns: elapsed milliseconds since last set
* Notes: none
**************************************************************************/
uint32_t timer_elapsed_start_offset(struct etimer *t, uint32_t offset)
{
    uint32_t now;
    uint32_t elapsed;

    now = timer_milliseconds();
    elapsed = now - t->start;
    if (offset) {
        t->start = now + offset;
    } else {
        t->start = now;
    }

    return elapsed;
}

/*************************************************************************
* Description: Start a timer from now.
* Returns: elapsed milliseconds since last set
* Notes: none
**************************************************************************/
uint32_t timer_elapsed_start(struct etimer *t)
{
    return timer_elapsed_start_offset(t, 0);
}

/*************************************************************************
* Description: Return the elapsed time
* Returns: true if interval has elapsed
* Notes: none
**************************************************************************/
bool timer_elapsed_milliseconds(struct etimer *t, uint32_t interval)
{
    bool status = false;
    uint32_t delta;

    delta = timer_milliseconds() - t->start;
    if (delta >= interval) {
        status = true;
    }

    return status;
}

/*************************************************************************
* Description: Return the elapsed time
* Returns: true if interval has elapsed
* Notes: none
**************************************************************************/
bool timer_elapsed_seconds(struct etimer *t, uint32_t interval)
{
    /* convert to seconds */
    interval *= 1000L;

    return timer_elapsed_milliseconds(t, interval);
}

/*************************************************************************
* Description: Return the elapsed time
* Returns: true if interval has elapsed
* Notes: none
**************************************************************************/
bool timer_elapsed_minutes(struct etimer *t, uint32_t interval)
{
    /* convert to seconds */
    interval *= 1000L;
    /* convert to minutes */
    interval *= 60L;

    return timer_elapsed_milliseconds(t, interval);
}

/*************************************************************************
* Description: Return the elapsed time
* Returns: number of milliseconds elapsed
* Notes: none
**************************************************************************/
uint32_t timer_elapsed_time(struct etimer *t)
{
    uint32_t now;
    uint32_t elapsed;

    now = timer_milliseconds();
    elapsed = now - t->start;

    return elapsed;
}

