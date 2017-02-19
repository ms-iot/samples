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

static int interface = SOCKET_ERROR;    /* SOCKET_ERROR means no open interface */

/*-----------------------------------*/
static void Error(
    const char *Msg)
{
    int Code = WSAGetLastError();
#ifdef HOST
    printf("%s, error code: %i\n", Msg, Code);
#else
    printf("%s, error code: %s\n", Msg, xn_geterror_string(Code));
#endif
    exit(1);
}

#ifndef HOST
/*-----------------------------------*/
void InterfaceCleanup(
    void)
{
    if (interface != SOCKET_ERROR) {
        xn_interface_close(interface);
        interface = SOCKET_ERROR;
#if DEVICE_ID == PRISM_PCMCIA_DEVICE
        RTPCShutDown();
#endif
    }
}

static void NetInitialize(
    void)
/* initialize the TCP/IP stack */
{
    int Result;

#ifndef HOST
    if (!RTKDebugVersion())     /* switch of all diagnostics and error messages of RTIP-32 */
        xn_callbacks()->cb_wr_screen_string_fnc = NULL;

#ifdef RTUSB_VER
    RTURegisterCallback(USBAX172);      /* ax172 and ax772 drivers */
    RTURegisterCallback(USBAX772);
    RTURegisterCallback(USBKeyboard);   /* support USB keyboards */
    FindUSBControllers();       /* install USB host controllers */
    Sleep(2000);        /* give the USB stack time to enumerate devices */
#endif

#ifdef DHCP
    XN_REGISTER_DHCP_CLI()      /* and optionally the DHCP client */
#endif
        Result = xn_rtip_init();        /* Initialize the RTIP stack */
    if (Result != 0)
        Error("xn_rtip_init failed");

    atexit(InterfaceCleanup);   /* make sure the driver is shut down properly */
    RTCallDebugger(RT_DBG_CALLRESET, (DWORD) exit, 0);  /* even if we get restarted by the debugger */

    Result = BIND_DRIVER(MINOR_0);      /* tell RTIP what Ethernet driver we want (see netcfg.h) */
    if (Result != 0)
        Error("driver initialization failed");

#if DEVICE_ID == PRISM_PCMCIA_DEVICE
    /* if this is a PCMCIA device, start the PCMCIA driver */
    if (RTPCInit(-1, 0, 2, NULL) == 0)
        Error("No PCMCIA controller found");
#endif

    /* Open the interface */
    interface =
        xn_interface_open_config(DEVICE_ID, MINOR_0, ED_IO_ADD, ED_IRQ,
        ED_MEM_ADD);
    if (interface == SOCKET_ERROR)
        Error("xn_interface_open_config failed");
    else {
        struct _iface_info ii;
#ifdef BACDL_ETHERNET
        BACNET_ADDRESS my_address;
        unsigned i;
#endif
        xn_interface_info(interface, &ii);
        printf
            ("Interface opened, MAC address: %02x-%02x-%02x-%02x-%02x-%02x\n",
            ii.my_ethernet_address[0], ii.my_ethernet_address[1],
            ii.my_ethernet_address[2], ii.my_ethernet_address[3],
            ii.my_ethernet_address[4], ii.my_ethernet_address[5]);
#ifdef BACDL_ETHERNET
        for (i = 0; i < 6; i++) {
            my_address.mac[i] = ii.my_ethernet_address[i];
        }
        ethernet_set_my_address(&my_address);
#endif
    }

#if DEVICE_ID == PRISM_PCMCIA_DEVICE || DEVICE_ID == PRISM_DEVICE
    xn_wlan_setup(interface,    /* iface_no: value returned by xn_interface_open_config() */
        "network name", /* SSID    : network name set in the access point */
        "station name", /* Name    : name of this node */
        0,      /* Channel : 0 for access points, 1..14 for ad-hoc */
        0,      /* KeyIndex: 0 .. 3 */
        "12345",        /* WEP Key : key to use (5 or 13 bytes) */
        0);     /* Flags   : see manual and Wlanapi.h for details */
    Sleep(1000);        /* wireless devices need a little time before they can be used */
#endif /* WLAN device */

#if defined(AUTO_IP)    /* use xn_autoip() to get an IP address */
    Result = xn_autoip(interface, MinIP, MaxIP, NetMask, TargetIP);
    if (Result == SOCKET_ERROR)
        Error("xn_autoip failed");
    else {
        printf("Auto-assigned IP address %i.%i.%i.%i\n", TargetIP[0],
            TargetIP[1], TargetIP[2], TargetIP[3]);
        /* define default gateway and DNS server */
        xn_rt_add(RT_DEFAULT, ip_ffaddr, DefaultGateway, 1, interface, RT_INF);
        xn_set_server_list((DWORD *) DNSServer, 1);
    }
#elif defined(DHCP)     /* use DHCP */
    {
        DHCP_param param[] = { {SUBNET_MASK, 1}
        , {DNS_OP, 1}
        , {ROUTER_OPTION, 1}
        };
        DHCP_session DS;
        DHCP_conf DC;

        xn_init_dhcp_conf(&DC); /* load default DHCP options */
        DC.plist = param;       /* add MASK, DNS, and gateway options */
        DC.plist_entries = sizeof(param) / sizeof(param[0]);
        printf("Contacting DHCP server, please wait...\n");
        Result = xn_dhcp(interface, &DS, &DC);  /* contact DHCP server */
        if (Result == SOCKET_ERROR)
            Error("xn_dhcp failed");
        memcpy(TargetIP, DS.client_ip, 4);
        printf("My IP address is: %i.%i.%i.%i\n", TargetIP[0], TargetIP[1],
            TargetIP[2], TargetIP[3]);
    }
#else
    /* Set the IP address and interface */
    printf("Using static IP address %i.%i.%i.%i\n", TargetIP[0], TargetIP[1],
        TargetIP[2], TargetIP[3]);
    Result = xn_set_ip(interface, TargetIP, NetMask);
    /* define default gateway and DNS server */
    xn_rt_add(RT_DEFAULT, ip_ffaddr, DefaultGateway, 1, interface, RT_INF);
    xn_set_server_list((DWORD *) DNSServer, 1);
#endif

#else /* HOST defined, run on Windows */

    WSADATA wd;
    Result = WSAStartup(0x0101, &wd);

#endif

    if (Result != 0)
        Error("TCP/IP stack initialization failed");
}
#endif

