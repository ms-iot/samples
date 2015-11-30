/**************************************************************************
*
* Copyright (C) 2011 Steve Karg <skarg@users.sourceforge.net>
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

/* counter for the the timer which wraps every 49.7 days */
static volatile uint32_t Millisecond_Counter;
static volatile uint8_t Millisecond_Counter_Byte;
/* forward prototype for interrupt service routine */
void int_cmt0_isr(
    void);

/*************************************************************************
* Description: Timer Interrupt Handler
* Returns: nothing
* Notes: none
*************************************************************************/
static void timer_interrupt_handler(
    void)
{
    Millisecond_Counter++;
    Millisecond_Counter_Byte++;
}

/*************************************************************************
* Description: Timer Interrupt Service Routine
* Returns: nothing
* Notes: none
*************************************************************************/
void int_cmt0_isr(
    void)
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

    timer_value = Millisecond_Counter;

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
    /* Declare error flag */
    bool err = true;

    /* CMT is configured for a 1ms interval, and executes the callback
       function CB_CompareMatch on every compare match */
    err &= R_CMT_Create(3, PDL_CMT_PERIOD, 1E-3, int_cmt0_isr, 3);

    /* Halt in while loop when RPDL errors detected */
    while (!err);
}
