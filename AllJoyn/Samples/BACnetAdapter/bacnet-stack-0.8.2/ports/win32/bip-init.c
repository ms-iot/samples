/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg
 Contributions by Thomas Neumann in 2008.

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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include <stdbool.h>    /* for the standard bool type. */
#include "bacdcode.h"
#include "config.h"
#include "bip.h"
#include "net.h"

#if defined(_MSC_VER)
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#endif

bool BIP_Debug = false;

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

/* To fill a need, we invent the gethostaddr() function. */
static long gethostaddr(
    void)
{
    struct hostent *host_ent;
    char host_name[255];

    if (gethostname(host_name, sizeof(host_name)) != 0)
        return -1;

    if ((host_ent = gethostbyname(host_name)) == NULL)
        return -1;
    if (BIP_Debug) {
        printf("host: %s at %u.%u.%u.%u\n", host_name,
            (unsigned) ((uint8_t *) host_ent->h_addr)[0],
            (unsigned) ((uint8_t *) host_ent->h_addr)[1],
            (unsigned) ((uint8_t *) host_ent->h_addr)[2],
            (unsigned) ((uint8_t *) host_ent->h_addr)[3]);
    }
    /* note: network byte order */
    return *(long *) host_ent->h_addr;
}

#if (!defined(USE_INADDR) || (USE_INADDR == 0)) && \
 (!defined(USE_CLASSADDR) || (USE_CLASSADDR == 0))
/* returns the subnet mask in network byte order */
static uint32_t getIpMaskForIpAddress(
    uint32_t ipAddress)
{
    /* Allocate information for up to 16 NICs */
    IP_ADAPTER_INFO AdapterInfo[16];
    /* Save memory size of buffer */
    DWORD dwBufLen = sizeof(AdapterInfo);
    uint32_t ipMask = INADDR_BROADCAST;
    bool found = false;

    PIP_ADAPTER_INFO pAdapterInfo;

    /* GetAdapterInfo:
       [out] buffer to receive data
       [in] size of receive data buffer */
    DWORD dwStatus = GetAdaptersInfo(AdapterInfo,
        &dwBufLen);
    if (dwStatus == ERROR_SUCCESS) {
        /* Verify return value is valid, no buffer overflow
           Contains pointer to current adapter info */
        pAdapterInfo = AdapterInfo;

        do {
            IP_ADDR_STRING *pIpAddressInfo = &pAdapterInfo->IpAddressList;
            do {
                unsigned long adapterAddress =
                    inet_addr(pIpAddressInfo->IpAddress.String);
                unsigned long adapterMask =
                    inet_addr(pIpAddressInfo->IpMask.String);
                if (adapterAddress == ipAddress) {
                    ipMask = adapterMask;
                    found = true;
                }
                pIpAddressInfo = pIpAddressInfo->Next;
            } while (pIpAddressInfo && !found);
            /* Progress through linked list */
            pAdapterInfo = pAdapterInfo->Next;
            /* Terminate on last adapter */
        } while (pAdapterInfo && !found);
    }

    return ipMask;
}
#endif

static void set_broadcast_address(
    uint32_t net_address)
{
#if defined(USE_INADDR) && USE_INADDR
    /*   Note: sometimes INADDR_BROADCAST does not let me get
       any unicast messages.  Not sure why... */
    net_address = net_address;
    bip_set_broadcast_addr(INADDR_BROADCAST);
#elif defined(USE_CLASSADDR) && USE_CLASSADDR
    long broadcast_address = 0;

    if (IN_CLASSA(ntohl(net_address)))
        broadcast_address =
            (ntohl(net_address) & ~IN_CLASSA_HOST) | IN_CLASSA_HOST;
    else if (IN_CLASSB(ntohl(net_address)))
        broadcast_address =
            (ntohl(net_address) & ~IN_CLASSB_HOST) | IN_CLASSB_HOST;
    else if (IN_CLASSC(ntohl(net_address)))
        broadcast_address =
            (ntohl(net_address) & ~IN_CLASSC_HOST) | IN_CLASSC_HOST;
    else if (IN_CLASSD(ntohl(net_address)))
        broadcast_address =
            (ntohl(net_address) & ~IN_CLASSD_HOST) | IN_CLASSD_HOST;
    else
        broadcast_address = INADDR_BROADCAST;
    bip_set_broadcast_addr(htonl(broadcast_address));
#else
    /* these are network byte order variables */
    long broadcast_address = 0;
    long net_mask = 0;

    net_mask = getIpMaskForIpAddress(net_address);
    if (BIP_Debug) {
        struct in_addr address;
        address.s_addr = net_mask;
        printf("IP Mask: %s\n", inet_ntoa(address));
    }
    broadcast_address = (net_address & net_mask) | (~net_mask);
    bip_set_broadcast_addr(broadcast_address);
#endif
}

