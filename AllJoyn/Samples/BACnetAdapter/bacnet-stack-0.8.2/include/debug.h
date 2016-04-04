/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "bacdef.h"

#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED 0
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void debug_printf(
        const char *format,
        ...);
#if DEBUG_ENABLED
    /* Nothing more here */
#else
    /* If your compiler supports it, this is more compact:
       inline void debug_printf(
       const char *format,
       ...) {
       format = format;
       }
     */
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
