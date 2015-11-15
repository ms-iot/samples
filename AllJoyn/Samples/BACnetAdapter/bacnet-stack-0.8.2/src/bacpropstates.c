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

#include "bacdcode.h"
#include "npdu.h"
#include "timestamp.h"
#include "bacpropstates.h"

/** @file bacpropstates.c  Encode/Decode BACnet Application Property States */

int bacapp_decode_property_state(
    uint8_t * apdu,
    BACNET_PROPERTY_STATE * value)
{
    int len = 0;
    uint32_t len_value_type;
    int section_length;
    uint32_t enumValue;
    uint8_t tagnum;

    section_length =
        decode_tag_number_and_value(&apdu[len], &tagnum, &len_value_type);

    if (-1 == section_length) {
        return -1;
    }
    value->tag = tagnum;
    len += section_length;
    switch (value->tag) {
        case BOOLEAN_VALUE:
            value->state.booleanValue = decode_boolean(len_value_type);
            break;

        case BINARY_VALUE:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.binaryValue = (BACNET_BINARY_PV) enumValue;
            break;

        case EVENT_TYPE:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.eventType = (BACNET_EVENT_TYPE) enumValue;
            break;

        case POLARITY:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.polarity = (BACNET_POLARITY) enumValue;
            break;

        case PROGRAM_CHANGE:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.programChange = (BACNET_PROGRAM_REQUEST) enumValue;
            break;

        case PROGRAM_STATE:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.programState = (BACNET_PROGRAM_STATE) enumValue;
            break;

        case REASON_FOR_HALT:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.programError = (BACNET_PROGRAM_ERROR) enumValue;
            break;

        case RELIABILITY:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.reliability = (BACNET_RELIABILITY) enumValue;
            break;

        case STATE:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.state = (BACNET_EVENT_STATE) enumValue;
            break;

        case SYSTEM_STATUS:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.systemStatus = (BACNET_DEVICE_STATUS) enumValue;
            break;

        case UNITS:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.units = (BACNET_ENGINEERING_UNITS) enumValue;
            break;

        case UNSIGNED_VALUE:
            if (-1 == (section_length =
                    decode_unsigned(&apdu[len], len_value_type,
                        &value->state.unsignedValue))) {
                return -1;
            }
            break;

        case LIFE_SAFETY_MODE:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.lifeSafetyMode = (BACNET_LIFE_SAFETY_MODE) enumValue;
            break;

        case LIFE_SAFETY_STATE:
            if (-1 == (section_length =
                    decode_enumerated(&apdu[len], len_value_type,
                        &enumValue))) {
                return -1;
            }
            value->state.lifeSafetyState =
                (BACNET_LIFE_SAFETY_STATE) enumValue;
            break;

        default:
            return -1;
    }
    len += section_length;

    return len;
}

int bacapp_decode_context_property_state(
    uint8_t * apdu,
    uint8_t tag_number,
    BACNET_PROPERTY_STATE * value)
{
    int len = 0;
    int section_length;

    if (decode_is_opening_tag_number(&apdu[len], tag_number)) {
        len++;
        section_length = bacapp_decode_property_state(&apdu[len], value);

        if (section_length == -1) {
            len = -1;
        } else {
            len += section_length;
            if (decode_is_closing_tag_number(&apdu[len], tag_number)) {
                len++;
            } else {
                len = -1;
            }
        }
    } else {
        len = -1;
    }
    return len;
}

int bacapp_encode_property_state(
    uint8_t * apdu,
    BACNET_PROPERTY_STATE * value)
{
    int len = 0;        /* length of each encoding */
    if (value && apdu) {
        switch (value->tag) {
            case BOOLEAN_VALUE:
                len =
                    encode_context_boolean(&apdu[0], 0,
                    value->state.booleanValue);
                break;

            case BINARY_VALUE:
                len =
                    encode_context_enumerated(&apdu[0], 1,
                    value->state.binaryValue);
                break;

            case EVENT_TYPE:
                len =
                    encode_context_enumerated(&apdu[0], 2,
                    value->state.eventType);
                break;

            case POLARITY:
                len =
                    encode_context_enumerated(&apdu[0], 3,
                    value->state.polarity);
                break;

            case PROGRAM_CHANGE:
                len =
                    encode_context_enumerated(&apdu[0], 4,
                    value->state.programChange);
                break;

            case PROGRAM_STATE:
                len =
                    encode_context_enumerated(&apdu[0], 5,
                    value->state.programState);
                break;

            case REASON_FOR_HALT:
                len =
                    encode_context_enumerated(&apdu[0], 6,
                    value->state.programError);
                break;

            case RELIABILITY:
                len =
                    encode_context_enumerated(&apdu[0], 7,
                    value->state.reliability);
                break;

            case STATE:
                len =
                    encode_context_enumerated(&apdu[0], 8, value->state.state);
                break;

            case SYSTEM_STATUS:
                len =
                    encode_context_enumerated(&apdu[0], 9,
                    value->state.systemStatus);
                break;

            case UNITS:
                len =
                    encode_context_enumerated(&apdu[0], 10,
                    value->state.units);
                break;

            case UNSIGNED_VALUE:
                len =
                    encode_context_unsigned(&apdu[0], 11,
                    value->state.unsignedValue);
                break;

            case LIFE_SAFETY_MODE:
                len =
                    encode_context_enumerated(&apdu[0], 12,
                    value->state.lifeSafetyMode);
                break;

            case LIFE_SAFETY_STATE:
                len =
                    encode_context_enumerated(&apdu[0], 13,
                    value->state.lifeSafetyState);
                break;

            default:
                /* FIXME: assert(0); - return a negative len? */
                break;
        }
    }
    return len;
}

