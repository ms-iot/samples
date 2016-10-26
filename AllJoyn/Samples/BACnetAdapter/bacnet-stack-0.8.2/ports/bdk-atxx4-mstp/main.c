/************************************************************************
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
*************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "hardware.h"
#include "init.h"
#include "stack.h"
#include "timer.h"
#include "input.h"
#include "led.h"
#include "adc.h"
#include "nvdata.h"
#include "timer.h"
#include "rs485.h"
#include "serial.h"
#include "bacnet.h"
#include "test.h"
#include "watchdog.h"
#include "version.h"

/* global - currently the version of the stack */
char *BACnet_Version = BACNET_VERSION_TEXT;

/* For porting to IAR, see:
   http://www.avrfreaks.net/wiki/index.php/Documentation:AVR_GCC/IarToAvrgcc*/

int main(
    void)
{
    init();
    /* Configure the watchdog timer - Disabled for debugging */
#ifdef NDEBUG
    watchdog_init(2000);
#else
    watchdog_init(0);
#endif
    timer_init();
    adc_init();
    input_init();
    seeprom_init();
    rs485_init();
    serial_init();
    led_init();
    bacnet_init();
    test_init();
    /* Enable global interrupts */
    __enable_interrupt();
    for (;;) {
        watchdog_reset();
        input_task();
        bacnet_task();
        led_task();
        test_task();
    }
}
