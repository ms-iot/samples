/**
* @file
* @author Andriy Sukhynyuk, Vasyl Tkhir, Andriy Ivasiv
* @date 2012
* @brief Network layer for BACnet routing
*
* @section LICENSE
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
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network_layer.h"
#include "bacint.h"

uint16_t process_network_message(
    BACMSG * msg,
    MSG_DATA * data,
    uint8_t ** buff)
{

    BACNET_NPDU_DATA npdu_data;
    ROUTER_PORT *srcport;
    ROUTER_PORT *destport;
    uint16_t net;
    uint8_t error_code;
    int16_t buff_len = 0;
    int apdu_offset;
    int apdu_len;

    memmove(data, msg->data, sizeof(MSG_DATA));

    apdu_offset = npdu_decode(data->pdu, &data->dest, NULL, &npdu_data);
    apdu_len = data->pdu_len - apdu_offset;

    srcport = find_snet(msg->origin);
    data->src.net = srcport->route_info.net;

    switch (npdu_data.network_message_type) {

        case NETWORK_MESSAGE_WHO_IS_ROUTER_TO_NETWORK:
            PRINT(INFO, "Recieved Who-Is-Router-To-Network message\n");
            if (apdu_len) {
                /* if NET specified */
                decode_unsigned16(&data->pdu[apdu_offset], &net);
                if (srcport->route_info.net == net) {
                    PRINT(INFO, "Message discarded: NET directly connected\n");
                    return -2;
                }

                destport = find_dnet(net, NULL);        /* see if NET can be reached */
                if (destport) {
                    /* if TRUE send reply */
                    PRINT(INFO, "Sending I-Am-Router-To-Network message\n");
                    buff_len =
                        create_network_message
                        (NETWORK_MESSAGE_I_AM_ROUTER_TO_NETWORK, data, buff,
                        &net);
                } else {
                    data->dest.net = net;       /* NET to look for */
                    return -1;  /* else initiate NET search procedure */
                }
            } else {
                /* if NET is omitted (message sent with -1) */
                PRINT(INFO, "Sending I-Am-Router-To-Network message\n");
                buff_len =
                    create_network_message
                    (NETWORK_MESSAGE_I_AM_ROUTER_TO_NETWORK, data, buff, NULL);
            }

            break;

        case NETWORK_MESSAGE_I_AM_ROUTER_TO_NETWORK:
            {
                PRINT(INFO, "Recieved I-Am-Router-To-Network message\n");
                int net_count = apdu_len / 2;
                int i;
                for (i = 0; i < net_count; i++) {
                    decode_unsigned16(&data->pdu[apdu_offset + 2 * i], &net);   /* decode received NET values */
                    add_dnet(&srcport->route_info, net, data->src);     /* and update routing table */
                }
                break;
            }
        case NETWORK_MESSAGE_REJECT_MESSAGE_TO_NETWORK:
            {
                /* first octet of the message contains rejection reason */
                /* next two octets contain NET (can be decoded for additional info on error) */
                error_code = data->pdu[apdu_offset];
                switch (error_code) {
                    case 0:
                        PRINT(ERROR, "Error!\n");
                        break;
                    case 1:
                        PRINT(ERROR, "Error: Network unreachable\n");
                        break;
                    case 2:
                        PRINT(ERROR, "Error: Network is busy\n");
                        break;
                    case 3:
                        PRINT(ERROR, "Error: Unknown network message type\n");
                        break;
                    case 4:
                        PRINT(ERROR, "Error: Message too long\n");
                        break;
                }
                break;
            }
        case NETWORK_MESSAGE_INIT_RT_TABLE:
            PRINT(INFO, "Recieved Initialize-Routing-Table message\n");
            if (data->pdu[apdu_offset] > 0) {
                int net_count = data->pdu[apdu_offset];
                while (net_count--) {
                    int i = 1;
                    decode_unsigned16(&data->pdu[apdu_offset + i], &net);       /* decode received NET values */
                    add_dnet(&srcport->route_info, net, data->src);     /* and update routing table */
                    if (data->pdu[apdu_offset + i + 3] > 0)     /* find next NET value */
                        i = data->pdu[apdu_offset + i + 3] + 4;
                    else
                        i = i + 4;
                }
                buff_len =
                    create_network_message(NETWORK_MESSAGE_INIT_RT_TABLE_ACK,
                    data, buff, NULL);
            } else
                buff_len =
                    create_network_message(NETWORK_MESSAGE_INIT_RT_TABLE_ACK,
                    data, buff, &buff);
            break;

        case NETWORK_MESSAGE_INIT_RT_TABLE_ACK:
            PRINT(INFO, "Recieved Initialize-Routing-Table-Ack message\n");
            if (data->pdu[apdu_offset] > 0) {
                int net_count = data->pdu[apdu_offset];
                while (net_count--) {
                    int i = 1;
                    decode_unsigned16(&data->pdu[apdu_offset + i], &net);       /* decode received NET values */
                    add_dnet(&srcport->route_info, net, data->src);     /* and update routing table */
                    if (data->pdu[apdu_offset + i + 3] > 0)     /* find next NET value */
                        i = data->pdu[apdu_offset + i + 3] + 4;
                    else
                        i = i + 4;
                }
            }
            break;

        case NETWORK_MESSAGE_INVALID:
        case NETWORK_MESSAGE_I_COULD_BE_ROUTER_TO_NETWORK:
        case NETWORK_MESSAGE_ROUTER_BUSY_TO_NETWORK:
        case NETWORK_MESSAGE_ROUTER_AVAILABLE_TO_NETWORK:
        case NETWORK_MESSAGE_ESTABLISH_CONNECTION_TO_NETWORK:
        case NETWORK_MESSAGE_DISCONNECT_CONNECTION_TO_NETWORK:
            /* hell if I know what to do with these messages */
            break;

        default:
            PRINT(ERROR, "Error: Message unsupported\n");
            break;
    }

    return buff_len;
}