/******************************************************************
* DESCRIPTION:  Converts the byte stored address to an inet address
* RETURN:       none
* ALGORITHM:    none
* NOTES:        none
******************************************************************/
static void RTIP_To_Network_Address(
    BYTE * octet_address,
    struct in_addr *addr)
{
    uint32_t ip_address = 0;    /* for decoding the subnet mask */

    decode_unsigned32(octet_address, &ip_address);
    addr->s_addr = htonl(ip_address);

    return;
}

static void set_broadcast_address(
    uint32_t net_address)
{
    long broadcast_address = 0;
    long mask = 0;

    /*   Note: sometimes INADDR_BROADCAST does not let me get
       any unicast messages.  Not sure why... */
#if USE_INADDR
    (void) net_address;
    bip_set_broadcast_addr(INADDR_BROADCAST);
#else
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
#endif
}

bool bip_init(
    char *ifname)
{
    int rv = 0; /* return from socket lib calls */
    struct sockaddr_in sin = { -1 };
    int value = 1;
    int sock_fd = -1;
    struct in_addr my_addr;

    (void) ifname;

    NetInitialize();

    RTIP_To_Network_Address(TargetIP, &my_addr);
    bip_set_addr(my_addr.s_addr);
    set_broadcast_address(my_addr.s_addr);
    bip_set_port(htons((0xBAC0));
        /* assumes that the driver has already been initialized */
        sock_fd = socket(AF_INET, SOCK_DGRAM, IPROTO_UDP);
        bip_set_socket(sock_fd);
        if (sock_fd < 0)
        return false;
        /* bind the socket to the local port number and IP address */
        sin.sin_family = AF_INET; sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sin.sin_port = bip_get_port(); memset(&(sin.sin_zero), '\0', 8);
        rv =
        bind(sock_fd, (const struct sockaddr *) &sin, sizeof(struct sockaddr));
        if (rv < 0) {
        close(sock_fd); bip_set_socket(-1); return false;}

    return true;}
