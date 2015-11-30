/**************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
* Multimedia Timer contribution by Cameron Crothers, 2008
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
#include <stdlib.h>
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#define STRICT 1
#include <windows.h>
#include "net.h"
#include <MMSystem.h>
#include "timer.h"

/* Offset between Windows epoch 1/1/1601 and
   Unix epoch 1/1/1970 in 100 nanosec units */
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) || defined(__BORLANDC__)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

/* counter for the various timers */
static volatile uint32_t Millisecond_Counter[MAX_MILLISECOND_TIMERS];

/* Windows timer period - in milliseconds */
static uint32_t Timer_Period = 1;

#if defined(_MSC_VER) || defined(__BORLANDC__)
struct timezone {
    int tz_minuteswest; /* minutes W of Greenwich */
    int tz_dsttime;     /* type of dst correction */
};

/*************************************************************************
* Description: simulate the gettimeofday Linux function
* Returns: zero
* Note: The resolution of GetSystemTimeAsFileTime() is 15625 microseconds.
* The resolution of _ftime() is about 16 milliseconds.
* To get microseconds accuracy we need to use QueryPerformanceCounter or
* timeGetTime for the elapsed time.
*************************************************************************/

int gettimeofday(
    struct timeval *tp,
    void *tzp)
{
    static int tzflag = 0;
    struct timezone *tz;
    /* start calendar time in microseconds */
    static LONGLONG usec_timer = 0;
    LONGLONG usec_elapsed = 0;
    /* elapsed time in milliseconds */
    static uint32_t time_start = 0;
    /* semi-accurate time from File Timer */
    FILETIME ft;
    uint32_t elapsed_milliseconds = 0;

    tzp = tzp;
    if (usec_timer == 0) {
        /* a 64-bit value representing the number of
           100-nanosecond intervals since January 1, 1601 (UTC). */
        GetSystemTimeAsFileTime(&ft);
        usec_timer = ft.dwHighDateTime;
        usec_timer <<= 32;
        usec_timer |= ft.dwLowDateTime;
        /*converting file time to unix epoch 1970 */
        usec_timer /= 10;       /*convert into microseconds */
        usec_timer -= DELTA_EPOCH_IN_MICROSECS;
        tp->tv_sec = (long) (usec_timer / 1000000UL);
        tp->tv_usec = (long) (usec_timer % 1000000UL);
        time_start = timeGetTime();
    } else {
        elapsed_milliseconds = timeGetTime() - time_start;
        usec_elapsed = usec_timer + ((LONGLONG) elapsed_milliseconds * 1000UL);
        tp->tv_sec = (long) (usec_elapsed / 1000000UL);
        tp->tv_usec = (long) (usec_elapsed % 1000000UL);
    }
    if (tzp) {
        if (!tzflag) {
            _tzset();
            tzflag++;
        }
        tz = tzp;
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;
}
#endif

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
    uint32_t timer_value = timer_milliseconds(index);
    if (index < MAX_MILLISECOND_TIMERS) {
        Millisecond_Counter[index] = timeGetTime();
    }
    return timer_value;
}

/*************************************************************************
* Description: Shut down for timer
* Returns: none
* Notes: none
*************************************************************************/
static void timer_cleanup(
    void)
{
    timeEndPeriod(Timer_Period);
}

/*************************************************************************
* Description: Initialization for timer
* Returns: none
* Notes: none
*************************************************************************/
void timer_init(
    void)
{
    TIMECAPS tc;

    /* set timer resolution */
    if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) {
        fprintf(stderr, "Failed to get timer resolution parameters\n");
    }
    /* configure for 1ms resolution - if possible */
    Timer_Period = min(max(tc.wPeriodMin, 1L), tc.wPeriodMax);
    if (Timer_Period != 1L) {
        fprintf(stderr,
            "Failed to set timer to 1ms.  " "Time period set to %ums\n",
            (unsigned) Timer_Period);
    }
    timeBeginPeriod(Timer_Period);
    atexit(timer_cleanup);
}

#ifdef TEST_TIMER_WIN
static uint32_t timeval_diff_ms(
    struct timeval *old,
    struct timeval *now)
{
    uint32_t ms = 0;

    /* convert to milliseconds */
    ms = (now->tv_sec - old->tv_sec) * 1000 + (now->tv_usec -
        old->tv_usec) / 1000;

    return ms;
}

int main(
    int argc,
    char *argv[])
{
    long now = 0, last = 0, delta = 0;
    struct timeval tv;
    struct timeval old_tv = { 0 };

    timer_init();
    printf("Testing granularity of timeGetTime()...\n");
    timer_reset(0);
    for (;;) {
        now = timeGetTime();
        delta = now - last;
        if (delta) {
            if (delta > 1) {
                printf("Delta is %ld.\n", delta);
            }
            last = now;
        }
        if (timer_elapsed_milliseconds(0, 5000)) {
            break;
        }
    }
    printf("Testing granularity of gettimeofday()...\n");
    for (;;) {
        gettimeofday(&tv, NULL);
        delta = timeval_diff_ms(&old_tv, &tv);
        if (delta) {
            if (delta > 1) {
                printf("Delta is %ld.\n", delta);
            }
            old_tv.tv_sec = tv.tv_sec;
            old_tv.tv_usec = tv.tv_usec;
        }
        if (timer_elapsed_milliseconds(0, 10000)) {
            break;
        }
    }

}
#endif
