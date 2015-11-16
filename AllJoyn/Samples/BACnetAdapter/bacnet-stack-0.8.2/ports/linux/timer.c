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
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "timer.h"

/** @file linux/timer.c  Provides Linux-specific time and timer functions. */

/* counter for the various timers */
static volatile uint32_t Millisecond_Counter[MAX_MILLISECOND_TIMERS];

/* start time for the clock */
static struct timespec start;
/* The timeGetTime function retrieves the system time, in milliseconds. 
   The system time is the time elapsed since Windows was started. */
uint32_t timeGetTime(
    void)
{
    struct timespec now;
    uint32_t ticks;

    clock_gettime(CLOCK_MONOTONIC, &now);

    ticks =
        (now.tv_sec - start.tv_sec) * 1000 + (now.tv_nsec -
        start.tv_nsec) / 1000000;

    return ticks;
}

/*************************************************************************
* Description: returns the current millisecond count
* Returns: none
* Notes: none
*************************************************************************/
uint32_t timer_milliseconds(
    unsigned index)
{
    uint32_t now = timeGetTime();
    uint32_t delta_time = 0;

    if (index < MAX_MILLISECOND_TIMERS) {
        if (Millisecond_Counter[index] <= now) {
            delta_time = now - Millisecond_Counter[index];
        } else {
            delta_time = (UINT32_MAX - Millisecond_Counter[index]) + now + 1;
        }
    }

    return delta_time;
}

/*************************************************************************
* Description: compares the current time count with a value
* Returns: true if the time has elapsed
* Notes: none
*************************************************************************/
bool timer_elapsed_milliseconds(
    unsigned index,
    uint32_t value)
{
    return (timer_milliseconds(index) >= value);
}

/*************************************************************************
* Description: compares the current time count with a value
* Returns: true if the time has elapsed
* Notes: none
*************************************************************************/
bool timer_elapsed_seconds(
    unsigned index,
    uint32_t seconds)
{
    return ((timer_milliseconds(index) / 1000) >= seconds);
}

/*************************************************************************
* Description: compares the current time count with a value
* Returns: true if the time has elapsed
* Notes: none
*************************************************************************/
bool timer_elapsed_minutes(
    unsigned index,
    uint32_t minutes)
{
    return ((timer_milliseconds(index) / (1000 * 60)) >= minutes);
}

/*************************************************************************
* Description: Sets the timer counter to zero.
* Returns: none
* Notes: none
*************************************************************************/
uint32_t timer_reset(
    unsigned index)
{
    uint32_t timer_value = 0;

    if (index < MAX_MILLISECOND_TIMERS) {
        timer_value = timer_milliseconds(index);
        Millisecond_Counter[index] = timeGetTime();
    }

    return timer_value;
}

/*************************************************************************
* Description: Initialization for timer
* Returns: none
* Notes: none
*************************************************************************/
void timer_init(
    void)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
}
