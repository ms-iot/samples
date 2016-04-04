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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bacerror.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
#include "handlers.h"

/** @file h_alarm_sum.c  Handles Get Alarm Summary request. */

static get_alarm_summary_function Get_Alarm_Summary[MAX_BACNET_OBJECT_TYPE];

void handler_get_alarm_summary_set(
    BACNET_OBJECT_TYPE object_type,
    get_alarm_summary_function pFunction)
{
    if (object_type < MAX_BACNET_OBJECT_TYPE) {
        Get_Alarm_Summary[object_type] = pFunction;
    }
}

void handler_get_alarm_summary(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    int len = 0;
    int pdu_len = 0;
    int apdu_len = 0;
    int bytes_sent = 0;
    int alarm_value = 0;
    unsigned i = 0;
    unsigned j = 0;
    bool error = false;
    BACNET_ADDRESS my_address;
    BACNET_NPDU_DATA npdu_data;
    BACNET_GET_ALARM_SUMMARY_DATA getalarm_data;



    /* encode the NPDU portion of the packet */
    datalink_get_my_address(&my_address);
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], src, &my_address,
        &npdu_data);
    if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        apdu_len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
            true);
#if PRINT_ENABLED
        fprintf(stderr,
            "GetAlarmSummary: Segmented message. Sending Abort!\n");
#endif
        goto GET_ALARM_SUMMARY_ABORT;
    }

    /* init header */
    apdu_len =
        get_alarm_summary_ack_encode_apdu_init(&Handler_Transmit_Buffer
        [pdu_len], service_data->invoke_id);


    for (i = 0; i < MAX_BACNET_OBJECT_TYPE; i++) {
        if (Get_Alarm_Summary[i]) {
            for (j = 0; j < 0xffff; j++) {
                alarm_value = Get_Alarm_Summary[i] (j, &getalarm_data);
                if (alarm_value > 0) {
                    len =
                        get_alarm_summary_ack_encode_apdu_data
                        (&Handler_Transmit_Buffer[pdu_len + apdu_len],
                        service_data->max_resp - apdu_len, &getalarm_data);
                    if (len <= 0) {
                        error = true;
                        goto GET_ALARM_SUMMARY_ERROR;
                    } else
                        apdu_len += len;
                } else if (alarm_value < 0) {
                    break;
                }
            }
        }
    }


#if PRINT_ENABLED
    fprintf(stderr, "GetAlarmSummary: Sending response!\n");
#endif

  GET_ALARM_SUMMARY_ERROR:
    if (error) {
        if (len == BACNET_STATUS_ABORT) {
            /* BACnet APDU too small to fit data, so proper response is Abort */
            apdu_len =
                abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                service_data->invoke_id,
                ABORT_REASON_SEGMENTATION_NOT_SUPPORTED, true);
#if PRINT_ENABLED
            fprintf(stderr,
                "GetAlarmSummary: Reply too big to fit into APDU!\n");
#endif
        } else {
            apdu_len =
                bacerror_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                service_data->invoke_id, SERVICE_CONFIRMED_GET_ALARM_SUMMARY,
                ERROR_CLASS_PROPERTY, ERROR_CODE_OTHER);
#if PRINT_ENABLED
            fprintf(stderr, "GetAlarmSummary: Sending Error!\n");
#endif
        }
    }


  GET_ALARM_SUMMARY_ABORT:
    pdu_len += apdu_len;
    bytes_sent =
        datalink_send_pdu(src, &npdu_data, &Handler_Transmit_Buffer[0],
        pdu_len);
#if PRINT_ENABLED
    if (bytes_sent <= 0) {
        /*fprintf(stderr, "Failed to send PDU (%s)!\n", strerror(errno)); */
    }
#else
    bytes_sent = bytes_sent;
#endif

    return;
}
