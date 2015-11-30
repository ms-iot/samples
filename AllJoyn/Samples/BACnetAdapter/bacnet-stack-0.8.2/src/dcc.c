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
#include "dcc.h"

/** @file dcc.c  Enable/Disable Device Communication Control (DCC) */

/* note: the disable and time are not expected to survive
   over a power cycle or reinitialization. */
/* note: time duration is given in Minutes, but in order to be accurate,
   we need to count down in seconds. */
/* infinite time duration is defined as 0 */
static uint32_t DCC_Time_Duration_Seconds = 0;
static BACNET_COMMUNICATION_ENABLE_DISABLE DCC_Enable_Disable =
    COMMUNICATION_ENABLE;
/* password is optionally supported */

BACNET_COMMUNICATION_ENABLE_DISABLE dcc_enable_status(
    void)
{
    return DCC_Enable_Disable;
}

bool dcc_communication_enabled(
    void)
{
    return (DCC_Enable_Disable == COMMUNICATION_ENABLE);
}

/* When network communications are completely disabled,
   only DeviceCommunicationControl and ReinitializeDevice APDUs
   shall be processed and no messages shall be initiated.*/
bool dcc_communication_disabled(
    void)
{
    return (DCC_Enable_Disable == COMMUNICATION_DISABLE);
}

/* When the initiation of communications is disabled,
   all APDUs shall be processed and responses returned as
   required and no messages shall be initiated with the
   exception of I-Am requests, which shall be initiated only in
   response to Who-Is messages. In this state, a device that
   supports I-Am request initiation shall send one I-Am request
   for any Who-Is request that is received if and only if
   the Who-Is request does not contain an address range or
   the device is included in the address range. */
bool dcc_communication_initiation_disabled(
    void)
{
    return (DCC_Enable_Disable == COMMUNICATION_DISABLE_INITIATION);
}

/* note: 0 indicates either expired, or infinite duration */
uint32_t dcc_duration_seconds(
    void)
{
    return DCC_Time_Duration_Seconds;
}

/* called every second or so.  If more than one second,
  then seconds should be the number of seconds to tick away */
void dcc_timer_seconds(
    uint32_t seconds)
{
    if (DCC_Time_Duration_Seconds) {
        if (DCC_Time_Duration_Seconds > seconds)
            DCC_Time_Duration_Seconds -= seconds;
        else
            DCC_Time_Duration_Seconds = 0;
        /* just expired - do something */
        if (DCC_Time_Duration_Seconds == 0)
            DCC_Enable_Disable = COMMUNICATION_ENABLE;
    }
}

bool dcc_set_status_duration(
    BACNET_COMMUNICATION_ENABLE_DISABLE status,
    uint16_t minutes)
{
    bool valid = false;

    /* valid? */
    if (status < MAX_BACNET_COMMUNICATION_ENABLE_DISABLE) {
        DCC_Enable_Disable = status;
        if (status == COMMUNICATION_ENABLE) {
            DCC_Time_Duration_Seconds = 0;
        } else {
            DCC_Time_Duration_Seconds = minutes * 60;
        }
        valid = true;
    }

    return valid;
}

#if BACNET_SVC_DCC_A
/* encode service */
int dcc_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    uint16_t timeDuration,      /* 0=optional */
    BACNET_COMMUNICATION_ENABLE_DISABLE enable_disable,
    BACNET_CHARACTER_STRING * password)
{       /* NULL=optional */
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL;
        apdu_len = 4;
        /* optional timeDuration */
        if (timeDuration) {
            len = encode_context_unsigned(&apdu[apdu_len], 0, timeDuration);
            apdu_len += len;
        }
        /* enable disable */
        len = encode_context_enumerated(&apdu[apdu_len], 1, enable_disable);
        apdu_len += len;
        /* optional password */
        if (password) {
            /* FIXME: must be at least 1 character, limited to 20 characters */
            len =
                encode_context_character_string(&apdu[apdu_len], 2, password);
            apdu_len += len;
        }
    }

    return apdu_len;
}
#endif

