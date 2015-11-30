/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg, modified by Kevin Liao

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
#include <assert.h>
#include <direct.h>
#include <io.h>

#include "bacdef.h"
#include "ethernet.h"
#include "bacdcode.h"


/* Uses WinPCap to access raw ethernet */
/* Notes:                                               */
/* To make ethernet.c work under win32, you have to:    */
/* 1. install winpcap 3.1 development pack;             */
/* 2. install Microsoft Platform SDK Feb 2003.          */
/* 3. remove or modify functions used for log such as   */
/* "LogError()", "LogInfo()", which were implemented    */
/*  as a wrapper of Log4cpp.                            */
/* -- Kevin Liao                                        */

/* includes for accessing ethernet by using winpcap */
#include "pcap.h"
#include "packet32.h"
#include "ntddndis.h"
#include "remote-ext.h"


/* commonly used comparison address for ethernet */
uint8_t Ethernet_Broadcast[MAX_MAC_LEN] =
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
/* commonly used empty address for ethernet quick compare */
uint8_t Ethernet_Empty_MAC[MAX_MAC_LEN] = { 0, 0, 0, 0, 0, 0 };

/* my local device data - MAC address */
uint8_t Ethernet_MAC_Address[MAX_MAC_LEN] = { 0 };

/* couple of var for using winpcap */
static char pcap_errbuf[PCAP_ERRBUF_SIZE + 1];
static pcap_t *pcap_eth802_fp = NULL;   /* 802.2 file handle, from winpcap */
static unsigned eth_timeout = 100;


/* couple of external func for runtime error logging, you can simply    */
/* replace them with standard "printf(...)"                             */
/* Logging extern functions: Info level */
extern void LogInfo(
    const char *msg);
/* Logging extern functions: Error level*/
extern void LogError(
    const char *msg);
/* Logging extern functions: Debug level*/
extern void LogDebug(
    const char *msg);


bool ethernet_valid(
    void)
{
    return (pcap_eth802_fp != NULL);
}

void ethernet_cleanup(
    void)
{
    if (pcap_eth802_fp) {
        pcap_close(pcap_eth802_fp);
        pcap_eth802_fp = NULL;
    }
    LogInfo("ethernet.c: ethernet_cleanup() ok.\n");
}

void ethernet_set_timeout(
    unsigned timeout)
{
    eth_timeout = timeout;
}

/*----------------------------------------------------------------------
 Portable function to set a socket into nonblocking mode.
 Calling this on a socket causes all future read() and write() calls on
 that socket to do only as much as they can immediately, and return 
 without waiting.
 If no data can be read or written, they return -1 and set errno
 to EAGAIN (or EWOULDBLOCK).
 Thanks to Bjorn Reese for this code.
----------------------------------------------------------------------*/
/**
 * We don't need to use this function since WinPCap has provided one
 *   named "pcap_setnonblock()".
 * Kevin, 2006.08.15
 */
/*
int setNonblocking(int fd)
{
    int flags;

    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
*/

bool ethernet_init(
    char *if_name)
{
    PPACKET_OID_DATA pOidData;
    LPADAPTER lpAdapter;
    pcap_if_t *pcap_all_if;
    pcap_if_t *dev;
    BOOLEAN result;
    CHAR str[sizeof(PACKET_OID_DATA) + 128];
    int i;
    char msgBuf[200];

    if (ethernet_valid())
        ethernet_cleanup();

    /**
     * Find the interface user specified
     */
    /* Retrieve the device list */
    if (pcap_findalldevs(&pcap_all_if, pcap_errbuf) == -1) {
        sprintf(msgBuf, "ethernet.c: error in pcap_findalldevs: %s\n",
            pcap_errbuf);
        LogError(msgBuf);
        return false;
    }
    /* Scan the list printing every entry */
    for (dev = pcap_all_if; dev; dev = dev->next) {
        if (strcmp(if_name, dev->name) == 0)
            break;
    }
    pcap_freealldevs(pcap_all_if);      /* we don't need it anymore */
    if (dev == NULL) {
        sprintf(msgBuf, "ethernet.c: specified interface not found: %s\n",
            if_name);
        LogError(msgBuf);
        return false;
    }

    /**
     * Get local MAC address
     */
    ZeroMemory(str, sizeof(PACKET_OID_DATA) + 128);
    lpAdapter = PacketOpenAdapter(if_name);
    if (lpAdapter == NULL) {
        ethernet_cleanup();
        sprintf(msgBuf, "ethernet.c: error in PacketOpenAdapter(\"%s\")\n",
            if_name);
        LogError(msgBuf);
        return false;
    }
    pOidData = (PPACKET_OID_DATA) str;
    pOidData->Oid = OID_802_3_CURRENT_ADDRESS;
    pOidData->Length = 6;
    result = PacketRequest(lpAdapter, FALSE, pOidData);
    if (!result) {
        PacketCloseAdapter(lpAdapter);
        ethernet_cleanup();
        LogError("ethernet.c: error in PacketRequest()\n");
        return false;
    }
    for (i = 0; i < 6; ++i)
        Ethernet_MAC_Address[i] = pOidData->Data[i];
    PacketCloseAdapter(lpAdapter);

    /**
     * Open interface for subsequent sending and receiving
     */
    /* Open the output device */
    pcap_eth802_fp = pcap_open(if_name, /* name of the device */
        MAX_MPDU,       /* portion of the packet to capture */
        PCAP_OPENFLAG_PROMISCUOUS,      /* promiscuous mode */
        eth_timeout,    /* read timeout */
        NULL,   /* authentication on the remote machine */
        pcap_errbuf     /* error buffer */
        );
    if (pcap_eth802_fp == NULL) {
        PacketCloseAdapter(lpAdapter);
        ethernet_cleanup();
        sprintf(msgBuf,
            "ethernet.c: unable to open the adapter. %s is not supported by WinPcap\n",
            if_name);
        LogError(msgBuf);
        return false;
    }

    LogInfo("ethernet.c: ethernet_init() ok.\n");

    atexit(ethernet_cleanup);

    return ethernet_valid();
}

