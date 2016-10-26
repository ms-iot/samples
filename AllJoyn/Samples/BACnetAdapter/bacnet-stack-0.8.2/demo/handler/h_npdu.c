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
#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacint.h"
#include "bacenum.h"
#include "bits.h"
#include "npdu.h"
#include "apdu.h"
#include "handlers.h"
#include "client.h"

#if PRINT_ENABLED
#include <stdio.h>
#endif

/** @file h_npdu.c  Handles messages at the NPDU level of the BACnet stack. */

/** Handler for the NPDU portion of a received packet.
 *  Aside from error-checking, if the NPDU doesn't contain routing info,
 *  this handler doesn't do much besides stepping over the NPDU header
 *  and passing the remaining bytes to the apdu_handler.
 *  @note The routing (except src) and NCPI information, including
 *  npdu_data->data_expecting_reply, are discarded.
 * @see routing_npdu_handler
 *
 * @ingroup MISCHNDLR
 *
 * @param src  [out] Returned with routing source information if the NPDU
 *                   has any and if this points to non-null storage for it.
 *                   If src->net and src->len are 0 on return, there is no
 *                   routing source information.
 *                   This src describes the original source of the message when
 *                   it had to be routed to reach this BACnet Device, and this
 *                   is passed down into the apdu_handler; however, I don't
 *                   think this project's code has any use for the src info
 *                   on return from this handler, since the response has
 *                   already been sent via the apdu_handler.
 *  @param pdu [in]  Buffer containing the NPDU and APDU of the received packet.
 *  @param pdu_len [in] The size of the received message in the pdu[] buffer.
 */
void npdu_handler(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* PDU data */
    uint16_t pdu_len)
{       /* length PDU  */
    int apdu_offset = 0;
    BACNET_ADDRESS dest = { 0 };
    BACNET_NPDU_DATA npdu_data = { 0 };

    /* only handle the version that we know how to handle */
    if (pdu[0] == BACNET_PROTOCOL_VERSION) {
        apdu_offset = npdu_decode(&pdu[0], &dest, src, &npdu_data);
        if (npdu_data.network_layer_message) {
            /*FIXME: network layer message received!  Handle it! */
#if PRINT_ENABLED
            fprintf(stderr, "NPDU: Network Layer Message discarded!\n");
#endif
        } else if ((apdu_offset > 0) && (apdu_offset <= pdu_len)) {
            if ((dest.net == 0) || (dest.net == BACNET_BROADCAST_NETWORK)) {
                /* only handle the version that we know how to handle */
                /* and we are not a router, so ignore messages with
                   routing information cause they are not for us */
                if ((dest.net == BACNET_BROADCAST_NETWORK) &&
                    ((pdu[apdu_offset] & 0xF0) ==
                        PDU_TYPE_CONFIRMED_SERVICE_REQUEST)) {
                    /* hack for 5.4.5.1 - IDLE */
                    /* ConfirmedBroadcastReceived */
                    /* then enter IDLE - ignore the PDU */
                } else {
                    apdu_handler(src, &pdu[apdu_offset],
                        (uint16_t) (pdu_len - apdu_offset));
                }
            } else {
#if PRINT_ENABLED
                printf("NPDU: DNET=%u.  Discarded!\n", (unsigned) dest.net);
#endif
            }
        }
    } else {
#if PRINT_ENABLED
        printf("NPDU: BACnet Protocol Version=%u.  Discarded!\n",
            (unsigned) pdu[0]);
#endif
    }

    return;
}
