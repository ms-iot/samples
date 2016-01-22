/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 by Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307
 USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/

/** @file sbuf.c  Static buffer library for deeply embedded system. */

/* Functional Description: Static buffer library for deeply
   embedded system. See the unit tests for usage examples. */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "sbuf.h"

void sbuf_init(
    STATIC_BUFFER * b,  /* static buffer structure */
    char *data, /* data block */
    unsigned size)
{       /* actual size, in bytes, of the data block or array of data */
    if (b) {
        b->data = data;
        b->size = size;
        b->count = 0;
    }

    return;
}

/* returns true if count==0, false if count > 0 */
bool sbuf_empty(
    STATIC_BUFFER const *b)
{
    return (b ? (b->count == 0) : false);
}

char *sbuf_data(
    STATIC_BUFFER const *b)
{
    return (b ? b->data : NULL);
}

unsigned sbuf_size(
    STATIC_BUFFER * b)
{
    return (b ? b->size : 0);
}

unsigned sbuf_count(
    STATIC_BUFFER * b)
{
    return (b ? b->count : 0);
}

/* returns true if successful, false if not enough room to append data */
bool sbuf_put(
    STATIC_BUFFER * b,  /* static buffer structure */
    unsigned offset,    /* where to start */
    char *data, /* data to place in buffer */
    unsigned data_size)
{       /* how many bytes to add */
    bool status = false;        /* return value */

    if (b && b->data) {
        if (((offset + data_size) < b->size)) {
            b->count = offset + data_size;
            while (data_size) {
                b->data[offset] = *data;
                offset++;
                data++;
                data_size--;
            }
            status = true;
        }
    }

    return status;
}

/* returns true if successful, false if not enough room to append data */
bool sbuf_append(
    STATIC_BUFFER * b,  /* static buffer structure */
    char *data, /* data to place in buffer */
    unsigned data_size)
{       /* how many bytes to add */
    unsigned count = 0;

    if (b) {
        count = b->count;
    }

    return sbuf_put(b, count, data, data_size);
}

/* returns true if successful, false if not enough room to append data */
bool sbuf_truncate(
    STATIC_BUFFER * b,  /* static buffer structure */
    unsigned count)
{       /* total number of bytes in to remove */
    bool status = false;        /* return value */

    if (b) {
        if (count < b->size) {
            b->count = count;
            status = true;
        }
    }

    return status;
}

#ifdef TEST
#include <assert.h>
#include <string.h>

#include "ctest.h"

void testStaticBuffer(
    Test * pTest)
{
    STATIC_BUFFER sbuffer;
    char *data1 = "Joshua";
    char *data2 = "Anna";
    char *data3 = "Christopher";
    char *data4 = "Mary";
    char data_buffer[480] = "";
    char test_data_buffer[480] = "";
    char *data;
    unsigned count;

    sbuf_init(&sbuffer, NULL, 0);
    ct_test(pTest, sbuf_empty(&sbuffer) == true);
    ct_test(pTest, sbuf_data(&sbuffer) == NULL);
    ct_test(pTest, sbuf_size(&sbuffer) == 0);
    ct_test(pTest, sbuf_count(&sbuffer) == 0);
    ct_test(pTest, sbuf_append(&sbuffer, data1, strlen(data1)) == false);

    sbuf_init(&sbuffer, data_buffer, sizeof(data_buffer));
    ct_test(pTest, sbuf_empty(&sbuffer) == true);
    ct_test(pTest, sbuf_data(&sbuffer) == data_buffer);
    ct_test(pTest, sbuf_size(&sbuffer) == sizeof(data_buffer));
    ct_test(pTest, sbuf_count(&sbuffer) == 0);

    ct_test(pTest, sbuf_append(&sbuffer, data1, strlen(data1)) == true);
    ct_test(pTest, sbuf_append(&sbuffer, data2, strlen(data2)) == true);
    ct_test(pTest, sbuf_append(&sbuffer, data3, strlen(data3)) == true);
    ct_test(pTest, sbuf_append(&sbuffer, data4, strlen(data4)) == true);
    strcat(test_data_buffer, data1);
    strcat(test_data_buffer, data2);
    strcat(test_data_buffer, data3);
    strcat(test_data_buffer, data4);
    ct_test(pTest, sbuf_count(&sbuffer) == strlen(test_data_buffer));

    data = sbuf_data(&sbuffer);
    count = sbuf_count(&sbuffer);
    ct_test(pTest, memcmp(data, test_data_buffer, count) == 0);
    ct_test(pTest, count == strlen(test_data_buffer));

    ct_test(pTest, sbuf_truncate(&sbuffer, 0) == true);
    ct_test(pTest, sbuf_count(&sbuffer) == 0);
    ct_test(pTest, sbuf_size(&sbuffer) == sizeof(data_buffer));
    ct_test(pTest, sbuf_append(&sbuffer, data4, strlen(data4)) == true);
    data = sbuf_data(&sbuffer);
    count = sbuf_count(&sbuffer);
    ct_test(pTest, memcmp(data, data4, count) == 0);
    ct_test(pTest, count == strlen(data4));

    return;
}

#ifdef TEST_STATIC_BUFFER
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("static buffer", NULL);

    /* individual tests */
    rc = ct_addTestFunction(pTest, testStaticBuffer);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);

    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_STATIC_BUFFER */
#endif /* TEST */