#ifdef TEST
#include <string.h>     /* for memset */

void testPropStates(
    Test * pTest)
{
    BACNET_PROPERTY_STATE inData;
    BACNET_PROPERTY_STATE outData;
    uint8_t appMsg[MAX_APDU];
    int inLen;
    int outLen;

    inData.tag = BOOLEAN_VALUE;
    inData.state.booleanValue = true;

    inLen = bacapp_encode_property_state(appMsg, &inData);

    memset(&outData, 0, sizeof(outData));

    outLen = bacapp_decode_property_state(appMsg, &outData);

    ct_test(pTest, outLen == inLen);
    ct_test(pTest, inData.tag == outData.tag);
    ct_test(pTest, inData.state.booleanValue == outData.state.booleanValue);

        /****************
	*****************
	****************/
    inData.tag = BINARY_VALUE;
    inData.state.binaryValue = BINARY_ACTIVE;

    inLen = bacapp_encode_property_state(appMsg, &inData);

    memset(&outData, 0, sizeof(outData));

    outLen = bacapp_decode_property_state(appMsg, &outData);

    ct_test(pTest, outLen == inLen);
    ct_test(pTest, inData.tag == outData.tag);
    ct_test(pTest, inData.state.binaryValue == outData.state.binaryValue);

        /****************
	*****************
	****************/
    inData.tag = EVENT_TYPE;
    inData.state.eventType = EVENT_BUFFER_READY;

    inLen = bacapp_encode_property_state(appMsg, &inData);

    memset(&outData, 0, sizeof(outData));

    outLen = bacapp_decode_property_state(appMsg, &outData);

    ct_test(pTest, outLen == inLen);
    ct_test(pTest, inData.tag == outData.tag);
    ct_test(pTest, inData.state.eventType == outData.state.eventType);

        /****************
	*****************
	****************/
    inData.tag = POLARITY;
    inData.state.polarity = POLARITY_REVERSE;

    inLen = bacapp_encode_property_state(appMsg, &inData);

    memset(&outData, 0, sizeof(outData));

    outLen = bacapp_decode_property_state(appMsg, &outData);

    ct_test(pTest, outLen == inLen);
    ct_test(pTest, inData.tag == outData.tag);
    ct_test(pTest, inData.state.polarity == outData.state.polarity);

        /****************
	*****************
	****************/
    inData.tag = PROGRAM_CHANGE;
    inData.state.programChange = PROGRAM_REQUEST_RESTART;

    inLen = bacapp_encode_property_state(appMsg, &inData);

    memset(&outData, 0, sizeof(outData));

    outLen = bacapp_decode_property_state(appMsg, &outData);

    ct_test(pTest, outLen == inLen);
    ct_test(pTest, inData.tag == outData.tag);
    ct_test(pTest, inData.state.programChange == outData.state.programChange);

        /****************
	*****************
	****************/
    inData.tag = UNSIGNED_VALUE;
    inData.state.unsignedValue = 0xdeadbeef;

    inLen = bacapp_encode_property_state(appMsg, &inData);

    memset(&outData, 0, sizeof(outData));

    outLen = bacapp_decode_property_state(appMsg, &outData);

    ct_test(pTest, outLen == inLen);
    ct_test(pTest, inData.tag == outData.tag);
    ct_test(pTest, inData.state.unsignedValue == outData.state.unsignedValue);

}

#ifdef TEST_PROP_STATES
#include <assert.h>
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Event", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testPropStates);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}

#endif /* TEST_PROP_STATES */
#endif /* TEST */
