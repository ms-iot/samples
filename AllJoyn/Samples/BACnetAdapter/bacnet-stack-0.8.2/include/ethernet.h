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
#ifndef ETHERNET_H
#define ETHERNET_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bacdef.h"
#include "npdu.h"

/* specific defines for Ethernet */
#define MAX_HEADER (6+6+2+1+1+1)
#define MAX_MPDU (MAX_HEADER+MAX_PDU)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    bool ethernet_valid(
        void);
    void ethernet_cleanup(
        void);
    bool ethernet_init(
        char *interface_name);

/* function to send a packet out the 802.2 socket */
/* returns number of bytes sent on success, negative on failure */
    int ethernet_send_pdu(
        BACNET_ADDRESS * dest,  /* destination address */
        BACNET_NPDU_DATA * npdu_data,   /* network information */
        uint8_t * pdu,  /* any data to be sent - may be null */
        unsigned pdu_len);      /* number of bytes of data */

/* receives an 802.2 framed packet */
/* returns the number of octets in the PDU, or zero on failure */
    uint16_t ethernet_receive(
        BACNET_ADDRESS * src,   /* source address */
        uint8_t * pdu,  /* PDU data */
        uint16_t max_pdu,       /* amount of space available in the PDU  */
        unsigned timeout);      /* milliseconds to wait for a packet */

    void ethernet_set_my_address(
        BACNET_ADDRESS * my_address);
    void ethernet_get_my_address(
        BACNET_ADDRESS * my_address);
    void ethernet_get_broadcast_address(
        BACNET_ADDRESS * dest); /* destination address */

    /* some functions from Linux driver */
    void ethernet_debug_address(
        const char *info,
        BACNET_ADDRESS * dest);
    int ethernet_send(
        uint8_t * mtu,
        int mtu_len);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
