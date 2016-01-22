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
#include <stdbool.h>
#include <stdint.h>
#include "timer.h"

/* generic elapsed timer handling */
/* interval not to exceed 49.7 days */
/* interval of 1ms may be 0 to 1ms */

/*************************************************************************
* Description: Sets the start time for an elapsed timer
* Returns: the value of the start timer
* Notes: none
*************************************************************************/
void timer_elapsed_start(
    struct etimer *t)
{
    uint32_t now = timer_milliseconds();

    if (t) {
        t->start = now;
    }
}

/*************************************************************************
* Description: Gets the amount of elapsed time in milliseconds
* Returns: elapsed time in milliseconds
* Notes: none
*************************************************************************/
uint32_t timer_elapsed_time(
    struct etimer *t)
{
    uint32_t now = timer_milliseconds();
    uint32_t delta = 0;

    if (t) {
        delta = now - t->start;
    }

    return delta;
}

/*************************************************************************
* Description: Sets the start time with an offset
* Returns: elapsed time in milliseconds
* Notes: none
*************************************************************************/
void timer_elapsed_start_offset(
    struct etimer *t,
    uint32_t offset)
{
    uint32_t now = timer_milliseconds();

    if (t) {
        t->start = now + offset;
    }
}

/*************************************************************************
* Description: Tests to see if time has elapsed
* Returns: true if time has elapsed
* Notes: none
*************************************************************************/
bool timer_elapsed_milliseconds(
    struct etimer *t,
    uint32_t milliseconds)
{
    return (timer_elapsed_time(t) >= milliseconds);
}

/*************************************************************************
* Description: Tests to see if time has elapsed
* Returns: true if time has elapsed
* Notes: none
*************************************************************************/
bool timer_elapsed_seconds(
    struct etimer * t,
    uint32_t seconds)
{
    uint32_t milliseconds = seconds;

    milliseconds *= 1000L;

    return timer_elapsed_milliseconds(t, milliseconds);
}

/*************************************************************************
* Description: Tests to see if time has elapsed
* Returns: true if time has elapsed
* Notes: none
*************************************************************************/
bool timer_elapsed_minutes(
    struct etimer * t,
    uint32_t minutes)
{
    uint32_t milliseconds = minutes;

    milliseconds *= 1000L;
    milliseconds *= 60L;

    return timer_elapsed_milliseconds(t, milliseconds);
}

/*************************************************************************
* Description: Tests to see if time has elapsed
* Returns: true if time has elapsed
* Notes: none
*************************************************************************/
bool timer_elapsed_milliseconds_short(
    struct etimer * t,
    uint16_t value)
{
    uint32_t milliseconds;

    milliseconds = value;

    return (timer_elapsed_time(t) >= milliseconds);
}

/*************************************************************************
* Description: Tests to see if time has elapsed
* Returns: true if time has elapsed
* Notes: none
*************************************************************************/
bool timer_elapsed_seconds_short(
    struct etimer * t,
    uint16_t value)
{
    return timer_elapsed_seconds(t, value);
}

/*************************************************************************
* Description: Tests to see if time has elapsed
* Returns: true if time has elapsed
* Notes: none
*************************************************************************/
bool timer_elapsed_minutes_short(
    struct etimer * t,
    uint16_t value)
{
    return timer_elapsed_minutes(t, value);
}

/*************************************************************************
* Description: Starts an interval timer
* Returns: nothing
* Notes: none
*************************************************************************/
void timer_interval_start(
    struct itimer *t,
    uint32_t interval)
{
    if (t) {
        t->start = timer_milliseconds();
        t->interval = interval;
    }
}

/*************************************************************************
* Description: Starts an interval timer
* Returns: nothing
* Notes: none
*************************************************************************/
void timer_interval_start_seconds(
    struct itimer *t,
    uint32_t seconds)
{
    uint32_t interval = seconds;

    interval *= 1000L;
    timer_interval_start(t, interval);
}

/*************************************************************************
* Description: Starts an interval timer
* Returns: nothing
* Notes: none
*************************************************************************/
void timer_interval_start_minutes(
    struct itimer *t,
    uint32_t minutes)
{
    uint32_t interval = minutes;

    interval *= 1000L;
    interval *= 60L;
    timer_interval_start(t, interval);
}

/*************************************************************************
* Description: Determines the amount of time that has elapsed
* Returns: elapsed milliseconds
* Notes: none
*************************************************************************/
uint32_t timer_interval_elapsed(
    struct itimer *t)
{
    uint32_t now = timer_milliseconds();
    uint32_t delta = 0;

    if (t) {
        delta = now - t->start;
    }

