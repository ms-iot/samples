/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2008 John Minack

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
#include <assert.h>
#include "event.h"
#include "bacdcode.h"
#include "npdu.h"
#include "timestamp.h"

/** @file event.c  Encode/Decode Event Notifications */

int uevent_notify_encode_apdu(
    uint8_t * apdu,
    BACNET_EVENT_NOTIFICATION_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */


    if (apdu) {
        apdu[0] = PDU_TYPE_UNCONFIRMED_SERVICE_REQUEST;
        apdu[1] = SERVICE_UNCONFIRMED_EVENT_NOTIFICATION;       /* service choice */
        apdu_len = 2;

        len += event_notify_encode_service_request(&apdu[apdu_len], data);

        if (len > 0) {
            apdu_len += len;
        } else {
            apdu_len = 0;
        }
    }

    return apdu_len;
}

int cevent_notify_encode_apdu(
    uint8_t * apdu,
    uint8_t invoke_id,
    BACNET_EVENT_NOTIFICATION_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        apdu[0] = PDU_TYPE_CONFIRMED_SERVICE_REQUEST;
        apdu[1] = encode_max_segs_max_apdu(0, MAX_APDU);
        apdu[2] = invoke_id;
        apdu[3] = SERVICE_CONFIRMED_EVENT_NOTIFICATION; /* service choice */
        apdu_len = 4;

        len += event_notify_encode_service_request(&apdu[apdu_len], data);

        if (len > 0) {
            apdu_len += len;
        } else {
            apdu_len = 0;
        }
    }

    return apdu_len;
}

