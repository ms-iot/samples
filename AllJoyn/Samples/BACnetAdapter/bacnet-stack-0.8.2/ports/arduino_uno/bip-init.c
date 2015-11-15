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
#include <stdio.h>
#include "bacdcode.h"
#include "bip.h"
#include "socketWrapper.h"
#include "w5100Wrapper.h"
//#include "net.h"

/** @file linux/bip-init.c  Initializes BACnet/IP interface (Linux). */

bool BIP_Debug = false;

/* gets an IP address by name, where name can be a
   string that is an IP address in dotted form, or
   a name that is a domain name
   returns 0 if not found, or
   an IP address in network byte order */
long bip_getaddrbyname(const char *host_name)
{
    return 0;
}

/** Gets the local IP address and local broadcast address from the system,
 *  and saves it into the BACnet/IP data structures.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        Eg, for Linux, ifname is eth0, ath0, arc0, and others.
 */
void bip_set_interface(char *ifname)
{

    uint8_t local_address[] = { 0, 0, 0, 0 };
    uint8_t broadcast_address[] = { 0, 0, 0, 0 };
    uint8_t netmask[] = { 0, 0, 0, 0 };
    uint8_t invertedNetmask[] = { 0, 0, 0, 0 };

    getIPAddress_func(CW5100Class_new(), local_address);
    bip_set_addr(local_address);
    if (BIP_Debug) {
        fprintf(stderr, "Interface: %s\n", ifname);
        fprintf(stderr, "IP Address: %d.%d.%d.%d\n", local_address[0],
            local_address[1], local_address[2], local_address[3]);
    }

    /* setup local broadcast address */
    getSubnetMask_func(CW5100Class_new(), netmask);
    for (int i = 0; i < 4; i++) {       //FIXME: IPv4 ?
        invertedNetmask[i] = ~netmask[i];
        broadcast_address[i] = (local_address[i] | invertedNetmask[i]);
    }

    bip_set_broadcast_addr(broadcast_address);
    if (BIP_Debug) {
        fprintf(stderr, "IP Broadcast Address: %d.%d.%d.%d\n",
            broadcast_address[0], broadcast_address[1], broadcast_address[2],
            broadcast_address[3]);
    }
}

/** Initialize the BACnet/IP services at the given interface.
 * @ingroup DLBIP
 * -# Gets the local IP address and local broadcast address from the system,
 *  and saves it into the BACnet/IP data structures.
 * -# Opens a UDP socket
 * -# Configures the socket for sending and receiving
 * -# Configures the socket so it can send broadcasts
 * -# Binds the socket to the local IP address at the specified port for
 *    BACnet/IP (by default, 0xBAC0 = 47808).
 *
 * @note For Linux, ifname is eth0, ath0, arc0, and others.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        If NULL, the "eth0" interface is assigned.
 * @return True if the socket is successfully opened for BACnet/IP,
 *         else False if the socket functions fail.
 */
bool bip_init(char *ifname)
{
    uint8_t sock_fd = 0;
    bool isOpen = false;

    if (ifname)
        bip_set_interface(ifname);
    else
        bip_set_interface("eth0");

    /* assumes that the driver has already been initialized */
    for (sock_fd = 0; sock_fd < MAX_SOCK_NUM; sock_fd++) {
        if (readSnSR_func(CW5100Class_new(), sock_fd) == SnSR_CLOSED()) {
            socket_func(sock_fd, SnMR_UDP(), (uint16_t) 47808, 0);
            listen_func(sock_fd);
            isOpen = true;
            break;
        }
    }

    if (!isOpen) {
        bip_set_socket(MAX_SOCK_NUM);
        return false;
    } else {
        bip_set_socket(sock_fd);
    }

    return true;
}

/** Cleanup and close out the BACnet/IP services by closing the socket.
 * @ingroup DLBIP
  */
void bip_cleanup(void)
{
    int sock_fd = 0;

    if (bip_valid()) {
        sock_fd = bip_socket();
        close_func(sock_fd);
    }
    bip_set_socket(MAX_SOCK_NUM);

    return;
}

/** Get the netmask of the BACnet/IP's interface via an ioctl() call.
 * @param netmask [out] The netmask, in host order.
 * @return 0 on success, else the error from the ioctl() call.
 */
int bip_get_local_netmask(uint8_t * netmask)
{
    getSubnetMask_func(CW5100Class_new(), netmask);
    return 0;
}