/* decode the service request only */
int dcc_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    uint16_t * timeDuration,
    BACNET_COMMUNICATION_ENABLE_DISABLE * enable_disable,
    BACNET_CHARACTER_STRING * password)
{
    unsigned len = 0;
    uint8_t tag_number = 0;
    uint32_t len_value_type = 0;
    uint32_t value32 = 0;

    /* check for value pointers */
    if (apdu_len) {
        /* Tag 0: timeDuration, in minutes --optional--
         * But if not included, take it as indefinite,
         * which we return as "very large" */
        if (decode_is_context_tag(&apdu[len], 0)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            len += decode_unsigned(&apdu[len], len_value_type, &value32);
            if (timeDuration) {
                *timeDuration = (uint16_t) value32;
            }
        } else if (timeDuration) {
            /* zero indicates infinite duration and
               results in no timeout */
            *timeDuration = 0;
        }
        /* Tag 1: enable_disable */
        if (!decode_is_context_tag(&apdu[len], 1)) {
            return -1;
        }
        len +=
            decode_tag_number_and_value(&apdu[len], &tag_number,
            &len_value_type);
        len += decode_enumerated(&apdu[len], len_value_type, &value32);
        if (enable_disable) {
            *enable_disable = (BACNET_COMMUNICATION_ENABLE_DISABLE) value32;
        }
        /* Tag 2: password --optional-- */
        if (len < apdu_len) {
            if (!decode_is_context_tag(&apdu[len], 2)) {
                return -1;
            }
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value_type);
            len +=
                decode_character_string(&apdu[len], len_value_type, password);
        } else if (password) {
            characterstring_init_ansi(password, NULL);
        }
    }

    return (int) len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

int dcc_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    uint16_t * timeDuration,
    BACNET_COMMUNICATION_ENABLE_DISABLE * enable_disable,
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
    if (apdu[3] != SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL)
        return -1;
    offset = 4;

    if (apdu_len > offset) {
        len =
            dcc_decode_service_request(&apdu[offset], apdu_len - offset,
            timeDuration, enable_disable, password);
    }

    return len;
}

void test_DeviceCommunicationControlData(
    Test * pTest,
    uint8_t invoke_id,
    uint16_t timeDuration,
    BACNET_COMMUNICATION_ENABLE_DISABLE enable_disable,
    BACNET_CHARACTER_STRING * password)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t test_invoke_id = 0;
    uint16_t test_timeDuration = 0;
    BACNET_COMMUNICATION_ENABLE_DISABLE test_enable_disable;
    BACNET_CHARACTER_STRING test_password;

    len =
        dcc_encode_apdu(&apdu[0], invoke_id, timeDuration, enable_disable,
        password);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len =
        dcc_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_timeDuration, &test_enable_disable, &test_password);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest, test_timeDuration == timeDuration);
    ct_test(pTest, test_enable_disable == enable_disable);
    ct_test(pTest, characterstring_same(&test_password, password));
}

void test_DeviceCommunicationControl(
    Test * pTest)
{
    uint8_t invoke_id = 128;
    uint16_t timeDuration = 0;
    BACNET_COMMUNICATION_ENABLE_DISABLE enable_disable;
    BACNET_CHARACTER_STRING password;

    timeDuration = 0;
    enable_disable = COMMUNICATION_DISABLE_INITIATION;
    characterstring_init_ansi(&password, "John 3:16");
    test_DeviceCommunicationControlData(pTest, invoke_id, timeDuration,
        enable_disable, &password);

    timeDuration = 12345;
    enable_disable = COMMUNICATION_DISABLE;
    test_DeviceCommunicationControlData(pTest, invoke_id, timeDuration,
        enable_disable, NULL);

    return;
}

#ifdef TEST_DEVICE_COMMUNICATION_CONTROL
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet DeviceCommunicationControl", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, test_DeviceCommunicationControl);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_DEVICE_COMMUNICATION_CONTROL */
#endif /* TEST */
