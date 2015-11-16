/**
* @file
* @author Steve Karg
* @date 2004
* @brief Generic interrupt safe FIFO library for deeply embedded system.
*
* @section LICENSE
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to:
* The Free Software Foundation, Inc.
* 59 Temple Place - Suite 330
* Boston, MA  02111-1307
* USA.
*
* As a special exception, if other files instantiate templates or
* use macros or inline functions from this file, or you compile
* this file and link it with other works to produce a work based
* on this file, this file does not by itself cause the resulting
* work to be covered by the GNU General Public License. However
* the source code for this file must still be made available in
* accordance with section (3) of the GNU General Public License.
*
* This exception does not invalidate any other reasons why a work
* based on this file might be covered by the GNU General Public
* License.
*
* @section DESCRIPTION
*
* Generic interrupt safe FIFO library for deeply embedded system
* This library only uses a byte sized chunk for a data element.
* It uses a data store whose size is a power of 2 (8, 16, 32, 64, ...)
* and doesn't waste any data bytes.  It has very low overhead, and
* utilizes modulo for indexing the data in the data store.
*
* To use this library, first declare a data store, sized for a power of 2:
* {@code
* static volatile uint8_t data_store[64];
* }
*
* Then declare the FIFO tracking structure:
* {@code
* static FIFO_BUFFER queue;
* }
*
* Initialize the queue with the data store:
* {@code
* FIFO_Init(&queue, data_store, sizeof(data_store));
* }
*
* Then begin to use the FIFO queue by giving it data, retreiving data,
* and checking the FIFO queue to see if it is empty or full:
* {@code
* uint8_t in_data = 0;
* uint8_t out_data = 0;
* uint8_t add_data[5] = {0};
* uint8_t pull_data[5] = {0};
* unsigned count = 0;
* bool status = false;
*
* status = FIFO_Put(&queue, in_data);
* if (!FIFO_Empty(&queue)) {
*     out_data = FIFO_Get(&queue);
* }
* if (FIFO_Available(&queue, sizeof(add_data))) {
*     status = FIFO_Add(&queue, add_data, sizeof(add_data));
* }
* count = FIFO_Count(&queue);
* if (count == sizeof(add_data)) {
*     count = FIFO_Pull(&queue, &pull_data[0], sizeof(pull_data));
* }
*
* }
*
* Normally the FIFO is used by a producer, such as in interrupt service
* routine, which places data into the queue using FIFO_Put(), and a consumer,
* such as a main loop handler, which pulls data from the queue by first
* checking the queue for data using FIFO_Empty(), and then pulling data from
* the queue using FIFO_Get().
*
*/
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "fifo.h"

/**
* Returns the number of bytes in the FIFO
*
* @param b - pointer to FIFO_BUFFER structure
*
* @return Number of bytes in the FIFO
*/
unsigned FIFO_Count(
    FIFO_BUFFER const *b)
{
    unsigned head, tail;        /* used to avoid volatile decision */

    if (b) {
        head = b->head;
        tail = b->tail;
        return head - tail;
    } else {
        return 0;
    }
}

/**
* Returns the full status of the FIFO
*
* @param b - pointer to FIFO_BUFFER structure
*
* @return true if the FIFO is full, false if it is not.
*/
bool FIFO_Full(
    FIFO_BUFFER const *b)
{
    return (b ? (FIFO_Count(b) == b->buffer_len) : true);
}

/**
* Tests to see if space is available in the FIFO
*
* @param b - pointer to FIFO_BUFFER structure
* @param count [in] - number of bytes tested for availability
*
* @return true if the number of bytes sought is available
*/
bool FIFO_Available(
    FIFO_BUFFER const *b,
    unsigned count)
{
    return (b ? (count <= (b->buffer_len - FIFO_Count(b))) : false);
}

/**
* Returns the empty status of the FIFO
*
* @param b - pointer to FIFO_BUFFER structure
* @return true if the FIFO is empty, false if it is not.
*/
bool FIFO_Empty(
    FIFO_BUFFER const *b)
{
    return (b ? (FIFO_Count(b) == 0) : true);
}

/**
* Peeks at the data from the front of the FIFO without removing it.
* Use FIFO_Empty() or FIFO_Available() function to see if there is
* data to retrieve since this function doesn't return a flag indicating
* success or failure.
*
* @param b - pointer to FIFO_BUFFER structure
*
* @return byte of data, or zero if nothing in the list
*/
uint8_t FIFO_Peek(
    FIFO_BUFFER const *b)
{
    unsigned index;

    if (b) {
        index = b->tail % b->buffer_len;
        return (b->buffer[index]);
    }

    return 0;
}

