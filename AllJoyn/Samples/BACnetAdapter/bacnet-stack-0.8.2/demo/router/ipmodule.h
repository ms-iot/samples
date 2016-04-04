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
#ifndef UDPMODULE_H
#define UDPMODULE_H

#include <stdint.h>
#include <stdbool.h>
#include "portthread.h"
#include "bip.h"

#define MAX_BIP_APDU	1476
#define MAX_BIP_PDU		(MAX_NPDU + MAX_BIP_APDU)
#define MAX_BIP_MPDU	(MAX_HEADER + MAX_BIP_PDU)
/* Yes, we know this is longer than an Ethernet Frame,
   a UDP payload and an IPv6 packet.
   Grandfathered in from BACnet Ethernet days,
   and we can rely on the lower layers of the
   Ethernet stack to fragment/reassemble the BACnet MPDUs */

typedef struct ip_data {
    int socket;
    uint16_t port;
    struct in_addr local_addr;
    struct in_addr broadcast_addr;
    uint8_t *buff;
    uint16_t max_buff;
} IP_DATA;


void *dl_ip_thread(
    void *pArgs);

bool dl_ip_init(
    ROUTER_PORT * port,
    IP_DATA * data);

int dl_ip_send(
    IP_DATA * data,
    BACNET_ADDRESS * dest,
    uint8_t * pdu,
    unsigned pdu_len);

int dl_ip_recv(
    IP_DATA * data,
    MSG_DATA ** msg,    /* on recieve fill up message */
    BACNET_ADDRESS * src,
    unsigned timeout);

void dl_ip_cleanup(
    IP_DATA * data);

#endif /* end of UDPMODULE_H */
