/**
* @file
* @author Andriy Sukhynyuk, Vasyl Tkhir, Andriy Ivasiv
* @date 2012
* @brief Datalink IP module
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
#include "ipmodule.h"
#include "bacint.h"

#ifdef TEST_PACKET
uint8_t test_packet[] = { 0x81, 0x0a, 0x00, 0x16,       /* BVLC header */
    0x01, 0x24, 0x00, 0x01, 0x01, 0x0b, 0xff,   /* NPDU */
    0x00, 0x03, 0x01, 0x0c, 0x0c, 0x00, 0x00, 0x00, 0x02, 0x19, 0x55
};      /* APDU */
#endif

extern int get_local_address_ioctl(
    char *ifname,
    struct in_addr *addr,
    int request);

void *dl_ip_thread(
    void *pArgs)
{
    MSGBOX_ID msgboxid;
    BACMSG msg_storage, *bacmsg = NULL;
    MSG_DATA *msg_data;
    ROUTER_PORT *port = (ROUTER_PORT *) pArgs;
    IP_DATA ip_data;    /* port specific parameters */
    BACNET_ADDRESS address = { 0 };
    int status;
    uint8_t shutdown = 0;

    /* initialize router port */
    if (!dl_ip_init(port, &ip_data)) {
        port->state = INIT_FAILED;
        return NULL;
    }

    /* allocate buffer */
    ip_data.max_buff = MAX_BIP_MPDU;
    ip_data.buff = (uint8_t *) malloc(ip_data.max_buff);

    if (ip_data.buff == NULL) {
        port->state = INIT_FAILED;
        return NULL;
    }

    msgboxid = create_msgbox();
    if (msgboxid == INVALID_MSGBOX_ID) {
        PRINT(ERROR, "Error: Failed to create message box");
        port->state = INIT_FAILED;
        return NULL;
    }

    port->port_id = msgboxid;
    port->state = RUNNING;

    while (!shutdown) {

        /* check for incoming messages */
        bacmsg = recv_from_msgbox(port->port_id, &msg_storage);

        if (bacmsg) {
            switch (bacmsg->type) {
                case DATA:{
                        msg_data = (MSG_DATA *) bacmsg->data;
                        memmove(&address.net, &msg_data->dest.net, 2);
                        memmove(&address.mac_len, &msg_data->dest.len, 1);
                        memmove(&address.mac[0], &msg_data->dest.adr[0],
                            MAX_MAC_LEN);

                        dl_ip_send(&ip_data, &address, msg_data->pdu,
                            msg_data->pdu_len);

                        check_data(msg_data);

                        break;
                    }

                case SERVICE:{
                        switch (bacmsg->subtype) {
                            case SHUTDOWN:
                                del_msgbox(port->port_id);
                                shutdown = 1;
                                break;
                            default:
                                break;
                        }
                        break;
                    }

                default:
                    break;
            }
        } else {
            status = dl_ip_recv(&ip_data, &msg_data, &address, 1000);
            if (status > 0) {
                memmove(&msg_data->src.len, &address.mac_len, 1);
                memmove(&msg_data->src.adr[0], &address.mac[0], MAX_MAC_LEN);
                msg_storage.origin = port->port_id;
                msg_storage.type = DATA;
                msg_storage.data = msg_data;

                if (!send_to_msgbox(port->main_id, &msg_storage)) {
                    free_data(msg_data);
                }
            }
        }
    }

    /* cleanup procedure */
    dl_ip_cleanup(&ip_data);
    port->state = FINISHED;
    return NULL;
}

