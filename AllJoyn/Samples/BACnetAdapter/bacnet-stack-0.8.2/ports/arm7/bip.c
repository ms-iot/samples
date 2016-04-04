/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/

#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include <stdbool.h>    /* for the standard bool type. */
#include "bacdcode.h"
#include "bip.h"
#include "eth.h"
#include "net.h"        /* custom per port */

static int BIP_Socket = -1;
/* port to use - stored in host byte order */
static uint16_t BIP_Port = 0xBAC0;
/* IP Address - stored in host byte order */
static struct in_addr BIP_Address;
/* Broadcast Address - stored in host byte order */
static struct in_addr BIP_Broadcast_Address;

void bip_set_socket(
    int sock_fd)
{
    BIP_Socket = sock_fd;
}

int bip_socket(
    void)
{
    return BIP_Socket;
}

bool bip_valid(
    void)
{
    return (BIP_Socket != -1);
}

void bip_cleanup(
    void)
{
/*    if (bip_valid()) */
/*        close(BIP_Socket); */
    BIP_Socket = -1;

    return;
}

/* set using network byte order */
void bip_set_addr(
    uint32_t net_address)
{
/*    BIP_Address.s_addr = ntohl(net_address); */
    BIP_Address.s_addr = net_address;
}

/* returns host byte order */
uint32_t bip_get_addr(
    void)
{
    return BIP_Address.s_addr;
}

/* set using network byte order */
void bip_set_broadcast_addr(
    uint32_t net_address)
{
/*    BIP_Broadcast_Address.s_addr = ntohl(net_address); */
    BIP_Broadcast_Address.s_addr = net_address;
}

/* returns host byte order */
uint32_t bip_get_broadcast_addr(
    void)
{
    return BIP_Broadcast_Address.s_addr;
}

/* set using host byte order */
void bip_set_port(
    uint16_t port)
{
    BIP_Port = port;
}

/* returns host byte order */
uint16_t bip_get_port(
    void)
{
    return BIP_Port;
}

/* function to send a packet out the BACnet/IP socket (Annex J) */
/* returns number of bytes sent on success, negative number on failure */
int bip_send_pdu(
    BACNET_ADDRESS * dest,      /* destination address */
    BACNET_NPDU_DATA * npdu_data,       /* network information */
    uint8_t * pdu,      /* any data to be sent - may be null */
    unsigned pdu_len)
{       /* number of bytes of data */

    struct sockaddr_in bip_dest;
    uint8_t mtu[4];
    int mtu_len = 0;
    int bytes_sent = 0;
    UDP_HDR udphdr;
    IP_HDR iphdr;
    uint8_t mac[6];

    (void) npdu_data;

    /* assumes that the driver has already been initialized */
/*  if (BIP_Socket < 0) */
/*      return BIP_Socket; */

    mtu[0] = BVLL_TYPE_BACNET_IP;
    bip_dest.sin_family = AF_INET;

    if (dest->mac_len == 6) {
        memcpy(&(bip_dest.sin_addr.s_addr), &dest->mac[0], 4);
        decode_unsigned16(&dest->mac[4], &(bip_dest.sin_port));
        memset(&(bip_dest.sin_zero), '\0', 8);
        mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
    }
    /* broadcast */
    else if (dest->mac_len == 0) {
        bip_dest.sin_addr.s_addr = BIP_Broadcast_Address.s_addr;
        bip_dest.sin_port = htons(BIP_Port);
        memset(&(bip_dest.sin_zero), '\0', 8);
        mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
    } else
        return -1;

    mtu_len = 2;
    mtu_len +=
        encode_unsigned16(&mtu[mtu_len],
        (uint16_t) (pdu_len + 4 /*inclusive */ ));
    mtu_len += pdu_len;

    /* IP address should be in network byte order */
    ARPIsResolved(bip_dest.sin_addr.s_addr, mac);

    iphdr.vhl = 0x45;
    iphdr.tos = 0;
    iphdr.iplen = htons(mtu_len + sizeof(IP_HDR) + sizeof(UDP_HDR));
    ++ipid;
    iphdr.ipid = htons(ipid);
    iphdr.ipoffset[0] = iphdr.ipoffset[1] = 0;
    iphdr.ttl = UIP_TTL;
    iphdr.proto = UIP_PROTO_UDP;
    iphdr.ipchksum = 0;
    iphdr.srcipaddr = BIP_Address.s_addr;
    iphdr.destipaddr = bip_dest.sin_addr.s_addr;

    /* Calculate IP checksum. */
    iphdr.ipchksum = ~(ip_getchksum((uint8_t *) & iphdr));

    /* Ports are sent in Network byte order.  BIP_Port is stored in Host byte order */
    udphdr.srcport = htons(BIP_Port);
    udphdr.destport = bip_dest.sin_port;
    udphdr.udplen = htons(mtu_len + sizeof(UDP_HDR));
    /* Not using UDP checksums */
    udphdr.udpchksum = 0;

    EthernetSendHeader(mac, UIP_ETHTYPE_IP);
    EthernetSend((uint8_t *) & iphdr, sizeof(IP_HDR));
    EthernetSend((uint8_t *) & udphdr, sizeof(UDP_HDR));
    EthernetSend(mtu, 4);
    EthernetSend(pdu, pdu_len);
    EthernetFlush();
    uip_stat.ip.sent++;

}

/* receives a BACnet/IP packet */
/* returns the number of octets in the PDU, or zero on failure */
uint16_t bip_receive(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* PDU data */
    uint16_t max_pdu,   /* amount of space available in the PDU  */
    unsigned timeout)
{       /* number of milliseconds to wait for a packet */
    return 0;
}

void bip_get_my_address(
    BACNET_ADDRESS * my_address)
{
    int i = 0;

    my_address->mac_len = 6;
    (void) encode_unsigned32(&my_address->mac[0], htonl(BIP_Address.s_addr));
    (void) encode_unsigned16(&my_address->mac[4], htons(BIP_Port));
    my_address->net = 0;        /* local only, no routing */
    my_address->len = 0;        /* no SLEN */
    for (i = 0; i < MAX_MAC_LEN; i++) {
        /* no SADR */
        my_address->adr[i] = 0;
    }

    return;
}

void bip_get_broadcast_address(
    BACNET_ADDRESS * dest)
{       /* destination address */
    int i = 0;  /* counter */

    if (dest) {
        dest->mac_len = 6;
        (void) encode_unsigned32(&dest->mac[0],
            htonl(BIP_Broadcast_Address.s_addr));
        (void) encode_unsigned16(&dest->mac[4], htons(BIP_Port));
        dest->net = BACNET_BROADCAST_NETWORK;
        dest->len = 0;  /* no SLEN */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            /* no SADR */
            dest->adr[i] = 0;
        }
    }

    return;
}
