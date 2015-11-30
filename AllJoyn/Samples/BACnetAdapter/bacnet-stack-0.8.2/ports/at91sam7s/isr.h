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
*
*********************************************************************/

#ifndef ISR_H
#define ISR_H

#include <stdint.h>

#if defined(__ICCARM__)
#include <intrinsics.h>
#define isr_enable() __enable_interrupt()
#define isr_disable() __disable_interrupt()
#endif
#if defined(__GNUC__)
#define isr_enable() enableIRQ();enableFIQ();
#define isr_disable() disableFIQ();disableIRQ();
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    unsigned disableIRQ(
        void);

    unsigned restoreIRQ(
        unsigned oldCPSR);

    unsigned enableIRQ(
        void);

    unsigned disableFIQ(
        void);

    unsigned restoreFIQ(
        unsigned oldCPSR);

    unsigned enableFIQ(
        void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
