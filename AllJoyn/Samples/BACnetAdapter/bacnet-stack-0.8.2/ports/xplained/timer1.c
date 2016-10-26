/*
 * @file
 *
 * @brief 1ms timer configuration
 *
 * Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
 *
 * Created: 10/24/2013 8:58:56 PM
 * Author: Steve
 *
 * @page License
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
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tc.h"
#include "timer.h"

/* define which timer counter we are using */
#define MY_TIMER TCE0

/* number of callbacks supported */
#ifndef TIMER_CALLBACK_MAX
#define TIMER_CALLBACK_MAX 8
#endif

/* counter for the base timer */
static volatile uint32_t Millisecond_Counter;

/* callback data structure */
struct timer_data_t {
    volatile uint32_t interval;
    volatile uint32_t milliseconds;
    timer_callback_function callback;
};
static volatile struct timer_data_t Callback_Data[TIMER_CALLBACK_MAX];

/**
 * Handles the interrupt from the timer
 */
static void my_callback(void)
{
    uint32_t now, t, interval, i;

    Millisecond_Counter++;
    now = Millisecond_Counter;
    for (i = 0; i < TIMER_CALLBACK_MAX; i++) {
        /* check for callback */
        if (Callback_Data[i].callback) {
            t = Callback_Data[i].milliseconds;
            if (now >= t) {
                Callback_Data[i].callback();
                interval = Callback_Data[i].interval;
                if (interval) {
                    Callback_Data[i].milliseconds = now + interval;
                } else {
                    /* disable any one-shot timers */
                    Callback_Data[i].callback = NULL;
                }
            }            
        }
    }        
}

/**
 * Returns the continuous milliseconds count, which rolls over
 *
 * @return the current milliseconds count
 */
uint32_t timer_milliseconds(void)
{
    uint32_t timer_value; /* return value */

    tc_set_overflow_interrupt_level(&MY_TIMER, TC_INT_LVL_OFF);
    timer_value = Millisecond_Counter;
    tc_set_overflow_interrupt_level(&MY_TIMER, TC_INT_LVL_LO);

    return timer_value;
}

/**
 * Configures and enables a repeating callback function
 *
 * @param callback - pointer to a #timer_callback_function function
 * @param milliseconds - how often to call the function
 *
 * @return true if successfully added and enabled
 */
bool timer_callback(
    timer_callback_function callback,
    uint32_t milliseconds)
{
    bool status = false;
    uint32_t now, i;

    tc_set_overflow_interrupt_level(&MY_TIMER, TC_INT_LVL_OFF);
    now = Millisecond_Counter;
    for (i = 0; i < TIMER_CALLBACK_MAX; i++) {
        if (Callback_Data[i].callback == NULL) {
            Callback_Data[i].interval = milliseconds;
            /* set the first expiration time */
            Callback_Data[i].milliseconds = now + milliseconds;
            Callback_Data[i].callback = callback;
            status = true;
            break;
        }
    }                    
    tc_set_overflow_interrupt_level(&MY_TIMER, TC_INT_LVL_LO);

    return status;
}

/**
 * Configures and enables a one-shot callback function
 *
 * @param callback - pointer to a #timer_callback_function function
 * @param milliseconds - how long to wait before calling the function
 *
 * @return true if successfully added and enabled
 */
bool timer_callback_oneshot(
    timer_callback_function callback,
    uint32_t milliseconds)
{
    bool status = false;
    uint32_t now, i;

    tc_set_overflow_interrupt_level(&MY_TIMER, TC_INT_LVL_OFF);
    now = Millisecond_Counter;
    for (i = 0; i < TIMER_CALLBACK_MAX; i++) {
        if (Callback_Data[i].callback == NULL) {
            /* set the first expiration time */
            Callback_Data[i].milliseconds = now + milliseconds;
            Callback_Data[i].interval = 0;
            Callback_Data[i].callback = callback;
            status = true;
            break;
        }
    }                    
    tc_set_overflow_interrupt_level(&MY_TIMER, TC_INT_LVL_LO);

    return status;
}

/**
 * Timer setup for 1 millisecond timer
 */
void timer_init(void)
{
    unsigned long period;

    tc_enable(&MY_TIMER);
    tc_set_overflow_interrupt_callback(&MY_TIMER, my_callback);
    tc_set_wgm(&MY_TIMER, TC_WG_NORMAL);
    tc_write_count(&MY_TIMER, 1);
    period = sysclk_get_peripheral_bus_hz(&MY_TIMER);
    period /= 1000;
    tc_write_period(&MY_TIMER, period);
    tc_set_overflow_interrupt_level(&MY_TIMER, TC_INT_LVL_LO);
    tc_write_clock_source(&MY_TIMER, TC_CLKSEL_DIV1_gc);
}
