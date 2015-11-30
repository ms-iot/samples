/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2008 Steve Karg

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
 Boston, MA  02111-1307, USA.

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
#include <stddef.h>
#include "memcopy.h"
#include <string.h>

/** @file memcopy.c  Custom memcopy function */

/* copy len bytes from src to offset of dest if there is enough space. */
/* returns 0 if there is not enough space, or the number of bytes copied. */
size_t memcopy(
    void *dest,
    void *src,
    size_t offset,      /* where in dest to put the data */
    size_t len, /* amount of data to copy */
    size_t max)
{       /* total size of destination */
/*    size_t i; */
/*    size_t copy_len = 0; */
/*    char *s1, *s2; */

/*    s1 = dest; */
/*    s2 = src; */
    if (len <= (max - offset)) {
        memcpy(&((char *) dest)[offset], src, len);
        return (len);
/*        for (i = 0; i < len; i++) { */
/*            s1[offset + i] = s2[i]; */
/*            copy_len++; */
/*        } */
    }

    return 0;
}

#ifdef TEST
#include <assert.h>
#include <string.h>

#include "ctest.h"

void test_memcopy(
    Test * pTest)
{
    char *data1 = "Joshua";
    char *data2 = "Anna";
    char buffer[480] = "";
    char big_buffer[480] = "";
    size_t len = 0;

    len = memcopy(&buffer[0], &data1[0], 0, sizeof(data1), sizeof(buffer));
    ct_test(pTest, len == sizeof(data1));
    ct_test(pTest, memcmp(&buffer[0], &data1[0], len) == 0);
    len = memcopy(&buffer[0], &data2[0], len, sizeof(data2), sizeof(buffer));
    ct_test(pTest, len == sizeof(data2));
    len =
        memcopy(&buffer[0], &big_buffer[0], 1, sizeof(big_buffer),
        sizeof(buffer));
    ct_test(pTest, len == 0);
}

#ifdef TEST_MEM_COPY
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("Memory Copy", NULL);

    /* individual tests */
    rc = ct_addTestFunction(pTest, test_memcopy);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);

    ct_destroy(pTest);

    return 0;
}
#endif
#endif /* TEST */