/* function to send a packet out the 802.2 socket */
/* returns bytes sent success, negative on failure */
int ethernet_send(
    BACNET_ADDRESS * dest,      /* destination address */
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* any data to be sent - may be null */
    unsigned pdu_len    /* number of bytes of data */
    )
{
    int bytes = 0;
    uint8_t mtu[MAX_MPDU] = { 0 };
    int mtu_len = 0;
    int i = 0;

    /* don't waste time if the socket is not valid */
    if (!ethernet_valid()) {
        LogError("ethernet.c: invalid 802.2 ethernet interface descriptor!\n");
        return -1;
    }
    /* load destination ethernet MAC address */
    if (dest->mac_len == 6) {
        for (i = 0; i < 6; i++) {
            mtu[mtu_len] = dest->mac[i];
            mtu_len++;
        }
    } else {
        LogError("ethernet.c: invalid destination MAC address!\n");
        return -2;
    }

    /* load source ethernet MAC address */
    if (src->mac_len == 6) {
        for (i = 0; i < 6; i++) {
            mtu[mtu_len] = src->mac[i];
            mtu_len++;
        }
    } else {
        LogError("ethernet.c: invalid source MAC address!\n");
        return -3;
    }
    if ((14 + 3 + pdu_len) > MAX_MPDU) {
        LogError("ethernet.c: PDU is too big to send!\n");
        return -4;
    }
    /* packet length */
    mtu_len += encode_unsigned16(&mtu[12], 3 /*DSAP,SSAP,LLC */  + pdu_len);
    /* Logical PDU portion */
    mtu[mtu_len++] = 0x82;      /* DSAP for BACnet */
    mtu[mtu_len++] = 0x82;      /* SSAP for BACnet */
    mtu[mtu_len++] = 0x03;      /* Control byte in header */
    memcpy(&mtu[mtu_len], pdu, pdu_len);
    mtu_len += pdu_len;

    /* Send the packet */
    if (pcap_sendpacket(pcap_eth802_fp, mtu, mtu_len) != 0) {
        /* did it get sent? */
        char msgBuf[200];
        sprintf(msgBuf, "ethernet.c: error sending packet: %s\n",
            pcap_geterr(pcap_eth802_fp));
        LogError(msgBuf);
        return -5;
    }

    return mtu_len;
}

/* function to send a packet out the 802.2 socket */
/* returns number of bytes sent on success, negative on failure */
int ethernet_send_pdu(
    BACNET_ADDRESS * dest,      /* destination address */
    uint8_t * pdu,      /* any data to be sent - may be null */
    unsigned pdu_len    /* number of bytes of data */
    )
{
    int i = 0;  /* counter */
    BACNET_ADDRESS src = { 0 }; /* source address */

    for (i = 0; i < 6; i++) {
        src.mac[i] = Ethernet_MAC_Address[i];
        src.mac_len++;
    }
    /* function to send a packet out the 802.2 socket */
    /* returns 1 on success, 0 on failure */
    return ethernet_send(dest,  /* destination address */
        &src,   /* source address */
        pdu,    /* any data to be sent - may be null */
        pdu_len /* number of bytes of data */
        );
}

