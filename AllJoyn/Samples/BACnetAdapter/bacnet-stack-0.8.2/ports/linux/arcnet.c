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

#include <stdint.h>
#include "bacdef.h"
#include "npdu.h"
#include "arcnet.h"
#include "net.h"

/** @file linux/arcnet.c  Provides Linux-specific functions for Arcnet. */

/* my local device data - MAC address */
uint8_t ARCNET_MAC_Address = 0;
/* ARCNET file handle */
static int ARCNET_Sock_FD = -1;
/* ARCNET socket address (has the interface name) */
static struct sockaddr ARCNET_Socket_Address;
/* Broadcast address */
#define ARCNET_BROADCAST 0

/*
Hints:  

When using a PCI20-485D ARCNET card from Contemporary Controls,
you might need to know about the following settings:

Assuming a 20MHz clock on the COM20020 chip:

clockp Clock Prescaler DataRate
------ --------------- --------
0           8          2.5 Mbps
1           16         1.25 Mbps
2           32         625 Kbps
3           64         312.5 Kbps 
4           128        156.25Kbps

1. Install the arcnet driver and arcnet raw mode driver:
# modprobe com20020_pci clockp=4
# modprobe arc_rawmode

2. Use ifconfig to bring up the interface
# ifconfig arc0 up

3. The hardware address (MAC address) is set using the dipswitch
   on the back of the card.  0 is broadcast, so don't use 0.

4. The backplane mode on the PCI20-485D card is done in hardware,
   so the driver does not need to do backplane mode.  If you 
   use another type of PCI20 card, you could pass in backplane=1 or 
   backplane=0 as an option to the modprobe of com20020_pci.

*/

bool arcnet_valid(
    void)
{
    return (ARCNET_Sock_FD >= 0);
}

void arcnet_cleanup(
    void)
{
    if (arcnet_valid())
        close(ARCNET_Sock_FD);
    ARCNET_Sock_FD = -1;

    return;
}

static int arcnet_bind(
    char *interface_name)
{
    int sock_fd = -1;   /* return value */
    struct ifreq ifr;
    int rv;     /* return value - error value from df or ioctl call */
    int uid = 0;

    /* check to see if we are being run as root */
    uid = getuid();
    if (uid != 0) {
        fprintf(stderr,
            "arcnet: Unable to open an af_packet socket.  "
            "Try running with root priveleges.\n");
        return sock_fd;
    }
    fprintf(stderr, "arcnet: opening \"%s\"\n", interface_name);
    /* note: on some systems you may have to add or enable in */
    /* modules.conf (or in modutils/alias on Debian with update-modules) */
    /* alias net-pf-17 af_packet */
    /* Then follow it by: # modprobe af_packet */
    if ((sock_fd = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ALL))) < 0) {
        /* Error occured */
        fprintf(stderr, "arcnet: Error opening socket: %s\n", strerror(errno));
        fprintf(stderr,
            "You might need to add the following to modules.conf\n"
            "(or in /etc/modutils/alias on Debian with update-modules):\n"
            "alias net-pf-17 af_packet\n"
            "Also, add af_packet to /etc/modules.\n" "Then follow it by:\n"
            "# modprobe af_packet\n");
        exit(-1);
    }

    if (ARCNET_Sock_FD >= 0) {
        /* Bind the socket to an interface name so we only get packets from it */
        ARCNET_Socket_Address.sa_family = ARPHRD_ARCNET;
        /*ARCNET_Socket_Address.sa_family = PF_INET; */
        /* Clear the memory before copying */
        memset(ARCNET_Socket_Address.sa_data, '\0',
            sizeof(ARCNET_Socket_Address.sa_data));
        /* Strcpy the interface name into the address */
        strncpy(ARCNET_Socket_Address.sa_data, interface_name,
            sizeof(ARCNET_Socket_Address.sa_data) - 1);
        fprintf(stderr, "arcnet: binding \"%s\"\n",
            ARCNET_Socket_Address.sa_data);
        if (bind(sock_fd, &ARCNET_Socket_Address,
                sizeof(ARCNET_Socket_Address)) != 0) {
            /* Bind problem, close socket and return */
            fprintf(stderr, "arcnet: Unable to bind socket : %s\n",
                strerror(errno));
            fprintf(stderr,
                "You might need to add the following to modules.conf\n"
                "(or in /etc/modutils/alias on Debian with update-modules):\n"
                "alias net-pf-17 af_packet\n"
                "Also, add af_packet to /etc/modules.\n" "Then follow it by:\n"
                "# modprobe af_packet\n");
            /* Close the socket */
            close(sock_fd);
            exit(-1);
        }
    }
    strncpy(ifr.ifr_name, interface_name, sizeof(ifr.ifr_name));
    rv = ioctl(sock_fd, SIOCGIFHWADDR, &ifr);
    if (rv != -1)       /* worked okay */
        ARCNET_MAC_Address = ifr.ifr_hwaddr.sa_data[0];
    /* copy this info into the local copy since bind wiped it out */
    ARCNET_Socket_Address.sa_family = ARPHRD_ARCNET;
    /*ARCNET_Socket_Address.sa_family = PF_INET; */
    /* Clear the memory before copying */
    memset(ARCNET_Socket_Address.sa_data, '\0',
        sizeof(ARCNET_Socket_Address.sa_data));
    /* Strcpy the interface name into the address */
    strncpy(ARCNET_Socket_Address.sa_data, interface_name,
        sizeof(ARCNET_Socket_Address.sa_data) - 1);
    fprintf(stderr, "arcnet: MAC=%02Xh iface=\"%s\"\n", ARCNET_MAC_Address,
        ARCNET_Socket_Address.sa_data);

    atexit(arcnet_cleanup);

    return sock_fd;
}