int event_notify_encode_service_request(
    uint8_t * apdu,
    BACNET_EVENT_NOTIFICATION_DATA * data)
{
    int len = 0;        /* length of each encoding */
    int apdu_len = 0;   /* total length of the apdu, return value */

    if (apdu) {
        /* tag 0 - processIdentifier */
        len =
            encode_context_unsigned(&apdu[apdu_len], 0,
            data->processIdentifier);
        apdu_len += len;
        /* tag 1 - initiatingObjectIdentifier */
        len =
            encode_context_object_id(&apdu[apdu_len], 1,
            (int) data->initiatingObjectIdentifier.type,
            data->initiatingObjectIdentifier.instance);
        apdu_len += len;

        /* tag 2 - eventObjectIdentifier */
        len =
            encode_context_object_id(&apdu[apdu_len], 2,
            (int) data->eventObjectIdentifier.type,
            data->eventObjectIdentifier.instance);
        apdu_len += len;

        /* tag 3 - timeStamp */

        len =
            bacapp_encode_context_timestamp(&apdu[apdu_len], 3,
            &data->timeStamp);
        apdu_len += len;

        /* tag 4 - noticicationClass */

        len =
            encode_context_unsigned(&apdu[apdu_len], 4,
            data->notificationClass);
        apdu_len += len;

        /* tag 5 - priority */

        len = encode_context_unsigned(&apdu[apdu_len], 5, data->priority);
        apdu_len += len;

        /* tag 6 - eventType */
        len = encode_context_enumerated(&apdu[apdu_len], 6, data->eventType);
        apdu_len += len;

        /* tag 7 - messageText */
        if (data->messageText) {
            len =
                encode_context_character_string(&apdu[apdu_len], 7,
                data->messageText);
            apdu_len += len;
        }
        /* tag 8 - notifyType */
        len = encode_context_enumerated(&apdu[apdu_len], 8, data->notifyType);
        apdu_len += len;

        switch (data->notifyType) {
            case NOTIFY_ALARM:
            case NOTIFY_EVENT:
                /* tag 9 - ackRequired */

                len =
                    encode_context_boolean(&apdu[apdu_len], 9,
                    data->ackRequired);
                apdu_len += len;

                /* tag 10 - fromState */
                len =
                    encode_context_enumerated(&apdu[apdu_len], 10,
                    data->fromState);
                apdu_len += len;
                break;

            default:
                break;
        }

        /* tag 11 - toState */
        len = encode_context_enumerated(&apdu[apdu_len], 11, data->toState);
        apdu_len += len;

        switch (data->notifyType) {
            case NOTIFY_ALARM:
            case NOTIFY_EVENT:
                /* tag 12 - event values */
                len = encode_opening_tag(&apdu[apdu_len], 12);
                apdu_len += len;

                switch (data->eventType) {
                    case EVENT_CHANGE_OF_BITSTRING:
                        len = encode_opening_tag(&apdu[apdu_len], 0);
                        apdu_len += len;

                        len =
                            encode_context_bitstring(&apdu[apdu_len], 0,
                            &data->notificationParams.
                            changeOfBitstring.referencedBitString);
                        apdu_len += len;

                        len =
                            encode_context_bitstring(&apdu[apdu_len], 1,
                            &data->notificationParams.
                            changeOfBitstring.statusFlags);
                        apdu_len += len;

                        len = encode_closing_tag(&apdu[apdu_len], 0);
                        apdu_len += len;
                        break;

                    case EVENT_CHANGE_OF_STATE:
                        len = encode_opening_tag(&apdu[apdu_len], 1);
                        apdu_len += len;

                        len = encode_opening_tag(&apdu[apdu_len], 0);
                        apdu_len += len;

                        len =
                            bacapp_encode_property_state(&apdu[apdu_len],
                            &data->notificationParams.changeOfState.newState);
                        apdu_len += len;

                        len = encode_closing_tag(&apdu[apdu_len], 0);
                        apdu_len += len;

                        len =
                            encode_context_bitstring(&apdu[apdu_len], 1,
                            &data->notificationParams.
                            changeOfState.statusFlags);
                        apdu_len += len;

                        len = encode_closing_tag(&apdu[apdu_len], 1);
                        apdu_len += len;
                        break;

                    case EVENT_CHANGE_OF_VALUE:
                        len = encode_opening_tag(&apdu[apdu_len], 2);
                        apdu_len += len;

                        len = encode_opening_tag(&apdu[apdu_len], 0);
                        apdu_len += len;

                        switch (data->notificationParams.changeOfValue.tag) {
                            case CHANGE_OF_VALUE_REAL:
                                len =
                                    encode_context_real(&apdu[apdu_len], 1,
                                    data->notificationParams.
                                    changeOfValue.newValue.changeValue);
                                apdu_len += len;
                                break;

                            case CHANGE_OF_VALUE_BITS:
                                len =
                                    encode_context_bitstring(&apdu[apdu_len],
                                    0,
                                    &data->notificationParams.
                                    changeOfValue.newValue.changedBits);
                                apdu_len += len;
                                break;

                            default:
                                return 0;
                        }

                        len = encode_closing_tag(&apdu[apdu_len], 0);
                        apdu_len += len;

                        len =
                            encode_context_bitstring(&apdu[apdu_len], 1,
                            &data->notificationParams.
                            changeOfValue.statusFlags);
                        apdu_len += len;

                        len = encode_closing_tag(&apdu[apdu_len], 2);
                        apdu_len += len;
                        break;


                    case EVENT_FLOATING_LIMIT:
                        len = encode_opening_tag(&apdu[apdu_len], 4);
                        apdu_len += len;

                        len =
                            encode_context_real(&apdu[apdu_len], 0,
                            data->notificationParams.
                            floatingLimit.referenceValue);
                        apdu_len += len;

                        len =
                            encode_context_bitstring(&apdu[apdu_len], 1,
                            &data->notificationParams.
                            floatingLimit.statusFlags);
                        apdu_len += len;

                        len =
                            encode_context_real(&apdu[apdu_len], 2,
                            data->notificationParams.
                            floatingLimit.setPointValue);
                        apdu_len += len;

                        len =
                            encode_context_real(&apdu[apdu_len], 3,
                            data->notificationParams.floatingLimit.errorLimit);
                        apdu_len += len;

                        len = encode_closing_tag(&apdu[apdu_len], 4);
                        apdu_len += len;
                        break;


                    case EVENT_OUT_OF_RANGE:
                        len = encode_opening_tag(&apdu[apdu_len], 5);
                        apdu_len += len;

                        len =
                            encode_context_real(&apdu[apdu_len], 0,
                            data->notificationParams.
                            outOfRange.exceedingValue);
                        apdu_len += len;

                        len =
                            encode_context_bitstring(&apdu[apdu_len], 1,
                            &data->notificationParams.outOfRange.statusFlags);
                        apdu_len += len;

                        len =
                            encode_context_real(&apdu[apdu_len], 2,
                            data->notificationParams.outOfRange.deadband);
                        apdu_len += len;

                        len =
                            encode_context_real(&apdu[apdu_len], 3,
                            data->notificationParams.outOfRange.exceededLimit);
                        apdu_len += len;

                        len = encode_closing_tag(&apdu[apdu_len], 5);
                        apdu_len += len;
                        break;

                    case EVENT_CHANGE_OF_LIFE_SAFETY:
                        len = encode_opening_tag(&apdu[apdu_len], 8);
                        apdu_len += len;

                        len =
                            encode_context_enumerated(&apdu[apdu_len], 0,
                            data->notificationParams.
                            changeOfLifeSafety.newState);
                        apdu_len += len;

                        len =
                            encode_context_enumerated(&apdu[apdu_len], 1,
                            data->notificationParams.
                            changeOfLifeSafety.newMode);
                        apdu_len += len;

                        len =
                            encode_context_bitstring(&apdu[apdu_len], 2,
                            &data->notificationParams.
                            changeOfLifeSafety.statusFlags);
                        apdu_len += len;

                        len =
                            encode_context_enumerated(&apdu[apdu_len], 3,
                            data->notificationParams.
                            changeOfLifeSafety.operationExpected);
                        apdu_len += len;

                        len = encode_closing_tag(&apdu[apdu_len], 8);
                        apdu_len += len;
                        break;

                    case EVENT_BUFFER_READY:
                        len = encode_opening_tag(&apdu[apdu_len], 10);
                        apdu_len += len;

                        len =
                            bacapp_encode_context_device_obj_property_ref(&apdu
                            [apdu_len], 0,
                            &data->notificationParams.
                            bufferReady.bufferProperty);
                        apdu_len += len;

                        len =
                            encode_context_unsigned(&apdu[apdu_len], 1,
                            data->notificationParams.
                            bufferReady.previousNotification);
                        apdu_len += len;

                        len =
                            encode_context_unsigned(&apdu[apdu_len], 2,
                            data->notificationParams.
                            bufferReady.currentNotification);
                        apdu_len += len;

                        len = encode_closing_tag(&apdu[apdu_len], 10);
                        apdu_len += len;
                        break;
                    case EVENT_UNSIGNED_RANGE:
                        len = encode_opening_tag(&apdu[apdu_len], 11);
                        apdu_len += len;

                        len =
                            encode_context_unsigned(&apdu[apdu_len], 0,
                            data->notificationParams.
                            unsignedRange.exceedingValue);
                        apdu_len += len;

                        len =
                            encode_context_bitstring(&apdu[apdu_len], 1,
                            &data->notificationParams.
                            unsignedRange.statusFlags);
                        apdu_len += len;

                        len =
                            encode_context_unsigned(&apdu[apdu_len], 2,
                            data->notificationParams.
                            unsignedRange.exceededLimit);
                        apdu_len += len;

                        len = encode_closing_tag(&apdu[apdu_len], 11);
                        apdu_len += len;
                        break;
                    case EVENT_EXTENDED:
                    case EVENT_COMMAND_FAILURE:
                    default:
                        assert(0);
                        break;
                }
                len = encode_closing_tag(&apdu[apdu_len], 12);
                apdu_len += len;
                break;
            case NOTIFY_ACK_NOTIFICATION:
                /* FIXME: handle this case */
            default:
                break;
        }
    }
    return apdu_len;
}

