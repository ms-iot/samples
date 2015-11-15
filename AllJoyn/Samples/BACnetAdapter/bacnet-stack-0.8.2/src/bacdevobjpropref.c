/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2008 John Minack

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
#include <stdint.h>
#include <stdbool.h>
#include "bacdcode.h"
#include "npdu.h"
#include "timestamp.h"
#include "bacdevobjpropref.h"

/** @file bacdevobjpropref.c  BACnet Application Device Object (Property) Reference */

int bacapp_encode_context_device_obj_property_ref(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE * value)
{
    int len;
    int apdu_len = 0;

    len = encode_opening_tag(&apdu[apdu_len], tag_number);
    apdu_len += len;

    len = bacapp_encode_device_obj_property_ref(&apdu[apdu_len], value);
    apdu_len += len;

    len = encode_closing_tag(&apdu[apdu_len], tag_number);
    apdu_len += len;

    return apdu_len;
}

int bacapp_encode_device_obj_property_ref(
    uint8_t * apdu,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE * value)
{
    int len;
    int apdu_len = 0;

    len =
        encode_context_object_id(&apdu[apdu_len], 0,
        (int) value->objectIdentifier.type, value->objectIdentifier.instance);
    apdu_len += len;

    len =
        encode_context_enumerated(&apdu[apdu_len], 1,
        value->propertyIdentifier);
    apdu_len += len;

    /* Array index is optional so check if needed before inserting */
    if (value->arrayIndex != BACNET_ARRAY_ALL) {
        len = encode_context_unsigned(&apdu[apdu_len], 2, value->arrayIndex);
        apdu_len += len;
    }

    /* Likewise, device id is optional so see if needed
     * (set type to BACNET_NO_DEV_TYPE or something other than OBJECT_DEVICE to
	 * omit */

    if (value->deviceIndentifier.type == OBJECT_DEVICE) {
        len =
            encode_context_object_id(&apdu[apdu_len], 3,
            (int) value->deviceIndentifier.type,
            value->deviceIndentifier.instance);
        apdu_len += len;
    }
    return apdu_len;
}

int bacapp_decode_device_obj_property_ref(
    uint8_t * apdu,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE * value)
{
    int len;
    int apdu_len = 0;
    uint32_t enumValue;
    if (-1 == (len =
            decode_context_object_id(&apdu[apdu_len], 0,
                &value->objectIdentifier.type,
                &value->objectIdentifier.instance))) {
        return -1;
    }
    apdu_len += len;

    if (-1 == (len =
            decode_context_enumerated(&apdu[apdu_len], 1, &enumValue))) {
        return -1;
    }
    value->propertyIdentifier = (BACNET_PROPERTY_ID) enumValue;
    apdu_len += len;

    if (decode_is_context_tag(&apdu[apdu_len], 2)) {
        if (-1 == (len =
                decode_context_unsigned(&apdu[apdu_len], 2,
                    &value->arrayIndex))) {
            return -1;
        }
        apdu_len += len;
    } else {
        value->arrayIndex = BACNET_ARRAY_ALL;
    }

    if (decode_is_context_tag(&apdu[apdu_len], 3)) {
        if (-1 == (len =
                decode_context_object_id(&apdu[apdu_len], 3,
                    &value->deviceIndentifier.type,
                    &value->deviceIndentifier.instance))) {
            return -1;
        }
        apdu_len += len;
    } else {
    	value->deviceIndentifier.type = BACNET_NO_DEV_TYPE;
    	value->deviceIndentifier.instance = BACNET_NO_DEV_ID;
    }

    return apdu_len;
}

int bacapp_decode_context_device_obj_property_ref(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE * value)
{
    int len = 0;
    int section_length;

    if (decode_is_opening_tag_number(&apdu[len], tag_number)) {
        len++;
        section_length =
            bacapp_decode_device_obj_property_ref(&apdu[len], value);

        if (section_length == -1) {
            len = -1;
        } else {
            len += section_length;
            if (decode_is_closing_tag_number(&apdu[len], tag_number)) {
                len++;
            } else {
                len = -1;
            }
        }
    } else {
        len = -1;
    }
    return len;
}


/* Functions for BACnetDeviceObjectReference: */
int bacapp_encode_context_device_obj_ref(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_DEVICE_OBJECT_REFERENCE * value)
{
    int len;
    int apdu_len = 0;

    len = encode_opening_tag(&apdu[apdu_len], tag_number);
    apdu_len += len;

    len = bacapp_encode_device_obj_ref(&apdu[apdu_len], value);
    apdu_len += len;

    len = encode_closing_tag(&apdu[apdu_len], tag_number);
    apdu_len += len;

    return apdu_len;
}

