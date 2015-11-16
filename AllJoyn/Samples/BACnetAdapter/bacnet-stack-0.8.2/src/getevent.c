/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2009 Steve Karg

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
#include "getevent.h"

/** @file getevent.c  Encode/Decode GetEvent services */

/* encode service */
int getevent_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_OBJECT_ID * lastReceivedObjectIdentifier)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_GET_EVENT_INFORMATION;
        apdu_len = 4;
        /* encode optional parameter */
        if (lastReceivedObjectIdentifier) {
            len =
                encode_context_object_id(&apdu[apdu_len], 0,
                (int) lastReceivedObjectIdentifier->type,
                lastReceivedObjectIdentifier->instance);
            apdu_len += len;
        }
    }

    return apdu_len;
}

/* decode the service request only */
int getevent_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_OBJECT_ID * lastReceivedObjectIdentifier)
{
    unsigned len = 0;

    /* check for value pointers */
    if (apdu_len && lastReceivedObjectIdentifier) {
        /* Tag 0: Object ID - optional */
        if (!decode_is_context_tag(&apdu[len++], 0))
            return -1;
        len +=
            decode_object_id(&apdu[len], &lastReceivedObjectIdentifier->type,
            &lastReceivedObjectIdentifier->instance);
    }

    return (int) len;
}

int getevent_ack_encode_apdu_init(
    uint8_t * apdu,
    size_t max_apdu,
    uint8_t invoke_id)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu && (max_apdu >= 4)) {
        apdu[0] = PDU_TYPE_COMPLEX_ACK; /* complex ACK service */
        apdu[1] = invoke_id;    /* original invoke id from request */
        apdu[2] = SERVICE_CONFIRMED_GET_EVENT_INFORMATION;
        apdu_len = 3;
        /* service ack follows */
        /* Tag 0: listOfEventSummaries */
        apdu_len += encode_opening_tag(&apdu[apdu_len], 0);
    }

    return apdu_len;
}

int getevent_ack_encode_apdu_data(
    uint8_t * apdu,
    size_t max_apdu,
    BACNET_GET_EVENT_INFORMATION_DATA * get_event_data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    BACNET_GET_EVENT_INFORMATION_DATA *event_data;
    unsigned i = 0;     /* counter */

    /* unused parameter */
    max_apdu = max_apdu;
    if (apdu) {
        event_data = get_event_data;
        while (event_data) {
            /* Tag 0: objectIdentifier */
            apdu_len +=
                encode_context_object_id(&apdu[apdu_len], 0,
                (int) event_data->objectIdentifier.type,
                event_data->objectIdentifier.instance);
            /* Tag 1: eventState */
            apdu_len +=
                encode_context_enumerated(&apdu[apdu_len], 1,
                event_data->eventState);
            /* Tag 2: acknowledgedTransitions */
            apdu_len +=
                encode_context_bitstring(&apdu[apdu_len], 2,
                &event_data->acknowledgedTransitions);
            /* Tag 3: eventTimeStamps */
            apdu_len += encode_opening_tag(&apdu[apdu_len], 3);
            for (i = 0; i < 3; i++) {
                apdu_len +=
                    bacapp_encode_timestamp(&apdu[apdu_len],
                    &event_data->eventTimeStamps[i]);
            }
            apdu_len += encode_closing_tag(&apdu[apdu_len], 3);
            /* Tag 4: notifyType */
            apdu_len +=
                encode_context_enumerated(&apdu[apdu_len], 4,
                event_data->notifyType);
            /* Tag 5: eventEnable */
            apdu_len +=
                encode_context_bitstring(&apdu[apdu_len], 5,
                &event_data->eventEnable);
            /* Tag 6: eventPriorities */
            apdu_len += encode_opening_tag(&apdu[apdu_len], 6);
            for (i = 0; i < 3; i++) {
                apdu_len +=
                    encode_application_unsigned(&apdu[apdu_len],
                    event_data->eventPriorities[i]);
            }
            apdu_len += encode_closing_tag(&apdu[apdu_len], 6);
            event_data = event_data->next;
        }
    }

    return apdu_len;
}

int getevent_ack_encode_apdu_end(
    uint8_t * apdu,
    size_t max_apdu,
    bool moreEvents)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    /* unused parameter */
    max_apdu = max_apdu;
    if (apdu) {
        apdu_len += encode_closing_tag(&apdu[apdu_len], 0);
        apdu_len += encode_context_boolean(&apdu[apdu_len], 1, moreEvents);
    }

    return apdu_len;
}

