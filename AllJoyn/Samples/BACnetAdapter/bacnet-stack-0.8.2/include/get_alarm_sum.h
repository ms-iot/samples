/**************************************************************************
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
*********************************************************************/

#ifndef BACNET_GET_ALARM_SUM_H_
#define BACNET_GET_ALARM_SUM_H_

#include "bacenum.h"
#include <stdint.h>
#include <stdbool.h>
#include "bacapp.h"
#include "timestamp.h"


typedef struct BACnet_Get_Alarm_Summary_Data {
    BACNET_OBJECT_ID objectIdentifier;
    BACNET_EVENT_STATE alarmState;
    BACNET_BIT_STRING acknowledgedTransitions;
} BACNET_GET_ALARM_SUMMARY_DATA;


/* return 0 if no active alarm at this index
   return -1 if end of list
   return +1 if active alarm */
typedef int (
    *get_alarm_summary_function) (
    unsigned index,
    BACNET_GET_ALARM_SUMMARY_DATA * getalarm_data);


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    int get_alarm_summary_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id);

    /* set GetAlarmSummary function */
    void handler_get_alarm_summary_set(
        BACNET_OBJECT_TYPE object_type,
        get_alarm_summary_function pFunction);

    /* encode service */
    int get_alarm_summary_ack_encode_apdu_init(
        uint8_t * apdu,
        uint8_t invoke_id);

    int get_alarm_summary_ack_encode_apdu_data(
        uint8_t * apdu,
        size_t max_apdu,
        BACNET_GET_ALARM_SUMMARY_DATA * get_alarm_data);

    int get_alarm_summary_ack_decode_apdu_data(
        uint8_t * apdu,
        size_t max_apdu,
        BACNET_GET_ALARM_SUMMARY_DATA * get_alarm_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @defgroup ALMEVNT Alarm and Event Management BIBBs
 * @ingroup ALMEVNT
 * 13.1 ConfirmedCOVNotification Service <br>
 * The GetAlarmSummary service is used by a client BACnet-user to obtain
 * a summary of "active alarms." The term "active alarm" refers to
 * BACnet standard objects that have an Event_State property whose value
 * is not equal to NORMAL and a Notify_Type property whose value is ALARM.
 * The GetEnrollmentSummary service provides a more sophisticated approach
 * with various kinds of filters.
 */
#endif /* BACNET_GET_ALARM_SUM_H_ */
