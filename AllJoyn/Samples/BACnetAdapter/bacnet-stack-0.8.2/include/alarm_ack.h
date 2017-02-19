/**************************************************************************
*
* Copyright (C) 2009 John Minack
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
#ifndef ALARM_ACK_H_
#define ALARM_ACK_H_

#include "bacenum.h"
#include <stdint.h>
#include <stdbool.h>
#include "bacapp.h"
#include "timestamp.h"

typedef struct {
    uint32_t ackProcessIdentifier;
    BACNET_OBJECT_ID eventObjectIdentifier;
    BACNET_EVENT_STATE eventStateAcked;
    BACNET_TIMESTAMP eventTimeStamp;
    BACNET_CHARACTER_STRING ackSource;
    BACNET_TIMESTAMP ackTimeStamp;
} BACNET_ALARM_ACK_DATA;

/* return +1 if alarm was acknowledged
   return -1 if any error occurred
   return -2 abort */
typedef int (
    *alarm_ack_function) (
    BACNET_ALARM_ACK_DATA * alarmack_data,
    BACNET_ERROR_CODE * error_code);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************************
**
** Creates a Alarm Acknowledge APDU
**
****************************************************/
    int alarm_ack_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_ALARM_ACK_DATA * data);

/***************************************************
**
** Encodes the service data part of Alarm Acknowledge
**
****************************************************/
    int alarm_ack_encode_service_request(
        uint8_t * apdu,
        BACNET_ALARM_ACK_DATA * data);

/***************************************************
**
** Decodes the service data part of Alarm Acknowledge
**
****************************************************/
    int alarm_ack_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_ALARM_ACK_DATA * data);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ALARM_ACK_H_ */
