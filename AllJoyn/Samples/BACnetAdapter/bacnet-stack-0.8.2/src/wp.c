/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

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
#include "bacenum.h"
#include "bacdcode.h"
#include "bacdef.h"
#include "wp.h"

/** @file wp.c  Encode/Decode BACnet Write Property APDUs  */
#if BACNET_SVC_WP_A
/* encode service */
int wp_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_WRITE_PROPERTY_DATA * wpdata)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    int len = 0;        /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_WRITE_PROPERTY;     /* service choice */
        apdu_len = 4;
        len =
            encode_context_object_id(&apdu[apdu_len], 0, wpdata->object_type,
            wpdata->object_instance);
        apdu_len += len;
        len =
            encode_context_enumerated(&apdu[apdu_len], 1,
            wpdata->object_property);
        apdu_len += len;
        /* optional array index; ALL is -1 which is assumed when missing */
        if (wpdata->array_index != BACNET_ARRAY_ALL) {
            len =
                encode_context_unsigned(&apdu[apdu_len], 2,
                wpdata->array_index);
            apdu_len += len;
        }
        /* propertyValue */
        len = encode_opening_tag(&apdu[apdu_len], 3);
        apdu_len += len;
        for (len = 0; len < wpdata->application_data_len; len++) {
            apdu[apdu_len + len] = wpdata->application_data[len];
        }
        apdu_len += wpdata->application_data_len;
        len = encode_closing_tag(&apdu[apdu_len], 3);
        apdu_len += len;
        /* optional priority - 0 if not set, 1..16 if set */
        if (wpdata->priority != BACNET_NO_PRIORITY) {
            len =
                encode_context_unsigned(&apdu[apdu_len], 4, wpdata->priority);
            apdu_len += len;
        }
    }

    return apdu_len;
}
#endif

/* decode the service request only */
/* FIXME: there could be various error messages returned
   using unique values less than zero */
int wp_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_WRITE_PROPERTY_DATA * wpdata)
{
    int len = 0;
    int tag_len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    uint16_t type = 0;  /* for decoding */
    uint32_t property = 0;      /* for decoding */
    uint32_t unsigned_value = 0;
    int i = 0;  /* loop counter */

    /* check for value pointers */
    if (apdu_len && wpdata) {
        /* Tag 0: Object ID          */
        if (!decode_is_context_tag(&apdu[len++], 0))
            return -1;
        len += decode_object_id(&apdu[len], &type, &wpdata->object_instance);
        wpdata->object_type = (BACNET_OBJECT_TYPE) type;
        /* Tag 1: Property ID */
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number,
            &len_value_type);
        if (tag_number != 1)
            return -1;
        len += decode_enumerated(&apdu[len], len_value_type, &property);
        wpdata->object_property = (BACNET_PROPERTY_ID) property;
        /* Tag 2: Optional Array Index */
        /* note: decode without incrementing len so we can check for opening tag */
        tag_len =
            decode_tag_number_and_value(&apdu[len], &tag_number,
            &len_value_type);
        if (tag_number == 2) {
            len += tag_len;
            len +=
                decode_unsigned(&apdu[len], len_value_type, &unsigned_value);
            wpdata->array_index = unsigned_value;
        } else
            wpdata->array_index = BACNET_ARRAY_ALL;
        /* Tag 3: opening context tag */
        if (!decode_is_opening_tag_number(&apdu[len], 3))
            return -1;
        /* determine the length of the data blob */
        wpdata->application_data_len =
            bacapp_data_len(&apdu[len], apdu_len - len,
            (BACNET_PROPERTY_ID) property);
        /* a tag number of 3 is not extended so only one octet */
        len++;
        /* copy the data from the APDU */
        for (i = 0; i < wpdata->application_data_len; i++) {
            wpdata->application_data[i] = apdu[len + i];
        }
        /* add on the data length */
        len += wpdata->application_data_len;
        if (!decode_is_closing_tag_number(&apdu[len], 3))
            return -2;
        /* a tag number of 3 is not extended so only one octet */
        len++;
        /* Tag 4: optional Priority - assumed MAX if not explicitly set */
        wpdata->priority = BACNET_MAX_PRIORITY;
        if ((unsigned) len < apdu_len) {
            tag_len =
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            if (tag_number == 4) {
                len += tag_len;
                len =
                    decode_unsigned(&apdu[len], len_value_type,
                    &unsigned_value);
                if ((unsigned_value >= BACNET_MIN_PRIORITY)
                    && (unsigned_value <= BACNET_MAX_PRIORITY)) {
                    wpdata->priority = (uint8_t) unsigned_value;
                } else
                    return -5;
            }
        }
    }

    return len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

