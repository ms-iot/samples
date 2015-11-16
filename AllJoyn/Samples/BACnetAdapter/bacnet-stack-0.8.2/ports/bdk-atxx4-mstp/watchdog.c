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
#include "hardware.h"
#include "watchdog.h"

#if !defined(__GNUC__)
static inline void wdt_enable(
    int value)
{
    __disable_interrupt();
    __watchdog_reset();
    /* Start timed equence */
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    /* Set new prescaler(time-out) value = 64K cycles (~0.5 s) */
    WDTCSR = (1 << WDE) | (value);
    /* we aren't ready to enable interrupts here
       __enable_interrupt(); */
}

static inline void wdt_disable(
    void)
{
    __disable_interrupt();
    __watchdog_reset();
    /* Clear WDRF in MCUSR */
    MCUSR &= ~(1 << WDRF);
    /* Write logical one to WDCE and WDE */
    /* Keep old prescaler setting to prevent unintentional time-out */
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    /* Turn off WDT */
    WDTCSR = 0x00;
    __enable_interrupt();
}

static inline void wdt_reset(
    void)
{
    __watchdog_reset();
}
#endif

/*************************************************************************
* Description: Reset the watchdog timer
* Returns: none
* Notes: none
**************************************************************************/
void watchdog_reset(
    void)
{
    wdt_reset();
}

/*************************************************************************
* Description: Initialize the watchdog timer
* Returns: none
* Notes: none
**************************************************************************/
void watchdog_init(
    unsigned milliseconds)
{
    unsigned value = WDTO_15MS;
    if (milliseconds) {
        if (milliseconds <= 15) {
            value = WDTO_15MS;
        } else if (milliseconds <= 30) {
            value = WDTO_30MS;
        } else if (milliseconds <= 60) {
            value = WDTO_60MS;
        } else if (milliseconds <= 120) {
            value = WDTO_120MS;
        } else if (milliseconds <= 500) {
            value = WDTO_500MS;
        } else if (milliseconds <= 1000) {
            value = WDTO_1S;
        } else {
            value = WDTO_2S;
        }
        wdt_enable(value);
    } else {
        wdt_disable();
    }
}