/**
* Gets a byte from the front of the FIFO, and removes it.
* Use FIFO_Empty() or FIFO_Available() function to see if there is
* data to retrieve since this function doesn't return a flag indicating
* success or failure.
*
* @param b - pointer to FIFO_BUFFER structure
*
* @return the data
*/
uint8_t FIFO_Get(
    FIFO_BUFFER * b)
{
    uint8_t data_byte = 0;
    unsigned index;

    if (!FIFO_Empty(b)) {
        index = b->tail % b->buffer_len;
        data_byte = b->buffer[index];
        b->tail++;
    }
    return data_byte;
}

/**
* Pulls one or more bytes from the front of the FIFO, and removes them
* from the FIFO.  If less bytes are available, only the available bytes
* are retrieved.
*
* @param b - pointer to FIFO_BUFFER structure
* @param buffer [out] - buffer to hold the pulled bytes
* @param length [in] - number of bytes to pull from the FIFO
*
* @return      the number of bytes actually pulled from the FIFO
*/
unsigned FIFO_Pull(
    FIFO_BUFFER * b,
    uint8_t * buffer,
    unsigned length)
{
    unsigned count;
    uint8_t data_byte;
    unsigned index;

    count = FIFO_Count(b);
    if (count > length) {
        /* adjust to limit the number of bytes pulled */
        count = length;
    }
    if (length > count) {
        /* adjust the return value */
        length = count;
    }
    while (count) {
        index = b->tail % b->buffer_len;
        data_byte = b->buffer[index];
        b->tail++;
        if (buffer) {
            *buffer = data_byte;
            buffer++;
        }
        count--;
    }

    return length;
}

/**
* Adds a byte of data to the FIFO
*
* @param  b - pointer to FIFO_BUFFER structure
* @param  data_byte [in] - data to put into the FIFO
*
* @return true on successful add, false if not added
*/
bool FIFO_Put(
    FIFO_BUFFER * b,
    uint8_t data_byte)
{
    bool status = false;        /* return value */
    unsigned index;

    if (b) {
        /* limit the buffer to prevent overwriting */
        if (!FIFO_Full(b)) {
            index = b->head % b->buffer_len;
            b->buffer[index] = data_byte;
            b->head++;
            status = true;
        }
    }

    return status;
}

/**
* Adds one or more bytes of data to the FIFO
*
* @param  b - pointer to FIFO_BUFFER structure
* @param  buffer [out] - data bytes to add to the FIFO
* @param  count [in] - number of bytes to add to the FIFO
*
* @return true if space available and added, false if not added
*/
bool FIFO_Add(
    FIFO_BUFFER * b,
    uint8_t * buffer,
    unsigned count)
{
    bool status = false;        /* return value */
    unsigned index;

    /* limit the buffer to prevent overwriting */
    if (FIFO_Available(b, count) && buffer) {
        while (count) {
            index = b->head % b->buffer_len;
            b->buffer[index] = *buffer;
            b->head++;
            buffer++;
            count--;
        }
        status = true;
    }

    return status;
}

/**
* Flushes any data in the FIFO buffer
*
* @param  b - pointer to FIFO_BUFFER structure
*
* @return none
*/
void FIFO_Flush(
    FIFO_BUFFER * b)
{
    unsigned head;      /* used to avoid volatile decision */

    if (b) {
        head = b->head;
        b->tail = head;
    }
}