int bacapp_encode_device_obj_ref(
    uint8_t * apdu,
    BACNET_DEVICE_OBJECT_REFERENCE * value)
{
    int len;
    int apdu_len = 0;

    /* Device id is optional so see if needed
     * (set type to BACNET_NO_DEV_TYPE or something other than OBJECT_DEVICE to
	 * omit */

    if (value->deviceIndentifier.type == OBJECT_DEVICE) {
        len =
            encode_context_object_id(&apdu[apdu_len], 0,
            (int) value->deviceIndentifier.type,
            value->deviceIndentifier.instance);
        apdu_len += len;
    }

    len =
        encode_context_object_id(&apdu[apdu_len], 1,
        (int) value->objectIdentifier.type, value->objectIdentifier.instance);
    apdu_len += len;

    return apdu_len;
}

int bacapp_decode_device_obj_ref(
    uint8_t * apdu,
    BACNET_DEVICE_OBJECT_REFERENCE * value)
{
    int len;
    int apdu_len = 0;

    /* Device ID is optional */
    if (decode_is_context_tag(&apdu[apdu_len], 0)) {
        if (-1 == (len =
                decode_context_object_id(&apdu[apdu_len], 0,
                    &value->deviceIndentifier.type,
                    &value->deviceIndentifier.instance))) {
            return -1;
        }
        apdu_len += len;
    } else {
    	value->deviceIndentifier.type = BACNET_NO_DEV_TYPE;
    	value->deviceIndentifier.instance = BACNET_NO_DEV_ID;
    }

    if (-1 == (len =
            decode_context_object_id(&apdu[apdu_len], 1,
                &value->objectIdentifier.type,
                &value->objectIdentifier.instance))) {
        return -1;
    }
    apdu_len += len;

    return apdu_len;
}

int bacapp_decode_context_device_obj_ref(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_DEVICE_OBJECT_REFERENCE * value)
{
    int len = 0;
    int section_length;

    if (decode_is_opening_tag_number(&apdu[len], tag_number)) {
        len++;
        section_length = bacapp_decode_device_obj_ref(&apdu[len], value);

        if (section_length == -1) {
            len = -1;
        } else {
            len += section_length;
            if (decode_is_closing_tag_number(&apdu[len], tag_number)) {
                len++;
            } else {
                len = -1;
            }
        }
    } else {
        len = -1;
    }
    return len;
}


#ifdef TEST

void testDevIdPropRef(
    Test * pTest)
{
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE inData;
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE outData;
    uint8_t buffer[MAX_APDU];
    int inLen;
    int outLen;


    inData.objectIdentifier.instance = 0x1234;
    inData.objectIdentifier.type = 15;

    inData.propertyIdentifier = 25;

    inData.arrayIndex = 0x5678;

    inData.deviceIndentifier.instance = 0x4343;
    inData.deviceIndentifier.type = 28;

    inLen = bacapp_encode_device_obj_property_ref(buffer, &inData);
    outLen = bacapp_decode_device_obj_property_ref(buffer, &outData);

    ct_test(pTest, outLen == inLen);

    ct_test(pTest,
        inData.objectIdentifier.instance == outData.objectIdentifier.instance);
    ct_test(pTest,
        inData.objectIdentifier.type == outData.objectIdentifier.type);

    ct_test(pTest, inData.propertyIdentifier == outData.propertyIdentifier);

    ct_test(pTest, inData.arrayIndex == outData.arrayIndex);

    ct_test(pTest,
        inData.deviceIndentifier.instance ==
        outData.deviceIndentifier.instance);
    ct_test(pTest,
        inData.deviceIndentifier.type == outData.deviceIndentifier.type);
}

void testDevIdRef(
    Test * pTest)
{
    BACNET_DEVICE_OBJECT_REFERENCE inData;
    BACNET_DEVICE_OBJECT_REFERENCE outData;
    uint8_t buffer[MAX_APDU];
    int inLen;
    int outLen;

    inData.deviceIndentifier.instance = 0x4343;
    inData.deviceIndentifier.type = 28;

    inLen = bacapp_encode_device_obj_ref(buffer, &inData);
    outLen = bacapp_decode_device_obj_ref(buffer, &outData);

    ct_test(pTest, outLen == inLen);

    ct_test(pTest,
        inData.deviceIndentifier.instance ==
        outData.deviceIndentifier.instance);
    ct_test(pTest,
        inData.deviceIndentifier.type == outData.deviceIndentifier.type);
}


#ifdef TEST_DEV_ID_PROP_REF
#include <assert.h>
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Prop Ref", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testDevIdPropRef);
    assert(rc);
    rc = ct_addTestFunction(pTest, testDevIdRef);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}

#endif /* TEST_DEV_ID_PROP_REF */
#endif /* TEST */
