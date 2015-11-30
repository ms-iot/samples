/**************************************************************************
*
* Copyright (C) 2010 Steve Karg <skarg@users.sourceforge.net>
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
/* Acknowledging the contribution of code and ideas used here that
 * came from Paul Chapman's vmac demo project. */

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
#include "device.h"
#include "client.h"
#include "bactext.h"
#include "debug.h"

#if PRINT_ENABLED
#include <stdio.h>
#endif
#if defined(BACDL_BIP)
#include "bvlc.h"
#endif


/** @file h_routed_npdu.c  Handles messages at the NPDU level of the BACnet stack,
 * including routing and network control messages. */


/** Handler to manage the Network Layer Control Messages received in a packet.
 *  This handler is called if the NCPI bit 7 indicates that this packet is a
 *  network layer message and there is no further DNET to pass it to.
 *  The NCPI has already been decoded into the npdu_data structure.
 * @ingroup MISCHNDLR
 *
 * @param src  [in] The routing source information, if any.
 *                   If src->net and src->len are 0, there is no
 *                   routing source information.
 * @param DNET_list [in] List of our reachable downstream BACnet Network numbers.
 * 					 Normally just one valid entry; terminated with a -1 value.
 * @param npdu_data [in] Contains a filled-out structure with information
 * 					 decoded from the NCPI and other NPDU bytes.
 *  @param npdu [in]  Buffer containing the rest of the NPDU, following the
 *  				 bytes that have already been decoded.
 *  @param npdu_len [in] The length of the remaining NPDU message in npdu[].
 */
static void network_control_handler(
    BACNET_ADDRESS * src,
    int *DNET_list,
    BACNET_NPDU_DATA * npdu_data,
    uint8_t * npdu,
    uint16_t npdu_len)
{
    uint16_t npdu_offset = 0;
    uint16_t dnet = 0;
    uint16_t len = 0;

    switch (npdu_data->network_message_type) {
        case NETWORK_MESSAGE_WHO_IS_ROUTER_TO_NETWORK:
            /* Send I-am-router-to-network with our one-network list if
             * our specific network is requested, or no specific
             * network is requested. Silently drop other DNET requests.
             */
            if (npdu_len >= 2) {
                uint16_t network;
                len += decode_unsigned16(&npdu[len], &network);
                if (network == DNET_list[0]) {
                    Send_I_Am_Router_To_Network(DNET_list);
                }
            } else {
                Send_I_Am_Router_To_Network(DNET_list);
            }
            break;
        case NETWORK_MESSAGE_I_AM_ROUTER_TO_NETWORK:
            /* Per the standard, we are supposed to process this message and
             * add its DNETs to our routing table.
             * However, since we only have one upstream port that these
             * messages can come from and replies go to, it doesn't seem
             * to provide us any value to do this; when we need to send to
             * some remote device, we will start by pushing it out the
             * upstream port and let the attached router(s) take it from there.
             * Consequently, we'll do nothing interesting here.
             * -- Unless we act upon NETWORK_MESSAGE_ROUTER_BUSY_TO_NETWORK
             * later for congestion control - then it could matter.
             */
            debug_printf("%s for Networks: ",
                bactext_network_layer_msg_name
                (NETWORK_MESSAGE_I_AM_ROUTER_TO_NETWORK));
            while (npdu_len >= 2) {
                len = decode_unsigned16(&npdu[npdu_offset], &dnet);
                debug_printf("%hu", dnet);
                npdu_len -= len;
                npdu_offset += len;
                if (npdu_len >= 2) {
                    debug_printf(", ");
                }
            }
            debug_printf("\n");
            break;
        case NETWORK_MESSAGE_I_COULD_BE_ROUTER_TO_NETWORK:
            /* Do nothing, same as previous case. */
            break;
        case NETWORK_MESSAGE_REJECT_MESSAGE_TO_NETWORK:
            if (npdu_len >= 3) {
                decode_unsigned16(&npdu[1], &dnet);
                debug_printf("Received %s for Network: ",
                    bactext_network_layer_msg_name
                    (NETWORK_MESSAGE_I_COULD_BE_ROUTER_TO_NETWORK));
                debug_printf("%hu,  Reason code: %d \n", dnet, npdu[0]);
            }
            break;
        case NETWORK_MESSAGE_ROUTER_BUSY_TO_NETWORK:
        case NETWORK_MESSAGE_ROUTER_AVAILABLE_TO_NETWORK:
            /* Do nothing - don't support upstream traffic congestion control */
            break;
        case NETWORK_MESSAGE_INIT_RT_TABLE:
            /* If sent with Number of Ports == 0, we respond with
             * NETWORK_MESSAGE_INIT_RT_TABLE_ACK and a list of all our
             * reachable networks.
             */
            if (npdu_len > 0) {
                /* If Number of Ports is 0, broadcast our "full" table */
                if (npdu[0] == 0)
                    Send_Initialize_Routing_Table_Ack(NULL, DNET_list);
                else {
                    /* If they sent us a list, just politely ACK it
                     * with no routing list of our own.  But we don't DO
                     * anything with the info, either.
                     */
                    int listTerminator = -1;
                    Send_Initialize_Routing_Table_Ack(src, &listTerminator);
                }
                break;
            }
            /* Else, fall through to do nothing. */
        case NETWORK_MESSAGE_INIT_RT_TABLE_ACK:
            /* Do nothing with the routing table info, since don't support
             * upstream traffic congestion control */
            break;
        case NETWORK_MESSAGE_ESTABLISH_CONNECTION_TO_NETWORK:
        case NETWORK_MESSAGE_DISCONNECT_CONNECTION_TO_NETWORK:
            /* Do nothing - don't support PTP half-router control */
            break;
        default:
            /* An unrecognized message is bad; send an error response. */
            Send_Reject_Message_To_Network(src,
                NETWORK_REJECT_UNKNOWN_MESSAGE_TYPE, DNET_list[0]);
            /* Sending our DNET doesn't make a lot of sense, does it? */
            break;
    }
}

