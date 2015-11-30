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
#ifndef KEY_H
#define KEY_H

#include <stdint.h>

/* This file has the macros that encode and decode the */
/* keys for the keylist when used with BACnet Object Id's */
typedef uint32_t KEY;

/* assuming a 32 bit KEY */
#define KEY_TYPE_OFFSET  22     /* bits - for BACnet */
#define KEY_TYPE_MASK    0x000003FFL
#define KEY_ID_MASK  0x003FFFFFL
#define KEY_ID_MAX  (KEY_ID_MASK + 1L)
#define KEY_TYPE_MAX  (KEY_TYPE_MASK + 1L)
#define KEY_LAST(key) ((key & KEY_ID_MASK) == KEY_ID_MAX)

#define KEY_ENCODE(type,id) ( ((unsigned int)\
((unsigned int)(type) & KEY_TYPE_MASK) << KEY_TYPE_OFFSET) |\
 ((unsigned int)(id) & KEY_ID_MASK) )

#define KEY_DECODE_TYPE(key) ((int)(((unsigned int)(key) >> KEY_TYPE_OFFSET)\
 & KEY_TYPE_MASK))

#define KEY_DECODE_ID(key) ((int)((unsigned int)(key) & KEY_ID_MASK))

#endif
