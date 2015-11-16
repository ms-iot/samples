/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2009 John Minack

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

#include "alarm_ack.h"

/** @file alarm_ack.c  Handles Event Notifications (ACKs) */

/***************************************************
**
** Creates an Unconfirmed Event Notification APDU
**
****************************************************/
int alarm_ack_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_ALARM_ACK_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM;  /* service choice */
        apdu_len = 4;

        len = alarm_ack_encode_service_request(&apdu[apdu_len], data);
        apdu_len += len;
    }

    return apdu_len;
}


/***************************************************
**
** Encodes the service data part of Event Notification
**
****************************************************/
int alarm_ack_encode_service_request(
    uint8_t * apdu,
    BACNET_ALARM_ACK_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        len =
            encode_context_unsigned(&apdu[apdu_len], 0,
            data->ackProcessIdentifier);
        apdu_len += len;

        len =
            encode_context_object_id(&apdu[apdu_len], 1,
            (int) data->eventObjectIdentifier.type,
            data->eventObjectIdentifier.instance);
        apdu_len += len;

        len =
            encode_context_enumerated(&apdu[apdu_len], 2,
            data->eventStateAcked);
        apdu_len += len;

        len =
            bacapp_encode_context_timestamp(&apdu[apdu_len], 3,
            &data->eventTimeStamp);
        apdu_len += len;

        len =
            encode_context_character_string(&apdu[apdu_len], 4,
            &data->ackSource);
        apdu_len += len;

        len =
            bacapp_encode_context_timestamp(&apdu[apdu_len], 5,
            &data->ackTimeStamp);
        apdu_len += len;
    }

    return apdu_len;
}


/***************************************************
**
** Decodes the service data part of Event Notification
**
****************************************************/
int alarm_ack_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_ALARM_ACK_DATA * data)
{
    int len = 0;
    int section_len;
    uint32_t enumValue;

    /* unused parameter */
    apdu_len = apdu_len;

    if (-1 == (section_len =
            decode_context_unsigned(&apdu[len], 0,
                &data->ackProcessIdentifier))) {
        return -1;
    }
    len += section_len;

    if (-1 == (section_len =
            decode_context_object_id(&apdu[len], 1,
                &data->eventObjectIdentifier.type,
                &data->eventObjectIdentifier.instance))) {
        return -1;
    }
    len += section_len;

    if (-1 == (section_len =
            decode_context_enumerated(&apdu[len], 2, &enumValue))) {
        return -1;
    }
    data->eventStateAcked = (BACNET_EVENT_STATE) enumValue;
    len += section_len;

    if (-1 == (section_len =
            bacapp_decode_context_timestamp(&apdu[len], 3,
                &data->eventTimeStamp))) {
        return -1;
    }
    len += section_len;

    if (-1 == (section_len =
            decode_context_character_string(&apdu[len], 4,
                &data->ackSource))) {
        return -1;
    }
    len += section_len;

    if (-1 == (section_len =
            bacapp_decode_context_timestamp(&apdu[len], 5,
                &data->ackTimeStamp))) {
        return -1;
    }
    len += section_len;

    return len;
}

#ifdef TEST

#include <assert.h>
#include <string.h>
#include "ctest.h"


void testAlarmAck(
    Test * pTest)
{
    BACNET_ALARM_ACK_DATA testAlarmAckIn;
    BACNET_ALARM_ACK_DATA testAlarmAckOut;

    uint8_t buffer[MAX_APDU];
    int inLen;
    int outLen;

    testAlarmAckIn.ackProcessIdentifier = 0x1234;
    characterstring_init_ansi(&testAlarmAckIn.ackSource, "This is a test");
    testAlarmAckIn.ackTimeStamp.tag = TIME_STAMP_SEQUENCE;
    testAlarmAckIn.ackTimeStamp.value.sequenceNum = 0x4331;
    testAlarmAckIn.eventObjectIdentifier.instance = 567;
    testAlarmAckIn.eventObjectIdentifier.type = OBJECT_DEVICE;
    testAlarmAckIn.eventTimeStamp.tag = TIME_STAMP_TIME;
    testAlarmAckIn.eventTimeStamp.value.time.hour = 10;
    testAlarmAckIn.eventTimeStamp.value.time.min = 11;
    testAlarmAckIn.eventTimeStamp.value.time.sec = 12;
    testAlarmAckIn.eventTimeStamp.value.time.hundredths = 14;
    testAlarmAckIn.eventStateAcked = EVENT_STATE_OFFNORMAL;

    memset(&testAlarmAckOut, 0, sizeof(testAlarmAckOut));


    inLen = alarm_ack_encode_service_request(buffer, &testAlarmAckIn);
    outLen = alarm_ack_decode_service_request(buffer, inLen, &testAlarmAckOut);

    ct_test(pTest, inLen == outLen);

    ct_test(pTest,
        testAlarmAckIn.ackProcessIdentifier ==
        testAlarmAckOut.ackProcessIdentifier);

    ct_test(pTest,
        testAlarmAckIn.ackTimeStamp.tag == testAlarmAckOut.ackTimeStamp.tag);
    ct_test(pTest,
        testAlarmAckIn.ackTimeStamp.value.sequenceNum ==
        testAlarmAckOut.ackTimeStamp.value.sequenceNum);

    ct_test(pTest,
        testAlarmAckIn.ackProcessIdentifier ==
        testAlarmAckOut.ackProcessIdentifier);

    ct_test(pTest,
        testAlarmAckIn.eventObjectIdentifier.instance ==
        testAlarmAckOut.eventObjectIdentifier.instance);
    ct_test(pTest,
        testAlarmAckIn.eventObjectIdentifier.type ==
        testAlarmAckOut.eventObjectIdentifier.type);

    ct_test(pTest,
        testAlarmAckIn.eventTimeStamp.tag ==
        testAlarmAckOut.eventTimeStamp.tag);
    ct_test(pTest,
        testAlarmAckIn.eventTimeStamp.value.time.hour ==
        testAlarmAckOut.eventTimeStamp.value.time.hour);
    ct_test(pTest,
        testAlarmAckIn.eventTimeStamp.value.time.min ==
        testAlarmAckOut.eventTimeStamp.value.time.min);
    ct_test(pTest,
        testAlarmAckIn.eventTimeStamp.value.time.sec ==
        testAlarmAckOut.eventTimeStamp.value.time.sec);
    ct_test(pTest,
        testAlarmAckIn.eventTimeStamp.value.time.hundredths ==
        testAlarmAckOut.eventTimeStamp.value.time.hundredths);

    ct_test(pTest,
        testAlarmAckIn.eventStateAcked == testAlarmAckOut.eventStateAcked);

}


#ifdef TEST_ALARM_ACK
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Alarm Ack", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testAlarmAck);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}

#endif /* TEST_ALARM_ACK */
#endif /* TEST */