bool dl_ip_init(
    ROUTER_PORT * port,
    IP_DATA * ip_data)
{
    struct sockaddr_in sin;
    int socket_opt = 0;
    int status = 0;     /* for error checking */

    /* setup port for later use */
    ip_data->port = htons(port->params.bip_params.port);

    /* get local address */
    status =
        get_local_address_ioctl(port->iface, &ip_data->local_addr,
        SIOCGIFADDR);
    if (status < 0) {
        return false;
    }
    /* get broadcast address */
    status =
        get_local_address_ioctl(port->iface, &ip_data->broadcast_addr,
        SIOCGIFBRDADDR);
    if (status < 0) {
        return false;
    }

    ip_data->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ip_data->socket < 0)
        return false;

    /* setup socket options */

    socket_opt = 1;
    status =
        setsockopt(ip_data->socket, SOL_SOCKET, SO_REUSEADDR, &socket_opt,
        sizeof(socket_opt));
    if (status < 0) {
        close(ip_data->socket);
        return false;
    }

    status =
        setsockopt(ip_data->socket, SOL_SOCKET, SO_BROADCAST, &socket_opt,
        sizeof(socket_opt));
    if (status < 0) {
        close(ip_data->socket);
        return false;
    }

    /* bind the socket to the local port number */
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = ip_data->port;

    memset(&sin.sin_zero, '\0', sizeof(sin.sin_zero));

    status =
        bind(ip_data->socket, (const struct sockaddr *) &sin,
        sizeof(struct sockaddr));
    if (status < 0) {
        close(ip_data->socket);
        return false;
    }

    /* add BIP address to router port structure */
    memcpy(&port->route_info.mac[0], &ip_data->local_addr.s_addr, 4);
    memcpy(&port->route_info.mac[4], &port->params.bip_params.port, 2);
    port->route_info.mac_len = 6;

    PRINT(INFO, "Interface: %s\n", port->iface);
    PRINT(INFO, "IP Address: %s\n", inet_ntoa(ip_data->local_addr));
    PRINT(INFO, "IP Broadcast Address: %s\n",
        inet_ntoa(ip_data->broadcast_addr));
    PRINT(INFO, "UDP Port: 0x%04X [%hu]\n", (port->params.bip_params.port),
        (port->params.bip_params.port));

    return true;
}

