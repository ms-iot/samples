/**************************************************************************
*
* Copyright (C) 2008 Steve Karg <skarg@users.sourceforge.net>
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
#include <errno.h>
#include <string.h>
#include "config.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "address.h"
#include "tsm.h"
#include "dcc.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "datalink.h"
#include "cov.h"
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"
#include "client.h"

/** @file s_cov.c  Send a Change of Value (COV) update or a Subscribe COV request. */

int ucov_notify_encode_pdu(
    uint8_t * buffer,
    BACNET_ADDRESS * dest,
    BACNET_NPDU_DATA * npdu_data,
    BACNET_COV_DATA * cov_data)
{
    int len = 0;
    int pdu_len = 0;
    BACNET_ADDRESS my_address;
    datalink_get_my_address(&my_address);

    /* unconfirmed is a broadcast */
    datalink_get_broadcast_address(dest);
    /* encode the NPDU portion of the packet */
    npdu_encode_npdu_data(npdu_data, false, MESSAGE_PRIORITY_NORMAL);
    pdu_len = npdu_encode_pdu(&buffer[0], dest, &my_address, npdu_data);

    /* encode the APDU portion of the packet */
    len = ucov_notify_encode_apdu(&buffer[pdu_len], cov_data);
    pdu_len += len;

    return pdu_len;
}

/** Sends an Unconfirmed COV Notification.
 * @ingroup DSCOV
 *
 * @param buffer [in,out] The buffer to build the message in for sending.
 * @param cov_data [in]  The COV update information to be encoded.
 * @return Size of the message sent (bytes), or a negative value on error.
 */
int Send_UCOV_Notify(
    uint8_t * buffer,
    BACNET_COV_DATA * cov_data)
{
    int pdu_len = 0;
    BACNET_ADDRESS dest;
    int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;

    pdu_len = ucov_notify_encode_pdu(buffer, &dest, &npdu_data, cov_data);
    bytes_sent = datalink_send_pdu(&dest, &npdu_data, &buffer[0], pdu_len);

    return bytes_sent;
}

/** Sends a COV Subscription request.
 * @ingroup DSCOV
 *
 * @param device_id [in] ID of the destination device
 * @param cov_data [in]  The COV subscription information to be encoded.
 * @return invoke id of outgoing message, or 0 if communication is disabled or
 *         no slot is available from the tsm for sending.
 */
uint8_t Send_COV_Subscribe(
    uint32_t device_id,
    BACNET_SUBSCRIBE_COV_DATA * cov_data)
{
    BACNET_ADDRESS dest;
    BACNET_ADDRESS my_address;
    unsigned max_apdu = 0;
    uint8_t invoke_id = 0;
    bool status = false;
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;

    if (!dcc_communication_enabled())
        return 0;
    /* is the device bound? */
    status = address_get_by_device(device_id, &max_apdu, &dest);
    /* is there a tsm available? */
    if (status) {
        invoke_id = tsm_next_free_invokeID();
    }
    if (invoke_id) {
        /* encode the NPDU portion of the packet */
        datalink_get_my_address(&my_address);
        npdu_encode_npdu_data(&npdu_data, true, MESSAGE_PRIORITY_NORMAL);
        pdu_len =
            npdu_encode_pdu(&Handler_Transmit_Buffer[0], &dest, &my_address,
            &npdu_data);
        /* encode the APDU portion of the packet */
        len =
            cov_subscribe_encode_apdu(&Handler_Transmit_Buffer[pdu_len],
            invoke_id, cov_data);
        pdu_len += len;
        /* will it fit in the sender?
           note: if there is a bottleneck router in between
           us and the destination, we won't know unless
           we have a way to check for that and update the
           max_apdu in the address binding table. */
        if ((unsigned) pdu_len < max_apdu) {
            tsm_set_confirmed_unsegmented_transaction(invoke_id, &dest,
                &npdu_data, &Handler_Transmit_Buffer[0], (uint16_t) pdu_len);
            bytes_sent =
                datalink_send_pdu(&dest, &npdu_data,
                &Handler_Transmit_Buffer[0], pdu_len);
#if PRINT_ENABLED
            if (bytes_sent <= 0)
                fprintf(stderr, "Failed to Send SubscribeCOV Request (%s)!\n",
                    strerror(errno));
#endif
        } else {
            tsm_free_invoke_id(invoke_id);
            invoke_id = 0;
#if PRINT_ENABLED
            fprintf(stderr,
                "Failed to Send SubscribeCOV Request "
                "(exceeds destination maximum APDU)!\n");
#endif
        }
    }

    return invoke_id;
}
