/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "address.h"
#include "tsm.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "datalink.h"
#include "bactext.h"
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"
#include "client.h"
#include "debug.h"

/** @file s_router.c  Methods to send various BACnet Router Network Layer Messages. */

/** Initialize an npdu_data structure with given parameters and good defaults,
 * and add the Network Layer Message fields.
 * The name is a misnomer, as it doesn't do any actual encoding here.
 * @see npdu_encode_npdu_data for a simpler version to use when sending an
 *           APDU instead of a Network Layer Message.
 *
 * @param npdu_data [out] Returns a filled-out structure with information
 * 					 provided by the other arguments and good defaults.
 * @param network_message_type [in] The type of Network Layer Message.
 * @param data_expecting_reply [in] True if message should have a reply.
 * @param priority [in] One of the 4 priorities defined in section 6.2.2,
 *                      like B'11' = Life Safety message
 */
static void npdu_encode_npdu_network(
    BACNET_NPDU_DATA * npdu_data,
    BACNET_NETWORK_MESSAGE_TYPE network_message_type,
    bool data_expecting_reply,
    BACNET_MESSAGE_PRIORITY priority)
{
    if (npdu_data) {
        npdu_data->data_expecting_reply = data_expecting_reply;
        npdu_data->protocol_version = BACNET_PROTOCOL_VERSION;
        npdu_data->network_layer_message = true;        /* false if APDU */
        npdu_data->network_message_type = network_message_type; /* optional */
        npdu_data->vendor_id = 0;       /* optional, if net message type is > 0x80 */
        npdu_data->priority = priority;
        npdu_data->hop_count = HOP_COUNT_DEFAULT;
    }
}


/** Function to encode and send any supported Network Layer Message.
 * The payload for the message is encoded from information in the iArgs[] array.
 * The contents of iArgs are are, per message type:
 * - NETWORK_MESSAGE_WHO_IS_ROUTER_TO_NETWORK: Single int for DNET requested
 * - NETWORK_MESSAGE_I_AM_ROUTER_TO_NETWORK: Array of DNET(s) to send,
 * 		terminated with -1
 * - NETWORK_MESSAGE_REJECT_MESSAGE_TO_NETWORK: array of 2 ints,
 *      first is reason, second is DNET of interest
 * - NETWORK_MESSAGE_ROUTER_BUSY_TO_NETWORK: same as I-Am-Router msg
 * - NETWORK_MESSAGE_ROUTER_AVAILABLE_TO_NETWORK: same as I-Am-Router msg
 * - NETWORK_MESSAGE_INIT_RT_TABLE and NETWORK_MESSAGE_INIT_RT_TABLE_ACK:
 *      Array of DNET(s) to process as "Ports", terminated with -1.  Each DNET
 *      will be expanded to a BACNET_ROUTER_PORT (with simple defaults for
 *      most fields) and encoded.
 *
 * @param network_message_type [in] The type of message to be sent.
 * @param dst [in/out] If not NULL, contains the destination for the message.
 * @param iArgs [in] An optional array of values whose meaning depends on
 *                   the type of message.
 * @return Number of bytes sent, or <=0 if no message was sent.
 */
