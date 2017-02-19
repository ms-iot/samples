/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2006 Steve Karg

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
#include <stdio.h>
#include <string.h>
#include "filename.h"

/** @file filename.c  Function for filename manipulation */

char *filename_remove_path(
    const char *filename_in)
{
    char *filename_out = (char *) filename_in;

    /* allow the device ID to be set */
    if (filename_in) {
        filename_out = strrchr(filename_in, '\\');
        if (!filename_out) {
            filename_out = strrchr(filename_in, '/');
        }
        /* go beyond the slash */
        if (filename_out) {
            filename_out++;
        } else {
            /* no slash in filename */
            filename_out = (char *) filename_in;
        }
    }

    return filename_out;
}

#ifdef TEST
#include <assert.h>
#include <string.h>

#include "ctest.h"

void testFilename(
    Test * pTest)
{
    char *data1 = "c:\\Joshua\\run";
    char *data2 = "/home/Anna/run";
    char *data3 = "c:\\Program Files\\Christopher\\run.exe";
    char *data4 = "//Mary/data/run";
    char *data5 = "bin\\run";
    char *filename = NULL;

    filename = filename_remove_path(data1);
    ct_test(pTest, strcmp("run", filename) == 0);
    filename = filename_remove_path(data2);
    ct_test(pTest, strcmp("run", filename) == 0);
    filename = filename_remove_path(data3);
    ct_test(pTest, strcmp("run.exe", filename) == 0);
    filename = filename_remove_path(data4);
    ct_test(pTest, strcmp("run", filename) == 0);
    filename = filename_remove_path(data5);
    ct_test(pTest, strcmp("run", filename) == 0);

    return;
}

#ifdef TEST_FILENAME
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("filename remove path", NULL);

    /* individual tests */
    rc = ct_addTestFunction(pTest, testFilename);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);

    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_FILENAME */
#endif /* TEST */
