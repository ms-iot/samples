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
#include "ihave.h"

/** @file ihave.c  Encode/Decode I-Have service */

int ihave_encode_apdu(
    uint8_t * apdu,
    BACNET_I_HAVE_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu && data) {
        apdu[0] = PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST;
        apdu[1] = SERVICE_UNCONFIRMED_I_HAVE;
        apdu_len = 2;
        /* deviceIdentifier */
        len =
            encode_application_object_id(&apdu[apdu_len],
            (int) data->device_id.type, data->device_id.instance);
        apdu_len += len;
        /* objectIdentifier */
        len =
            encode_application_object_id(&apdu[apdu_len],
            (int) data->object_id.type, data->object_id.instance);
        apdu_len += len;
        /* objectName */
        len =
            encode_application_character_string(&apdu[apdu_len],
            &data->object_name);
        apdu_len += len;
    }

    return apdu_len;
}

#if BACNET_SVC_I_HAVE_A

/* decode the service request only */
int ihave_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_I_HAVE_DATA * data)
{
    int len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    uint16_t decoded_type = 0;  /* for decoding */

    if (apdu_len && data) {
        /* deviceIdentifier */
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        if (tag_number == BACNET_APPLICATION_TAG_OBJECT_ID) {
            len +=
                decode_object_id(&apdu[len], &decoded_type,
                &data->device_id.instance);
            data->device_id.type = decoded_type;
        } else
            return -1;
        /* objectIdentifier */
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        if (tag_number == BACNET_APPLICATION_TAG_OBJECT_ID) {
            len +=
                decode_object_id(&apdu[len], &decoded_type,
                &data->object_id.instance);
            data->object_id.type = decoded_type;
        } else
            return -1;
        /* objectName */
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        if (tag_number == BACNET_APPLICATION_TAG_CHARACTER_STRING) {
            len +=
                decode_character_string(&apdu[len], len_value,
                &data->object_name);
        } else
            return -1;
    } else
        return -1;

    return len;
}

int ihave_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_I_HAVE_DATA * data)
{
    int len = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST)
        return -1;
    if (apdu[1] != SERVICE_UNCONFIRMED_I_HAVE)
        return -1;
    len = ihave_decode_service_request(&apdu[2], apdu_len - 2, data);

    return len;
}
#endif

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

void testIHaveData(
    Test * pTest,
    BACNET_I_HAVE_DATA * data)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    BACNET_I_HAVE_DATA test_data;

    len = ihave_encode_apdu(&apdu[0], data);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len = ihave_decode_apdu(&apdu[0], apdu_len, &test_data);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_data.device_id.type == data->device_id.type);
    ct_test(pTest, test_data.device_id.instance == data->device_id.instance);
    ct_test(pTest, test_data.object_id.type == data->object_id.type);
    ct_test(pTest, test_data.object_id.instance == data->object_id.instance);
    ct_test(pTest, characterstring_same(&test_data.object_name,
            &data->object_name));
}

void testIHave(
    Test * pTest)
{
    BACNET_I_HAVE_DATA data;

    characterstring_init_ansi(&data.object_name, "Patricia - my love!");
    data.device_id.type = OBJECT_DEVICE;
    for (data.device_id.instance = 1;
        data.device_id.instance <= BACNET_MAX_INSTANCE;
        data.device_id.instance <<= 1) {
        for (data.object_id.type = OBJECT_ANALOG_INPUT;
            data.object_id.type < MAX_BACNET_OBJECT_TYPE;
            data.object_id.type++) {
            for (data.object_id.instance = 1;
                data.object_id.instance <= BACNET_MAX_INSTANCE;
                data.object_id.instance <<= 1) {
                testIHaveData(pTest, &data);
            }
        }
    }
}

#ifdef TEST_I_HAVE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet I-Have", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testIHave);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_WHOIS */
#endif /* TEST */
