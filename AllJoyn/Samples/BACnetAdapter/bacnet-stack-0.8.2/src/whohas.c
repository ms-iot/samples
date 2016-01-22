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
#include <stdint.h>
#include "bacenum.h"
#include "bacdcode.h"
#include "bacdef.h"
#include "whohas.h"

/** @file whohas.c  Encode/Decode Who-Has requests */

/* encode service  - use -1 for limit for unlimited */

int whohas_encode_apdu(
    uint8_t * apdu,
    BACNET_WHO_HAS_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu && data) {
        apdu[0] = PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST;
        apdu[1] = SERVICE_UNCONFIRMED_WHO_HAS;  /* service choice */
        apdu_len = 2;
        /* optional limits - must be used as a pair */
        if ((data->low_limit >= 0)
            && (data->low_limit <= BACNET_MAX_INSTANCE)
            && (data->high_limit >= 0)
            && (data->high_limit <= BACNET_MAX_INSTANCE)) {
            len = encode_context_unsigned(&apdu[apdu_len], 0, data->low_limit);
            apdu_len += len;
            len =
                encode_context_unsigned(&apdu[apdu_len], 1, data->high_limit);
            apdu_len += len;
        }
        if (data->is_object_name) {
            len =
                encode_context_character_string(&apdu[apdu_len], 3,
                &data->object.name);
            apdu_len += len;
        } else {
            len =
                encode_context_object_id(&apdu[apdu_len], 2,
                (int) data->object.identifier.type,
                data->object.identifier.instance);
            apdu_len += len;
        }
    }

    return apdu_len;
}

/* decode the service request only */
int whohas_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_WHO_HAS_DATA * data)
{
    int len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    uint32_t decoded_value = 0; /* for decoding */
    uint16_t decoded_type = 0;  /* for decoding */

    if (apdu_len && data) {
        /* optional limits - must be used as a pair */
        if (decode_is_context_tag(&apdu[len], 0)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len += decode_unsigned(&apdu[len], len_value, &decoded_value);
            if (decoded_value <= BACNET_MAX_INSTANCE)
                data->low_limit = decoded_value;
            if (!decode_is_context_tag(&apdu[len], 1))
                return -1;
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len += decode_unsigned(&apdu[len], len_value, &decoded_value);
            if (decoded_value <= BACNET_MAX_INSTANCE)
                data->high_limit = decoded_value;
        } else {
            data->low_limit = -1;
            data->high_limit = -1;
        }
        /* object id */
        if (decode_is_context_tag(&apdu[len], 2)) {
            data->is_object_name = false;
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len +=
                decode_object_id(&apdu[len], &decoded_type,
                &data->object.identifier.instance);
            data->object.identifier.type = decoded_type;
        }
        /* object name */
        else if (decode_is_context_tag(&apdu[len], 3)) {
            data->is_object_name = true;
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            len +=
                decode_character_string(&apdu[len], len_value,
                &data->object.name);
        }
        /* missing required parameters */
        else
            return -1;
    }

    return len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

int whohas_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_WHO_HAS_DATA * data)
{
    int len = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST)
        return -1;
    if (apdu[1] != SERVICE_UNCONFIRMED_WHO_HAS)
        return -1;
    /* optional limits - must be used as a pair */
    if (apdu_len > 2) {
        len = whohas_decode_service_request(&apdu[2], apdu_len - 2, data);
    }

    return len;
}

void testWhoHasData(
    Test * pTest,
    BACNET_WHO_HAS_DATA * data)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    BACNET_WHO_HAS_DATA test_data;

    len = whohas_encode_apdu(&apdu[0], data);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len = whohas_decode_apdu(&apdu[0], apdu_len, &test_data);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_data.low_limit == data->low_limit);
    ct_test(pTest, test_data.high_limit == data->high_limit);
    ct_test(pTest, test_data.is_object_name == data->is_object_name);
    /* Object ID */
    if (data->is_object_name == false) {
        ct_test(pTest,
            test_data.object.identifier.type == data->object.identifier.type);
        ct_test(pTest,
            test_data.object.identifier.instance ==
            data->object.identifier.instance);
    }
    /* Object Name */
    else {
        ct_test(pTest, characterstring_same(&test_data.object.name,
                &data->object.name));
    }
}

void testWhoHas(
    Test * pTest)
{
    BACNET_WHO_HAS_DATA data;

    data.low_limit = -1;
    data.high_limit = -1;
    data.is_object_name = false;
    data.object.identifier.type = OBJECT_ANALOG_INPUT;
    data.object.identifier.instance = 1;
    testWhoHasData(pTest, &data);

    for (data.low_limit = 0; data.low_limit <= BACNET_MAX_INSTANCE;
        data.low_limit += (BACNET_MAX_INSTANCE / 4)) {
        for (data.high_limit = 0; data.high_limit <= BACNET_MAX_INSTANCE;
            data.high_limit += (BACNET_MAX_INSTANCE / 4)) {
            data.is_object_name = false;
            for (data.object.identifier.type = OBJECT_ANALOG_INPUT;
                data.object.identifier.type < MAX_BACNET_OBJECT_TYPE;
                data.object.identifier.type++) {
                for (data.object.identifier.instance = 1;
                    data.object.identifier.instance <= BACNET_MAX_INSTANCE;
                    data.object.identifier.instance <<= 1) {
                    testWhoHasData(pTest, &data);
                }
            }
            data.is_object_name = true;
            characterstring_init_ansi(&data.object.name, "patricia");
            testWhoHasData(pTest, &data);
        }
    }
}

#ifdef TEST_WHOHAS
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Who-Has", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testWhoHas);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_WHOIS */
#endif /* TEST */