/** An APDU pre-handler that makes sure that the subsequent APDU handler call
 * operates on the right Device Object(s), as addressed by the destination
 * (routing) information.
 *
 * @note Even when the destination is "routed" to our virtual BACnet network,
 * the src information does not need to change to reflect that (as it normally
 * would for a routed message) because the reply will be sent from the level
 * of the gateway Device.
 *
 * @param src [in] The BACNET_ADDRESS of the message's source.
 * @param dest [in] The BACNET_ADDRESS of the message's destination.
 * @param DNET_list [in] List of our reachable downstream BACnet Network numbers.
 * 					 Normally just one valid entry; terminated with a -1 value.
 * @param apdu [in] The apdu portion of the request, to be processed.
 * @param apdu_len [in] The total (remaining) length of the apdu.
 */
static void routed_apdu_handler(
    BACNET_ADDRESS * src,
    BACNET_ADDRESS * dest,
    int *DNET_list,
    uint8_t * apdu,
    uint16_t apdu_len)
{
    int cursor = 0;     /* Starting hint */
    bool bGotOne = false;

    if (!Routed_Device_Is_Valid_Network(dest->net, DNET_list)) {
        /* We don't know how to reach this one.
         * The protocol doesn't specifically state this, but if this message
         * was broadcast to us, we should assume "someone else" is handling
         * it and not get involved (ie, send a Reject-Message).
         * Since we can't reach other routers that src couldn't already reach,
         * we don't try the standard path of asking Who-Is-Router-to-Network. */
#if defined(BACDL_BIP)
        /* If wasn't unicast to us, must have been one of the bcast types.
         * Drop it. */
        if (bvlc_get_function_code() != BVLC_ORIGINAL_UNICAST_NPDU)
            return;
#endif
        /* Upper level handlers knew that this was sent as a bcast,
         * but our only other way to guess at that here is if the dest->adr
         * is absent, then we know this is some sort of bcast.
         */
        if (dest->len > 0) {
            Send_Reject_Message_To_Network(src, NETWORK_REJECT_NO_ROUTE,
                dest->net);
        }       /* else, silently drop it */
        return;
    }

    while (Routed_Device_GetNext(dest, DNET_list, &cursor)) {
        apdu_handler(src, apdu, apdu_len);
        bGotOne = true;
        if (cursor < 0) /* If no more matches, */
            break;      /* We don't need to keep looking */
    }
    if (!bGotOne) {
        /* Just silently drop this packet. */
    }
}

/** Handler for the NPDU portion of a received packet, which may have routing.
 *  This is a fuller handler than the regular npdu_handler, as it manages
 *  - Decoding of the NCPI byte
 *  - Further processing by network_control_handler() if this is a  network
 *    layer message.
 *  - Further processing by routed_apdu_handler() if it contains an APDU
 *    - Normally (no routing) by apdu_handler()
 *    - With Routing (a further destination was indicated) by the decoded
 *      destination.
 *  - Errors in decoding.
 * @note The npdu_data->data_expecting_reply status is discarded.
 * @see npdu_handler
 * @ingroup NMRC
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
 * @param DNET_list [in] List of our reachable downstream BACnet Network numbers.
 * 					 Normally just one valid entry; terminated with a -1 value.
 *  @param pdu [in]  Buffer containing the NPDU and APDU of the received packet.
 *  @param pdu_len [in] The size of the received message in the pdu[] buffer.
 */
void routing_npdu_handler(
    BACNET_ADDRESS * src,
    int *DNET_list,
    uint8_t * pdu,
    uint16_t pdu_len)
{
    int apdu_offset = 0;
    BACNET_ADDRESS dest = { 0 };
    BACNET_NPDU_DATA npdu_data = { 0 };

    /* only handle the version that we know how to handle */
    if (pdu[0] == BACNET_PROTOCOL_VERSION) {
        apdu_offset = npdu_decode(&pdu[0], &dest, src, &npdu_data);
        if (apdu_offset <= 0) {
            debug_printf("NPDU: Decoding failed; Discarded!\n");
        } else if (npdu_data.network_layer_message) {
            if ((dest.net == 0) || (dest.net == BACNET_BROADCAST_NETWORK)) {
                network_control_handler(src, DNET_list, &npdu_data,
                    &pdu[apdu_offset], (uint16_t) (pdu_len - apdu_offset));
            } else {
                /* The DNET is set, but we don't support downstream routers,
                 * so we just silently drop this network layer message,
                 * since only routers can handle it (even if for our DNET) */
            }
        } else if (apdu_offset <= pdu_len) {
            if ((dest.net == 0) || (npdu_data.hop_count > 1))
                routed_apdu_handler(src, &dest, DNET_list, &pdu[apdu_offset],
                    (uint16_t) (pdu_len - apdu_offset));
            /* Else, hop_count bottomed out and we discard this one. */
        }
    } else {
        /* Should we send NETWORK_MESSAGE_REJECT_MESSAGE_TO_NETWORK? */
        debug_printf
            ("NPDU: Unsupported BACnet Protocol Version=%u.  Discarded!\n",
            (unsigned) pdu[0]);
    }

    return;
}