int getevent_ack_decode_service_request(
    uint8_t * apdu,
    int apdu_len,       /* total length of the apdu */
    BACNET_GET_EVENT_INFORMATION_DATA * get_event_data,
    bool * moreEvents)
{
    uint8_t tag_number = 0;
    uint32_t len_value = 0;
    int len = 0;        /* total length of decodes */
    uint32_t enum_value = 0;    /* for decoding */
    BACNET_GET_EVENT_INFORMATION_DATA *event_data;
    unsigned i = 0;     /* counter */

    /* FIXME: check apdu_len against the len during decode   */
    event_data = get_event_data;
    if (apdu && apdu_len && event_data && moreEvents) {
        if (!decode_is_opening_tag_number(&apdu[len], 0)) {
            return -1;
        }
        len++;
        while (event_data) {
            /* Tag 0: objectIdentifier */
            if (decode_is_context_tag(&apdu[len], 0)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                len +=
                    decode_object_id(&apdu[len],
                    &event_data->objectIdentifier.type,
                    &event_data->objectIdentifier.instance);
            } else {
                return -1;
            }
            /* Tag 1: eventState */
            if (decode_is_context_tag(&apdu[len], 1)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                len += decode_enumerated(&apdu[len], len_value, &enum_value);
                event_data->eventState = enum_value;
            } else {
                return -1;
            }
            /* Tag 2: acknowledgedTransitions */
            if (decode_is_context_tag(&apdu[len], 2)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                len +=
                    decode_bitstring(&apdu[len], len_value,
                    &event_data->acknowledgedTransitions);
            } else {
                return -1;
            }
            /* Tag 3: eventTimeStamps */
            if (decode_is_opening_tag_number(&apdu[len], 3)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                for (i = 0; i < 3; i++) {
                    len +=
                        bacapp_decode_timestamp(&apdu[len],
                        &event_data->eventTimeStamps[i]);
                }
            } else {
                return -1;
            }
            if (decode_is_closing_tag_number(&apdu[len], 3)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
            } else {
                return -1;
            }
            /* Tag 4: notifyType */
            if (decode_is_context_tag(&apdu[len], 4)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                len += decode_enumerated(&apdu[len], len_value, &enum_value);
                event_data->notifyType = enum_value;
            } else {
                return -1;
            }
            /* Tag 5: eventEnable */
            if (decode_is_context_tag(&apdu[len], 5)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                len +=
                    decode_bitstring(&apdu[len], len_value,
                    &event_data->eventEnable);
            } else {
                return -1;
            }
            /* Tag 6: eventPriorities */
            if (decode_is_opening_tag_number(&apdu[len], 6)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                for (i = 0; i < 3; i++) {
                    len +=
                        decode_tag_number_and_value(&apdu[len], &tag_number,
                        &len_value);
                    len +=
                        decode_unsigned(&apdu[len], len_value,
                        &event_data->eventPriorities[i]);
                }
            } else {
                return -1;
            }
            if (decode_is_closing_tag_number(&apdu[len], 6)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
            } else {
                return -1;
            }
            if (decode_is_closing_tag_number(&apdu[len], 0)) {
                len +=
                    decode_tag_number_and_value(&apdu[len], &tag_number,
                    &len_value);
                event_data->next = NULL;
            }
            event_data = event_data->next;
        }
        if (decode_is_context_tag(&apdu[len], 1)) {
            len +=
                decode_tag_number_and_value(&apdu[len], &tag_number,
                &len_value);
            if (len_value == 1)
                *moreEvents = decode_context_boolean(&apdu[len++]);
            else
                *moreEvents = false;
        } else {
            return -1;
        }
    }

    return len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

int getevent_decode_apdu(
    uint8_t * apdu,
    unsigned apdu_len,
    uint8_t * invoke_id,
    BACNET_OBJECT_ID * lastReceivedObjectIdentifier)
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
    if (apdu[3] != SERVICE_CONFIRMED_GET_EVENT_INFORMATION)
        return -1;
    offset = 4;

    if (apdu_len > offset) {
        len =
            getevent_decode_service_request(&apdu[offset], apdu_len - offset,
            lastReceivedObjectIdentifier);
    }

    return len;
}

int getevent_ack_decode_apdu(
    uint8_t * apdu,
    int apdu_len,       /* total length of the apdu */
    uint8_t * invoke_id,
    BACNET_GET_EVENT_INFORMATION_DATA * get_event_data,
    bool * moreEvents)
{
    int len = 0;
    int offset = 0;

    if (!apdu)
        return -1;
    /* optional checking - most likely was already done prior to this call */
    if (apdu[0] != PDU_TYPE_COMPLEX_ACK)
        return -1;
    *invoke_id = apdu[1];
    if (apdu[2] != SERVICE_CONFIRMED_GET_EVENT_INFORMATION)
        return -1;
    offset = 3;
    if (apdu_len > offset) {
        len =
            getevent_ack_decode_service_request(&apdu[offset],
            apdu_len - offset, get_event_data, moreEvents);
    }

    return len;
}