int Send_Network_Layer_Message(
    BACNET_NETWORK_MESSAGE_TYPE network_message_type,
    BACNET_ADDRESS * dst,
    int *iArgs)
{
    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    int *pVal = iArgs;  /* Start with first value */
    bool data_expecting_reply = false;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS bcastDest;

    if (iArgs == NULL)
        return 0;       /* Can't do anything here */

    /* If dst was NULL, get our (local net) broadcast MAC address. */
    if (dst == NULL) {
        datalink_get_broadcast_address(&bcastDest);
        dst = &bcastDest;
    }

    if (network_message_type == NETWORK_MESSAGE_INIT_RT_TABLE)
        data_expecting_reply = true;    /* DER in this one case */
    npdu_encode_npdu_network(&npdu_data, network_message_type,
        data_expecting_reply, MESSAGE_PRIORITY_NORMAL);

    /* We don't need src information, since a message can't originate from
     * our downstream BACnet network.
     */
    pdu_len =
        npdu_encode_pdu(&Handler_Transmit_Buffer[0], dst, NULL, &npdu_data);

    /* Now encode the optional payload bytes, per message type */
    switch (network_message_type) {
        case NETWORK_MESSAGE_WHO_IS_ROUTER_TO_NETWORK:
            if (*pVal >= 0) {
                len =
                    encode_unsigned16(&Handler_Transmit_Buffer[pdu_len],
                    (uint16_t) * pVal);
                pdu_len += len;
            }
            /* else, don't encode a DNET */
            break;

        case NETWORK_MESSAGE_I_AM_ROUTER_TO_NETWORK:
        case NETWORK_MESSAGE_ROUTER_BUSY_TO_NETWORK:
        case NETWORK_MESSAGE_ROUTER_AVAILABLE_TO_NETWORK:
            while (*pVal >= 0) {
                len =
                    encode_unsigned16(&Handler_Transmit_Buffer[pdu_len],
                    (uint16_t) * pVal);
                pdu_len += len;
                pVal++;
            }
            break;

        case NETWORK_MESSAGE_REJECT_MESSAGE_TO_NETWORK:
            /* Encode the Reason byte, then the DNET */
            Handler_Transmit_Buffer[pdu_len++] = (uint8_t) * pVal;
            pVal++;
            len =
                encode_unsigned16(&Handler_Transmit_Buffer[pdu_len],
                (uint16_t) * pVal);
            pdu_len += len;
            break;

        case NETWORK_MESSAGE_INIT_RT_TABLE:
        case NETWORK_MESSAGE_INIT_RT_TABLE_ACK:
            /* First, count the number of Ports we will encode */
            len = 0;    /* Re-purpose len as our counter here */
            while (*pVal >= 0) {
                len++;
                pVal++;
            }
            Handler_Transmit_Buffer[pdu_len++] = (uint8_t) len;

            if (len > 0) {
                uint8_t portID = 1;
                pVal = iArgs;   /* Reset to beginning */
                /* Now encode each (virtual) BACNET_ROUTER_PORT.
                 * We will simply use a positive index for PortID,
                 * and have no PortInfo.
                 */
                while (*pVal >= 0) {
                    len =
                        encode_unsigned16(&Handler_Transmit_Buffer[pdu_len],
                        (uint16_t) * pVal);
                    pdu_len += len;
                    Handler_Transmit_Buffer[pdu_len++] = portID++;
                    Handler_Transmit_Buffer[pdu_len++] = 0;
                    debug_printf("  Sending Routing Table entry for %u \n",
                        *pVal);
                    pVal++;
                }
            }
            break;

        default:
            debug_printf("Not sent: %s message unsupported \n",
                bactext_network_layer_msg_name(network_message_type));
            return 0;
            break;      /* Will never reach this line */
    }

    if (dst != NULL)
        debug_printf("Sending %s message to BACnet network %u \n",
            bactext_network_layer_msg_name(network_message_type), dst->net);
    else
        debug_printf("Sending %s message to local BACnet network \n",
            bactext_network_layer_msg_name(network_message_type));

    /* Now send the message */
    bytes_sent =
        datalink_send_pdu(dst, &npdu_data, &Handler_Transmit_Buffer[0],
        pdu_len);
#if PRINT_ENABLED
    if (bytes_sent <= 0) {
        int wasErrno = errno;   /* preserve the errno */
        debug_printf("Failed to send %s message (%s)!\n",
            bactext_network_layer_msg_name(network_message_type),
            strerror(wasErrno));
    }
#endif
    return bytes_sent;
}


/** Finds a specific router, or all reachable BACnet networks.
 * The response(s) will come in I-am-router-to-network message(s).
 * @ingroup NMRC
 *
 * @param dst [in] If NULL, request will be broadcast to the local BACnet
 *                 network.  Optionally may designate a particular router
 *                 destination to respond.
 * @param dnet [in] Which BACnet network to request for; if -1, no DNET
 *                 will be sent and the receiving router(s) will send
 *                 their full list of reachable BACnet networks.
 */