bool arcnet_init(
    char *interface_name)
{
    if (interface_name)
        ARCNET_Sock_FD = arcnet_bind(interface_name);
    else
        ARCNET_Sock_FD = arcnet_bind("arc0");

    return arcnet_valid();
}

/* function to send a PDU out the socket */
/* returns number of bytes sent on success, negative on failure */
int arcnet_send_pdu(
    BACNET_ADDRESS * dest,      /* destination address */
    BACNET_NPDU_DATA * npdu_data,       /* network information */
    uint8_t * pdu,      /* any data to be sent - may be null */
    unsigned pdu_len)
{       /* number of bytes of data */
    BACNET_ADDRESS src = { 0 }; /* source address */
    int bytes = 0;
    uint8_t mtu[512] = { 0 };
    int mtu_len = 0;
    struct archdr *pkt = (struct archdr *) mtu;

    (void) npdu_data;
    src.mac[0] = ARCNET_MAC_Address;
    src.mac_len = 1;

    /* don't waste time if the socket is not valid */
    if (ARCNET_Sock_FD < 0) {
        fprintf(stderr, "arcnet: socket is invalid!\n");
        return -1;
    }
    /* load destination MAC address */
    if (dest->mac_len == 1)
        pkt->hard.dest = dest->mac[0];
    else {
        fprintf(stderr, "arcnet: invalid destination MAC address!\n");
        return -2;
    }
    if (src.mac_len == 1)
        pkt->hard.source = src.mac[0];
    else {
        fprintf(stderr, "arcnet: invalid source MAC address!\n");
        return -3;
    }
    /* Logical PDU portion */
    pkt->soft.raw[0] = 0xCD;    /* SC for BACnet */
    pkt->soft.raw[1] = 0x82;    /* DSAP for BACnet */
    pkt->soft.raw[2] = 0x82;    /* SSAP for BACnet */
    pkt->soft.raw[3] = 0x03;    /* LLC Control byte in header */
    /* packet length */
    mtu_len = ARC_HDR_SIZE + 4 /*SC,DSAP,SSAP,LLC */  + pdu_len;
    if (mtu_len > 512) {
        fprintf(stderr, "arcnet: PDU is too big to send!\n");
        return -4;
    }
    memcpy(&pkt->soft.raw[4], pdu, pdu_len);
    /* Send the packet */
    bytes =
        sendto(ARCNET_Sock_FD, &mtu, mtu_len, 0,
        (struct sockaddr *) &ARCNET_Socket_Address,
        sizeof(ARCNET_Socket_Address));
    /* did it get sent? */
    if (bytes < 0)
        fprintf(stderr, "arcnet: Error sending packet: %s\n", strerror(errno));

    return bytes;
}

