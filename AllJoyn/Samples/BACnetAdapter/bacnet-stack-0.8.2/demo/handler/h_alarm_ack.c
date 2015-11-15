/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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
#include <errno.h>

#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bacerror.h"
#include "bactext.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
#include "alarm_ack.h"
#include "handlers.h"

/** @file h_alarm_ack.c  Handles Alarm Acknowledgment. */

static alarm_ack_function Alarm_Ack[MAX_BACNET_OBJECT_TYPE];

void handler_alarm_ack_set(
    BACNET_OBJECT_TYPE object_type,
    alarm_ack_function pFunction)
{
    if (object_type < MAX_BACNET_OBJECT_TYPE) {
        Alarm_Ack[object_type] = pFunction;
    }
}

/** Handler for an Alarm/Event Acknowledgement.
 * @ingroup ALMACK
 * This handler will be invoked by apdu_handler() if it has been enabled
 * by a call to apdu_set_confirmed_handler().
 * This handler builds a response packet, which is
 * - an Abort if
 *   - the message is segmented
 *   - if decoding fails
 * - Otherwise, sends a simple ACK
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_alarm_ack(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    int ack_result = 0;
    BACNET_ADDRESS my_address;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ALARM_ACK_DATA data;
    BACNET_ERROR_CODE error_code;

    /* encode the NPDU portion of the packet */
    datalink_get_my_address(&my_address);
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], src, &my_address,
        &npdu_data);
    if (service_data->segmented_message) {
        /* we don't support segmentation - send an abort */
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
            true);
#if PRINT_ENABLED
        fprintf(stderr, "Alarm Ack: Segmented message.  Sending Abort!\n");
#endif
        goto AA_ABORT;
    }

    len =
        alarm_ack_decode_service_request(service_request, service_len, &data);
#if PRINT_ENABLED
    if (len <= 0)
        fprintf(stderr, "Alarm Ack: Unable to decode Request!\n");
#endif
    if (len < 0) {
        /* bad decoding - send an abort */
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_OTHER, true);
#if PRINT_ENABLED
        fprintf(stderr, "Alarm Ack: Bad Encoding.  Sending Abort!\n");
#endif
        goto AA_ABORT;
    }
#if PRINT_ENABLED
    fprintf(stderr,
        "Alarm Ack Operation: Received acknowledge for object id (%d, %lu) from %s for process id %lu \n",
        data.eventObjectIdentifier.type,
        (unsigned long) data.eventObjectIdentifier.instance,
        data.ackSource.value, (unsigned long) data.ackProcessIdentifier);
#endif


    if (Alarm_Ack[data.eventObjectIdentifier.type]) {

        ack_result =
            Alarm_Ack[data.eventObjectIdentifier.type] (&data, &error_code);

        switch (ack_result) {
            case 1:
                len =
                    encode_simple_ack(&Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id,
                    SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM);
#if PRINT_ENABLED
                fprintf(stderr, "Alarm Acknowledge: " "Sending Simple Ack!\n");
#endif
                break;

            case -1:
                len =
                    bacerror_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id,
                    SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM, ERROR_CLASS_OBJECT,
                    error_code);
#if PRINT_ENABLED
                fprintf(stderr, "Alarm Acknowledge: error %s!\n",
                    bactext_error_code_name(error_code));
#endif
                break;

            default:
                len =
                    abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                    service_data->invoke_id, ABORT_REASON_OTHER, true);
#if PRINT_ENABLED
                fprintf(stderr, "Alarm Acknowledge: abort other!\n");
#endif
                break;
        }
    } else {
        len =
            bacerror_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, SERVICE_CONFIRMED_ACKNOWLEDGE_ALARM,
            ERROR_CLASS_OBJECT, ERROR_CODE_NO_ALARM_CONFIGURED);
#if PRINT_ENABLED
        fprintf(stderr, "Alarm Acknowledge: error %s!\n",
            bactext_error_code_name(ERROR_CODE_NO_ALARM_CONFIGURED));
#endif
    }


  AA_ABORT:
    pdu_len += len;
    bytes_sent =
        datalink_send_pdu(src, &npdu_data, &Handler_Transmit_Buffer[0],
        pdu_len);
#if PRINT_ENABLED
    if (bytes_sent <= 0)
        fprintf(stderr, "Alarm Acknowledge: " "Failed to send PDU (%s)!\n",
            strerror(errno));
#endif

    return;
}
