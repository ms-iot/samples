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
#include "rd.h"

/** @file rd.c  Encode/Decode Reinitialize Device APDUs */
#if BACNET_SVC_RD_A
/* encode service */
int rd_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_REINITIALIZED_STATE state,
    BACNET_CHARACTER_STRING * password)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_REINITIALIZE_DEVICE;
        apdu_len = 4;
        len = encode_context_enumerated(&apdu[apdu_len], 0, state);
        apdu_len += len;
        /* optional password */
        if (password) {
            /* FIXME: must be at least 1 character, limited to 20 characters */
            len =
                encode_context_character_string(&apdu[apdu_len], 1, password);
            apdu_len += len;
        }
    }

    return apdu_len;
}
#endif

/* decode the service request only */
int rd_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_REINITIALIZED_STATE * state,
    BACNET_CHARACTER_STRING * password)
{
    unsigned len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    uint32_t value = 0;

    /* check for value pointers */
    if (apdu_len) {
        /* Tag 0: reinitializedStateOfDevice */
        if (!decode_is_context_tag(&apdu[len], 0))
            return -1;
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number,
            &len_value_type);
        len += decode_enumerated(&apdu[len], len_value_type, &value);
        if (state)
            *state = (BACNET_REINITIALIZED_STATE) value;
        /* Tag 1: password - optional */
        if (len < apdu_len) {
            if (!decode_is_context_tag(&apdu[len], 1))
                return -1;
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            len +=
                decode_character_string(&apdu[len], len_value_type, password);
        }
    }

    return (int) len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

int rd_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_REINITIALIZED_STATE * state,
    BACNET_CHARACTER_STRING * password)
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
    if (apdu[3] != SERVICE_CONFIRMED_REINITIALIZE_DEVICE)
        return -1;
    offset = 4;

    if (apdu_len > offset) {
        len =
            rd_decode_service_request(&apdu[offset], apdu_len - offset, state,
            password);
    }

    return len;
}

void test_ReinitializeDevice(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 128;
    uint8_t test_invoke_id = 0;
    BACNET_REINITIALIZED_STATE state;
    BACNET_REINITIALIZED_STATE test_state;
    BACNET_CHARACTER_STRING password;
    BACNET_CHARACTER_STRING test_password;

    state = BACNET_REINIT_WARMSTART;
    characterstring_init_ansi(&password, "John 3:16");
    len = rd_encode_apdu(&apdu[0], invoke_id, state, &password);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len =
        rd_decode_apdu(&apdu[0], apdu_len, &test_invoke_id, &test_state,
        &test_password);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, test_state == state);
    ct_test(pTest, characterstring_same(&test_password, &password));

    return;
}

#ifdef TEST_REINITIALIZE_DEVICE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet ReinitializeDevice", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, test_ReinitializeDevice);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_REINITIALIZE_DEVICE */
#endif /* TEST */
