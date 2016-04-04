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
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "keylist.h"
#include "objects.h"

/** @file objects.c  Manage Device Objects. */

/* list of devices */
static OS_Keylist Device_List = NULL;

void objects_init(
    void)
{
    if (!Device_List)
        Device_List = Keylist_Create();
}

int objects_device_count(
    void)
{
    objects_init();
    return Keylist_Count(Device_List);
}

OBJECT_DEVICE_T *objects_device_data(
    int index)
{
    objects_init();
    return Keylist_Data_Index(Device_List, index);
}

OBJECT_DEVICE_T *objects_device_by_instance(
    uint32_t device_instance)
{
    objects_init();
    return Keylist_Data(Device_List, device_instance);
}

OBJECT_DEVICE_T *objects_device_new(
    uint32_t device_instance)
{
    OBJECT_DEVICE_T *pDevice = NULL;
    KEY key = device_instance;

    if (Device_List) {
        /* does this device already exist? */
        pDevice = Keylist_Data(Device_List, key);
        if (pDevice) {
            memset(pDevice, 0, sizeof(OBJECT_DEVICE_T));
        } else {
            pDevice = calloc(1, sizeof(OBJECT_DEVICE_T));
            if (pDevice) {
                pDevice->Object_Identifier.type = OBJECT_DEVICE;
                pDevice->Object_Identifier.instance = device_instance;
                pDevice->Object_Type = OBJECT_DEVICE;
                pDevice->Object_List = Keylist_Create();
                Keylist_Data_Add(Device_List, key, pDevice);
            } else {
                fprintf(stderr,
                    "Objects: Unable to allocate device %lu buffer\n",
                    (unsigned long) device_instance);
            }
        }
    }

    return pDevice;
}

OBJECT_DEVICE_T *objects_device_delete(
    int index)
{
    OBJECT_DEVICE_T *pDevice = NULL;
    BACNET_OBJECT_ID *pObject;

    if (Device_List) {
        pDevice = Keylist_Data_Delete_By_Index(Device_List, index);
        if (pDevice) {
            fprintf(stderr, "Objects: removing device %lu",
                (unsigned long) pDevice->Object_Identifier.instance);
            if (pDevice->Object_List) {
                do {
                    pObject =
                        Keylist_Data_Delete_By_Index(pDevice->Object_List, 0);
                    /* free any dynamic memory used */
                    if (pObject) {
                        free(pObject);
                    }
                } while (pObject);
                Keylist_Delete(pDevice->Object_List);
            }
            free(pDevice);
        }
    }
    return pDevice;
}

#ifdef TEST
#include <assert.h>
#include <string.h>

#include "ctest.h"

/* test the object creation and deletion */
void testBACnetObjectsCompare(
    Test * pTest,
    OBJECT_DEVICE_T * pDevice,
    uint32_t device_id)
{
    ct_test(pTest, pDevice != NULL);
    if (pDevice) {
        ct_test(pTest, pDevice->Object_List != NULL);
        ct_test(pTest, pDevice->Object_Identifier.instance == device_id);
        ct_test(pTest, pDevice->Object_Identifier.type == OBJECT_DEVICE);
        ct_test(pTest, pDevice->Object_Type == OBJECT_DEVICE);
    }
}

void testBACnetObjects(
    Test * pTest)
{
    uint32_t device_id = 0;
    unsigned test_point = 0;
    const unsigned max_test_points = 20;
    OBJECT_DEVICE_T *pDevice;

    for (test_point = 0; test_point < max_test_points; test_point++) {
        device_id = test_point * (BACNET_MAX_INSTANCE / max_test_points);
        pDevice = objects_device_new(device_id);
        testBACnetObjectsCompare(pTest, pDevice, device_id);
        pDevice = objects_device_by_instance(device_id);
        testBACnetObjectsCompare(pTest, pDevice, device_id);
    }
    ct_test(pTest, max_test_points == objects_device_count());
    for (test_point = 0; test_point < max_test_points; test_point++) {
        device_id = test_point * (BACNET_MAX_INSTANCE / max_test_points);
        pDevice = objects_device_by_instance(device_id);
        testBACnetObjectsCompare(pTest, pDevice, device_id);
    }
    for (test_point = 0; test_point < max_test_points; test_point++) {
        device_id = test_point * (BACNET_MAX_INSTANCE / max_test_points);
        pDevice = objects_device_data(test_point);
        testBACnetObjectsCompare(pTest, pDevice, Keylist_Key(Device_List,
                test_point));
    }
    for (test_point = 0; test_point < max_test_points; test_point++) {
        pDevice = objects_device_delete(0);
    }
}

#ifdef TEST_OBJECT_LIST
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Objects", NULL);

    /* individual tests */
    rc = ct_addTestFunction(pTest, testBACnetObjects);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);

    ct_destroy(pTest);

    return 0;
}
#endif
#endif