/* receives an framed packet */
/* returns the number of octets in the PDU, or zero on failure */
uint16_t arcnet_receive(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* PDU data */
    uint16_t max_pdu,   /* amount of space available in the PDU  */
    unsigned timeout)
{       /* milliseconds to wait for a packet */
    int received_bytes;
    uint8_t buf[512] = { 0 };   /* data */
    uint16_t pdu_len = 0;       /* return value */
    fd_set read_fds;
    int max;
    struct timeval select_timeout;
    struct archdr *pkt = (struct archdr *) buf;

    /* Make sure the socket is open */
    if (ARCNET_Sock_FD <= 0)
        return 0;

    /* we could just use a non-blocking socket, but that consumes all
       the CPU time.  We can use a timeout; it is only supported as
       a select. */
    if (timeout >= 1000) {
        select_timeout.tv_sec = timeout / 1000;
        select_timeout.tv_usec =
            1000 * (timeout - select_timeout.tv_sec * 1000);
    } else {
        select_timeout.tv_sec = 0;
        select_timeout.tv_usec = 1000 * timeout;
    }
    FD_ZERO(&read_fds);
    FD_SET(ARCNET_Sock_FD, &read_fds);
    max = ARCNET_Sock_FD;

    if (select(max + 1, &read_fds, NULL, NULL, &select_timeout) > 0)
        received_bytes = read(ARCNET_Sock_FD, &buf[0], sizeof(buf));
    else
        return 0;

    /* See if there is a problem */
    if (received_bytes < 0) {
        /* EAGAIN Non-blocking I/O has been selected  */
        /* using O_NONBLOCK and no data */
        /* was immediately available for reading. */
        if (errno != EAGAIN)
            fprintf(stderr, "ethernet: Read error in receiving packet: %s\n",
                strerror(errno));
        return 0;
    }

    if (received_bytes == 0)
        return 0;

    /* printf("arcnet: received %u bytes (offset=%02Xh %02Xh) "
       "from %02Xh (proto==%02Xh)\n",
       received_bytes, pkt->offset[0], pkt->offset[1], 
       pkt->hard.source, pkt->soft.raw[0]);
     */

    if (pkt->hard.source == ARCNET_MAC_Address) {
        fprintf(stderr, "arcnet: self sent packet?\n");
        return 0;
    }
    if (pkt->soft.raw[0] != 0xCD) {
        /* fprintf(stderr,"arcnet: Non-BACnet packet.\n"); */
        return 0;
    }
    if ((pkt->hard.dest != ARCNET_MAC_Address) &&
        (pkt->hard.dest != ARCNET_BROADCAST)) {
        fprintf(stderr, "arcnet: This packet is not for us.\n");
        return 0;
    }
    if ((pkt->soft.raw[1] != 0x82) ||   /* DSAP */
        (pkt->soft.raw[2] != 0x82) ||   /* LSAP */
        (pkt->soft.raw[3] != 0x03)) {   /* LLC Control */
        fprintf(stderr, "arcnet: BACnet packet has invalid LLC.\n");
        return 0;
    }
    /* It must be addressed to us or be a Broadcast */
    if ((pkt->hard.dest != ARCNET_MAC_Address) &&
        (pkt->hard.dest != ARCNET_BROADCAST)) {
        fprintf(stderr, "arcnet: This packet is not for us.\n");
        return 0;
    }
    /* copy the source address */
    src->mac_len = 1;
    src->mac[0] = pkt->hard.source;
    /* compute the PDU length */
    pdu_len = received_bytes - ARC_HDR_SIZE;
    pdu_len -= 4 /* SC, DSAP, SSAP, LLC Control */ ;
    /* copy the buffer into the PDU */
    if (pdu_len < max_pdu)
        memmove(&pdu[0], &pkt->soft.raw[4], pdu_len);
    /* silently ignore packets that are too large */
    else
        pdu_len = 0;

    return pdu_len;
}

void arcnet_get_my_address(
    BACNET_ADDRESS * my_address)
{
    int i = 0;

    my_address->mac_len = 1;
    my_address->mac[0] = ARCNET_MAC_Address;
    my_address->net = 0;        /* DNET=0 is local only, no routing */
    my_address->len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        my_address->adr[i] = 0;
    }

    return;
}

void arcnet_get_broadcast_address(
    BACNET_ADDRESS * dest)
{       /* destination address */
    int i = 0;  /* counter */

    if (dest) {
        dest->mac[0] = ARCNET_BROADCAST;
        dest->mac_len = 1;
        dest->net = BACNET_BROADCAST_NETWORK;
        dest->len = 0;  /* always zero when DNET is broadcast */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            dest->adr[i] = 0;
        }
    }

    return;
}