int dl_ip_send(
    IP_DATA * data,
    BACNET_ADDRESS * dest,
    uint8_t * pdu,
    unsigned pdu_len)
{
    struct sockaddr_in bip_dest = { 0 };
    int buff_len = 0;
    int bytes_sent = 0;

    if (data->socket < 0)
        return -1;

    data->buff[0] = BVLL_TYPE_BACNET_IP;
    bip_dest.sin_family = AF_INET;
    if (dest->net == BACNET_BROADCAST_NETWORK) {
        /* broadcast */
        bip_dest.sin_addr.s_addr = data->broadcast_addr.s_addr;
        bip_dest.sin_port = data->port;
        data->buff[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    } else if (dest->mac_len == 6) {
        memcpy(&bip_dest.sin_addr.s_addr, &dest->mac[0], 4);
        memcpy(&bip_dest.sin_port, &dest->mac[4], 2);
        data->buff[1] = BVLC_ORIGINAL_UNICAST_NPDU;
    } else {
        /* invalid address */
        return -1;
    }

    buff_len = 2;
    buff_len +=
        encode_unsigned16(&data->buff[buff_len],
        (uint16_t) (pdu_len + 4 /*inclusive */ ));
    memcpy(&data->buff[buff_len], pdu, pdu_len);
    buff_len += pdu_len;

    /* send the packet */
    bytes_sent =
        sendto(data->socket, (char *) data->buff, buff_len, 0,
        (struct sockaddr *) &bip_dest, sizeof(struct sockaddr));

    PRINT(DEBUG, "send to %s\n", inet_ntoa(bip_dest.sin_addr));

    return bytes_sent;
}

int dl_ip_recv(
    IP_DATA * data,
    MSG_DATA ** msg_data,
    BACNET_ADDRESS * src,
    unsigned timeout)
{
    int received_bytes = 0;
    uint16_t buff_len = 0;      /* return value */
    fd_set read_fds;
    struct timeval select_timeout;
    struct sockaddr_in sin = { 0 };
    socklen_t sin_len = sizeof(sin);

    /* make sure the socket is open */
    if (data->socket < 0)
        return 0;

    if (timeout >= 1000) {
        select_timeout.tv_sec = timeout / 1000;
        select_timeout.tv_usec =
            1000 * (timeout - select_timeout.tv_sec * 1000);
    } else {
        select_timeout.tv_sec = 0;
        select_timeout.tv_usec = 1000 * timeout;
    }

    FD_ZERO(&read_fds);
    FD_SET(data->socket, &read_fds);

#ifdef TEST_PACKET
    received_bytes = sizeof(test_packet);
    memmove(data->buff, &test_packet, received_bytes);
    sin.sin_addr.s_addr = 0x7E1D40A;
    sin.sin_port = 0xC0BA;
#else
    int ret = select(data->socket + 1, &read_fds, NULL, NULL, &select_timeout);
    /* see if there is a packet for us */
    if (ret > 0)
        received_bytes =
            recvfrom(data->socket, (char *) &data->buff[0], data->max_buff, 0,
            (struct sockaddr *) &sin, &sin_len);
    else
        return 0;
#endif
    PRINT(DEBUG, "received from %s\n", inet_ntoa(sin.sin_addr));

    /* check for errors */
    if (received_bytes <= 0) {
        return 0;
    }

    /* the signature of a BACnet/IP packet */
    if (data->buff[0] != BVLL_TYPE_BACNET_IP)
        return 0;

    switch (data->buff[1]) {
        case BVLC_ORIGINAL_UNICAST_NPDU:
        case BVLC_ORIGINAL_BROADCAST_NPDU:{
                if ((sin.sin_addr.s_addr == data->local_addr.s_addr) &&
                    (sin.sin_port == data->port)) {
                    buff_len = 0;

                    PRINT(DEBUG, "BIP: src is me. Discarded!\n");

                } else {
                    src->mac_len = 6;
                    memcpy(&src->mac[0], &sin.sin_addr.s_addr, 4);
                    memcpy(&src->mac[4], &sin.sin_port, 2);

                    (void) decode_unsigned16(&data->buff[2], &buff_len);
                    /* subtract off the BVLC header */
                    buff_len -= 4;
                    if (buff_len < data->max_buff) {
                        /* allocate data message stucture */
                        (*msg_data) = (MSG_DATA *) malloc(sizeof(MSG_DATA));
                        (*msg_data)->pdu_len = buff_len;
                        (*msg_data)->pdu =
                            (uint8_t *) malloc((*msg_data)->pdu_len);
                        /* fill up data message structure */
                        memmove(&(*msg_data)->pdu[0], &data->buff[4],
                            (*msg_data)->pdu_len);
                        memmove(&(*msg_data)->src, src,
                            sizeof(BACNET_ADDRESS));
                    }
                    /* ignore packets that are too large */
                    else {
                        buff_len = 0;

                        PRINT(ERROR, "BIP: PDU too large. Discarded!.\n");

                    }
                }
            }
            break;

        case BVLC_FORWARDED_NPDU:{
                memcpy(&sin.sin_addr.s_addr, &data->buff[4], 4);
                memcpy(&sin.sin_port, &data->buff[8], 2);
                if ((sin.sin_addr.s_addr == data->local_addr.s_addr) &&
                    (sin.sin_port == data->port)) {
                    buff_len = 0;
                } else {
                    src->mac_len = 6;
                    memcpy(&src->mac[0], &sin.sin_addr.s_addr, 4);
                    memcpy(&src->mac[4], &sin.sin_port, 2);

                    (void) decode_unsigned16(&data->buff[2], &buff_len);
                    /* subtract off the BVLC header */
                    buff_len -= 10;
                    if (buff_len < data->max_buff) {
                        /* allocate data message stucture */
                        (*msg_data) = (MSG_DATA *) malloc(sizeof(MSG_DATA));
                        (*msg_data)->pdu_len = buff_len;
                        (*msg_data)->pdu =
                            (uint8_t *) malloc((*msg_data)->pdu_len);
                        /* fill up data message structure */
                        memmove(&(*msg_data)->pdu[0], &data->buff[4 + 6],
                            (*msg_data)->pdu_len);
                        memmove(&(*msg_data)->src, src,
                            sizeof(BACNET_ADDRESS));
                    } else {
                        /* ignore packets that are too large */
                        buff_len = 0;
                    }
                }
            }
            break;
        default:

            PRINT(ERROR, "BIP: BVLC discarded!\n");

            break;
    }
    return buff_len;
}

void dl_ip_cleanup(
    IP_DATA * ip_data)
{
    /* free buffer */
    if (ip_data->buff)
        free(ip_data->buff);
    /* close socket */
    if (ip_data->socket > 0)
        close(ip_data->socket);
    return;
}
