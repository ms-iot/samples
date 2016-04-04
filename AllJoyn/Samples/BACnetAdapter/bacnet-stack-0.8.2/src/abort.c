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
#include "abort.h"

/** @file abort.c  Abort Encoding/Decoding */
/* Helper function to avoid needing additional entries in service data structures
 * when passing back abort status.
 * Convert from error code to abort code.
 * Anything not defined converts to ABORT_REASON_OTHER.
 * Will need reworking if it is required to return proprietary abort codes.
 */
BACNET_ABORT_REASON abort_convert_error_code(
    BACNET_ERROR_CODE error_code)
{
    BACNET_ABORT_REASON abort_code = ABORT_REASON_OTHER;

    switch (error_code) {
        case ERROR_CODE_ABORT_BUFFER_OVERFLOW:
            abort_code = ABORT_REASON_BUFFER_OVERFLOW;
            break;
        case ERROR_CODE_ABORT_INVALID_APDU_IN_THIS_STATE:
            abort_code = ABORT_REASON_INVALID_APDU_IN_THIS_STATE;
            break;
        case ERROR_CODE_ABORT_PREEMPTED_BY_HIGHER_PRIORITY_TASK:
            abort_code = ABORT_REASON_PREEMPTED_BY_HIGHER_PRIORITY_TASK;
            break;
        case ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED:
            abort_code = ABORT_REASON_SEGMENTATION_NOT_SUPPORTED;
            break;
        case ERROR_CODE_ABORT_PROPRIETARY:
            abort_code = ABORT_REASON_PROPRIETARY_FIRST;
            break;
        case ERROR_CODE_ABORT_OTHER:
        default:
            abort_code = ABORT_REASON_OTHER;
            break;
    }

    return (abort_code);
}

/* encode service */
int abort_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    uint8_t abort_reason,
    bool server)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        if (server)
            apdu[0] = PDU_TYPE_ABORT | 1;
        else
            apdu[0] = PDU_TYPE_ABORT;
        apdu[1] = invoke_id;
        apdu[2] = abort_reason;
        apdu_len = 3;
    }

    return apdu_len;
}

#if !BACNET_SVC_SERVER
/* decode the service request only */
int abort_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    uint8_t * abort_reason)
{
    int len = 0;

    if (apdu_len > 0) {
        if (invoke_id)
            *invoke_id = apdu[0];
        if (abort_reason)
            *abort_reason = apdu[1];
    }

    return len;
}
#endif

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

/* decode the whole APDU - mainly used for unit testing */
int abort_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    uint8_t * abort_reason,
    bool * server)
{
    int len = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu_len > 0) {
        if ((apdu[0] & 0xF0) != PDU_TYPE_ABORT)
            return -1;
        if (apdu[0] & 1)
            *server = true;
        else
            *server = false;
        if (apdu_len > 1) {
            len =
                abort_decode_service_request(&apdu[1], apdu_len - 1, invoke_id,
                abort_reason);
        }
    }

    return len;
}

void testAbortAPDU(
    Test * pTest,
    uint8_t invoke_id,
    uint8_t abort_reason,
    bool server)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t test_invoke_id = 0;
    uint8_t test_abort_reason = 0;
    bool test_server = false;

    len = abort_encode_apdu(&apdu[0], invoke_id, abort_reason, server);
    apdu_len = len;
    ct_test(pTest, len != 0);
    len =
        abort_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_abort_reason, &test_server);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, test_abort_reason == abort_reason);
    ct_test(pTest, test_server == server);

    return;
}


void testAbort(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 0;
    uint8_t test_invoke_id = 0;
    uint8_t abort_reason = 0;
    uint8_t test_abort_reason = 0;
    bool server = false;
    bool test_server = false;

    len = abort_encode_apdu(&apdu[0], invoke_id, abort_reason, server);
    ct_test(pTest, len != 0);
    apdu_len = len;
    len =
        abort_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_abort_reason, &test_server);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, test_abort_reason == abort_reason);
    ct_test(pTest, test_server == server);

    /* change type to get negative response */
    apdu[0] = PDU_TYPE_REJECT;
    len =
        abort_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_abort_reason, &test_server);
    ct_test(pTest, len == -1);

    /* test NULL APDU */
    len =
        abort_decode_apdu(NULL, apdu_len, &test_invoke_id, &test_abort_reason,
        &test_server);
    ct_test(pTest, len == -1);

    /* force a zero length */
    len =
        abort_decode_apdu(&apdu[0], 0, &test_invoke_id, &test_abort_reason,
        &test_server);
    ct_test(pTest, len == 0);

    /* check them all...   */
    for (invoke_id = 0; invoke_id < 255; invoke_id++) {
        for (abort_reason = 0; abort_reason < 255; abort_reason++) {
            testAbortAPDU(pTest, invoke_id, abort_reason, false);
            testAbortAPDU(pTest, invoke_id, abort_reason, true);
        }
    }
}

#ifdef TEST_ABORT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Abort", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testAbort);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_ABORT */
#endif /* TEST */
