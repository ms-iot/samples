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
#include "bacerror.h"

/** @file bacerror.c  Encode/Decode BACnet Errors */

/* encode service */
int bacerror_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_CONFIRMED_SERVICE service,
    BACNET_ERROR_CLASS error_class,
    BACNET_ERROR_CODE error_code)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_ERROR;
        apdu[1] = invoke_id;
        apdu[2] = service;
        apdu_len = 3;
        /* service parameters */
        apdu_len +=
            encode_application_enumerated(&apdu[apdu_len], error_class);
        apdu_len += encode_application_enumerated(&apdu[apdu_len], error_code);
    }

    return apdu_len;
}

#if !BACNET_SVC_SERVER
/* decode the application class and code */
int bacerror_decode_error_class_and_code(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code)
{
    int len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    uint32_t decoded_value = 0;

    if (apdu_len) {
        /* error class */
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number,
            &len_value_type);
        if (tag_number != BACNET_APPLICATION_TAG_ENUMERATED)
            return 0;
        len += decode_enumerated(&apdu[len], len_value_type, &decoded_value);
        if (error_class)
            *error_class = (BACNET_ERROR_CLASS) decoded_value;
        /* error code */
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number,
            &len_value_type);
        if (tag_number != BACNET_APPLICATION_TAG_ENUMERATED)
            return 0;
        len += decode_enumerated(&apdu[len], len_value_type, &decoded_value);
        if (error_code)
            *error_code = (BACNET_ERROR_CODE) decoded_value;
    }

    return len;
}

/* decode the service request only */
int bacerror_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_CONFIRMED_SERVICE * service,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code)
{
    int len = 0;

    if (apdu_len > 2) {
        if (invoke_id)
            *invoke_id = apdu[0];
        if (service)
            *service = (BACNET_CONFIRMED_SERVICE) apdu[1];
        /* decode the application class and code */
        len =
            bacerror_decode_error_class_and_code(&apdu[2], apdu_len - 2,
            error_class, error_code);
    }

    return len;
}
#endif

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

/* decode the whole APDU - mainly used for unit testing */
int bacerror_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_CONFIRMED_SERVICE * service,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code)
{
    int len = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu_len) {
        if (apdu[0] != PDU_TYPE_ERROR)
            return -1;
        if (apdu_len > 1) {
            len =
                bacerror_decode_service_request(&apdu[1], apdu_len - 1,
                invoke_id, service, error_class, error_code);
        }
    }

    return len;
}

void testBACError(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 0;
    BACNET_CONFIRMED_SERVICE service = 0;
    BACNET_ERROR_CLASS error_class = 0;
    BACNET_ERROR_CODE error_code = 0;
    uint8_t test_invoke_id = 0;
    BACNET_CONFIRMED_SERVICE test_service = 0;
    BACNET_ERROR_CLASS test_error_class = 0;
    BACNET_ERROR_CODE test_error_code = 0;

    len =
        bacerror_encode_apdu(&apdu[0], invoke_id, service, error_class,
        error_code);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len =
        bacerror_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_service, &test_error_class, &test_error_code);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, test_service == service);
    ct_test(pTest, test_error_class == error_class);
    ct_test(pTest, test_error_code == error_code);

    /* change type to get negative response */
    apdu[0] = PDU_TYPE_ABORT;
    len =
        bacerror_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_service, &test_error_class, &test_error_code);
    ct_test(pTest, len == -1);

    /* test NULL APDU */
    len =
        bacerror_decode_apdu(NULL, apdu_len, &test_invoke_id, &test_service,
        &test_error_class, &test_error_code);
    ct_test(pTest, len == -1);

    /* force a zero length */
    len =
        bacerror_decode_apdu(&apdu[0], 0, &test_invoke_id, &test_service,
        &test_error_class, &test_error_code);
    ct_test(pTest, len == 0);


    /* check them all...   */
    for (service = 0; service < MAX_BACNET_CONFIRMED_SERVICE; service++) {
        for (error_class = 0; error_class < ERROR_CLASS_PROPRIETARY_FIRST;
            error_class++) {
            for (error_code = 0; error_code < ERROR_CODE_PROPRIETARY_FIRST;
                error_code++) {
                len =
                    bacerror_encode_apdu(&apdu[0], invoke_id, service,
                    error_class, error_code);
                apdu_len = len;
                ct_test(pTest, len != 0);
                len =
                    bacerror_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
                    &test_service, &test_error_class, &test_error_code);
                ct_test(pTest, len != -1);
                ct_test(pTest, test_invoke_id == invoke_id);
                ct_test(pTest, test_service == service);
                ct_test(pTest, test_error_class == error_class);
                ct_test(pTest, test_error_code == error_code);
            }
        }
    }

    /* max boundaries */
    service = 255;
    error_class = ERROR_CLASS_PROPRIETARY_LAST;
    error_code = ERROR_CODE_PROPRIETARY_LAST;
    len =
        bacerror_encode_apdu(&apdu[0], invoke_id, service, error_class,
        error_code);
    apdu_len = len;
    ct_test(pTest, len != 0);
    len =
        bacerror_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_service, &test_error_class, &test_error_code);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, test_service == service);
    ct_test(pTest, test_error_class == error_class);
    ct_test(pTest, test_error_code == error_code);

}

#ifdef TEST_BACERROR
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Error", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBACError);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_ERROR */
#endif /* TEST */