    return delta;
}

/*************************************************************************
* Description: Determines the amount of time that has elapsed
* Returns: elapsed milliseconds
* Notes: none
*************************************************************************/
uint32_t timer_interval(
    struct itimer * t)
{
    uint32_t interval = 0;

    if (t) {
        interval = t->interval;
    }

    return interval;
}

/*************************************************************************
* Description: Tests to see if time has elapsed
* Returns: true if time has elapsed
* Notes: none
*************************************************************************/
bool timer_interval_expired(
    struct itimer * t)
{
    bool expired = false;

    if (t) {
        if (t->interval) {
            expired = timer_interval_elapsed(t) >= t->interval;
        }
    }

    return expired;
}

/*************************************************************************
* Description: Sets the interval value to zero so it never expires
* Returns: nothing
* Notes: none
*************************************************************************/
void timer_interval_no_expire(
    struct itimer *t)
{
    if (t) {
        t->interval = 0;
    }
}

/*************************************************************************
* Description: Adds another interval to the start time.  Used for cyclic
*   timers that won't lose ticks.
* Returns: nothing
* Notes: none
*************************************************************************/
void timer_interval_reset(
    struct itimer *t)
{
    if (t) {
        t->start += t->interval;
    }
}

/*************************************************************************
* Description: Restarts the timer with the same interval
* Returns: nothing
* Notes: none
*************************************************************************/
void timer_interval_restart(
    struct itimer *t)
{
    if (t) {
        t->start = timer_milliseconds();
    }
}

/*************************************************************************
* Description: Return the elapsed time
* Returns: number of milliseconds elapsed
* Notes: only up to 255ms elapsed
**************************************************************************/
uint8_t timer_milliseconds_delta(
    uint8_t start)
{
    return (timer_milliseconds_byte() - start);
}

/*************************************************************************
* Description: Mark the start of a delta timer
* Returns: mark timer starting tick
* Notes: only up to 255ms elapsed
**************************************************************************/
uint8_t timer_milliseconds_mark(
    void)
{
    return timer_milliseconds_byte();
}

#ifdef TEST
#include <assert.h>
#include <string.h>

#include "ctest.h"

static uint32_t Milliseconds;

uint32_t timer_milliseconds(
    void)
{
    return Milliseconds;
}

uint32_t timer_milliseconds_set(
    uint32_t value)
{
    uint32_t old_value = Milliseconds;

    Milliseconds = value;

    return old_value;
}

void testElapsedTimer(
    Test * pTest)
{
    struct etimer t;
    uint32_t test_time = 0;

    timer_milliseconds_set(test_time);
    timer_elapsed_start(&t);
    ct_test(pTest, timer_elapsed_time(&t) == test_time);
    test_time = 0xffff;
    timer_milliseconds_set(test_time);
    ct_test(pTest, timer_elapsed_time(&t) == test_time);
    test_time = 0xffffffff;
    timer_milliseconds_set(test_time);
    ct_test(pTest, timer_elapsed_time(&t) == test_time);
}

void testIntervalTimer(
    Test * pTest)
{
    struct itimer t;
    uint32_t interval = 0;
    uint32_t test_time = 0;

    timer_milliseconds_set(test_time);
    timer_interval_start(&t, interval);
    test_time = 0xffff;
    timer_milliseconds_set(test_time);
    ct_test(pTest, timer_interval(&t) == interval);
    ct_test(pTest, timer_interval_elapsed(&t) == test_time);
    test_time = 0xffffffff;
    timer_milliseconds_set(test_time);
    ct_test(pTest, timer_interval(&t) == interval);
    ct_test(pTest, timer_interval_elapsed(&t) == test_time);
    test_time = 0;
    timer_milliseconds_set(test_time);
    interval = 0xffff;
    timer_interval_start(&t, interval);
    ct_test(pTest, timer_interval(&t) == interval);
    interval = 0xffffffff;
    timer_interval_start(&t, interval);
    ct_test(pTest, timer_interval(&t) == interval);

    interval = 0;
    timer_interval_start_seconds(&t, interval);
    ct_test(pTest, timer_interval(&t) == interval);
    interval = 60L;
    timer_interval_start_seconds(&t, interval);
    interval *= 1000L;
    ct_test(pTest, timer_interval(&t) == interval);

}


#ifdef TEST_TIMER
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("Timer", NULL);

    /* individual tests */
    rc = ct_addTestFunction(pTest, testElapsedTimer);
    assert(rc);
    rc = ct_addTestFunction(pTest, testIntervalTimer);
    assert(rc);


    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);

    ct_destroy(pTest);

    return 0;
}
#endif
#endif
