/**
* @file
* @author Daniel Blazevic <daniel.blazevic@gmail.com>
* @date 2013
* @brief GetAlarmSummary ACK service handling
*
* @section LICENSE
*
* Copyright (C) 2013 Daniel Blazevic <daniel.blazevic@gmail.com>
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
* The GetAlarmSummary ACK service handler is used by a client BACnet-user to
* obtain a summary of "active alarms." The term "active alarm" refers to
* BACnet standard objects that have an Event_State property whose value is
* not equal to NORMAL and a Notify_Type property whose value is ALARM.
*/
#include <assert.h>
#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bacerror.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
#include "handlers.h"
#include "get_alarm_sum.h"


/** Example function to handle a GetAlarmSummary ACK.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 * decoded from the APDU header of this message.
 */
void get_alarm_summary_ack_handler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
    uint16_t apdu_len = 0;
    uint16_t len = 0;
    BACNET_GET_ALARM_SUMMARY_DATA data;

    while (service_len - len) {
        apdu_len =
            get_alarm_summary_ack_decode_apdu_data(&service_request[len],
            service_len - len, &data);

        len += apdu_len;

        if (apdu_len > 0) {
            /* FIXME: Add code to process data */
        } else {
            break;
        }
    }
}