/* receives an 802.2 framed packet */
/* returns the number of octets in the PDU, or zero on failure */
uint16_t ethernet_receive(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* PDU data */
    uint16_t max_pdu,   /* amount of space available in the PDU  */
    unsigned timeout    /* number of milliseconds to wait for a packet. we ommit it due to winpcap API. */
    )
{
    struct pcap_pkthdr *header;
    int res;
    u_char *pkt_data;
    uint16_t pdu_len = 0;       /* return value */

    /* Make sure the socket is open */
    if (!ethernet_valid()) {
        LogError("ethernet.c: invalid 802.2 ethernet interface descriptor!\n");
        return 0;
    }

    /* Capture a packet */
    res = pcap_next_ex(pcap_eth802_fp, &header, &pkt_data);
    if (res < 0) {
        char msgBuf[200];
        sprintf(msgBuf, "ethernet.c: error in receiving packet: %s\n",
            pcap_geterr(pcap_eth802_fp));
        return 0;
    } else if (res == 0)
        return 0;

    if (header->len == 0 || header->caplen == 0)
        return 0;

    /* the signature of an 802.2 BACnet packet */
    if ((pkt_data[14] != 0x82) && (pkt_data[15] != 0x82)) {
        /*eth_log_error("ethernet.c: Non-BACnet packet\n"); */
        return 0;
    }
    /* copy the source address */
    src->mac_len = 6;
    memmove(src->mac, &pkt_data[6], 6);

    /* check destination address for when */
    /* the Ethernet card is in promiscious mode */
    if ((memcmp(&pkt_data[0], Ethernet_MAC_Address, 6) != 0)
        && (memcmp(&pkt_data[0], Ethernet_Broadcast, 6) != 0)) {
        /*eth_log_error( "ethernet.c: This packet isn't for us\n"); */
        return 0;
    }

    (void) decode_unsigned16(&pkt_data[12], &pdu_len);
    pdu_len -= 3 /* DSAP, SSAP, LLC Control */ ;
    /* copy the buffer into the PDU */
    if (pdu_len < max_pdu)
        memmove(&pdu[0], &pkt_data[17], pdu_len);
    /* ignore packets that are too large */
    else
        pdu_len = 0;

    return pdu_len;
}

void ethernet_set_my_address(
    BACNET_ADDRESS * my_address)
{
    int i = 0;

    for (i = 0; i < 6; i++) {
        Ethernet_MAC_Address[i] = my_address->mac[i];
    }

    return;
}

void ethernet_get_my_address(
    BACNET_ADDRESS * my_address)
{
    int i = 0;

    my_address->mac_len = 0;
    for (i = 0; i < 6; i++) {
        my_address->mac[i] = Ethernet_MAC_Address[i];
        my_address->mac_len++;
    }
    my_address->net = 0;        /* local only, no routing */
    my_address->len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        my_address->adr[i] = 0;
    }

    return;
}

void ethernet_get_broadcast_address(
    BACNET_ADDRESS * dest)
{       /* destination address */
    int i = 0;  /* counter */

    if (dest) {
        for (i = 0; i < 6; i++) {
            dest->mac[i] = Ethernet_Broadcast[i];
        }
        dest->mac_len = 6;
        dest->net = BACNET_BROADCAST_NETWORK;
        dest->len = 0;  /* denotes broadcast address  */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            dest->adr[i] = 0;
        }
    }

    return;
}

void ethernet_debug_address(
    const char *info,
    BACNET_ADDRESS * dest)
{
    int i = 0;  /* counter */
    char msgBuf[200];

    if (info) {
        sprintf(msgBuf, "%s", info);
        LogError(msgBuf);
    }
    /* if */
    if (dest) {
        sprintf(msgBuf, "Address:\n  MAC Length=%d\n  MAC Address=",
            dest->mac_len);
        LogInfo(msgBuf);
        for (i = 0; i < MAX_MAC_LEN; i++) {
            sprintf(msgBuf, "%02X ", (unsigned) dest->mac[i]);
            LogInfo(msgBuf);
        }       /* for */
        LogInfo("\n");
        sprintf(msgBuf, "  Net=%hu\n  Len=%d\n  Adr=", dest->net, dest->len);
        LogInfo(msgBuf);
        for (i = 0; i < MAX_MAC_LEN; i++) {
            sprintf(msgBuf, "%02X ", (unsigned) dest->adr[i]);
            LogInfo(msgBuf);
        }       /* for */
        LogInfo("\n");
    }
    /* if ( dest ) */
    return;
}