uint16_t create_network_message(
    BACNET_NETWORK_MESSAGE_TYPE network_message_type,
    MSG_DATA * data,
    uint8_t ** buff,
    void *val)
{

    int16_t buff_len;
    bool data_expecting_reply = false;
    BACNET_NPDU_DATA npdu_data;

    if (network_message_type == NETWORK_MESSAGE_INIT_RT_TABLE)
        data_expecting_reply = true;
    init_npdu(&npdu_data, network_message_type, data_expecting_reply);

    *buff = (uint8_t *) malloc(128);    /* resolve different length */

    /* manual destination setup for Init-RT-Table-Ack message */
    data->dest.net = BACNET_BROADCAST_NETWORK;
    buff_len = npdu_encode_pdu(*buff, &data->dest, NULL, &npdu_data);

    switch (network_message_type) {

        case NETWORK_MESSAGE_WHO_IS_ROUTER_TO_NETWORK:
            if (val != NULL) {
                uint8_t *valptr = (uint8_t *) val;
                uint16_t val16 = (valptr[0]) + (valptr[1] << 8);
                buff_len += encode_unsigned16(*buff + buff_len, val16);
            }
            break;

        case NETWORK_MESSAGE_I_AM_ROUTER_TO_NETWORK:
            if (val != NULL) {
                uint8_t *valptr = (uint8_t *) val;
                uint16_t val16 = (valptr[0]) + (valptr[1] << 8);
                buff_len += encode_unsigned16(*buff + buff_len, val16);
            } else {
                ROUTER_PORT *port = head;
                DNET *dnet;
                while (port != NULL) {
                    if (port->route_info.net != data->src.net) {
                        buff_len +=
                            encode_unsigned16(*buff + buff_len,
                            port->route_info.net);
                        dnet = port->route_info.dnets;
                        while (dnet != NULL) {
                            buff_len +=
                                encode_unsigned16(*buff + buff_len, dnet->net);
                            dnet = dnet->next;
                        }
                        port = port->next;
                    } else {
                        dnet = port->route_info.dnets;
                        while (dnet != NULL) {
                            buff_len +=
                                encode_unsigned16(*buff + buff_len, dnet->net);
                            dnet = dnet->next;
                        }
                        port = port->next;
                    }
                }
            }
            break;

        case NETWORK_MESSAGE_REJECT_MESSAGE_TO_NETWORK:
            {
                uint8_t *valptr = (uint8_t *) val;
                uint16_t val16 = (valptr[0]) + (valptr[1] << 8);
                buff_len += encode_unsigned16(*buff + buff_len, val16);
                break;
            }
        case NETWORK_MESSAGE_INIT_RT_TABLE:
        case NETWORK_MESSAGE_INIT_RT_TABLE_ACK:
            if ((uint8_t *) val) {
                (*buff)[buff_len++] = (uint8_t) port_count;

                if (port_count > 0) {
                    ROUTER_PORT *port = head;
                    uint8_t portID = 1;

                    while (port != NULL) {
                        buff_len +=
                            encode_unsigned16(*buff + buff_len,
                            port->route_info.net);
                        (*buff)[buff_len++] = portID++;
                        (*buff)[buff_len++] = 0;
                        port = port->next;
                    }
                }
            } else
                (*buff)[buff_len++] = (uint8_t) 0;
            break;

        case NETWORK_MESSAGE_INVALID:
        case NETWORK_MESSAGE_I_COULD_BE_ROUTER_TO_NETWORK:
        case NETWORK_MESSAGE_ROUTER_BUSY_TO_NETWORK:
        case NETWORK_MESSAGE_ROUTER_AVAILABLE_TO_NETWORK:
        case NETWORK_MESSAGE_ESTABLISH_CONNECTION_TO_NETWORK:
        case NETWORK_MESSAGE_DISCONNECT_CONNECTION_TO_NETWORK:
            /* hell if I know what to do with these messages */
            break;
    }

    return buff_len;
}

void send_network_message(
    BACNET_NETWORK_MESSAGE_TYPE network_message_type,
    MSG_DATA * data,
    uint8_t ** buff,
    void *val)
{

    BACMSG msg;
    ROUTER_PORT *port = head;
    int16_t buff_len;

    if (!data) {
        data = (MSG_DATA *) malloc(sizeof(MSG_DATA));
        data->dest.net = BACNET_BROADCAST_NETWORK;
        data->dest.len = 0;
    }

    buff_len = create_network_message(network_message_type, data, buff, val);

    /* form network message */
    data->pdu = *buff;
    data->pdu_len = buff_len;
    msg.origin = head->main_id;
    msg.type = DATA;
    msg.data = data;

    data->ref_count = port_count;
    while (port != NULL) {
        if (port->state == FINISHED) {
            port = port->next;
            continue;
        }
        send_to_msgbox(port->port_id, &msg);
        port = port->next;
    }
}

void init_npdu(
    BACNET_NPDU_DATA * npdu_data,
    BACNET_NETWORK_MESSAGE_TYPE network_message_type,
    bool data_expecting_reply)
{

    if (npdu_data) {
        npdu_data->data_expecting_reply = data_expecting_reply;
        npdu_data->protocol_version = BACNET_PROTOCOL_VERSION;
        npdu_data->network_layer_message = true;
        npdu_data->network_message_type = network_message_type;
        npdu_data->vendor_id = 0;
        npdu_data->priority = MESSAGE_PRIORITY_NORMAL;
        npdu_data->hop_count = HOP_COUNT_DEFAULT;
    }
}