int event_notify_decode_service_request(
    uint8_t * apdu,
    unsigned apdu_len,
    BACNET_EVENT_NOTIFICATION_DATA * data)
{
    int len = 0;        /* return value */
    int section_length = 0;
    uint32_t value = 0;

    if (apdu_len && data) {
        /* tag 0 - processIdentifier */
        if ((section_length =
                decode_context_unsigned(&apdu[len], 0,
                    &data->processIdentifier)) == -1) {
            return -1;
        } else {
            len += section_length;
        }

        /* tag 1 - initiatingObjectIdentifier */
        if ((section_length =
                decode_context_object_id(&apdu[len], 1,
                    &data->initiatingObjectIdentifier.type,
                    &data->initiatingObjectIdentifier.instance)) == -1) {
            return -1;
        } else {
            len += section_length;
        }
        /* tag 2 - eventObjectIdentifier */
        if ((section_length =
                decode_context_object_id(&apdu[len], 2,
                    &data->eventObjectIdentifier.type,
                    &data->eventObjectIdentifier.instance)) == -1) {
            return -1;
        } else {
            len += section_length;
        }
        /* tag 3 - timeStamp */
        if ((section_length =
                bacapp_decode_context_timestamp(&apdu[len], 3,
                    &data->timeStamp)) == -1) {
            return -1;
        } else {
            len += section_length;
        }
        /* tag 4 - noticicationClass */
        if ((section_length =
                decode_context_unsigned(&apdu[len], 4,
                    &data->notificationClass)) == -1) {
            return -1;
        } else {
            len += section_length;
        }
        /* tag 5 - priority */
        if ((section_length =
                decode_context_unsigned(&apdu[len], 5, &value)) == -1) {
            return -1;
        } else {
            if (value > 0xff) {
                return -1;
            } else {
                data->priority = (uint8_t) value;
                len += section_length;
            }
        }
        /* tag 6 - eventType */
        if ((section_length =
                decode_context_enumerated(&apdu[len], 6, &value)) == -1) {
            return -1;
        } else {
            data->eventType = (BACNET_EVENT_TYPE) value;
            len += section_length;
        }
        /* tag 7 - messageText */

        if (decode_is_context_tag(&apdu[len], 7)) {
            if (data->messageText != NULL) {
                if ((section_length =
                        decode_context_character_string(&apdu[len], 7,
                            data->messageText)) == -1) {
                    /*FIXME This is an optional parameter */
                    return -1;
                } else {
                    len += section_length;
                }
            } else {
                return -1;
            }
        } else {
            if (data->messageText != NULL) {
                characterstring_init_ansi(data->messageText, "");
            }
        }

        /* tag 8 - notifyType */
        if ((section_length =
                decode_context_enumerated(&apdu[len], 8, &value)) == -1) {
            return -1;
        } else {
            data->notifyType = (BACNET_NOTIFY_TYPE) value;
            len += section_length;
        }
        switch (data->notifyType) {
            case NOTIFY_ALARM:
            case NOTIFY_EVENT:
                /* tag 9 - ackRequired */
                section_length =
                    decode_context_boolean2(&apdu[len], 9, &data->ackRequired);
                if (section_length == -1) {
                    return -1;
                }
                len += section_length;

                /* tag 10 - fromState */
                if ((section_length =
                        decode_context_enumerated(&apdu[len], 10,
                            &value)) == -1) {
                    return -1;
                } else {
                    data->fromState = (BACNET_EVENT_STATE) value;
                    len += section_length;
                }
                break;
                /* In cases other than alarm and event
                   there's no data, so do not return an error
                   but continue normally */
            case NOTIFY_ACK_NOTIFICATION:
            default:
                break;

        }
        /* tag 11 - toState */
        if ((section_length =
                decode_context_enumerated(&apdu[len], 11, &value)) == -1) {
            return -1;
        } else {
            data->toState = (BACNET_EVENT_STATE) value;
            len += section_length;
        }
        /* tag 12 - eventValues */
        switch (data->notifyType) {
            case NOTIFY_ALARM:
            case NOTIFY_EVENT:
                if (decode_is_opening_tag_number(&apdu[len], 12)) {
                    len++;
                } else {
                    return -1;
                }
                if (decode_is_opening_tag_number(&apdu[len],
                        (uint8_t) data->eventType)) {
                    len++;
                } else {
                    return -1;
                }

                switch (data->eventType) {
                    case EVENT_CHANGE_OF_BITSTRING:
                        if (-1 == (section_length =
                                decode_context_bitstring(&apdu[len], 0,
                                    &data->
                                    notificationParams.changeOfBitstring.
                                    referencedBitString))) {
                            return -1;
                        }
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_bitstring(&apdu[len], 1,
                                    &data->
                                    notificationParams.changeOfBitstring.
                                    statusFlags))) {
                            return -1;
                        }
                        len += section_length;

                        break;

                    case EVENT_CHANGE_OF_STATE:
                        if (-1 == (section_length =
                                bacapp_decode_context_property_state(&apdu
                                    [len], 0,
                                    &data->notificationParams.
                                    changeOfState.newState))) {
                            return -1;
                        }
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_bitstring(&apdu[len], 1,
                                    &data->notificationParams.
                                    changeOfState.statusFlags))) {
                            return -1;
                        }
                        len += section_length;

                        break;

                    case EVENT_CHANGE_OF_VALUE:
                        if (!decode_is_opening_tag_number(&apdu[len], 0)) {
                            return -1;
                        }
                        len++;

                        if (decode_is_context_tag(&apdu[len],
                                CHANGE_OF_VALUE_BITS)) {

                            if (-1 == (section_length =
                                    decode_context_bitstring(&apdu[len], 0,
                                        &data->
                                        notificationParams.changeOfValue.
                                        newValue.changedBits))) {
                                return -1;
                            }

                            len += section_length;
                            data->notificationParams.changeOfValue.tag =
                                CHANGE_OF_VALUE_BITS;
                        } else if (decode_is_context_tag(&apdu[len],
                                CHANGE_OF_VALUE_REAL)) {
                            if (-1 == (section_length =
                                    decode_context_real(&apdu[len], 1,
                                        &data->
                                        notificationParams.changeOfValue.
                                        newValue.changeValue))) {
                                return -1;
                            }

                            len += section_length;
                            data->notificationParams.changeOfValue.tag =
                                CHANGE_OF_VALUE_REAL;
                        } else {
                            return -1;
                        }
                        if (!decode_is_closing_tag_number(&apdu[len], 0)) {
                            return -1;
                        }
                        len++;


                        if (-1 == (section_length =
                                decode_context_bitstring(&apdu[len], 1,
                                    &data->notificationParams.
                                    changeOfValue.statusFlags))) {
                            return -1;
                        }
                        len += section_length;
                        break;

                    case EVENT_FLOATING_LIMIT:
                        if (-1 == (section_length =
                                decode_context_real(&apdu[len], 0,
                                    &data->notificationParams.
                                    floatingLimit.referenceValue))) {
                            return -1;
                        }
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_bitstring(&apdu[len], 1,
                                    &data->notificationParams.
                                    floatingLimit.statusFlags))) {
                            return -1;
                        }
                        len += section_length;
                        if (-1 == (section_length =
                                decode_context_real(&apdu[len], 2,
                                    &data->notificationParams.
                                    floatingLimit.setPointValue))) {
                            return -1;
                        }
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_real(&apdu[len], 3,
                                    &data->notificationParams.
                                    floatingLimit.errorLimit))) {
                            return -1;
                        }
                        len += section_length;
                        break;

                    case EVENT_OUT_OF_RANGE:
                        if (-1 == (section_length =
                                decode_context_real(&apdu[len], 0,
                                    &data->notificationParams.
                                    outOfRange.exceedingValue))) {
                            return -1;
                        }
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_bitstring(&apdu[len], 1,
                                    &data->notificationParams.
                                    outOfRange.statusFlags))) {
                            return -1;
                        }
                        len += section_length;
                        if (-1 == (section_length =
                                decode_context_real(&apdu[len], 2,
                                    &data->notificationParams.
                                    outOfRange.deadband))) {
                            return -1;
                        }
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_real(&apdu[len], 3,
                                    &data->notificationParams.
                                    outOfRange.exceededLimit))) {
                            return -1;
                        }
                        len += section_length;
                        break;


                    case EVENT_CHANGE_OF_LIFE_SAFETY:
                        if (-1 == (section_length =
                                decode_context_enumerated(&apdu[len], 0,
                                    &value))) {
                            return -1;
                        }
                        data->notificationParams.changeOfLifeSafety.newState =
                            (BACNET_LIFE_SAFETY_STATE) value;
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_enumerated(&apdu[len], 1,
                                    &value))) {
                            return -1;
                        }
                        data->notificationParams.changeOfLifeSafety.newMode =
                            (BACNET_LIFE_SAFETY_MODE) value;
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_bitstring(&apdu[len], 2,
                                    &data->
                                    notificationParams.changeOfLifeSafety.
                                    statusFlags))) {
                            return -1;
                        }
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_enumerated(&apdu[len], 3,
                                    &value))) {
                            return -1;
                        }
                        data->notificationParams.
                            changeOfLifeSafety.operationExpected =
                            (BACNET_LIFE_SAFETY_OPERATION) value;
                        len += section_length;
                        break;

                    case EVENT_BUFFER_READY:
                        if (-1 == (section_length =
                                bacapp_decode_context_device_obj_property_ref
                                (&apdu[len], 0,
                                    &data->notificationParams.
                                    bufferReady.bufferProperty))) {
                            return -1;
                        }
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_unsigned(&apdu[len], 1,
                                    &data->notificationParams.
                                    bufferReady.previousNotification))) {
                            return -1;
                        }
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_unsigned(&apdu[len], 2,
                                    &data->notificationParams.
                                    bufferReady.currentNotification))) {
                            return -1;
                        }
                        len += section_length;
                        break;

                    case EVENT_UNSIGNED_RANGE:
                        if (-1 == (section_length =
                                decode_context_unsigned(&apdu[len], 0,
                                    &data->notificationParams.
                                    unsignedRange.exceedingValue))) {
                            return -1;
                        }
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_bitstring(&apdu[len], 1,
                                    &data->notificationParams.
                                    unsignedRange.statusFlags))) {
                            return -1;
                        }
                        len += section_length;

                        if (-1 == (section_length =
                                decode_context_unsigned(&apdu[len], 2,
                                    &data->notificationParams.
                                    unsignedRange.exceededLimit))) {
                            return -1;
                        }
                        len += section_length;
                        break;

                    default:
                        return -1;
                }
                if (decode_is_closing_tag_number(&apdu[len],
                        (uint8_t) data->eventType)) {
                    len++;
                } else {
                    return -1;
                }
                if (decode_is_closing_tag_number(&apdu[len], 12)) {
                    len++;
                } else {
                    return -1;
                }
                break;
                /* In cases other than alarm and event
                   there's no data, so do not return an error
                   but continue normally */
            case NOTIFY_ACK_NOTIFICATION:
            default:
                break;
        }
    }

    return len;
}

