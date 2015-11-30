/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#ifndef GETEVENT_H
#define GETEVENT_H

#include <stdint.h>
#include <stdbool.h>
#include "bacdef.h"
#include "bacenum.h"
#include "timestamp.h"
#include "event.h"

struct BACnet_Get_Event_Information_Data;
typedef struct BACnet_Get_Event_Information_Data {
    BACNET_OBJECT_ID objectIdentifier;
    BACNET_EVENT_STATE eventState;
    BACNET_BIT_STRING acknowledgedTransitions;
    BACNET_TIMESTAMP eventTimeStamps[3];
    BACNET_NOTIFY_TYPE notifyType;
    BACNET_BIT_STRING eventEnable;
    uint32_t eventPriorities[3];
    struct BACnet_Get_Event_Information_Data *next;
} BACNET_GET_EVENT_INFORMATION_DATA;

/* return 0 if no active event at this index
   return -1 if end of list
   return +1 if active event */
typedef int (
    *get_event_info_function) (
    unsigned index,
    BACNET_GET_EVENT_INFORMATION_DATA * getevent_data);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    int getevent_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_OBJECT_ID * lastReceivedObjectIdentifier);

    int getevent_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_OBJECT_ID * object_id);

    int getevent_ack_encode_apdu_init(
        uint8_t * apdu,
        size_t max_apdu,
        uint8_t invoke_id);

    int getevent_ack_encode_apdu_data(
        uint8_t * apdu,
        size_t max_apdu,
        BACNET_GET_EVENT_INFORMATION_DATA * get_event_data);

    int getevent_ack_encode_apdu_end(
        uint8_t * apdu,
        size_t max_apdu,
        bool moreEvents);

    int getevent_ack_decode_service_request(
        uint8_t * apdu,
        int apdu_len,   /* total length of the apdu */
        BACNET_GET_EVENT_INFORMATION_DATA * get_event_data,
        bool * moreEvents);

#ifdef TEST
#include "ctest.h"
    int getevent_decode_apdu(
        uint8_t * apdu,
        unsigned apdu_len,
        uint8_t * invoke_id,
        BACNET_OBJECT_ID * lastReceivedObjectIdentifier);

    int getevent_ack_decode_apdu(
        uint8_t * apdu,
        int apdu_len,   /* total length of the apdu */
        uint8_t * invoke_id,
        BACNET_GET_EVENT_INFORMATION_DATA * get_event_data,
        bool * moreEvents);

    void testGetEventInformationAck(
        Test * pTest);

    void testGetEventInformation(
        Test * pTest);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