/**
* Initializes the FIFO buffer with a data store
*
* @param  b - pointer to FIFO_BUFFER structure
* @param  buffer [in] - data bytes used to store bytes used by the FIFO
* @param  buffer_len [in] - size of the buffer in bytes - must be power of 2.
*
* @return      none
*/
void FIFO_Init(
    FIFO_BUFFER * b,
    volatile uint8_t * buffer,
    unsigned buffer_len)
{
    if (b && buffer && buffer_len) {
        b->head = 0;
        b->tail = 0;
        b->buffer = buffer;
        b->buffer_len = buffer_len;
    }

    return;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "ctest.h"

/**
* Unit Test for the FIFO buffer
*
* @param pTest - test tracking pointer
*/
void testFIFOBuffer(
    Test * pTest)
{
    /* FIFO data structure */
    FIFO_BUFFER test_buffer = { 0 };
    /* FIFO data store. Note:  size must be a power of two! */
    volatile uint8_t data_store[64] = { 0 };
    uint8_t add_data[40] = { "RoseSteveLouPatRachelJessicaDaniAmyHerb" };
    uint8_t test_add_data[40] = { 0 };
    uint8_t test_data = 0;
    unsigned index = 0;
    unsigned count = 0;
    bool status = 0;

    FIFO_Init(&test_buffer, data_store, sizeof(data_store));
    ct_test(pTest, FIFO_Empty(&test_buffer));

    /* load the buffer */
    for (test_data = 0; test_data < sizeof(data_store); test_data++) {
        ct_test(pTest, !FIFO_Full(&test_buffer));
        ct_test(pTest, FIFO_Available(&test_buffer, 1));
        status = FIFO_Put(&test_buffer, test_data);
        ct_test(pTest, status == true);
        ct_test(pTest, !FIFO_Empty(&test_buffer));
    }
    /* not able to put any more */
    ct_test(pTest, FIFO_Full(&test_buffer));
    ct_test(pTest, !FIFO_Available(&test_buffer, 1));
    status = FIFO_Put(&test_buffer, 42);
    ct_test(pTest, status == false);
    /* unload the buffer */
    for (index = 0; index < sizeof(data_store); index++) {
        ct_test(pTest, !FIFO_Empty(&test_buffer));
        test_data = FIFO_Peek(&test_buffer);
        ct_test(pTest, test_data == index);
        test_data = FIFO_Get(&test_buffer);
        ct_test(pTest, test_data == index);
        ct_test(pTest, FIFO_Available(&test_buffer, 1));
        ct_test(pTest, !FIFO_Full(&test_buffer));
    }
    ct_test(pTest, FIFO_Empty(&test_buffer));
    test_data = FIFO_Get(&test_buffer);
    ct_test(pTest, test_data == 0);
    test_data = FIFO_Peek(&test_buffer);
    ct_test(pTest, test_data == 0);
    ct_test(pTest, FIFO_Empty(&test_buffer));
    /* test the ring around the buffer */
    for (index = 0; index < sizeof(data_store); index++) {
        ct_test(pTest, FIFO_Empty(&test_buffer));
        ct_test(pTest, FIFO_Available(&test_buffer, 4));
        for (count = 1; count < 4; count++) {
            test_data = count;
            status = FIFO_Put(&test_buffer, test_data);
            ct_test(pTest, status == true);
            ct_test(pTest, !FIFO_Empty(&test_buffer));
        }
        for (count = 1; count < 4; count++) {
            ct_test(pTest, !FIFO_Empty(&test_buffer));
            test_data = FIFO_Peek(&test_buffer);
            ct_test(pTest, test_data == count);
            test_data = FIFO_Get(&test_buffer);
            ct_test(pTest, test_data == count);
        }
    }
    ct_test(pTest, FIFO_Empty(&test_buffer));
    /* test Add */
    ct_test(pTest, FIFO_Available(&test_buffer, sizeof(add_data)));
    status = FIFO_Add(&test_buffer, add_data, sizeof(add_data));
    ct_test(pTest, status == true);
    count = FIFO_Count(&test_buffer);
    ct_test(pTest, count == sizeof(add_data));
    ct_test(pTest, !FIFO_Empty(&test_buffer));
    for (index = 0; index < sizeof(add_data); index++) {
        /* unload the buffer */
        ct_test(pTest, !FIFO_Empty(&test_buffer));
        test_data = FIFO_Peek(&test_buffer);
        ct_test(pTest, test_data == add_data[index]);
        test_data = FIFO_Get(&test_buffer);
        ct_test(pTest, test_data == add_data[index]);
    }
    ct_test(pTest, FIFO_Empty(&test_buffer));
    /* test Pull */
    ct_test(pTest, FIFO_Available(&test_buffer, sizeof(add_data)));
    status = FIFO_Add(&test_buffer, add_data, sizeof(add_data));
    ct_test(pTest, status == true);
    count = FIFO_Count(&test_buffer);
    ct_test(pTest, count == sizeof(add_data));
    ct_test(pTest, !FIFO_Empty(&test_buffer));
    count = FIFO_Pull(&test_buffer, &test_add_data[0], sizeof(test_add_data));
    ct_test(pTest, FIFO_Empty(&test_buffer));
    ct_test(pTest, count == sizeof(test_add_data));
    for (index = 0; index < sizeof(add_data); index++) {
        ct_test(pTest, test_add_data[index] == add_data[index]);
    }
    ct_test(pTest, FIFO_Available(&test_buffer, sizeof(add_data)));
    status = FIFO_Add(&test_buffer, test_add_data, sizeof(add_data));
    ct_test(pTest, status == true);
    ct_test(pTest, !FIFO_Empty(&test_buffer));
    for (index = 0; index < sizeof(add_data); index++) {
        count = FIFO_Pull(&test_buffer, &test_add_data[0], 1);
        ct_test(pTest, count == 1);
        ct_test(pTest, test_add_data[0] == add_data[index]);
    }
    ct_test(pTest, FIFO_Empty(&test_buffer));
    /* test flush */
    status = FIFO_Add(&test_buffer, test_add_data, sizeof(test_add_data));
    ct_test(pTest, status == true);
    ct_test(pTest, !FIFO_Empty(&test_buffer));
    FIFO_Flush(&test_buffer);
    ct_test(pTest, FIFO_Empty(&test_buffer));

    return;
}

#ifdef TEST_FIFO_BUFFER
/**
* Main program entry for Unit Test
*
* @return  returns 0 on success, and non-zero on fail.
*/
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("FIFO Buffer", NULL);

    /* individual tests */
    rc = ct_addTestFunction(pTest, testFIFOBuffer);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);

    ct_destroy(pTest);

    return 0;
}
#endif
#endif
