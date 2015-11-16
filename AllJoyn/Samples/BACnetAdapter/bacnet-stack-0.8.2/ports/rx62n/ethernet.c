/**************************************************************************
*
* Copyright (C) 2011 Steve Karg <skarg@users.sourceforge.net>
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
#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include <stdbool.h>    /* for the standard bool type. */
#include "bacdef.h"
#include "ethernet.h"
#include "bacint.h"
#include "hardware.h"

/** @file rx62n/ethernet.c  Provides Renesas RX62N-specific functions
    for BACnet/Ethernet. */

/* commonly used comparison address for ethernet */
static uint8_t Ethernet_Broadcast[MAX_MAC_LEN] =
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/* IEEE maintains list of 48-bit MAC "addresses" AKA EUI-48 identifiers.
   An EUI-48 is structured into an initial 3-octet OUI
   (Organizationally Unique Identifier) and an additional 3 octets
   assigned by the OUI holder. */
/* see [RFC5342] for current information and registration procedures. */
/* The OUI 00-00-5E has been allocated to IANA. */
/* my local device data - MAC address */
static uint8_t Ethernet_MAC_Address[MAX_MAC_LEN] =
    { 0x00, 0x00, 0x5E, 0x00, 0x00, 0x01 };

/* status of the link */
static int32_t Ethernet_Status = R_ETHER_ERROR;

bool ethernet_valid(
    void)
{
    if (Ethernet_Status != R_ETHER_OK) {
        Ethernet_Status = R_Ether_Open(0, Ethernet_MAC_Address);
    }

    return (Ethernet_Status != R_ETHER_ERROR);
}

void ethernet_cleanup(
    void)
{
    R_Ether_Close(0);
    Ethernet_Status = R_ETHER_ERROR;

    return;
}

bool ethernet_init(
    char *interface_name)
{
    interface_name = interface_name;
    Ethernet_Status = R_Ether_Open(0, Ethernet_MAC_Address);

    return (Ethernet_Status == R_ETHER_OK);
}

int ethernet_send(
    uint8_t * mtu,
    int mtu_len)
{
    int bytes = 0;

    /* Send the packet */
    bytes = R_Ether_Write(0, mtu, mtu_len);

    return bytes;

}

/* function to send a packet out the 802.2 socket */
/* returns number of bytes sent on success, negative on failure */
int ethernet_send_pdu(
    BACNET_ADDRESS * dest,      /* destination address */
    BACNET_NPDU_DATA * npdu_data,       /* network information */
    uint8_t * pdu,      /* any data to be sent - may be null */
    unsigned pdu_len)
{       /* number of bytes of data */
    int i = 0;  /* counter */
    int bytes = 0;
    BACNET_ADDRESS src = { 0 }; /* source address for npdu */
    uint8_t mtu[MAX_MPDU] = { 0 };      /* our buffer */
    int mtu_len = 0;

    (void) npdu_data;
    /* load the BACnet address for NPDU data */
    for (i = 0; i < 6; i++) {
        src.mac[i] = Ethernet_MAC_Address[i];
        src.mac_len++;
    }

    /* don't waste time if the socket is not valid */
    if (!ethernet_valid()) {
        return -1;
    }
    /* load destination ethernet MAC address */
    if (dest->mac_len == 6) {
        for (i = 0; i < 6; i++) {
            mtu[i] = dest->mac[i];
        }
    } else {
        return -2;
    }

    /* load source ethernet MAC address */
    if (src.mac_len == 6) {
        for (i = 0; i < 6; i++) {
            mtu[6 + i] = src.mac[i];
        }
    } else {
        return -3;
    }
    /* Logical PDU portion */
    mtu[14] = 0x82;     /* DSAP for BACnet */
    mtu[15] = 0x82;     /* SSAP for BACnet */
    mtu[16] = 0x03;     /* Control byte in header */
    mtu_len = 17;
    if ((mtu_len + pdu_len) > MAX_MPDU) {
        return -4;
    }
    memcpy(&mtu[mtu_len], pdu, pdu_len);
    mtu_len += pdu_len;
    /* packet length - only the logical portion, not the address */
    encode_unsigned16(&mtu[12], 3 + pdu_len);

    /* Send the packet */
    bytes = R_Ether_Write(0, mtu, mtu_len);

    return bytes;
}

/* receives an 802.2 framed packet */
/* returns the number of octets in the PDU, or zero on failure */
uint16_t ethernet_receive(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* PDU data */
    uint16_t max_pdu,   /* amount of space available in the PDU  */
    unsigned timeout)
{       /* number of milliseconds to wait for a packet */
    int received_bytes;
    uint8_t buf[MAX_MPDU] = { 0 };      /* data */
    uint16_t pdu_len = 0;       /* return value */

    /* Make sure the socket is open */
    if (!ethernet_valid())
        return 0;

    received_bytes = R_Ether_Read(0, (void *) buf);

    if (received_bytes == 0)
        return 0;

    /* the signature of an 802.2 BACnet packet */
    if ((buf[14] != 0x82) && (buf[15] != 0x82)) {
        return 0;
    }
    /* copy the source address */
    src->mac_len = 6;
    memmove(src->mac, &buf[6], 6);

    /* check destination address for when */
    /* the Ethernet card is in promiscious mode */
    if ((memcmp(&buf[0], Ethernet_MAC_Address, 6) != 0)
        && (memcmp(&buf[0], Ethernet_Broadcast, 6) != 0)) {
        return 0;
    }

    (void) decode_unsigned16(&buf[12], &pdu_len);
    pdu_len -= 3 /* DSAP, SSAP, LLC Control */ ;
    /* copy the buffer into the PDU */
    if (pdu_len < max_pdu)
        memmove(&pdu[0], &buf[17], pdu_len);
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
    my_address->net = 0;        /* DNET=0 is local only, no routing */
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
        dest->len = 0;  /* always zero when DNET is broadcast */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            dest->adr[i] = 0;
        }
    }

    return;
}
