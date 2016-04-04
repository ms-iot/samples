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
#include "reject.h"

/** @file reject.c  Encode/Decode Reject APDUs */

/* Helper function to avoid needing additional entries in service data structures
 * when passing back reject status.
 * Convert from error code to reject code.
 * Anything not defined gets converted to REJECT_REASON_OTHER.
 * Will need reworking if it is required to return proprietary reject codes.
 */

BACNET_REJECT_REASON reject_convert_error_code(
    BACNET_ERROR_CODE error_code)
{
    BACNET_REJECT_REASON reject_code = REJECT_REASON_OTHER;

    switch (error_code) {
        case ERROR_CODE_REJECT_BUFFER_OVERFLOW:
            reject_code = REJECT_REASON_BUFFER_OVERFLOW;
            break;
        case ERROR_CODE_REJECT_INCONSISTENT_PARAMETERS:
            reject_code = REJECT_REASON_INCONSISTENT_PARAMETERS;
            break;
        case ERROR_CODE_REJECT_INVALID_PARAMETER_DATA_TYPE:
            reject_code = REJECT_REASON_INVALID_PARAMETER_DATA_TYPE;
            break;
        case ERROR_CODE_REJECT_INVALID_TAG:
            reject_code = REJECT_REASON_INVALID_TAG;
            break;
        case ERROR_CODE_REJECT_MISSING_REQUIRED_PARAMETER:
            reject_code = REJECT_REASON_MISSING_REQUIRED_PARAMETER;
            break;
        case ERROR_CODE_REJECT_PARAMETER_OUT_OF_RANGE:
            reject_code = REJECT_REASON_PARAMETER_OUT_OF_RANGE;
            break;
        case ERROR_CODE_REJECT_TOO_MANY_ARGUMENTS:
            reject_code = REJECT_REASON_TOO_MANY_ARGUMENTS;
            break;
        case ERROR_CODE_REJECT_UNDEFINED_ENUMERATION:
            reject_code = REJECT_REASON_UNDEFINED_ENUMERATION;
            break;
        case ERROR_CODE_REJECT_UNRECOGNIZED_SERVICE:
            reject_code = REJECT_REASON_UNRECOGNIZED_SERVICE;
            break;
        case ERROR_CODE_REJECT_PROPRIETARY:
            reject_code = REJECT_REASON_PROPRIETARY_FIRST;
            break;
        case ERROR_CODE_REJECT_OTHER:
        default:
            reject_code = REJECT_REASON_OTHER;
            break;
    }

    return (reject_code);
}

/* encode service */
int reject_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    uint8_t reject_reason)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_REJECT;
        apdu[1] = invoke_id;
        apdu[2] = reject_reason;
        apdu_len = 3;
    }

    return apdu_len;
}

#if !BACNET_SVC_SERVER
/* decode the service request only */
int reject_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    uint8_t * reject_reason)
{
    int len = 0;

    if (apdu_len) {
        if (invoke_id)
            *invoke_id = apdu[0];
        if (reject_reason)
            *reject_reason = apdu[1];
    }

    return len;
}
#endif

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

/* decode the whole APDU - mainly used for unit testing */
int reject_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    uint8_t * reject_reason)
{
    int len = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu_len) {
        if (apdu[0] != PDU_TYPE_REJECT)
            return -1;
        if (apdu_len > 1) {
            len =
                reject_decode_service_request(&apdu[1], apdu_len - 1,
                invoke_id, reject_reason);
        }
    }

    return len;
}

void testReject(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 0;
    uint8_t reject_reason = 0;
    uint8_t test_invoke_id = 0;
    uint8_t test_reject_reason = 0;

    len = reject_encode_apdu(&apdu[0], invoke_id, reject_reason);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len =
        reject_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_reject_reason);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, test_reject_reason == reject_reason);

    /* change type to get negative response */
    apdu[0] = PDU_TYPE_ABORT;
    len =
        reject_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_reject_reason);
    ct_test(pTest, len == -1);

    /* test NULL APDU */
    len =
        reject_decode_apdu(NULL, apdu_len, &test_invoke_id,
        &test_reject_reason);
    ct_test(pTest, len == -1);

    /* force a zero length */
    len =
        reject_decode_apdu(&apdu[0], 0, &test_invoke_id, &test_reject_reason);
    ct_test(pTest, len == 0);


    /* check them all...   */
    for (invoke_id = 0; invoke_id < 255; invoke_id++) {
        for (reject_reason = 0; reject_reason < 255; reject_reason++) {
            len = reject_encode_apdu(&apdu[0], invoke_id, reject_reason);
            apdu_len = len;
            ct_test(pTest, len != 0);
            len =
                reject_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
                &test_reject_reason);
            ct_test(pTest, len != -1);
            ct_test(pTest, test_invoke_id == invoke_id);
            ct_test(pTest, test_reject_reason == reject_reason);
        }
    }
}

#ifdef TEST_REJECT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Reject", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testReject);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_REJECT */
#endif /* TEST */
