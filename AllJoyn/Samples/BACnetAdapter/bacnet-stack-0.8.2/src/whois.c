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
#include "whois.h"

/** @file whois.c  Encode/Decode Who-Is requests  */

/* encode I-Am service  - use -1 for limit if you want unlimited */
int whois_encode_apdu(
    uint8_t * apdu,
    int32_t low_limit,
    int32_t high_limit)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST;
        apdu[1] = SERVICE_UNCONFIRMED_WHO_IS;   /* service choice */
        apdu_len = 2;
        /* optional limits - must be used as a pair */
        if ((low_limit >= 0) && (low_limit <= BACNET_MAX_INSTANCE) &&
            (high_limit >= 0) && (high_limit <= BACNET_MAX_INSTANCE)) {
            len = encode_context_unsigned(&apdu[apdu_len], 0, low_limit);
            apdu_len += len;
            len = encode_context_unsigned(&apdu[apdu_len], 1, high_limit);
            apdu_len += len;
        }
    }

    return apdu_len;
}

/* decode the service request only */
int whois_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    int32_t * pLow_limit,
    int32_t * pHigh_limit)
{
    int len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    uint32_t decoded_value = 0;

    /* optional limits - must be used as a pair */
    if (apdu_len) {
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
        if (tag_number != 0) {
            return BACNET_STATUS_ERROR;
        }
        if (apdu_len > (unsigned)len) {
            len += decode_unsigned(&apdu[len], len_value, &decoded_value);
            if (decoded_value <= BACNET_MAX_INSTANCE) {
                if (pLow_limit) {
                    *pLow_limit = decoded_value;
                }
            }
            if (apdu_len > (unsigned)len) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number, &len_value);
                if (tag_number != 1) {
                    return BACNET_STATUS_ERROR;
                }
                if (apdu_len > (unsigned)len) {
                    len += decode_unsigned(&apdu[len], len_value, &decoded_value);
                    if (decoded_value <= BACNET_MAX_INSTANCE) {
                        if (pHigh_limit) {
                            *pHigh_limit = decoded_value;
                        }
                    }
                } else {
                    return BACNET_STATUS_ERROR;
                }
            } else {
                return BACNET_STATUS_ERROR;
            }
        } else {
            return BACNET_STATUS_ERROR;
        }
    } else {
        if (pLow_limit) {
            *pLow_limit = -1;
        }
        if (pHigh_limit) {
            *pHigh_limit = -1;
        }
        len = 0;
    }

    return len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

int whois_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    int32_t * pLow_limit,
    int32_t * pHigh_limit)
{
    int len = 0;

    if (!apdu) {
        return BACNET_STATUS_ERROR;
    }
    /* optional limits - must be used as a pair */
    if (apdu_len >= 2) {
        /* optional checking - most likely was already done prior to this call */
        if (apdu[0] != PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST) {
            return BACNET_STATUS_ERROR;
        }
        if (apdu[1] != SERVICE_UNCONFIRMED_WHO_IS) {
            return BACNET_STATUS_ERROR;
        }
        len =
            whois_decode_service_request(&apdu[2], apdu_len - 2, pLow_limit,
            pHigh_limit);
    }

    return len;
}

void testWhoIs(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    int32_t low_limit = -1;
    int32_t high_limit = -1;
    int32_t test_low_limit = 0;
    int32_t test_high_limit = 0;

    /* normal who-is without limits */
    len = whois_encode_apdu(&apdu[0], low_limit, high_limit);
    ct_test(pTest, len > 0);
    apdu_len = len;

    len =
        whois_decode_apdu(&apdu[0], apdu_len, &test_low_limit,
        &test_high_limit);
    ct_test(pTest, len != BACNET_STATUS_ERROR);
    ct_test(pTest, test_low_limit == low_limit);
    ct_test(pTest, test_high_limit == high_limit);

    /* normal who-is with limits - complete range */
    for (low_limit = 0; low_limit <= BACNET_MAX_INSTANCE;
        low_limit += (BACNET_MAX_INSTANCE / 4)) {
        for (high_limit = 0; high_limit <= BACNET_MAX_INSTANCE;
            high_limit += (BACNET_MAX_INSTANCE / 4)) {
            len = whois_encode_apdu(&apdu[0], low_limit, high_limit);
            apdu_len = len;
            ct_test(pTest, len > 0);
            len =
                whois_decode_apdu(&apdu[0], apdu_len, &test_low_limit,
                &test_high_limit);
            ct_test(pTest, len != BACNET_STATUS_ERROR);
            ct_test(pTest, test_low_limit == low_limit);
            ct_test(pTest, test_high_limit == high_limit);
        }
    }
    /* abnormal case:
       who-is with no limits, but with APDU containing 2 limits */
    low_limit = 0;
    high_limit = 0;
    len = whois_encode_apdu(&apdu[0], low_limit, high_limit);
    ct_test(pTest, len > 0);
    apdu_len = len;
    low_limit = -1;
    high_limit = -1;
    len = whois_encode_apdu(&apdu[0], low_limit, high_limit);
    ct_test(pTest, len > 0);
    apdu_len = len;
    len =
        whois_decode_apdu(&apdu[0], apdu_len, &test_low_limit,
        &test_high_limit);
    ct_test(pTest, len != BACNET_STATUS_ERROR);
    ct_test(pTest, test_low_limit == low_limit);
    ct_test(pTest, test_high_limit == high_limit);
}

#ifdef TEST_WHOIS
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Who-Is", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testWhoIs);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_WHOIS */
#endif /* TEST */
