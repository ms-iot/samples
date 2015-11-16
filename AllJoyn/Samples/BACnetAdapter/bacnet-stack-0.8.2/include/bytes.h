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
#ifndef BYTES_H
#define BYTES_H

/* Defines the bit/byte/word/long conversions that are used in code */

#include <stdint.h>

#ifndef LO_NIB
#define LO_NIB(b) ((b) & 0xF)
#endif

#ifndef HI_NIB
#define HI_NIB(b) ((b) >> 4)

#endif



#ifndef LO_BYTE

#define LO_BYTE(w) ((uint8_t)(w))

#endif



#ifndef HI_BYTE

#define HI_BYTE(w) ((uint8_t)((uint16_t)(w) >> 8))

#endif



#ifndef LO_WORD

#define LO_WORD(x) ((uint16_t)(x))

#endif



#ifndef HI_WORD

#define HI_WORD(x) ((uint16_t)((uint32_t)(x) >> 16))

#endif



#ifndef MAKE_WORD

#define MAKE_WORD(lo,hi) \
    ((uint16_t)(((uint8_t)(lo))|(((uint16_t)((uint8_t)(hi)))<<8)))

#endif



#ifndef MAKE_LONG

#define MAKE_LONG(lo,hi) \
    ((uint32_t)(((uint16_t)(lo))|(((uint32_t)((uint16_t)(hi)))<<16)))

#endif



#endif /* end of header file */
