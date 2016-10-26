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
#include "bacdef.h"
#include "npdu.h"
#include "dcc.h"
#include "bacdcode.h"
#include "address.h"
#include "iam.h"

/** @file iam.c  Encode/Decode I-Am service */

/* encode I-Am service */
int iam_encode_apdu(
    uint8_t * apdu,
    uint32_t device_id,
    unsigned max_apdu,
    int segmentation,
    uint16_t vendor_id)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST;
        apdu[1] = SERVICE_UNCONFIRMED_I_AM;     /* service choice */
        apdu_len = 2;
        len =
            encode_application_object_id(&apdu[apdu_len], OBJECT_DEVICE,
            device_id);
        apdu_len += len;
        len = encode_application_unsigned(&apdu[apdu_len], max_apdu);
        apdu_len += len;
        len =
            encode_application_enumerated(&apdu[apdu_len],
            (uint32_t) segmentation);
        apdu_len += len;
        len = encode_application_unsigned(&apdu[apdu_len], vendor_id);
        apdu_len += len;
    }

    return apdu_len;
}

int iam_decode_service_request(
    uint8_t * apdu,
    uint32_t * pDevice_id,
    unsigned *pMax_apdu,
    int *pSegmentation,
    uint16_t * pVendor_id)
{
    int len = 0;
    int apdu_len = 0;   /* total length of the apdu, return value */
    uint16_t object_type = 0;   /* should be a Device Object */
    uint32_t object_instance = 0;
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    uint32_t decoded_value = 0;

    /* OBJECT ID - object id */
    len =
        decode_tag_number_and_value(&apdu[apdu_len], &tag_number, &len_value);
    apdu_len += len;
    if (tag_number != BACNET_APPLICATION_TAG_OBJECT_ID)
        return -1;
    len = decode_object_id(&apdu[apdu_len], &object_type, &object_instance);
    apdu_len += len;
    if (object_type != OBJECT_DEVICE)
        return -1;
    if (pDevice_id)
        *pDevice_id = object_instance;
    /* MAX APDU - unsigned */
    len =
        decode_tag_number_and_value(&apdu[apdu_len], &tag_number, &len_value);
    apdu_len += len;
    if (tag_number != BACNET_APPLICATION_TAG_UNSIGNED_INT)
        return -1;
    len = decode_unsigned(&apdu[apdu_len], len_value, &decoded_value);
    apdu_len += len;
    if (pMax_apdu)
        *pMax_apdu = (unsigned) decoded_value;
    /* Segmentation - enumerated */
    len =
        decode_tag_number_and_value(&apdu[apdu_len], &tag_number, &len_value);
    apdu_len += len;
    if (tag_number != BACNET_APPLICATION_TAG_ENUMERATED)
        return -1;
    len = decode_enumerated(&apdu[apdu_len], len_value, &decoded_value);
    apdu_len += len;
    if (decoded_value >= MAX_BACNET_SEGMENTATION)
        return -1;
    if (pSegmentation)
        *pSegmentation = (int) decoded_value;
    /* Vendor ID - unsigned16 */
    len =
        decode_tag_number_and_value(&apdu[apdu_len], &tag_number, &len_value);
    apdu_len += len;
    if (tag_number != BACNET_APPLICATION_TAG_UNSIGNED_INT)
        return -1;
    len = decode_unsigned(&apdu[apdu_len], len_value, &decoded_value);
    apdu_len += len;
    if (decoded_value > 0xFFFF)
        return -1;
    if (pVendor_id)
        *pVendor_id = (uint16_t) decoded_value;

    return apdu_len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

int iam_decode_apdu(
    uint8_t * apdu,
    uint32_t * pDevice_id,
    unsigned *pMax_apdu,
    int *pSegmentation,
    uint16_t * pVendor_id)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    /* valid data? */
    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST)
        return -1;
    if (apdu[1] != SERVICE_UNCONFIRMED_I_AM)
        return -1;
    apdu_len =
        iam_decode_service_request(&apdu[2], pDevice_id, pMax_apdu,
        pSegmentation, pVendor_id);

    return apdu_len;
}

void testIAm(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    uint32_t device_id = 42;
    unsigned max_apdu = 480;
    int segmentation = SEGMENTATION_NONE;
    uint16_t vendor_id = 42;
    uint32_t test_device_id = 0;
    unsigned test_max_apdu = 0;
    int test_segmentation = 0;
    uint16_t test_vendor_id = 0;

    len =
        iam_encode_apdu(&apdu[0], device_id, max_apdu, segmentation,
        vendor_id);
    ct_test(pTest, len != 0);

    len =
        iam_decode_apdu(&apdu[0], &test_device_id, &test_max_apdu,
        &test_segmentation, &test_vendor_id);

    ct_test(pTest, len != -1);
    ct_test(pTest, test_device_id == device_id);
    ct_test(pTest, test_vendor_id == vendor_id);
    ct_test(pTest, test_max_apdu == max_apdu);
    ct_test(pTest, test_segmentation == segmentation);
}

#ifdef TEST_IAM
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet I-Am", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testIAm);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_IAM */
#endif /* TEST */