void Send_Who_Is_Router_To_Network(
    BACNET_ADDRESS * dst,
    int dnet)
{
    Send_Network_Layer_Message(NETWORK_MESSAGE_WHO_IS_ROUTER_TO_NETWORK, dst,
        &dnet);
}

/** Broadcast an I-am-router-to-network message, giving the list of networks
 * we can reach.
 * The message will be sent to our normal DataLink Layer interface,
 * not the routed backend.
 * @ingroup NMRC
 *
 * @param DNET_list [in] List of BACnet network numbers for which I am a router,
 *                       terminated with -1
 */
void Send_I_Am_Router_To_Network(
    const int DNET_list[])
{
    /* Use a NULL dst here since we want a broadcast MAC address. */
    Send_Network_Layer_Message(NETWORK_MESSAGE_I_AM_ROUTER_TO_NETWORK, NULL,
        (int *) DNET_list);
}

/** Finds a specific router, or all reachable BACnet networks.
 * The response(s) will come in I-am-router-to-network message(s).
 * @ingroup NMRC
 *
 * @param dst [in] If NULL, request will be broadcast to the local BACnet
 *                 network.  Otherwise, designates a particular router
 *                 destination.
 * @param reject_reason [in] One of the BACNET_NETWORK_REJECT_REASONS codes.
 * @param dnet [in] Which BACnet network orginated the message.
 */
void Send_Reject_Message_To_Network(
    BACNET_ADDRESS * dst,
    uint8_t reject_reason,
    int dnet)
{
    int iArgs[2];
    iArgs[0] = reject_reason;
    iArgs[1] = dnet;
    Send_Network_Layer_Message(NETWORK_MESSAGE_REJECT_MESSAGE_TO_NETWORK, dst,
        iArgs);
    debug_printf("  Reject Reason=%d, DNET=%u\n", reject_reason, dnet);
}


/** Send an Initialize Routing Table message, built from an optional DNET[]
 * array.
 * There are two cases here:
 * 1) We are requesting a destination router's Routing Table.
 *    In that case, DNET[] should just have one entry of -1 (no routing table
 *    is sent).
 * 2) We are sending out our Routing Table for some reason (normally bcast it).
 * @ingroup NMRC
 *
 * @param dst [in] If NULL, msg will be broadcast to the local BACnet network.
 *                 Optionally may designate a particular router destination,
 *                 especially when requesting a Routing Table.
 * @param DNET_list [in] List of BACnet network numbers for which I am a router,
 *                       terminated with -1.  Will be just -1 when we are
 *                       requesting a routing table.
 */
void Send_Initialize_Routing_Table(
    BACNET_ADDRESS * dst,
    const int DNET_list[])
{
    /* Use a NULL dst here since we want a broadcast MAC address. */
    Send_Network_Layer_Message(NETWORK_MESSAGE_INIT_RT_TABLE, dst,
        (int *) DNET_list);
}


/** Sends our Routing Table, built from our DNET[] array, as an ACK.
 * There are two cases here:
 * 1) We are responding to a NETWORK_MESSAGE_INIT_RT_TABLE requesting our table.
 *    We will normally broadcast that response.
 * 2) We are ACKing the receipt of a NETWORK_MESSAGE_INIT_RT_TABLE containing a
 *    routing table, and then we will want to respond to that dst router.
 *    In that case, DNET[] should just have one entry of -1 (no routing table
 *    is sent).
 * @ingroup NMRC
 *
 * @param dst [in] If NULL, Ack will be broadcast to the local BACnet network.
 *                 Optionally may designate a particular router destination,
 *                 especially when ACKing receipt of this message type.
 * @param DNET_list [in] List of BACnet network numbers for which I am a router,
 *                       terminated with -1.  May be just -1 when no table
 *                       should be sent.
 */
void Send_Initialize_Routing_Table_Ack(
    BACNET_ADDRESS * dst,
    const int DNET_list[])
{
    Send_Network_Layer_Message(NETWORK_MESSAGE_INIT_RT_TABLE_ACK, dst,
        (int *) DNET_list);
}