#ifdef  TEST

#include <assert.h>
#include <string.h>
#include "ctest.h"


BACNET_EVENT_NOTIFICATION_DATA data;
BACNET_EVENT_NOTIFICATION_DATA data2;

void testBaseEventState(
    Test * pTest)
{
    ct_test(pTest, data.processIdentifier == data2.processIdentifier);
    ct_test(pTest,
        data.initiatingObjectIdentifier.instance ==
        data2.initiatingObjectIdentifier.instance);
    ct_test(pTest,
        data.initiatingObjectIdentifier.type ==
        data2.initiatingObjectIdentifier.type);
    ct_test(pTest,
        data.eventObjectIdentifier.instance ==
        data2.eventObjectIdentifier.instance);
    ct_test(pTest,
        data.eventObjectIdentifier.type == data2.eventObjectIdentifier.type);
    ct_test(pTest, data.notificationClass == data2.notificationClass);
    ct_test(pTest, data.priority == data2.priority);
    ct_test(pTest, data.notifyType == data2.notifyType);
    ct_test(pTest, data.fromState == data2.fromState);
    ct_test(pTest, data.toState == data2.toState);
    ct_test(pTest, data.toState == data2.toState);

    if (data.messageText != NULL && data2.messageText != NULL) {
        ct_test(pTest,
            data.messageText->encoding == data2.messageText->encoding);
        ct_test(pTest, data.messageText->length == data2.messageText->length);
        ct_test(pTest, strcmp(data.messageText->value,
                data2.messageText->value) == 0);
    }

    ct_test(pTest, data.timeStamp.tag == data2.timeStamp.tag);

    switch (data.timeStamp.tag) {
        case TIME_STAMP_SEQUENCE:
            ct_test(pTest,
                data.timeStamp.value.sequenceNum ==
                data2.timeStamp.value.sequenceNum);
            break;

        case TIME_STAMP_DATETIME:
            ct_test(pTest,
                data.timeStamp.value.dateTime.time.hour ==
                data2.timeStamp.value.dateTime.time.hour);
            ct_test(pTest,
                data.timeStamp.value.dateTime.time.min ==
                data2.timeStamp.value.dateTime.time.min);
            ct_test(pTest,
                data.timeStamp.value.dateTime.time.sec ==
                data2.timeStamp.value.dateTime.time.sec);
            ct_test(pTest,
                data.timeStamp.value.dateTime.time.hundredths ==
                data2.timeStamp.value.dateTime.time.hundredths);

            ct_test(pTest,
                data.timeStamp.value.dateTime.date.day ==
                data2.timeStamp.value.dateTime.date.day);
            ct_test(pTest,
                data.timeStamp.value.dateTime.date.month ==
                data2.timeStamp.value.dateTime.date.month);
            ct_test(pTest,
                data.timeStamp.value.dateTime.date.wday ==
                data2.timeStamp.value.dateTime.date.wday);
            ct_test(pTest,
                data.timeStamp.value.dateTime.date.year ==
                data2.timeStamp.value.dateTime.date.year);
            break;

        case TIME_STAMP_TIME:
            ct_test(pTest,
                data.timeStamp.value.time.hour ==
                data2.timeStamp.value.time.hour);
            ct_test(pTest,
                data.timeStamp.value.time.min ==
                data2.timeStamp.value.time.min);
            ct_test(pTest,
                data.timeStamp.value.time.sec ==
                data2.timeStamp.value.time.sec);
            ct_test(pTest,
                data.timeStamp.value.time.hundredths ==
                data2.timeStamp.value.time.hundredths);
            break;

        default:
            ct_fail(pTest, "Unknown type");
            break;
    }
}

