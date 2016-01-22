/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
#include "reject.h"
#include "rd.h"
/* custom handling in device object */
#include "device.h"
#include "handlers.h"

/** @file h_rd.c  Handles Reinitialize Device requests. */

/** Handler for a Reinitialize Device (RD) request.
 * @ingroup DMRD
 * This handler will be invoked by apdu_handler() if it has been enabled
 * by a call to apdu_set_confirmed_handler().
 * This handler builds a response packet, which is
 * - an Abort if
 *   - the message is segmented
 *   - if decoding fails
 * - an Error if
 *   - the RD password is incorrect
 *   - the Reinitialize Device operation fails
 * - a Reject if the request state is invalid
 * - else tries to send a simple ACK for the RD on success.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_reinitialize_device(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    BACNET_REINITIALIZE_DEVICE_DATA rd_data;
    int len = 0;
    int pdu_len = 0;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;

    /* encode the NPDU portion of the packet */
    datalink_get_my_address(&my_address);
    npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], src, &my_address,
        &npdu_data);
#if PRINT_ENABLED
    fprintf(stderr, "ReinitializeDevice!\n");
#endif
    if (service_data->segmented_message) {
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_SEGMENTATION_NOT_SUPPORTED,
            true);
#if PRINT_ENABLED
        fprintf(stderr,
            "ReinitializeDevice: Sending Abort - segmented message.\n");
#endif
        goto RD_ABORT;
    }
    /* decode the service request only */
    len =
        rd_decode_service_request(service_request, service_len, &rd_data.state,
        &rd_data.password);
#if PRINT_ENABLED
    if (len > 0) {
        fprintf(stderr, "ReinitializeDevice: state=%u password=%s\n",
            (unsigned) rd_data.state,
            characterstring_value(&rd_data.password));
    } else {
        fprintf(stderr, "ReinitializeDevice: Unable to decode request!\n");
    }
#endif
    /* bad decoding or something we didn't understand - send an abort */
    if (len < 0) {
        len =
            abort_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, ABORT_REASON_OTHER, true);
#if PRINT_ENABLED
        fprintf(stderr,
            "ReinitializeDevice: Sending Abort - could not decode.\n");
#endif
        goto RD_ABORT;
    }
    /* check the data from the request */
    if (rd_data.state >= MAX_BACNET_REINITIALIZED_STATE) {
        len =
            reject_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            service_data->invoke_id, REJECT_REASON_UNDEFINED_ENUMERATION);
#if PRINT_ENABLED
        fprintf(stderr,
            "ReinitializeDevice: Sending Reject - undefined enumeration\n");
#endif
    } else {
#if BAC_ROUTING
        /* Check to see if the current Device supports this service. */
        len =
            Routed_Device_Service_Approval
            (SERVICE_CONFIRMED_REINITIALIZE_DEVICE, (int) rd_data.state,
            &Handler_Transmit_Buffer[pdu_len], service_data->invoke_id);
        if (len > 0)
            goto RD_ABORT;
#endif

        if (Device_Reinitialize(&rd_data)) {
            len =
                encode_simple_ack(&Handler_Transmit_Buffer[pdu_len],
                service_data->invoke_id,
                SERVICE_CONFIRMED_REINITIALIZE_DEVICE);
#if PRINT_ENABLED
            fprintf(stderr, "ReinitializeDevice: Sending Simple Ack!\n");
#endif
        } else {
            len =
                bacerror_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
                service_data->invoke_id, SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
                rd_data.error_class, rd_data.error_code);
#if PRINT_ENABLED
            fprintf(stderr, "ReinitializeDevice: Sending Error.\n");
#endif
        }
    }
  RD_ABORT:
    pdu_len += len;
    len =
        datalink_send_pdu(src, &npdu_data, &Handler_Transmit_Buffer[0],
        pdu_len);
#if PRINT_ENABLED
    if (len <= 0) {
        fprintf(stderr, "ReinitializeDevice: Failed to send PDU (%s)!\n",
            strerror(errno));
    }
#endif

    return;
}