int wp_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_WRITE_PROPERTY_DATA * wpdata)
{
    int len = 0;
    unsigned offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_CONFIRMED_SERVICE_REQUEST)
        return -1;
    /*  apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU); */
    *invoke_id = apdu[2];       /* invoke id - filled in by net layer */
    if (apdu[3] != SERVICE_CONFIRMED_WRITE_PROPERTY)
        return -1;
    offset = 4;

    if (apdu_len > offset) {
        len =
            wp_decode_service_request(&apdu[offset], apdu_len - offset,
            wpdata);
    }

    return len;
}

void testWritePropertyTag(
    Test * pTest,
    BACNET_APPLICATION_DATA_VALUE * value)
{
    BACNET_WRITE_PROPERTY_DATA wpdata = { 0 };
    BACNET_WRITE_PROPERTY_DATA test_data = { 0 };
    BACNET_APPLICATION_DATA_VALUE test_value;
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 128;
    uint8_t test_invoke_id = 0;

    wpdata.application_data_len =
        bacapp_encode_application_data(&wpdata.application_data[0], value);
    len = wp_encode_apdu(&apdu[0], invoke_id, &wpdata);
    ct_test(pTest, len != 0);
    /* decode the data */
    apdu_len = len;
    len = wp_decode_apdu(&apdu[0], apdu_len, &test_invoke_id, &test_data);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_data.object_type == wpdata.object_type);
    ct_test(pTest, test_data.object_instance == wpdata.object_instance);
    ct_test(pTest, test_data.object_property == wpdata.object_property);
    ct_test(pTest, test_data.array_index == wpdata.array_index);
    /* decode the application value of the request */
    len =
        bacapp_decode_application_data(test_data.application_data,
        test_data.application_data_len, &test_value);
    ct_test(pTest, test_value.tag == value->tag);
    switch (test_value.tag) {
        case BACNET_APPLICATION_TAG_NULL:
            break;
        case BACNET_APPLICATION_TAG_BOOLEAN:
            ct_test(pTest, test_value.type.Boolean == value->type.Boolean);
            break;
        case BACNET_APPLICATION_TAG_UNSIGNED_INT:
            ct_test(pTest,
                test_value.type.Unsigned_Int == value->type.Unsigned_Int);
            break;
        case BACNET_APPLICATION_TAG_SIGNED_INT:
            ct_test(pTest,
                test_value.type.Signed_Int == value->type.Signed_Int);
            break;
        case BACNET_APPLICATION_TAG_REAL:
            ct_test(pTest, test_value.type.Real == value->type.Real);
            break;
        case BACNET_APPLICATION_TAG_ENUMERATED:
            ct_test(pTest,
                test_value.type.Enumerated == value->type.Enumerated);
            break;
        case BACNET_APPLICATION_TAG_DATE:
            ct_test(pTest, test_value.type.Date.year == value->type.Date.year);
            ct_test(pTest,
                test_value.type.Date.month == value->type.Date.month);
            ct_test(pTest, test_value.type.Date.day == value->type.Date.day);
            ct_test(pTest, test_value.type.Date.wday == value->type.Date.wday);
            break;
        case BACNET_APPLICATION_TAG_TIME:
            ct_test(pTest, test_value.type.Time.hour == value->type.Time.hour);
            ct_test(pTest, test_value.type.Time.min == value->type.Time.min);
            ct_test(pTest, test_value.type.Time.sec == value->type.Time.sec);
            ct_test(pTest,
                test_value.type.Time.hundredths ==
                value->type.Time.hundredths);
            break;
        case BACNET_APPLICATION_TAG_OBJECT_ID:
            ct_test(pTest,
                test_value.type.Object_Id.type == value->type.Object_Id.type);
            ct_test(pTest,
                test_value.type.Object_Id.instance ==
                value->type.Object_Id.instance);
            break;
        default:
            break;
    }
}