/* on Windows, ifname is the dotted ip address of the interface */
void bip_set_interface(
    char *ifname)
{
    struct in_addr address;

    /* setup local address */
    if (bip_get_addr() == 0) {
        bip_set_addr(inet_addr(ifname));
    }
    if (BIP_Debug) {
        address.s_addr = bip_get_addr();
        fprintf(stderr, "Interface: %s\n", ifname);
    }
    /* setup local broadcast address */
    if (bip_get_broadcast_addr() == 0) {
        address.s_addr = bip_get_addr();
        set_broadcast_address(address.s_addr);
    }
}

static char *winsock_error_code_text(
    int code)
{
    switch (code) {
        case WSAEACCES:
            return "Permission denied.";
        case WSAEINTR:
            return "Interrupted system call.";
        case WSAEBADF:
            return "Bad file number.";
        case WSAEFAULT:
            return "Bad address.";
        case WSAEINVAL:
            return "Invalid argument.";
        case WSAEMFILE:
            return "Too many open files.";
        case WSAEWOULDBLOCK:
            return "Operation would block.";
        case WSAEINPROGRESS:
            return "Operation now in progress. "
                "This error is returned if any Windows Sockets API "
                "function is called while a blocking function "
                "is in progress.";
        case WSAENOTSOCK:
            return "Socket operation on nonsocket.";
        case WSAEDESTADDRREQ:
            return "Destination address required.";
        case WSAEMSGSIZE:
            return "Message too long.";
        case WSAEPROTOTYPE:
            return "Protocol wrong type for socket.";
        case WSAENOPROTOOPT:
            return "Protocol not available.";
        case WSAEPROTONOSUPPORT:
            return "Protocol not supported.";
        case WSAESOCKTNOSUPPORT:
            return "Socket type not supported.";
        case WSAEOPNOTSUPP:
            return "Operation not supported on socket.";
        case WSAEPFNOSUPPORT:
            return "Protocol family not supported.";
        case WSAEAFNOSUPPORT:
            return "Address family not supported by protocol family.";
        case WSAEADDRINUSE:
            return "Address already in use.";
        case WSAEADDRNOTAVAIL:
            return "Cannot assign requested address.";
        case WSAENETDOWN:
            return "Network is down. "
                "This error may be reported at any time "
                "if the Windows Sockets implementation "
                "detects an underlying failure.";
        case WSAENETUNREACH:
            return "Network is unreachable.";
        case WSAENETRESET:
            return "Network dropped connection on reset.";
        case WSAECONNABORTED:
            return "Software caused connection abort.";
        case WSAECONNRESET:
            return "Connection reset by peer.";
        case WSAENOBUFS:
            return "No buffer space available.";
        case WSAEISCONN:
            return "Socket is already connected.";
        case WSAENOTCONN:
            return "Socket is not connected.";
        case WSAESHUTDOWN:
            return "Cannot send after socket shutdown.";
        case WSAETOOMANYREFS:
            return "Too many references: cannot splice.";
        case WSAETIMEDOUT:
            return "Connection timed out.";
        case WSAECONNREFUSED:
            return "Connection refused.";
        case WSAELOOP:
            return "Too many levels of symbolic links.";
        case WSAENAMETOOLONG:
            return "File name too long.";
        case WSAEHOSTDOWN:
            return "Host is down.";
        case WSAEHOSTUNREACH:
            return "No route to host.";
        case WSASYSNOTREADY:
            return "Returned by WSAStartup(), "
                "indicating that the network subsystem is unusable.";
        case WSAVERNOTSUPPORTED:
            return "Returned by WSAStartup(), "
                "indicating that the Windows Sockets DLL cannot support "
                "this application.";
        case WSANOTINITIALISED:
            return "Winsock not initialized. "
                "This message is returned by any function "
                "except WSAStartup(), "
                "indicating that a successful WSAStartup() has not yet "
                "been performed.";
        case WSAEDISCON:
            return "Disconnect.";
        case WSAHOST_NOT_FOUND:
            return "Host not found. " "This message indicates that the key "
                "(name, address, and so on) was not found.";
        case WSATRY_AGAIN:
            return "Nonauthoritative host not found. "
                "This error may suggest that the name service itself "
                "is not functioning.";
        case WSANO_RECOVERY:
            return "Nonrecoverable error. "
                "This error may suggest that the name service itself "
                "is not functioning.";
        case WSANO_DATA:
            return "Valid name, no data record of requested type. "
                "This error indicates that the key "
                "(name, address, and so on) was not found.";
        default:
            return "unknown";
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
 * @note For Windows, ifname is the dotted ip address of the interface.
 *
 * @param ifname [in] The named interface to use for the network layer.
 *        If NULL, the "eth0" interface is assigned.
 * @return True if the socket is successfully opened for BACnet/IP,
 *         else False if the socket functions fail.
 */
bool bip_init(
    char *ifname)
{
    int rv = 0; /* return from socket lib calls */
    struct sockaddr_in sin = { -1 };
    int value = 1;
    int sock_fd = -1;
    int Result;
    int Code;
    WSADATA wd;
    struct in_addr address;
    struct in_addr broadcast_address;

    Result = WSAStartup((1 << 8) | 1, &wd);
    /*Result = WSAStartup(MAKEWORD(2,2), &wd); */
    if (Result != 0) {
        Code = WSAGetLastError();
        printf("TCP/IP stack initialization failed\n" " error code: %i %s\n",
            Code, winsock_error_code_text(Code));
        exit(1);
    }
    atexit(bip_cleanup);

    if (ifname)
        bip_set_interface(ifname);
    /* has address been set? */
    address.s_addr = bip_get_addr();
    if (address.s_addr == 0) {
        address.s_addr = gethostaddr();
        if (address.s_addr == (unsigned) -1) {
            Code = WSAGetLastError();
            printf("Get host address failed\n" " error code: %i %s\n", Code,
                winsock_error_code_text(Code));
            exit(1);
        }
        bip_set_addr(address.s_addr);
    }
    if (BIP_Debug) {
        fprintf(stderr, "IP Address: %s\n", inet_ntoa(address));
    }
    /* has broadcast address been set? */
    if (bip_get_broadcast_addr() == 0) {
        set_broadcast_address(address.s_addr);
    }
    if (BIP_Debug) {
        broadcast_address.s_addr = bip_get_broadcast_addr();
        fprintf(stderr, "IP Broadcast Address: %s\n",
            inet_ntoa(broadcast_address));
        fprintf(stderr, "UDP Port: 0x%04X [%hu]\n", ntohs(bip_get_port()),
            ntohs(bip_get_port()));
    }
    /* assumes that the driver has already been initialized */
    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bip_set_socket(sock_fd);
    if (sock_fd < 0) {
        fprintf(stderr, "bip: failed to allocate a socket.\n");
        return false;
    }
    /* Allow us to use the same socket for sending and receiving */
    /* This makes sure that the src port is correct when sending */
    rv = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value,
        sizeof(value));
    if (rv < 0) {
        fprintf(stderr, "bip: failed to set REUSEADDR socket option.\n");
        close(sock_fd);
        bip_set_socket(-1);
        return false;
    }
    /* Enables transmission and receipt of broadcast messages on the socket. */
    rv = setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, (char *) &value,
        sizeof(value));
    if (rv < 0) {
        fprintf(stderr, "bip: failed to set BROADCAST socket option.\n");
        close(sock_fd);
        bip_set_socket(-1);
        return false;
    }
#if 0
    /* probably only for Apple... */
    /* rebind a port that is already in use.
       Note: all users of the port must specify this flag */
    rv = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, (char *) &value,
        sizeof(value));
    if (rv < 0) {
        fprintf(stderr, "bip: failed to set REUSEPORT socket option.\n");
        close(sock_fd);
        bip_set_socket(-1);
        return false;
    }
#endif
    /* bind the socket to the local port number and IP address */
    sin.sin_family = AF_INET;
#if defined(USE_INADDR) && USE_INADDR
    /* by setting sin.sin_addr.s_addr to INADDR_ANY,
       I am telling the IP stack to automatically fill
       in the IP address of the machine the process
       is running on.

       Some server computers have multiple IP addresses.
       A socket bound to one of these will not accept
       connections to another address. Frequently you prefer
       to allow any one of the computer's IP addresses
       to be used for connections.  Use INADDR_ANY (0L) to
       allow clients to connect using any one of the host's
       IP addresses. */
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
#else
    /* or we could use the specific adapter address
       note: already in network byte order */
    sin.sin_addr.s_addr = address.s_addr;
#endif
    sin.sin_port = bip_get_port();
    memset(&(sin.sin_zero), '\0', sizeof(sin.sin_zero));
    rv = bind(sock_fd, (const struct sockaddr *) &sin,
        sizeof(struct sockaddr));
    if (rv < 0) {
        fprintf(stderr, "bip: failed to bind to %s port %hu\n",
            inet_ntoa(sin.sin_addr), ntohs(bip_get_port()));
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
    WSACleanup();

    return;
}
