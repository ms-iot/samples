/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#ifndef BIP_H
#define BIP_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bacdef.h"
#include "npdu.h"
#include "net.h"

/* specific defines for BACnet/IP over Ethernet */
#define MAX_HEADER (1 + 1 + 2)
#define MAX_MPDU (MAX_HEADER+MAX_PDU)

#define BVLL_TYPE_BACNET_IP (0x81)

extern bool BIP_Debug;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    /* note: define init, set_interface, and cleanup in your port */
    /* on Linux, ifname is eth0, ath0, arc0, and others.
       on Windows, ifname is the dotted ip address of the interface */
    bool bip_init(
        char *ifname);
    void bip_set_interface(
        char *ifname);
    void bip_cleanup(
        void);

    /* common BACnet/IP functions */
    void bip_set_socket(
        int sock_fd);
    int bip_socket(
        void);
    bool bip_valid(
        void);
    void bip_get_broadcast_address(
        BACNET_ADDRESS * dest); /* destination address */
    void bip_get_my_address(
        BACNET_ADDRESS * my_address);

    /* function to send a packet out the BACnet/IP socket */
    /* returns zero on success, non-zero on failure */
    int bip_send_pdu(
        BACNET_ADDRESS * dest,  /* destination address */
        BACNET_NPDU_DATA * npdu_data,   /* network information */
        uint8_t * pdu,  /* any data to be sent - may be null */
        unsigned pdu_len);      /* number of bytes of data */

    /* receives a BACnet/IP packet */
    /* returns the number of octets in the PDU, or zero on failure */
    uint16_t bip_receive(
        BACNET_ADDRESS * src,   /* source address */
        uint8_t * pdu,  /* PDU data */
        uint16_t max_pdu,       /* amount of space available in the PDU  */
        unsigned timeout);      /* milliseconds to wait for a packet */

    /* use network byte order for setting */
    void bip_set_port(
        uint16_t port);
    /* returns network byte order */
    uint16_t bip_get_port(
        void);

    /* use network byte order for setting */
    void bip_set_addr(
        uint32_t net_address);
    /* returns network byte order */
    uint32_t bip_get_addr(
        void);

    /* use network byte order for setting */
    void bip_set_broadcast_addr(
        uint32_t net_address);
    /* returns network byte order */
    uint32_t bip_get_broadcast_addr(
        void);

    /* gets an IP address by name, where name can be a
       string that is an IP address in dotted form, or
       a name that is a domain name
       returns 0 if not found, or
       an IP address in network byte order */
    long bip_getaddrbyname(
        const char *host_name);


#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @defgroup DLBIP BACnet/IP DataLink Network Layer
 * @ingroup DataLink
 * Implementation of the Network Layer using BACnet/IP as the transport, as
 * described in Annex J.
 * The functions described here fulfill the roles defined generically at the
 * DataLink level by serving as the implementation of the function templates.
 *
 */
#endif