void testWriteProperty(
    Test * pTest)
{
    BACNET_APPLICATION_DATA_VALUE value;

    value.tag = BACNET_APPLICATION_TAG_NULL;
    testWritePropertyTag(pTest, &value);

    value.tag = BACNET_APPLICATION_TAG_BOOLEAN;
    value.type.Boolean = true;
    testWritePropertyTag(pTest, &value);
    value.type.Boolean = false;
    testWritePropertyTag(pTest, &value);

    value.tag = BACNET_APPLICATION_TAG_UNSIGNED_INT;
    value.type.Unsigned_Int = 0;
    testWritePropertyTag(pTest, &value);
    value.type.Unsigned_Int = 0xFFFF;
    testWritePropertyTag(pTest, &value);
    value.type.Unsigned_Int = 0xFFFFFFFF;
    testWritePropertyTag(pTest, &value);

    value.tag = BACNET_APPLICATION_TAG_SIGNED_INT;
    value.type.Signed_Int = 0;
    testWritePropertyTag(pTest, &value);
    value.type.Signed_Int = -1;
    testWritePropertyTag(pTest, &value);
    value.type.Signed_Int = 32768;
    testWritePropertyTag(pTest, &value);
    value.type.Signed_Int = -32768;
    testWritePropertyTag(pTest, &value);

    value.tag = BACNET_APPLICATION_TAG_REAL;
    value.type.Real = 0.0;
    testWritePropertyTag(pTest, &value);
    value.type.Real = -1.0;
    testWritePropertyTag(pTest, &value);
    value.type.Real = 1.0;
    testWritePropertyTag(pTest, &value);
    value.type.Real = 3.14159;
    testWritePropertyTag(pTest, &value);
    value.type.Real = -3.14159;
    testWritePropertyTag(pTest, &value);

    value.tag = BACNET_APPLICATION_TAG_ENUMERATED;
    value.type.Enumerated = 0;
    testWritePropertyTag(pTest, &value);
    value.type.Enumerated = 0xFFFF;
    testWritePropertyTag(pTest, &value);
    value.type.Enumerated = 0xFFFFFFFF;
    testWritePropertyTag(pTest, &value);

    value.tag = BACNET_APPLICATION_TAG_DATE;
    value.type.Date.year = 2005;
    value.type.Date.month = 5;
    value.type.Date.day = 22;
    value.type.Date.wday = 1;
    testWritePropertyTag(pTest, &value);

    value.tag = BACNET_APPLICATION_TAG_TIME;
    value.type.Time.hour = 23;
    value.type.Time.min = 59;
    value.type.Time.sec = 59;
    value.type.Time.hundredths = 12;
    testWritePropertyTag(pTest, &value);

    value.tag = BACNET_APPLICATION_TAG_OBJECT_ID;
    value.type.Object_Id.type = OBJECT_ANALOG_INPUT;
    value.type.Object_Id.instance = 0;
    testWritePropertyTag(pTest, &value);
    value.type.Object_Id.type = OBJECT_LIFE_SAFETY_ZONE;
    value.type.Object_Id.instance = BACNET_MAX_INSTANCE;
    testWritePropertyTag(pTest, &value);

    return;
}

#ifdef TEST_WRITE_PROPERTY
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet WriteProperty", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testWriteProperty);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_WRITE_PROPERTY */
#endif /* TEST */