void testEventEventState(
    Test * pTest)
{
    uint8_t buffer[MAX_APDU];
    int inLen;
    int outLen;
    BACNET_CHARACTER_STRING messageText;
    BACNET_CHARACTER_STRING messageText2;
    characterstring_init_ansi(&messageText,
        "This is a test of the message text\n");

    data.messageText = &messageText;
    data2.messageText = &messageText2;

    data.processIdentifier = 1234;
    data.initiatingObjectIdentifier.type = OBJECT_ANALOG_INPUT;
    data.initiatingObjectIdentifier.instance = 100;
    data.eventObjectIdentifier.type = OBJECT_ANALOG_INPUT;
    data.eventObjectIdentifier.instance = 200;
    data.timeStamp.value.sequenceNum = 1234;
    data.timeStamp.tag = TIME_STAMP_SEQUENCE;
    data.notificationClass = 50;
    data.priority = 50;
    data.notifyType = NOTIFY_ALARM;
    data.fromState = EVENT_STATE_NORMAL;
    data.toState = EVENT_STATE_OFFNORMAL;

    data.eventType = EVENT_CHANGE_OF_STATE;
    data.notificationParams.changeOfState.newState.tag = UNITS;
    data.notificationParams.changeOfState.newState.state.units =
        UNITS_SQUARE_METERS;

    bitstring_init(&data.notificationParams.changeOfState.statusFlags);
    bitstring_set_bit(&data.notificationParams.changeOfState.statusFlags,
        STATUS_FLAG_IN_ALARM, true);
    bitstring_set_bit(&data.notificationParams.changeOfState.statusFlags,
        STATUS_FLAG_FAULT, false);
    bitstring_set_bit(&data.notificationParams.changeOfState.statusFlags,
        STATUS_FLAG_OVERRIDDEN, false);
    bitstring_set_bit(&data.notificationParams.changeOfState.statusFlags,
        STATUS_FLAG_OUT_OF_SERVICE, false);


    inLen = event_notify_encode_service_request(&buffer[0], &data);

    outLen = event_notify_decode_service_request(&buffer[0], inLen, &data2);

    ct_test(pTest, inLen == outLen);
    testBaseEventState(pTest);

    ct_test(pTest,
        data.notificationParams.changeOfState.newState.tag ==
        data2.notificationParams.changeOfState.newState.tag);
    ct_test(pTest,
        data.notificationParams.changeOfState.newState.state.units ==
        data2.notificationParams.changeOfState.newState.state.units);

    ct_test(pTest,
        bitstring_same(&data.notificationParams.changeOfState.statusFlags,
            &data2.notificationParams.changeOfState.statusFlags));

        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/

    /*
     ** Same, but timestamp of
     */
    data.timeStamp.tag = TIME_STAMP_DATETIME;
    data.timeStamp.value.dateTime.time.hour = 1;
    data.timeStamp.value.dateTime.time.min = 2;
    data.timeStamp.value.dateTime.time.sec = 3;
    data.timeStamp.value.dateTime.time.hundredths = 4;

    data.timeStamp.value.dateTime.date.day = 1;
    data.timeStamp.value.dateTime.date.month = 1;
    data.timeStamp.value.dateTime.date.wday = 1;
    data.timeStamp.value.dateTime.date.year = 1945;

    memset(buffer, 0, MAX_APDU);
    inLen = event_notify_encode_service_request(&buffer[0], &data);

    memset(&data2, 0, sizeof(data2));
    data2.messageText = &messageText2;
    outLen = event_notify_decode_service_request(&buffer[0], inLen, &data2);

    ct_test(pTest, inLen == outLen);
    testBaseEventState(pTest);
    ct_test(pTest,
        data.notificationParams.changeOfState.newState.tag ==
        data2.notificationParams.changeOfState.newState.tag);
    ct_test(pTest,
        data.notificationParams.changeOfState.newState.state.units ==
        data2.notificationParams.changeOfState.newState.state.units);

        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/

    /*
     ** Event Type = EVENT_CHANGE_OF_BITSTRING
     */
    data.timeStamp.value.sequenceNum = 1234;
    data.timeStamp.tag = TIME_STAMP_SEQUENCE;

    data.eventType = EVENT_CHANGE_OF_BITSTRING;

    bitstring_init(&data.notificationParams.
        changeOfBitstring.referencedBitString);
    bitstring_set_bit(&data.notificationParams.
        changeOfBitstring.referencedBitString, 0, true);
    bitstring_set_bit(&data.notificationParams.
        changeOfBitstring.referencedBitString, 1, false);
    bitstring_set_bit(&data.notificationParams.
        changeOfBitstring.referencedBitString, 2, true);
    bitstring_set_bit(&data.notificationParams.
        changeOfBitstring.referencedBitString, 2, false);

    bitstring_init(&data.notificationParams.changeOfBitstring.statusFlags);

    bitstring_set_bit(&data.notificationParams.changeOfBitstring.statusFlags,
        STATUS_FLAG_IN_ALARM, true);
    bitstring_set_bit(&data.notificationParams.changeOfBitstring.statusFlags,
        STATUS_FLAG_FAULT, false);
    bitstring_set_bit(&data.notificationParams.changeOfBitstring.statusFlags,
        STATUS_FLAG_OVERRIDDEN, false);
    bitstring_set_bit(&data.notificationParams.changeOfBitstring.statusFlags,
        STATUS_FLAG_OUT_OF_SERVICE, false);

    memset(buffer, 0, MAX_APDU);
    inLen = event_notify_encode_service_request(&buffer[0], &data);

    memset(&data2, 0, sizeof(data2));
    data2.messageText = &messageText2;
    outLen = event_notify_decode_service_request(&buffer[0], inLen, &data2);

    ct_test(pTest, inLen == outLen);
    testBaseEventState(pTest);

    ct_test(pTest,
        bitstring_same(&data.notificationParams.changeOfBitstring.
            referencedBitString,
            &data2.notificationParams.changeOfBitstring.referencedBitString));

    ct_test(pTest,
        bitstring_same(&data.notificationParams.changeOfBitstring.statusFlags,
            &data2.notificationParams.changeOfBitstring.statusFlags));

        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
    /*
     ** Event Type = EVENT_CHANGE_OF_VALUE - float value
     */

    data.eventType = EVENT_CHANGE_OF_VALUE;
    data.notificationParams.changeOfValue.tag = CHANGE_OF_VALUE_REAL;
    data.notificationParams.changeOfValue.newValue.changeValue = 1.23f;

    bitstring_init(&data.notificationParams.changeOfValue.statusFlags);

    bitstring_set_bit(&data.notificationParams.changeOfValue.statusFlags,
        STATUS_FLAG_IN_ALARM, true);
    bitstring_set_bit(&data.notificationParams.changeOfValue.statusFlags,
        STATUS_FLAG_FAULT, false);
    bitstring_set_bit(&data.notificationParams.changeOfValue.statusFlags,
        STATUS_FLAG_OVERRIDDEN, false);
    bitstring_set_bit(&data.notificationParams.changeOfValue.statusFlags,
        STATUS_FLAG_OUT_OF_SERVICE, false);

    memset(buffer, 0, MAX_APDU);
    inLen = event_notify_encode_service_request(&buffer[0], &data);

    memset(&data2, 0, sizeof(data2));
    data2.messageText = &messageText2;
    outLen = event_notify_decode_service_request(&buffer[0], inLen, &data2);

    ct_test(pTest, inLen == outLen);
    testBaseEventState(pTest);

    ct_test(pTest,
        bitstring_same(&data.notificationParams.changeOfValue.statusFlags,
            &data2.notificationParams.changeOfValue.statusFlags));

    ct_test(pTest,
        data.notificationParams.changeOfValue.tag ==
        data2.notificationParams.changeOfValue.tag);

    ct_test(pTest,
        data.notificationParams.changeOfValue.newValue.changeValue ==
        data2.notificationParams.changeOfValue.newValue.changeValue);



    /*
     ** Event Type = EVENT_CHANGE_OF_VALUE - bitstring value
     */

    data.notificationParams.changeOfValue.tag = CHANGE_OF_VALUE_BITS;

    bitstring_init(&data.notificationParams.changeOfValue.
        newValue.changedBits);
    bitstring_set_bit(&data.notificationParams.changeOfValue.
        newValue.changedBits, 0, true);
    bitstring_set_bit(&data.notificationParams.changeOfValue.
        newValue.changedBits, 1, false);
    bitstring_set_bit(&data.notificationParams.changeOfValue.
        newValue.changedBits, 2, false);
    bitstring_set_bit(&data.notificationParams.changeOfValue.
        newValue.changedBits, 3, false);

    memset(buffer, 0, MAX_APDU);
    inLen = event_notify_encode_service_request(&buffer[0], &data);

    memset(&data2, 0, sizeof(data2));
    data2.messageText = &messageText2;
    outLen = event_notify_decode_service_request(&buffer[0], inLen, &data2);

    ct_test(pTest, inLen == outLen);
    testBaseEventState(pTest);

    ct_test(pTest,
        bitstring_same(&data.notificationParams.changeOfValue.statusFlags,
            &data2.notificationParams.changeOfValue.statusFlags));

    ct_test(pTest,
        data.notificationParams.changeOfValue.tag ==
        data2.notificationParams.changeOfValue.tag);

    ct_test(pTest,
        bitstring_same(&data.notificationParams.changeOfValue.newValue.
            changedBits,
            &data2.notificationParams.changeOfValue.newValue.changedBits));

        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
    /*
     ** Event Type = EVENT_FLOATING_LIMIT
     */
    data.eventType = EVENT_FLOATING_LIMIT;
    data.notificationParams.floatingLimit.referenceValue = 1.23f;
    data.notificationParams.floatingLimit.setPointValue = 2.34f;
    data.notificationParams.floatingLimit.errorLimit = 3.45f;

    bitstring_init(&data.notificationParams.floatingLimit.statusFlags);

    bitstring_set_bit(&data.notificationParams.floatingLimit.statusFlags,
        STATUS_FLAG_IN_ALARM, true);
    bitstring_set_bit(&data.notificationParams.floatingLimit.statusFlags,
        STATUS_FLAG_FAULT, false);
    bitstring_set_bit(&data.notificationParams.floatingLimit.statusFlags,
        STATUS_FLAG_OVERRIDDEN, false);
    bitstring_set_bit(&data.notificationParams.floatingLimit.statusFlags,
        STATUS_FLAG_OUT_OF_SERVICE, false);

    memset(buffer, 0, MAX_APDU);
    inLen = event_notify_encode_service_request(&buffer[0], &data);

    memset(&data2, 0, sizeof(data2));
    data2.messageText = &messageText2;
    outLen = event_notify_decode_service_request(&buffer[0], inLen, &data2);

    ct_test(pTest, inLen == outLen);
    testBaseEventState(pTest);

    ct_test(pTest,
        data.notificationParams.floatingLimit.referenceValue ==
        data2.notificationParams.floatingLimit.referenceValue);

    ct_test(pTest,
        data.notificationParams.floatingLimit.setPointValue ==
        data2.notificationParams.floatingLimit.setPointValue);

    ct_test(pTest,
        data.notificationParams.floatingLimit.errorLimit ==
        data2.notificationParams.floatingLimit.errorLimit);
    ct_test(pTest,
        bitstring_same(&data.notificationParams.floatingLimit.statusFlags,
            &data2.notificationParams.floatingLimit.statusFlags));


        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
    /*
     ** Event Type = EVENT_OUT_OF_RANGE
     */
    data.eventType = EVENT_OUT_OF_RANGE;
    data.notificationParams.outOfRange.exceedingValue = 3.45f;
    data.notificationParams.outOfRange.deadband = 2.34f;
    data.notificationParams.outOfRange.exceededLimit = 1.23f;

    bitstring_init(&data.notificationParams.outOfRange.statusFlags);

    bitstring_set_bit(&data.notificationParams.outOfRange.statusFlags,
        STATUS_FLAG_IN_ALARM, true);
    bitstring_set_bit(&data.notificationParams.outOfRange.statusFlags,
        STATUS_FLAG_FAULT, false);
    bitstring_set_bit(&data.notificationParams.outOfRange.statusFlags,
        STATUS_FLAG_OVERRIDDEN, false);
    bitstring_set_bit(&data.notificationParams.outOfRange.statusFlags,
        STATUS_FLAG_OUT_OF_SERVICE, false);

    memset(buffer, 0, MAX_APDU);
    inLen = event_notify_encode_service_request(&buffer[0], &data);

    memset(&data2, 0, sizeof(data2));
    data2.messageText = &messageText2;
    outLen = event_notify_decode_service_request(&buffer[0], inLen, &data2);

    ct_test(pTest, inLen == outLen);
    testBaseEventState(pTest);

    ct_test(pTest,
        data.notificationParams.outOfRange.deadband ==
        data2.notificationParams.outOfRange.deadband);

    ct_test(pTest,
        data.notificationParams.outOfRange.exceededLimit ==
        data2.notificationParams.outOfRange.exceededLimit);

    ct_test(pTest,
        data.notificationParams.outOfRange.exceedingValue ==
        data2.notificationParams.outOfRange.exceedingValue);
    ct_test(pTest,
        bitstring_same(&data.notificationParams.outOfRange.statusFlags,
            &data2.notificationParams.outOfRange.statusFlags));

        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
    /*
     ** Event Type = EVENT_CHANGE_OF_LIFE_SAFETY
     */
    data.eventType = EVENT_CHANGE_OF_LIFE_SAFETY;
    data.notificationParams.changeOfLifeSafety.newState =
        LIFE_SAFETY_STATE_ALARM;
    data.notificationParams.changeOfLifeSafety.newMode =
        LIFE_SAFETY_MODE_ARMED;
    data.notificationParams.changeOfLifeSafety.operationExpected =
        LIFE_SAFETY_OP_RESET;

    bitstring_init(&data.notificationParams.changeOfLifeSafety.statusFlags);

    bitstring_set_bit(&data.notificationParams.changeOfLifeSafety.statusFlags,
        STATUS_FLAG_IN_ALARM, true);
    bitstring_set_bit(&data.notificationParams.changeOfLifeSafety.statusFlags,
        STATUS_FLAG_FAULT, false);
    bitstring_set_bit(&data.notificationParams.changeOfLifeSafety.statusFlags,
        STATUS_FLAG_OVERRIDDEN, false);
    bitstring_set_bit(&data.notificationParams.changeOfLifeSafety.statusFlags,
        STATUS_FLAG_OUT_OF_SERVICE, false);

    memset(buffer, 0, MAX_APDU);
    inLen = event_notify_encode_service_request(&buffer[0], &data);

    memset(&data2, 0, sizeof(data2));
    data2.messageText = &messageText2;
    outLen = event_notify_decode_service_request(&buffer[0], inLen, &data2);

    ct_test(pTest, inLen == outLen);
    testBaseEventState(pTest);

    ct_test(pTest,
        data.notificationParams.changeOfLifeSafety.newMode ==
        data2.notificationParams.changeOfLifeSafety.newMode);

    ct_test(pTest,
        data.notificationParams.changeOfLifeSafety.newState ==
        data2.notificationParams.changeOfLifeSafety.newState);

    ct_test(pTest,
        data.notificationParams.changeOfLifeSafety.operationExpected ==
        data2.notificationParams.changeOfLifeSafety.operationExpected);

    ct_test(pTest,
        bitstring_same(&data.notificationParams.changeOfLifeSafety.statusFlags,
            &data2.notificationParams.changeOfLifeSafety.statusFlags));

        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
    /*
     ** Event Type = EVENT_UNSIGNED_RANGE
     */
    data.eventType = EVENT_UNSIGNED_RANGE;
    data.notificationParams.unsignedRange.exceedingValue = 1234;
    data.notificationParams.unsignedRange.exceededLimit = 2345;

    bitstring_init(&data.notificationParams.unsignedRange.statusFlags);

    bitstring_set_bit(&data.notificationParams.unsignedRange.statusFlags,
        STATUS_FLAG_IN_ALARM, true);
    bitstring_set_bit(&data.notificationParams.unsignedRange.statusFlags,
        STATUS_FLAG_FAULT, false);
    bitstring_set_bit(&data.notificationParams.unsignedRange.statusFlags,
        STATUS_FLAG_OVERRIDDEN, false);
    bitstring_set_bit(&data.notificationParams.unsignedRange.statusFlags,
        STATUS_FLAG_OUT_OF_SERVICE, false);

    memset(buffer, 0, MAX_APDU);
    inLen = event_notify_encode_service_request(&buffer[0], &data);

    memset(&data2, 0, sizeof(data2));
    data2.messageText = &messageText2;
    outLen = event_notify_decode_service_request(&buffer[0], inLen, &data2);

    ct_test(pTest, inLen == outLen);
    testBaseEventState(pTest);

    ct_test(pTest,
        data.notificationParams.unsignedRange.exceedingValue ==
        data2.notificationParams.unsignedRange.exceedingValue);

    ct_test(pTest,
        data.notificationParams.unsignedRange.exceededLimit ==
        data2.notificationParams.unsignedRange.exceededLimit);

    ct_test(pTest,
        bitstring_same(&data.notificationParams.unsignedRange.statusFlags,
            &data2.notificationParams.unsignedRange.statusFlags));

        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
        /**********************************************************************************/
    /*
     ** Event Type = EVENT_BUFFER_READY
     */
    data.eventType = EVENT_BUFFER_READY;
    data.notificationParams.bufferReady.previousNotification = 1234;
    data.notificationParams.bufferReady.currentNotification = 2345;
    data.notificationParams.bufferReady.bufferProperty.deviceIndentifier.type =
        OBJECT_DEVICE;
    data.notificationParams.bufferReady.bufferProperty.
        deviceIndentifier.instance = 500;
    data.notificationParams.bufferReady.bufferProperty.objectIdentifier.type =
        OBJECT_ANALOG_INPUT;
    data.notificationParams.bufferReady.bufferProperty.
        objectIdentifier.instance = 100;
    data.notificationParams.bufferReady.bufferProperty.propertyIdentifier =
        PROP_PRESENT_VALUE;
    data.notificationParams.bufferReady.bufferProperty.arrayIndex = 0;

    memset(buffer, 0, MAX_APDU);
    inLen = event_notify_encode_service_request(&buffer[0], &data);

    memset(&data2, 0, sizeof(data2));
    data2.messageText = &messageText2;
    outLen = event_notify_decode_service_request(&buffer[0], inLen, &data2);

    ct_test(pTest, inLen == outLen);
    testBaseEventState(pTest);

    ct_test(pTest,
        data.notificationParams.bufferReady.previousNotification ==
        data2.notificationParams.bufferReady.previousNotification);

    ct_test(pTest,
        data.notificationParams.bufferReady.currentNotification ==
        data2.notificationParams.bufferReady.currentNotification);


    ct_test(pTest,
        data.notificationParams.bufferReady.bufferProperty.
        deviceIndentifier.type ==
        data2.notificationParams.bufferReady.bufferProperty.
        deviceIndentifier.type);

    ct_test(pTest,
        data.notificationParams.bufferReady.bufferProperty.
        deviceIndentifier.instance ==
        data2.notificationParams.bufferReady.bufferProperty.
        deviceIndentifier.instance);

    ct_test(pTest,
        data.notificationParams.bufferReady.bufferProperty.
        objectIdentifier.instance ==
        data2.notificationParams.bufferReady.bufferProperty.
        objectIdentifier.instance);

    ct_test(pTest,
        data.notificationParams.bufferReady.bufferProperty.
        objectIdentifier.type ==
        data2.notificationParams.bufferReady.bufferProperty.
        objectIdentifier.type);

    ct_test(pTest,
        data.notificationParams.bufferReady.
        bufferProperty.propertyIdentifier ==
        data2.notificationParams.bufferReady.
        bufferProperty.propertyIdentifier);

    ct_test(pTest,
        data.notificationParams.bufferReady.bufferProperty.arrayIndex ==
        data2.notificationParams.bufferReady.bufferProperty.arrayIndex);
}

#ifdef TEST_EVENT

int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Event", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testEventEventState);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}

#endif /* TEST_EVENT */
#endif /* TEST */
