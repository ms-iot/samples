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
#include "hardware.h"
#include "timer.h"

#ifndef F_CPU
#error "F_CPU must be defined for Timer configuration."
#endif
/* Timer2 Prescaling: 1, 8, 32, 64, 128, 256, or 1024 */
#define TIMER_MICROSECONDS 1000UL
#define TIMER_TICKS(p) \
  (((((F_CPU)/(p))/1000UL) \
  *(TIMER_MICROSECONDS))/1000UL)
#define TIMER_TICKS_MAX 255UL
/* adjust the prescaler for the processor clock */
#if (TIMER_TICKS(1) <= TIMER_TICKS_MAX)
#define TIMER2_PRESCALER 1
#elif (TIMER_TICKS(8) <= TIMER_TICKS_MAX)
#define TIMER2_PRESCALER 8
#elif (TIMER_TICKS(32) <= TIMER_TICKS_MAX)
#define TIMER2_PRESCALER 32
#elif (TIMER_TICKS(64) <= TIMER_TICKS_MAX)
#define TIMER2_PRESCALER 64
#elif (TIMER_TICKS(128) <= TIMER_TICKS_MAX)
#define TIMER2_PRESCALER 128
#elif (TIMER_TICKS(256) <= TIMER_TICKS_MAX)
#define TIMER2_PRESCALER 256
#elif (TIMER_TICKS(1024) <= TIMER_TICKS_MAX)
#define TIMER2_PRESCALER 1024
#else
#error "TIMER2: F_CPU too large for timer prescaler."
#endif
#define TIMER2_TICKS TIMER_TICKS(TIMER2_PRESCALER)
/* Timer counts up from count to TIMER_TICKS_MAX and then signals overflow */
#define TIMER2_COUNT (TIMER_TICKS_MAX-TIMER2_TICKS)

/* counter for the the timer which wraps every 49.7 days */
static volatile uint32_t Millisecond_Counter;
static volatile uint8_t Millisecond_Counter_Byte;

/*************************************************************************
* Description: Timer Interrupt Handler
* Returns: nothing
* Notes: none
*************************************************************************/
static inline void timer_interrupt_handler(
    void)
{
    /* Set the counter for the next interrupt */
    TCNT2 = TIMER2_COUNT;
    Millisecond_Counter++;
    Millisecond_Counter_Byte++;
}

/*************************************************************************
* Description: Timer Interrupt Service Routine - Timer Overflowed!
* Returns: none
* Notes: Global interupts must be enabled
*************************************************************************/
ISR(TIMER2_OVF_vect)
{
    timer_interrupt_handler();
}

/*************************************************************************
* Description: returns the current millisecond count
* Returns: none
* Notes: This method only disables the timer overflow interrupt.
*************************************************************************/
uint32_t timer_milliseconds(
    void)
{
    uint32_t timer_value;       /* return value */

    /* Disable the overflow interrupt.
       Prevents value corruption that would happen if interrupted */
    BIT_CLEAR(TIMSK2, TOIE2);
    timer_value = Millisecond_Counter;
    /* Enable the overflow interrupt */
    BIT_SET(TIMSK2, TOIE2);

    return timer_value;
}

/*************************************************************************
* Description: returns the current millisecond count
* Returns: none
* Notes: This method only disables the timer overflow interrupt.
*************************************************************************/
uint8_t timer_milliseconds_byte(
    void)
{
    return Millisecond_Counter;
}

/*************************************************************************
* Description: Initialization for Timer
* Returns: none
* Notes: none
*************************************************************************/
void timer_init(
    void)
{
    /* Normal Operation */
    TCCR2A = 0;
    /* Timer2: prescale selections:
       CSn2 CSn1 CSn0 Description
       ---- ---- ---- -----------
       0    0    0  No Clock Source
       0    0    1  No prescaling
       0    1    0  CLKt2s/8
       0    1    1  CLKt2s/32
       1    0    0  CLKt2s/64
       1    0    1  CLKt2s/128
       1    1    0  CLKt2s/256
       1    1    1  CLKt2s/1024
     */
#if (TIMER2_PRESCALER==1)
    TCCR2B = _BV(CS20);
#elif (TIMER2_PRESCALER==8)
    TCCR2B = _BV(CS21);
#elif (TIMER2_PRESCALER==32)
    TCCR2B = _BV(CS21) | _BV(CS20);
#elif (TIMER2_PRESCALER==64)
    TCCR2B = _BV(CS22);
#elif (TIMER2_PRESCALER==128)
    TCCR2B = _BV(CS22) | _BV(CS20);
#elif (TIMER2_PRESCALER==256)
    TCCR2B = _BV(CS22) | _BV(CS21);
#elif (TIMER2_PRESCALER==1024)
    TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);
#else
#error Timer2 Prescale: Invalid Value
#endif
    /* Clear any TOV Flag set when the timer overflowed - by setting! */
    BIT_SET(TIFR2, TOV2);
    /* Initial value */
    TCNT2 = TIMER2_COUNT;
    /* Enable the overflow interrupt */
    BIT_SET(TIMSK2, TOIE2);
    power_timer2_enable();
}
