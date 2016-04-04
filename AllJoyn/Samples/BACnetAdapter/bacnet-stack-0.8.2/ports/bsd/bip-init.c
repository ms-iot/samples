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
#include "net.h"
#include <ifaddrs.h>

/** @file linux/bip-init.c  Initializes BACnet/IP interface (BSD/MAC OS X). */

bool BIP_Debug = true;

void *get_addr_ptr(
    struct sockaddr *sockaddr_ptr)
{
    void *addr_ptr;
    if (sockaddr_ptr->sa_family == AF_INET) {
        addr_ptr = &((struct sockaddr_in *) sockaddr_ptr)->sin_addr;
    } else if (sockaddr_ptr->sa_family == AF_INET6) {
        addr_ptr = &((struct sockaddr_in6 *) sockaddr_ptr)->sin6_addr;
    }
    return addr_ptr;
}

/* gets an IP address by name, where name can be a
   string that is an IP address in dotted form, or
   a name that is a domain name
   returns 0 if not found, or
   an IP address in network byte order */
long bip_getaddrbyname(
    const char *host_name)
{
    struct hostent *host_ent;

    if ((host_ent = gethostbyname(host_name)) == NULL)
        return 0;

    return *(long *) host_ent->h_addr;
}

/** Gets the local IP address and local broadcast address from the system,
 *  and saves it into the BACnet/IP data structures.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        Eg, for MAC OS X, ifname is en0, en1, and others.
 * @param addr [out] The netmask addr, broadcast addr, ip addr.
 * @param request [in] addr broadaddr netmask
 */
static int get_local_address(
    char *ifname,
    struct in_addr *addr,
    char *request)
{

    char rv;    /* return value */

    struct ifaddrs *ifaddrs_ptr;
    int status;
    status = getifaddrs(&ifaddrs_ptr);
    if (status == -1) {
        fprintf(stderr, "Error in 'getifaddrs': %d (%s)\n", errno,
            strerror(errno));
    }
    while (ifaddrs_ptr) {
        if ((ifaddrs_ptr->ifa_addr->sa_family == AF_INET) &&
            (strcmp(ifaddrs_ptr->ifa_name, ifname) == 0)) {
            void *addr_ptr;
            if (!ifaddrs_ptr->ifa_addr) {
                return rv;
            }
            if (strcmp(request, "addr") == 0) {
                addr_ptr = get_addr_ptr(ifaddrs_ptr->ifa_addr);
            } else if (strcmp(request, "broadaddr") == 0) {
                addr_ptr = get_addr_ptr(ifaddrs_ptr->ifa_broadaddr);
            } else if (strcmp(request, "netmask") == 0) {
                addr_ptr = get_addr_ptr(ifaddrs_ptr->ifa_netmask);
            }
            if (addr_ptr) {
                memcpy(addr, addr_ptr, sizeof(struct in_addr));
            }
        }
        ifaddrs_ptr = ifaddrs_ptr->ifa_next;
    }
    freeifaddrs(ifaddrs_ptr);
    return rv;
}

/** Gets the local IP address and local broadcast address from the system,
 *  and saves it into the BACnet/IP data structures.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        Eg, for MAC OS X, ifname is en0, en1, and others.
 */
void bip_set_interface(
    char *ifname)
{
    struct in_addr local_address;
    struct in_addr broadcast_address;
    int rv = 0;

    /* setup local address */
    char *request = "addr";
    rv = get_local_address(ifname, &local_address, request);
    if (rv < 0) {
        local_address.s_addr = 0;
    }
    bip_set_addr(local_address.s_addr);
    if (BIP_Debug) {
        fprintf(stderr, "Interface: %s\n", ifname);
        fprintf(stderr, "IP Address: %s\n", inet_ntoa(local_address));
    }
    /* setup local broadcast address */
    request = "broadaddr";
    rv = get_local_address(ifname, &broadcast_address, request);
    if (rv < 0) {
        broadcast_address.s_addr = ~0;
    }
    bip_set_broadcast_addr(broadcast_address.s_addr);
    if (BIP_Debug) {
        fprintf(stderr, "IP Broadcast Address: %s\n",
            inet_ntoa(broadcast_address));
        fprintf(stderr, "UDP Port: 0x%04X [%hu]\n", ntohs(bip_get_port()),
            ntohs(bip_get_port()));
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
 * @note For MAC OS X, ifname is en0, en1, and others.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        If NULL, the "en0" interface is assigned.
 * @return True if the socket is successfully opened for BACnet/IP,
 *         else False if the socket functions fail.
 */
bool bip_init(
    char *ifname)
{
    int status = 0;     /* return from socket lib calls */
    struct sockaddr_in sin;
    int sockopt = 0;
    int sock_fd = -1;

    if (ifname) {
        bip_set_interface(ifname);
        printf("interface %s", ifname);
    } else {
        bip_set_interface("en0");
    }
    /* assumes that the driver has already been initialized */
    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bip_set_socket(sock_fd);
    if (sock_fd < 0)
        return false;
    /* Allow us to use the same socket for sending and receiving */
    /* This makes sure that the src port is correct when sending */
    sockopt = 1;
    status =
        setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt,
        sizeof(sockopt));
    if (status < 0) {
        close(sock_fd);
        bip_set_socket(-1);
        return status;
    }
    /* allow us to send a broadcast */
    status =
        setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &sockopt,
        sizeof(sockopt));
    if (status < 0) {
        close(sock_fd);
        bip_set_socket(-1);
        return false;
    }
    /* bind the socket to the local port number and IP address */
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = bip_get_port();
    memset(&(sin.sin_zero), '\0', sizeof(sin.sin_zero));
    status =
        bind(sock_fd, (const struct sockaddr *) &sin, sizeof(struct sockaddr));
    if (status < 0) {
        close(sock_fd);
        bip_set_socket(-1);
        return false;
    }

    return true;
}

/** Cleanup and close out the BACnet/IP services by closing the socket.
 * @ingroup DLBIP
 */
void bip_cleanup(
    void)
{
    int sock_fd = 0;

    if (bip_valid()) {
        sock_fd = bip_socket();
        close(sock_fd);
    }
    bip_set_socket(-1);

    return;
}

/** Get the netmask of the BACnet/IP's interface via an getifaddrs() call.
 * @param netmask [out] The netmask, in host order.
 * @return 0 on success, else the error from the getifaddrs() call.
 */
int bip_get_local_netmask(
    struct in_addr *netmask)
{
    int rv;
    char *ifname = getenv("BACNET_IFACE");      /* will probably be null */
    if (ifname == NULL)
        ifname = "en0";
    printf("ifname %s", ifname);
    char *request = "netmask";
    rv = get_local_address(ifname, netmask, request);

    return rv;
}
