/**
* @file
* @author Krzysztof Malorny <malornykrzysztof@gmail.com>
* @date 2011
* @brief GetAlarmSummary service encoding and decoding
*
* @section LICENSE
*
* Copyright (C) 2011 Krzysztof Malorny <malornykrzysztof@gmail.com>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* @section DESCRIPTION
*
* The GetAlarmSummary service is used by a client BACnet-user
* to obtain a summary of "active alarms." The term "active alarm" refers
* to BACnet standard objects that have an Event_State property whose value
* is not equal to NORMAL and a Notify_Type property whose value is ALARM.
* The GetEnrollmentSummary service provides a more sophisticated approach
* with various kinds of filters
*/
#include <assert.h>

#include "bacdcode.h"
#include "get_alarm_sum.h"
#include "npdu.h"

/* encode service */
int get_alarm_summary_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id
        ) {
    int apdu_len = 0; /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_GET_ALARM_SUMMARY;
        apdu_len = 4;
    }

    return apdu_len;
}

/** Helper function encode the beginning of a GetAlarmSummary ACK.
 *
 * @param apdu - buffer where to put encoding
 * @param invoke_id - unique sequence number sent with the message
 *
 * @return number of bytes encoded
 */
int get_alarm_summary_ack_encode_apdu_init(
    uint8_t * apdu,
    uint8_t invoke_id)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_COMPLEX_ACK; /* complex ACK service */
        apdu[1] = invoke_id;    /* original invoke id from request */
        apdu[2] = SERVICE_CONFIRMED_GET_ALARM_SUMMARY;
        apdu_len = 3;
    }

    return apdu_len;
}

/** Helper function encode the data portion of a GetAlarmSummary ACK.
 *
 * @param apdu - buffer where to put encoding
 * @param max_apdu - number of bytes available in the buffer for encoding
 * @param get_alarm_data - BACNET_GET_ALARM_SUMMARY_DATA type with data
 *
 * @return number of bytes encoded, or BACNET_STATUS_ERROR if an error.
 */
int get_alarm_summary_ack_encode_apdu_data(
    uint8_t * apdu,
    size_t max_apdu,
    BACNET_GET_ALARM_SUMMARY_DATA * get_alarm_data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (!apdu) {
        apdu_len = BACNET_STATUS_ERROR;
    } else if (max_apdu >= 10) {
        /* tag 0 - Object Identifier */
        apdu_len +=
            encode_application_object_id(&apdu[apdu_len],
            (int) get_alarm_data->objectIdentifier.type,
            get_alarm_data->objectIdentifier.instance);
        /* tag 1 - Alarm State */
        apdu_len +=
            encode_application_enumerated(&apdu[apdu_len],
            get_alarm_data->alarmState);
        /* tag 2 - Acknowledged Transitions */
        apdu_len +=
            encode_application_bitstring(&apdu[apdu_len],
            &get_alarm_data->acknowledgedTransitions);
    } else {
        apdu_len = BACNET_STATUS_ABORT;
    }

    return apdu_len;
}

/** Helper function to decode the data portion of a GetAlarmSummary ACK.
 *
 * @param apdu - buffer where to put encoding
 * @param max_apdu - number of bytes available in the buffer for encoding
 * @param get_alarm_data - BACNET_GET_ALARM_SUMMARY_DATA type for data
 *
 * @return number of bytes decoded, or BACNET_STATUS_ERROR if an error.
 */
int get_alarm_summary_ack_decode_apdu_data(
    uint8_t * apdu,
    size_t max_apdu,
    BACNET_GET_ALARM_SUMMARY_DATA * get_alarm_data)
{
    int apdu_len = 0;   /* total length of the apdu, return value */
    BACNET_APPLICATION_DATA_VALUE value;

    if (!apdu) {
        apdu_len = BACNET_STATUS_ERROR;
    } else if (max_apdu >= 10) {
        /* tag 0 - Object Identifier */
        apdu_len +=
            bacapp_decode_application_data(&apdu[apdu_len],
            max_apdu - apdu_len, &value);
        if (value.tag == BACNET_APPLICATION_TAG_OBJECT_ID) {
            get_alarm_data->objectIdentifier = value.type.Object_Id;
        } else {
            return BACNET_STATUS_ERROR;
        }
        /* tag 1 - Alarm State */
        apdu_len +=
            bacapp_decode_application_data(&apdu[apdu_len],
            max_apdu - apdu_len, &value);
        if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {
            get_alarm_data->alarmState =
                (BACNET_EVENT_STATE) value.type.Enumerated;
        } else {
            return BACNET_STATUS_ERROR;
        }
        /* tag 2 - Acknowledged Transitions */
        apdu_len +=
            bacapp_decode_application_data(&apdu[apdu_len],
            max_apdu - apdu_len, &value);
        if (value.tag == BACNET_APPLICATION_TAG_BIT_STRING) {
            get_alarm_data->acknowledgedTransitions = value.type.Bit_String;
        } else {
            return BACNET_STATUS_ERROR;
        }
    }

    return apdu_len;
}
