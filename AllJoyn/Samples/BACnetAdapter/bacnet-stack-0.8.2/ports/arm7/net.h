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

#ifndef NET_H
#define NET_H

#include "stdint.h"

struct sockaddr {
    uint16_t sa_family;
    char sa_data[14];
};


struct in_addr {
    uint32_t s_addr;    /* load with inet_aton() */
};

struct sockaddr_in {
    int16_t sin_family; /* e.g. AF_INET */
    uint16_t sin_port;  /* e.g. htons(3490) */
    struct in_addr sin_addr;    /* see struct in_addr, below */
    char sin_zero[8];   /* zero this if you want to */
};


typedef int socklen_t;

/**
 * Convert 16-bit quantity from host byte order to network byte order.
 *
 * This macro is primarily used for converting constants from host
 * byte order to network byte order. For converting variables to
 * network byte order, use the htons() function instead.
 *
 * \hideinitializer
 */
#ifndef htons
#define htons(n) ((((uint16_t)((n) & 0xff)) << 8) | (((n) & 0xff00) >> 8))
#endif /* HTONS */

#define ntohs(n) (((((uint16_t)(n) & 0xFF)) << 8) | (((uint16_t)(n) & 0xFF00) >> 8))
#define htonl(n) (((((uint32_t)(n) & 0xFF)) << 24) | \
                  ((((uint32_t)(n) & 0xFF00)) << 8) | \
                  ((((uint32_t)(n) & 0xFF0000)) >> 8) | \
                  ((((uint32_t)(n) & 0xFF000000)) >> 24))

#define ntohl(n) (((((uint32_t)(n) & 0xFF)) << 24) | \
                  ((((uint32_t)(n) & 0xFF00)) << 8) | \
                  ((((uint32_t)(n) & 0xFF0000)) >> 8) | \
                  ((((uint32_t)(n) & 0xFF000000)) >> 24))


#define AF_UNIX         1       /* local to host (pipes, portals) */
#define AF_INET         2       /* internetwork: UDP, TCP, etc. */
#define AF_IMPLINK      3       /* arpanet imp addresses */
#define AF_PUP          4       /* pup protocols: e.g. BSP */
#define AF_CHAOS        5       /* mit CHAOS protocols */
#define AF_NS           6       /* XEROX NS protocols */
#define AF_IPX          AF_NS   /* IPX protocols: IPX, SPX, etc. */
#define AF_ISO          7       /* ISO protocols */
#define AF_OSI          AF_ISO  /* OSI is ISO */
#define AF_ECMA         8       /* european computer manufacturers */
#define AF_DATAKIT      9       /* datakit protocols */
#define AF_CCITT        10      /* CCITT protocols, X.25 etc */
#define AF_SNA          11      /* IBM SNA */
#define AF_DECnet       12      /* DECnet */
#define AF_DLI          13      /* Direct data link interface */
#define AF_LAT          14      /* LAT */
#define AF_HYLINK       15      /* NSC Hyperchannel */
#define AF_APPLETALK    16      /* AppleTalk */
#define AF_NETBIOS      17      /* NetBios-style addresses */
#define AF_VOICEVIEW    18      /* VoiceView */
#define AF_FIREFOX      19      /* Protocols from Firefox */
#define AF_UNKNOWN1     20      /* Somebody is using this! */
#define AF_BAN          21      /* Banyan */
#define AF_ATM          22      /* Native ATM Services */
#define AF_INET6        23      /* Internetwork Version 6 */
#define AF_CLUSTER      24      /* Microsoft Wolfpack */
#define AF_12844        25      /* IEEE 1284.4 WG AF */
#define AF_IRDA         26      /* IrDA */
#define AF_NETDES       28      /* Network Designers OSI & gateway
                                   enabled protocols */
#define AF_TCNPROCESS   29
#define AF_TCNMESSAGE   30
#define AF_ICLFXBM      31

#define AF_MAX          32

extern void set_address(
    uint32_t * net_address,
    uint8_t octet1,
    uint8_t octet2,
    uint8_t octet3,
    uint8_t octet4);

#endif
