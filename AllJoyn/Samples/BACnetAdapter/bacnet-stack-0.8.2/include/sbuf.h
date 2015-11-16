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
#ifndef SBUF_H
#define SBUF_H

/* Functional Description: Static buffer library for deeply
   embedded system. See the unit tests for usage examples. */

#include <stdint.h>
#include <stdbool.h>

struct static_buffer_t {
    char *data; /* block of memory or array of data */
    unsigned size;      /* actual size, in bytes, of the block of data */
    unsigned count;     /* number of bytes in use */
};
typedef struct static_buffer_t STATIC_BUFFER;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void sbuf_init(
        STATIC_BUFFER * b,      /* static buffer structure */
        char *data,     /* data block */
        unsigned size); /* actual size, in bytes, of the data block or array of data */

    /* returns true if size==0, false if size > 0 */
    bool sbuf_empty(
        STATIC_BUFFER const *b);
    /* returns the data block, or NULL if not initialized */
    char *sbuf_data(
        STATIC_BUFFER const *b);
    /* returns the max size of the data block */
    unsigned sbuf_size(
        STATIC_BUFFER * b);
    /* returns the number of bytes used in the data block */
    unsigned sbuf_count(
        STATIC_BUFFER * b);
    /* returns true if successful, false if not enough room to append data */
    bool sbuf_put(
        STATIC_BUFFER * b,      /* static buffer structure */
        unsigned offset,        /* where to start */
        char *data,     /* data to add */
        unsigned data_size);    /* how many to add */
    /* returns true if successful, false if not enough room to append data */
    bool sbuf_append(
        STATIC_BUFFER * b,      /* static buffer structure */
        char *data,     /* data to append */
        unsigned data_size);    /* how many to append */
    /* returns true if successful, false if count is bigger than size */
    bool sbuf_truncate(
        STATIC_BUFFER * b,      /* static buffer structure */
        unsigned count);        /* new number of bytes used in buffer */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