void testGetEventInformationAck(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 1;
    uint8_t test_invoke_id = 0;
    BACNET_GET_EVENT_INFORMATION_DATA event_data;
    BACNET_GET_EVENT_INFORMATION_DATA test_event_data;
    bool moreEvents = false;
    bool test_moreEvents = false;
    unsigned i = 0;

    event_data.objectIdentifier.type = OBJECT_BINARY_INPUT;
    event_data.objectIdentifier.instance = 1;
    event_data.eventState = EVENT_STATE_NORMAL;
    bitstring_init(&event_data.acknowledgedTransitions);
    bitstring_set_bit(&event_data.acknowledgedTransitions,
        TRANSITION_TO_OFFNORMAL, false);
    bitstring_set_bit(&event_data.acknowledgedTransitions, TRANSITION_TO_FAULT,
        false);
    bitstring_set_bit(&event_data.acknowledgedTransitions,
        TRANSITION_TO_NORMAL, false);
    for (i = 0; i < 3; i++) {
        event_data.eventTimeStamps[i].tag = TIME_STAMP_SEQUENCE;
        event_data.eventTimeStamps[i].value.sequenceNum = 0;
    }
    event_data.notifyType = NOTIFY_ALARM;
    bitstring_init(&event_data.eventEnable);
    bitstring_set_bit(&event_data.eventEnable, TRANSITION_TO_OFFNORMAL, true);
    bitstring_set_bit(&event_data.eventEnable, TRANSITION_TO_FAULT, true);
    bitstring_set_bit(&event_data.eventEnable, TRANSITION_TO_NORMAL, true);
    for (i = 0; i < 3; i++) {
        event_data.eventPriorities[i] = 1;
    }
    event_data.next = NULL;

    len = getevent_ack_encode_apdu_init(&apdu[0], sizeof(apdu), invoke_id);
    ct_test(pTest, len != 0);
    ct_test(pTest, len != -1);
    apdu_len = len;
    len =
        getevent_ack_encode_apdu_data(&apdu[apdu_len], sizeof(apdu) - apdu_len,
        &event_data);
    ct_test(pTest, len != 0);
    ct_test(pTest, len != -1);
    apdu_len += len;
    len =
        getevent_ack_encode_apdu_end(&apdu[apdu_len], sizeof(apdu) - apdu_len,
        moreEvents);
    ct_test(pTest, len != 0);
    ct_test(pTest, len != -1);
    apdu_len += len;
    len = getevent_ack_decode_apdu(&apdu[0], apdu_len,  /* total length of the apdu */
        &test_invoke_id, &test_event_data, &test_moreEvents);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);

    ct_test(pTest,
        event_data.objectIdentifier.type ==
        test_event_data.objectIdentifier.type);
    ct_test(pTest,
        event_data.objectIdentifier.instance ==
        test_event_data.objectIdentifier.instance);

    ct_test(pTest, event_data.eventState == test_event_data.eventState);
}

void testGetEventInformation(
    Test * pTest)
{
    uint8_t apdu[480] = { 0 };
    int len = 0;
    int apdu_len = 0;
    uint8_t invoke_id = 128;
    uint8_t test_invoke_id = 0;
    BACNET_OBJECT_ID lastReceivedObjectIdentifier;
    BACNET_OBJECT_ID test_lastReceivedObjectIdentifier;

    lastReceivedObjectIdentifier.type = OBJECT_BINARY_INPUT;
    lastReceivedObjectIdentifier.instance = 12345;
    len =
        getevent_encode_apdu(&apdu[0], invoke_id,
        &lastReceivedObjectIdentifier);
    ct_test(pTest, len != 0);
    apdu_len = len;

    len =
        getevent_decode_apdu(&apdu[0], apdu_len, &test_invoke_id,
        &test_lastReceivedObjectIdentifier);
    ct_test(pTest, len != -1);
    ct_test(pTest, test_invoke_id == invoke_id);
    ct_test(pTest,
        test_lastReceivedObjectIdentifier.type ==
        lastReceivedObjectIdentifier.type);
    ct_test(pTest,
        test_lastReceivedObjectIdentifier.instance ==
        lastReceivedObjectIdentifier.instance);

    return;
}

#ifdef TEST_GET_EVENT_INFORMATION
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet GetEventInformation", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testGetEventInformation);
    assert(rc);
    rc = ct_addTestFunction(pTest, testGetEventInformationAck);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif
#endif /* TEST */
