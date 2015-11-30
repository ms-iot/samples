/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2006 Steve Karg

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
#include <time.h>
#include "bacenum.h"
#include "bacdcode.h"
#include "bacint.h"
#include "bvlc.h"
#ifndef DEBUG_ENABLED
#define DEBUG_ENABLED 0
#endif
#include "debug.h"

/** @file bvlc.c  Handle the BACnet Virtual Link Control (BVLC),
 * which includes: BACnet Broadcast Management Device,
 * Broadcast Distribution Table, and
 * Foreign Device Registration.
 */

/** if we are a foreign device, store the
   remote BBMD address/port here in network byte order */
static struct sockaddr_in Remote_BBMD;

/** Global IP address for NAT handling */
static struct in_addr BVLC_Global_Address;

/** Flag to indicate if NAT handling is enabled/disabled */
static bool BVLC_NAT_Handling = false;

/** result from a client request */
BACNET_BVLC_RESULT BVLC_Result_Code = BVLC_RESULT_SUCCESSFUL_COMPLETION;

/** The current BVLC Function Code being handled. */
BACNET_BVLC_FUNCTION BVLC_Function_Code = BVLC_RESULT;  /* A safe default */

/* Define BBMD_ENABLED to get the functions that a
 * BBMD needs to handle its services.
 * Separately, define BBMD_CLIENT_ENABLED to get the
 * functions that allow a client to manage a BBMD.
 */
#if defined(BBMD_ENABLED) && BBMD_ENABLED


#ifndef MAX_BBMD_ENTRIES
#define MAX_BBMD_ENTRIES 128
#endif
static BBMD_TABLE_ENTRY BBMD_Table[MAX_BBMD_ENTRIES];

/*Each device that registers as a foreign device shall be placed
in an entry in the BBMD's Foreign Device Table (FDT). Each
entry shall consist of the 6-octet B/IP address of the registrant;
the 2-octet Time-to-Live value supplied at the time of
registration; and a 2-octet value representing the number of
seconds remaining before the BBMD will purge the registrant's FDT
entry if no re-registration occurs. This value will be initialized
to the 2-octet Time-to-Live value supplied at the time of
registration.*/
typedef struct {
    bool valid;
    /* BACnet/IP address */
    struct in_addr dest_address;
    /* BACnet/IP port number - not always 47808=BAC0h */
    uint16_t dest_port;
    /* seconds for valid entry lifetime */
    uint16_t time_to_live;
    /* our counter */
    time_t seconds_remaining;   /* includes 30 second grace period */
} FD_TABLE_ENTRY;

#ifndef MAX_FD_ENTRIES
#define MAX_FD_ENTRIES 128
#endif
static FD_TABLE_ENTRY FD_Table[MAX_FD_ENTRIES];


/** A timer function that is called about once a second.
 *
 * @param seconds - number of elapsed seconds since the last call
 */
void bvlc_maintenance_timer(
    time_t seconds)
{
    unsigned i = 0;

    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (FD_Table[i].valid) {
            if (FD_Table[i].seconds_remaining) {
                if (FD_Table[i].seconds_remaining < seconds) {
                    FD_Table[i].seconds_remaining = 0;
                } else {
                    FD_Table[i].seconds_remaining -= seconds;
                }
                if (FD_Table[i].seconds_remaining == 0) {
                    FD_Table[i].valid = false;
                }
            }
        }
    }
}

/** Copy the source internet address to the BACnet address
 *
 * FIXME: IPv6?
 *
 * @param src - returns the BACnet source address
 * @param sin - source address in network order
 *
 * @return number of bytes decoded
 */
static void bvlc_internet_to_bacnet_address(
    BACNET_ADDRESS * src,
    struct sockaddr_in *sin)
{
    if (src && sin) {
        memcpy(&src->mac[0], &sin->sin_addr.s_addr, 4);
        memcpy(&src->mac[4], &sin->sin_port, 2);
        src->mac_len = (uint8_t) 6;
        src->net = 0;
        src->len = 0;
    }

    return;
}

/** Encode the address entry.  Used for both read and write entries.
 *
 * Addressing within B/IP Networks
 * In the case of B/IP networks, six octets consisting of the four-octet
 * IP address followed by a two-octet UDP port number (both of
 * which shall be transmitted most significant octet first).
 * Note: for local storage, the storage order is NETWORK byte order.
 * Note: BACnet unsigned is encoded as most significant octet.
 *
 * @param pdu - buffer to extract encoded address
 * @param address - address in network order
 * @param port - UDP port number in network order
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_bip_address(
    uint8_t * pdu,
    struct in_addr *address,
    uint16_t port)
{
    int len = 0;

    if (pdu) {
        memcpy(&pdu[0], &address->s_addr, 4);
        memcpy(&pdu[4], &port, 2);
        len = 6;
    }

    return len;
}

/** Decode the address entry.  Used for both read and write entries.
 *
 * @param pdu - buffer to extract encoded address
 * @param address - address in network order
 * @param port - UDP port number in network order
 *
 * @return number of bytes decoded
 */
static int bvlc_decode_bip_address(
    uint8_t * pdu,
    struct in_addr *address,
    uint16_t * port)
{
    int len = 0;

    if (pdu) {
        memcpy(&address->s_addr, &pdu[0], 4);
        memcpy(port, &pdu[4], 2);
        len = 6;
    }

    return len;
}

/** Encode the address entry.  Used for both read and write entries.
 *
 * @param pdu - buffer to store the encoding
 * @param address - address in network order
 * @param port - UDP port number in network order
 * @param mask - address mask in network order
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_address_entry(
    uint8_t * pdu,
    struct in_addr *address,
    uint16_t port,
    struct in_addr *mask)
{
    int len = 0;

    if (pdu) {
        len = bvlc_encode_bip_address(pdu, address, port);
        memcpy(&pdu[len], &mask->s_addr, 4);
        len += 4;
    }

    return len;
}
#endif


/** Encode the BVLC Result message
 *
 * @param pdu - buffer to store the encoding
 * @param result_code - BVLC result code
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_bvlc_result(
    uint8_t * pdu,
    BACNET_BVLC_RESULT result_code)
{
    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_RESULT;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], 6);
        encode_unsigned16(&pdu[4], (uint16_t) result_code);
    }

    return 6;
}

#if defined(BBMD_CLIENT_ENABLED) && BBMD_CLIENT_ENABLED
/** Encode the initial part of the Read-Broadcast-Distribution-Table message
 *
 * @param pdu - buffer to store the encoding
 * @param entries - number of BDT entries
 *
 * @return number of bytes encoded
 */
int bvlc_encode_write_bdt_init(
    uint8_t * pdu,
    unsigned entries)
{
    int len = 0;
    uint16_t BVLC_length = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_WRITE_BROADCAST_DISTRIBUTION_TABLE;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        BVLC_length = 4 + (uint16_t) (entries * 10);
        encode_unsigned16(&pdu[2], BVLC_length);
        len = 4;
    }

    return len;
}
#endif

/** Encode a Read-Broadcast-Distribution-Table message
 *
 * @param pdu - buffer to store the encoding
 *
 * @return number of bytes encoded
 */
int bvlc_encode_read_bdt(
    uint8_t * pdu)
{
    int len = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_READ_BROADCAST_DIST_TABLE;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], 4);
        len = 4;
    }

    return len;
}

/**
 * Read the Read-Broadcast-Distribution-Table of a BBMD
 *
 * @param bbmd_address - IPv4 address (long) of BBMD to read,
 *  in network byte order.
 * @param bbmd_port - Network port of BBMD to read, in network byte order
 * @return Upon successful completion, returns the number of bytes sent.
 *  Otherwise, -1 shall be returned and errno set to indicate the error.
 */
int bvlc_bbmd_read_bdt(
    uint32_t bbmd_address,
    uint16_t bbmd_port)
{
    uint8_t mtu[MAX_MPDU] = { 0 };
    uint16_t mtu_len = 0;
    int rv = 0;
    struct sockaddr_in bbmd = { 0 };

    mtu_len = bvlc_encode_read_bdt(mtu);
    if (mtu_len > 0) {
        bbmd.sin_addr.s_addr = bbmd_address;
        bbmd.sin_port = bbmd_port;
        rv = bvlc_send_mpdu(&bbmd, &mtu[0], mtu_len);
    }

    return rv;
}

#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Encode the initial part of the Read BDT Ack message
 *
 * @param pdu - buffer to store the encoding
 * @param entries - number of BDT entries
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_read_bdt_ack_init(
    uint8_t * pdu,
    unsigned entries)
{
    int len = 0;
    uint16_t BVLC_length = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_READ_BROADCAST_DIST_TABLE_ACK;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        BVLC_length = 4 + (uint16_t) (entries * 10);
        encode_unsigned16(&pdu[2], BVLC_length);
        len = 4;
    }

    return len;
}

/** Encode a Read BDT Ack message
 *
 * @param pdu - buffer to store the encoding
 * @param max_pdu - size of the buffer to store the encoding
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_read_bdt_ack(
    uint8_t * pdu,
    uint16_t max_pdu)
{
    int pdu_len = 0;    /* return value */
    int len = 0;
    unsigned count = 0;
    unsigned i;

    for (i = 0; i < MAX_BBMD_ENTRIES; i++) {
        if (BBMD_Table[i].valid) {
            count++;
        }
    }
    len = bvlc_encode_read_bdt_ack_init(&pdu[0], count);
    pdu_len += len;
    for (i = 0; i < MAX_BBMD_ENTRIES; i++) {
        if (BBMD_Table[i].valid) {
            /* too much to send */
            if ((pdu_len + 10) > max_pdu) {
                pdu_len = 0;
                break;
            }
            len =
                bvlc_encode_address_entry(&pdu[pdu_len],
                &BBMD_Table[i].dest_address, BBMD_Table[i].dest_port,
                &BBMD_Table[i].broadcast_mask);
            pdu_len += len;
        }
    }

    return pdu_len;
}

/** Encode a Forwarded NPDU message
 *
 * @param pdu - buffer to store the encoding
 * @param sin - source address in network order
 * @param npdu - NPDU to forward
 * @param npdu_length - size of the NPDU to forward
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_forwarded_npdu(
    uint8_t * pdu,
    struct sockaddr_in *sin,
    uint8_t * npdu,
    unsigned npdu_length)
{
    int len = 0;

    unsigned i; /* for loop counter */

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_FORWARDED_NPDU;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], (uint16_t) (4 + 6 + npdu_length));
        len = 4;
        len +=
            bvlc_encode_bip_address(&pdu[len], &sin->sin_addr, sin->sin_port);
        for (i = 0; i < npdu_length; i++) {
            pdu[len] = npdu[i];
            len++;
        }
    }

    return len;
}
#endif

#if defined(BBMD_CLIENT_ENABLED) && BBMD_CLIENT_ENABLED
/** Encode a Read Foreign Device Table message
 *
 * @param pdu - buffer to store the encoding
 *
 * @return number of bytes encoded
 */
int bvlc_encode_read_fdt(
    uint8_t * pdu)
{
    int len = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_READ_FOREIGN_DEVICE_TABLE;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], 4);
        len = 4;
    }

    return len;
}
#endif


#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Encode the initial part of a Read Foreign Device Table Ack
 *
 * @param pdu - buffer to store the encoding
 * @param entries - number of foreign device entries in this Ack.
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_read_fdt_ack_init(
    uint8_t * pdu,
    unsigned entries)
{
    int len = 0;
    uint16_t BVLC_length = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_READ_FOREIGN_DEVICE_TABLE_ACK;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        BVLC_length = 4 + (uint16_t) (entries * 10);
        encode_unsigned16(&pdu[2], BVLC_length);
        len = 4;
    }

    return len;
}

/** Encode a Read Foreign Device Table Ack
 *
 * @param pdu - buffer to store the encoding
 * @param max_pdu - number of bytes available to encode
 *
 * @return number of bytes encoded
 */
static int bvlc_encode_read_fdt_ack(
    uint8_t * pdu,
    uint16_t max_pdu)
{
    int pdu_len = 0;    /* return value */
    int len = 0;
    unsigned count = 0;
    unsigned i;
    uint16_t seconds_remaining = 0;

    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (FD_Table[i].valid) {
            count++;
        }
    }
    len = bvlc_encode_read_fdt_ack_init(&pdu[0], count);
    pdu_len += len;
    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (FD_Table[i].valid) {
            /* too much to send */
            if ((pdu_len + 10) > max_pdu) {
                pdu_len = 0;
                break;
            }
            len =
                bvlc_encode_bip_address(&pdu[pdu_len],
                &FD_Table[i].dest_address, FD_Table[i].dest_port);
            pdu_len += len;
            len = encode_unsigned16(&pdu[pdu_len], FD_Table[i].time_to_live);
            pdu_len += len;
            seconds_remaining = (uint16_t) FD_Table[i].seconds_remaining;
            len = encode_unsigned16(&pdu[pdu_len], seconds_remaining);
            pdu_len += len;
        }
    }

    return pdu_len;
}
#endif


#if defined(BBMD_CLIENT_ENABLED) && BBMD_CLIENT_ENABLED
/** Encode an Foreign Device Table entry
 *
 * @param pdu - buffer to store the encoding
 * @param address - in network byte order
 * @param port - in network byte order
 *
 * @return number of bytes encoded
 */
int bvlc_encode_delete_fdt_entry(
    uint8_t * pdu,
    uint32_t address,
    uint16_t port)
{
    int len = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_DELETE_FOREIGN_DEVICE_TABLE_ENTRY;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], 10);
        /* FDT Entry */
        encode_unsigned32(&pdu[4], address);
        encode_unsigned16(&pdu[8], port);
        len = 10;
    }

    return len;
}
#endif

#if defined(BBMD_CLIENT_ENABLED) && BBMD_CLIENT_ENABLED
/** Encode an Original Unicast NPDU
 *
 * @param pdu - buffer to store the encoding
 * @param npdu - NPDU portion of message
 * @param npdu_length - number of bytes to encode
 *
 * @return number of bytes encoded
 */
int bvlc_encode_original_unicast_npdu(
    uint8_t * pdu,
    uint8_t * npdu,
    unsigned npdu_length)
{
    int len = 0;        /* return value */
    unsigned i = 0;     /* loop counter */
    uint16_t BVLC_length = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        BVLC_length = 4 + (uint16_t) npdu_length;
        len = encode_unsigned16(&pdu[2], BVLC_length) + 2;
        for (i = 0; i < npdu_length; i++) {
            pdu[len] = npdu[i];
            len++;
        }
    }

    return len;
}
#endif

#if defined(BBMD_CLIENT_ENABLED) && BBMD_CLIENT_ENABLED
/** Encode an Original Broadcast NPDU
 *
 * @param pdu - buffer to store the encoding
 * @param npdu - NPDU portion of message
 * @param npdu_length - number of bytes to encode
 *
 * @return number of bytes encoded
 */
int bvlc_encode_original_broadcast_npdu(
    uint8_t * pdu,
    uint8_t * npdu,
    unsigned npdu_length)
{
    int len = 0;        /* return value */
    unsigned i = 0;     /* loop counter */
    uint16_t BVLC_length = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        BVLC_length = 4 + (uint16_t) npdu_length;
        len = encode_unsigned16(&pdu[2], BVLC_length) + 2;
        for (i = 0; i < npdu_length; i++) {
            pdu[len] = npdu[i];
            len++;
        }
    }

    return len;
}
#endif


#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Create a Broadcast Distribution Table from message
 *
 * @param npdu - message from which the devices are decoded
 * @param npdu_length - number of bytes to decode
 *
 * @return true if all the entries fit in the table
 */
static bool bvlc_create_bdt(
    uint8_t * npdu,
    uint16_t npdu_length)
{
    bool status = false;
    unsigned i = 0;
    uint16_t pdu_offset = 0;

    for (i = 0; i < MAX_BBMD_ENTRIES; i++) {
        if (npdu_length >= 10) {
            BBMD_Table[i].valid = true;
            memcpy(&BBMD_Table[i].dest_address.s_addr, &npdu[pdu_offset], 4);
            pdu_offset += 4;
            memcpy(&BBMD_Table[i].dest_port, &npdu[pdu_offset], 2);
            pdu_offset += 2;
            memcpy(&BBMD_Table[i].broadcast_mask.s_addr, &npdu[pdu_offset], 4);
            pdu_offset += 4;
            npdu_length -= (4 + 2 + 4);
        } else {
            BBMD_Table[i].valid = false;
            BBMD_Table[i].dest_address.s_addr = 0;
            BBMD_Table[i].dest_port = 0;
            BBMD_Table[i].broadcast_mask.s_addr = 0;
        }
    }
    /* did they all fit? */
    if (npdu_length < 10) {
        status = true;
    }

    return status;
}

/** Register a Foreign Device in the Foreign Device Table
 *
 * @param sin - source address in network order
 * @param time_to_live - time in seconds
 *
 * @return true if the Foreign Device was added
 */
static bool bvlc_register_foreign_device(
    struct sockaddr_in *sin,
    uint16_t time_to_live)
{
    unsigned i = 0;
    bool status = false;

    /* am I here already?  If so, update my time to live... */
    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (FD_Table[i].valid) {
            if ((FD_Table[i].dest_address.s_addr == sin->sin_addr.s_addr) &&
                (FD_Table[i].dest_port == sin->sin_port)) {
                status = true;
                FD_Table[i].time_to_live = time_to_live;
                /*  Upon receipt of a BVLL Register-Foreign-Device message,
                   a BBMD shall start a timer with a value equal to the
                   Time-to-Live parameter supplied plus a fixed grace
                   period of 30 seconds. */
                FD_Table[i].seconds_remaining = time_to_live + 30;
                break;
            }
        }
    }
    if (!status) {
        for (i = 0; i < MAX_FD_ENTRIES; i++) {
            if (!FD_Table[i].valid) {
                FD_Table[i].dest_address.s_addr = sin->sin_addr.s_addr;
                FD_Table[i].dest_port = sin->sin_port;
                FD_Table[i].time_to_live = time_to_live;
                FD_Table[i].seconds_remaining = time_to_live + 30;
                FD_Table[i].valid = true;
                status = true;
                break;
            }
        }
    }


    return status;
}

/** Delete a Foreign Device from the Foreign Device Table
 *
 * @param pdu - BACnet/IP address in PDU form
 *
 * @return true if the Foreign Device was found and removed.
 */
static bool bvlc_delete_foreign_device(
    uint8_t * pdu)
{
    struct sockaddr_in sin = { 0 };     /* the ip address */
    bool status = false;        /* return value */
    unsigned i = 0;

    bvlc_decode_bip_address(pdu, &sin.sin_addr, &sin.sin_port);
    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (FD_Table[i].valid) {
            if ((FD_Table[i].dest_address.s_addr == sin.sin_addr.s_addr) &&
                (FD_Table[i].dest_port == sin.sin_port)) {
                FD_Table[i].valid = false;
                FD_Table[i].seconds_remaining = 0;
                status = true;
                break;
            }
        }
    }
    return status;
}
#endif

/**
 * The common send function for bvlc functions, using b/ip.
 *
 * @param dest - Points to a sockaddr_in structure containing the
 *  destination address. The length and format of the address depend
 *  on the address family of the socket (AF_INET).
 *  The address is in network byte order.
 * @param mtu - the bytes of data to send
 * @param mtu_len - the number of bytes of data to send
 * @return Upon successful completion, returns the number of bytes sent.
 *  Otherwise, -1 shall be returned and errno set to indicate the error.
 */
int bvlc_send_mpdu(
    struct sockaddr_in *dest,
    uint8_t * mtu,
    uint16_t mtu_len)
{
    struct sockaddr_in bvlc_dest = { 0 };

    /* assumes that the driver has already been initialized */
    if (bip_socket() < 0) {
        return 0;
    }
    /* load destination IP address */
    bvlc_dest.sin_family = AF_INET;
    bvlc_dest.sin_addr.s_addr = dest->sin_addr.s_addr;
    bvlc_dest.sin_port = dest->sin_port;
    memset(&(bvlc_dest.sin_zero), '\0', 8);
    /* Send the packet */
    return sendto(bip_socket(), (char *) mtu, mtu_len, 0,
        (struct sockaddr *) &bvlc_dest, sizeof(struct sockaddr));
}

#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Sends all Broadcast Devices a Forwarded NPDU
 *
 * @param sin - source address in network order
 * @param npdu - the NPDU
 * @param npdu_length - length of the NPDU
 * @param original - was the message an original (not forwarded)
 */
static void bvlc_bdt_forward_npdu(
    struct sockaddr_in *sin,
    uint8_t * npdu,
    uint16_t npdu_length,
    bool original)
{
    uint8_t mtu[MAX_MPDU] = { 0 };
    uint16_t mtu_len = 0;
    unsigned i = 0;     /* loop counter */
    struct sockaddr_in bip_dest = { 0 };

    /* If we are forwarding an original broadcast message and the NAT
     * handling is enabled, change the source address to NAT routers
     * global IP address so the recipient can reply (local IP address
     * is not accesible from internet side).
     *
     * If we are forwarding a message from peer BBMD or foreign device
     * or the NAT handling is disabled, leave the source address as is.
     */
    if(BVLC_NAT_Handling && original) {
        struct sockaddr_in nat_addr = *sin;
        nat_addr.sin_addr = BVLC_Global_Address;
        mtu_len = (uint16_t) bvlc_encode_forwarded_npdu(&mtu[0],
                             &nat_addr, npdu, npdu_length);
    }
    else {
        mtu_len = (uint16_t) bvlc_encode_forwarded_npdu(&mtu[0],
                             sin, npdu, npdu_length);
    }

    /* loop through the BDT and send one to each entry, except us */
    for (i = 0; i < MAX_BBMD_ENTRIES; i++) {
        if (BBMD_Table[i].valid) {
            /* The B/IP address to which the Forwarded-NPDU message is
               sent is formed by inverting the broadcast distribution
               mask in the BDT entry and logically ORing it with the
               BBMD address of the same entry. */
            bip_dest.sin_addr.s_addr =
                ((~BBMD_Table[i].broadcast_mask.
                    s_addr) | BBMD_Table[i].dest_address.s_addr);
            bip_dest.sin_port = BBMD_Table[i].dest_port;
            /* don't send to my broadcast address and same port */
            if ((bip_dest.sin_addr.s_addr == bip_get_broadcast_addr())
                && (bip_dest.sin_port == bip_get_port())) {
                continue;
            }
            /* don't send to my ip address and same port */
            if ((bip_dest.sin_addr.s_addr == bip_get_addr()) &&
                (bip_dest.sin_port == bip_get_port())) {
                continue;
            }
            /* NAT router port forwards BACnet packets from global IP to us.
             * Packets sent to that global IP by us would end up back, creating
             * a loop.
             */
            if (BVLC_NAT_Handling &&
                (bip_dest.sin_addr.s_addr == BVLC_Global_Address.s_addr) &&
                (bip_dest.sin_port == bip_get_port())) {
                continue;
            }
            bvlc_send_mpdu(&bip_dest, mtu, mtu_len);
            debug_printf("BVLC: BDT Sent Forwarded-NPDU to %s:%04X\n",
                inet_ntoa(bip_dest.sin_addr), ntohs(bip_dest.sin_port));
        }
    }

    return;
}

/** Send a BVLL Forwarded-NPDU message on its local IP subnet using
 * the local B/IP broadcast address as the destination address.
 *
 * @param sin - source address in network order
 * @param npdu - the NPDU
 * @param npdu_length - amount of space available in the NPDU
 */
static void bvlc_forward_npdu(
    struct sockaddr_in *sin,
    uint8_t * npdu,
    uint16_t npdu_length)
{
    uint8_t mtu[MAX_MPDU] = { 0 };
    uint16_t mtu_len = 0;
    struct sockaddr_in bip_dest = { 0 };

    mtu_len =
        (uint16_t) bvlc_encode_forwarded_npdu(&mtu[0], sin, npdu, npdu_length);
    bip_dest.sin_addr.s_addr = bip_get_broadcast_addr();
    bip_dest.sin_port = bip_get_port();
    bvlc_send_mpdu(&bip_dest, mtu, mtu_len);
    debug_printf("BVLC: Sent Forwarded-NPDU as local broadcast.\n");
}

/** Sends all Foreign Devices a Forwarded NPDU
 *
 * @param sin - source address in network order
 * @param npdu - returns the NPDU
 * @param max_npdu - amount of space available in the NPDU
 * @param original - was the message an original (not forwarded)
 */
static void bvlc_fdt_forward_npdu(
    struct sockaddr_in *sin,
    uint8_t * npdu,
    uint16_t max_npdu,
    bool original)
{
    uint8_t mtu[MAX_MPDU] = { 0 };
    uint16_t mtu_len = 0;
    unsigned i = 0;     /* loop counter */
    struct sockaddr_in bip_dest = { 0 };

    /* If we are forwarding an original broadcast message and the NAT
     * handling is enabled, change the source address to NAT routers
     * global IP address so the recipient can reply (local IP address
     * is not accesible from internet side.
     *
     * If we are forwarding a message from peer BBMD or foreign device
     * or the NAT handling is disabled, leave the source address as is.
     */
    if(BVLC_NAT_Handling && original) {
        struct sockaddr_in nat_addr = *sin;
        nat_addr.sin_addr = BVLC_Global_Address;
        mtu_len = (uint16_t)bvlc_encode_forwarded_npdu(&mtu[0],
                             &nat_addr, npdu, max_npdu);
    } else {
        mtu_len = (uint16_t)bvlc_encode_forwarded_npdu(&mtu[0],
                             sin, npdu, max_npdu);
    }

    /* loop through the FDT and send one to each entry */
    for (i = 0; i < MAX_FD_ENTRIES; i++) {
        if (FD_Table[i].valid && FD_Table[i].seconds_remaining) {
            bip_dest.sin_addr.s_addr = FD_Table[i].dest_address.s_addr;
            bip_dest.sin_port = FD_Table[i].dest_port;
            /* don't send to my ip address and same port */
            if ((bip_dest.sin_addr.s_addr == bip_get_addr()) &&
                (bip_dest.sin_port == bip_get_port())) {
                continue;
            }
            /* don't send to src ip address and same port */
            if ((bip_dest.sin_addr.s_addr == sin->sin_addr.s_addr) &&
                (bip_dest.sin_port == sin->sin_port)) {
                continue;
            }
            /* NAT router port forwards BACnet packets from global IP to us.
             * Packets sent to that global IP by us would end up back, creating
             * a loop.
             */
            if (BVLC_NAT_Handling &&
                (bip_dest.sin_addr.s_addr == BVLC_Global_Address.s_addr) &&
                (bip_dest.sin_port == bip_get_port())) {
                continue;
            }
            bvlc_send_mpdu(&bip_dest, mtu, mtu_len);
            debug_printf("BVLC: FDT Sent Forwarded-NPDU to %s:%04X\n",
                inet_ntoa(bip_dest.sin_addr), ntohs(bip_dest.sin_port));
        }
    }

    return;
}
#endif


/** Sends a BVLC Result
 *
 * @param dest - destination address
 * @param result_code - result code to send
 *
 * @return number of bytes encoded to send
 */
static int bvlc_send_result(
    struct sockaddr_in *dest,   /* the destination address */
    BACNET_BVLC_RESULT result_code)
{
    uint8_t mtu[MAX_MPDU] = { 0 };
    uint16_t mtu_len = 0;

    mtu_len = (uint16_t) bvlc_encode_bvlc_result(&mtu[0], result_code);
    if (mtu_len) {
        bvlc_send_mpdu(dest, mtu, mtu_len);
    }

    return mtu_len;
}

#if defined(BBMD_ENABLED) && BBMD_ENABLED
/** Sends a Read Broadcast Device Table ACK
 *
 * @param dest - destination address
 *
 * @return number of bytes encoded to send
 */
static int bvlc_send_bdt(
    struct sockaddr_in *dest)
{
    uint8_t mtu[MAX_MPDU] = { 0 };
    uint16_t mtu_len = 0;

    mtu_len = (uint16_t) bvlc_encode_read_bdt_ack(&mtu[0], sizeof(mtu));
    if (mtu_len) {
        bvlc_send_mpdu(dest, &mtu[0], mtu_len);
    }

    return mtu_len;
}

/** Sends a Read Foreign Device Table ACK
 *
 * @param dest - destination address
 *
 * @return number of bytes encoded to send
 */
static int bvlc_send_fdt(
    struct sockaddr_in *dest)
{
    uint8_t mtu[MAX_MPDU] = { 0 };
    uint16_t mtu_len = 0;

    mtu_len = (uint16_t) bvlc_encode_read_fdt_ack(&mtu[0], sizeof(mtu));
    if (mtu_len) {
        bvlc_send_mpdu(dest, &mtu[0], mtu_len);
    }

    return mtu_len;
}

/** Determines if a BDT member has a unicast mask
 *
 * @param sin - BDT member that is sought, network byte order address
 *
 * @return True if BDT member is found and has a unicast mask
 */
static bool bvlc_bdt_member_mask_is_unicast(
    struct sockaddr_in *sin)
{
    bool unicast = false;
    unsigned i = 0;     /* loop counter */

    for (i = 0; i < MAX_BBMD_ENTRIES; i++) {
        if (BBMD_Table[i].valid) {

            /* Skip ourself*/
            if ((BBMD_Table[i].dest_address.s_addr == bip_get_addr()) &&
                (BBMD_Table[i].dest_port == bip_get_port())) {
                continue;
            }

            /* find the source address in the table */
            if ((BBMD_Table[i].dest_address.s_addr == sin->sin_addr.s_addr) &&
                (BBMD_Table[i].dest_port == sin->sin_port)) {
                /* unicast mask? */
                if (BBMD_Table[i].broadcast_mask.s_addr == 0xFFFFFFFFL) {
                    unicast = true;
                    break;
                }
            }
        }
    }

    return unicast;
}

/** Receive a packet from the BACnet/IP socket (Annex J)
 *
 * @param src - returns the source address
 * @param npdu - returns the NPDU
 * @param max_npdu - amount of space available in the NPDU
 * @param timeout - number of milliseconds to wait for a packet
 *
 * @return Number of bytes received, or 0 if none or timeout.
 */
uint16_t bvlc_receive(
    BACNET_ADDRESS * src,
    uint8_t * npdu,
    uint16_t max_npdu,
    unsigned timeout)
{
    uint16_t npdu_len = 0;      /* return value */
    fd_set read_fds;
    int max = 0;
    struct timeval select_timeout;
    struct sockaddr_in sin = { 0 };
    struct sockaddr_in original_sin = { 0 };
    struct sockaddr_in dest = { 0 };
    socklen_t sin_len = sizeof(sin);
    int received_bytes = 0;
    uint16_t result_code = 0;
    uint16_t i = 0;
    bool status = false;
    uint16_t time_to_live = 0;

    /* Make sure the socket is open */
    if (bip_socket() < 0) {
        return 0;
    }

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
    FD_SET(bip_socket(), &read_fds);
    max = bip_socket();
    /* see if there is a packet for us */
    if (select(max + 1, &read_fds, NULL, NULL, &select_timeout) > 0) {
        received_bytes =
            recvfrom(bip_socket(), (char *) &npdu[0], max_npdu, 0,
            (struct sockaddr *) &sin, &sin_len);
    } else {
        return 0;
    }
    /* See if there is a problem */
    if (received_bytes < 0) {
        return 0;
    }
    /* no problem, just no bytes */
    if (received_bytes == 0) {
        return 0;
    }
    /* the signature of a BACnet/IP packet */
    if (npdu[0] != BVLL_TYPE_BACNET_IP) {
        return 0;
    }
    BVLC_Function_Code = npdu[1];
    /* decode the length of the PDU - length is inclusive of BVLC */
    (void) decode_unsigned16(&npdu[2], &npdu_len);
    /* subtract off the BVLC header */
    npdu_len -= 4;
    switch (BVLC_Function_Code) {
        case BVLC_RESULT:
            /* Upon receipt of a BVLC-Result message containing a result code
               of X'0000' indicating the successful completion of the
               registration, a foreign device shall start a timer with a value
               equal to the Time-to-Live parameter of the preceding Register-
               Foreign-Device message. At the expiration of the timer, the
               foreign device shall re-register with the BBMD by sending a BVLL
               Register-Foreign-Device message */
            /* Clients can now get this result */
            (void) decode_unsigned16(&npdu[4], &result_code);
            BVLC_Result_Code = (BACNET_BVLC_RESULT) result_code;
            debug_printf("BVLC: Result Code=%d\n", BVLC_Result_Code);
            /* not an NPDU */
            npdu_len = 0;
            break;
        case BVLC_WRITE_BROADCAST_DISTRIBUTION_TABLE:
            debug_printf("BVLC: Received Write-BDT.\n");
            /* Upon receipt of a BVLL Write-Broadcast-Distribution-Table
               message, a BBMD shall attempt to create or replace its BDT,
               depending on whether or not a BDT has previously existed.
               If the creation or replacement of the BDT is successful, the BBMD
               shall return a BVLC-Result message to the originating device with
               a result code of X'0000'. Otherwise, the BBMD shall return a
               BVLC-Result message to the originating device with a result code
               of X'0010' indicating that the write attempt has failed. */
            status = bvlc_create_bdt(&npdu[4], npdu_len);
            if (status) {
                bvlc_send_result(&sin, BVLC_RESULT_SUCCESSFUL_COMPLETION);
            } else {
                bvlc_send_result(&sin,
                    BVLC_RESULT_WRITE_BROADCAST_DISTRIBUTION_TABLE_NAK);
            }
            /* not an NPDU */
            npdu_len = 0;
            break;
        case BVLC_READ_BROADCAST_DIST_TABLE:
            debug_printf("BVLC: Received Read-BDT.\n");
            /* Upon receipt of a BVLL Read-Broadcast-Distribution-Table
               message, a BBMD shall load the contents of its BDT into a BVLL
               Read-Broadcast-Distribution-Table-Ack message and send it to the
               originating device. If the BBMD is unable to perform the
               read of its BDT, it shall return a BVLC-Result message to the
               originating device with a result code of X'0020' indicating that
               the read attempt has failed. */
            if (bvlc_send_bdt(&sin) <= 0) {
                bvlc_send_result(&sin,
                    BVLC_RESULT_READ_BROADCAST_DISTRIBUTION_TABLE_NAK);
            }
            /* not an NPDU */
            npdu_len = 0;
            break;
        case BVLC_READ_BROADCAST_DIST_TABLE_ACK:
            debug_printf("BVLC: Received Read-BDT-Ack.\n");
            /* FIXME: complete the code for client side read */
            /* not an NPDU */
            npdu_len = 0;
            break;
        case BVLC_FORWARDED_NPDU:
            /* Upon receipt of a BVLL Forwarded-NPDU message, a BBMD shall
               process it according to whether it was received from a peer
               BBMD as the result of a directed broadcast or a unicast
               transmission. A BBMD may ascertain the method by which Forwarded-
               NPDU messages will arrive by inspecting the broadcast distribution
               mask field in its own BDT entry since all BDTs are required
               to be identical. If the message arrived via directed broadcast,
               it was also received by the other devices on the BBMD's subnet. In
               this case the BBMD merely retransmits the message directly to each
               foreign device currently in the BBMD's FDT. If the
               message arrived via a unicast transmission it has not yet been
               received by the other devices on the BBMD's subnet. In this case,
               the message is sent to the devices on the BBMD's subnet using the
               B/IP broadcast address as well as to each foreign device
               currently in the BBMD's FDT. A BBMD on a subnet with no other
               BACnet devices may omit the broadcast using the B/IP
               broadcast address. The method by which a BBMD determines whether
               or not other BACnet devices are present is a local matter. */
            /* decode the 4 byte original address and 2 byte port */
            bvlc_decode_bip_address(&npdu[4], &original_sin.sin_addr,
                &original_sin.sin_port);
            debug_printf("BVLC: Received Forwarded-NPDU from %s:%04X.\n",
                inet_ntoa(original_sin.sin_addr), ntohs(original_sin.sin_port));
            npdu_len -= 6;
            /*  Broadcast locally if received via unicast from a BDT member */
            if (bvlc_bdt_member_mask_is_unicast(&sin)) {
                dest.sin_addr.s_addr = bip_get_broadcast_addr();
                dest.sin_port = bip_get_port();
				debug_printf("BVLC: Received unicast from BDT member, re-broadcasting locally to %s:%04X.\n",
					inet_ntoa(dest.sin_addr), ntohs(dest.sin_port));
                bvlc_send_mpdu(&dest, &npdu[0], npdu_len+4+6);
            }
            /* use the original addr from the BVLC for src */
            dest.sin_addr.s_addr = original_sin.sin_addr.s_addr;
            dest.sin_port = original_sin.sin_port;
            bvlc_fdt_forward_npdu(&dest, &npdu[4 + 6], npdu_len, false);
            debug_printf("BVLC: Received Forwarded-NPDU from %s:%04X.\n",
                inet_ntoa(dest.sin_addr), ntohs(dest.sin_port));
            bvlc_internet_to_bacnet_address(src, &dest);
            if (npdu_len < max_npdu) {
                /* shift the buffer to return a valid PDU */
                for (i = 0; i < npdu_len; i++) {
                    npdu[i] = npdu[4 + 6 + i];
                }
            } else {
                /* ignore packets that are too large */
                /* clients should check my max-apdu first */
                npdu_len = 0;
            }
            break;
        case BVLC_REGISTER_FOREIGN_DEVICE:
            /* Upon receipt of a BVLL Register-Foreign-Device message, a BBMD
               shall start a timer with a value equal to the Time-to-Live
               parameter supplied plus a fixed grace period of 30 seconds. If,
               within the period during which the timer is active, another BVLL
               Register-Foreign-Device message from the same device is received,
               the timer shall be reset and restarted. If the time expires
               without the receipt of another BVLL Register-Foreign-Device
               message from the same foreign device, the FDT entry for this
               device shall be cleared. */
            (void) decode_unsigned16(&npdu[4], &time_to_live);
            if (bvlc_register_foreign_device(&sin, time_to_live)) {
                bvlc_send_result(&sin, BVLC_RESULT_SUCCESSFUL_COMPLETION);
                debug_printf("BVLC: Registered a Foreign Device.\n");
            } else {
                bvlc_send_result(&sin,
                    BVLC_RESULT_REGISTER_FOREIGN_DEVICE_NAK);
                debug_printf("BVLC: Failed to Register a Foreign Device.\n");
            }
            /* not an NPDU */
            npdu_len = 0;
            break;
        case BVLC_READ_FOREIGN_DEVICE_TABLE:
            debug_printf("BVLC: Received Read-FDT.\n");
            /* Upon receipt of a BVLL Read-Foreign-Device-Table message, a
               BBMD shall load the contents of its FDT into a BVLL Read-
               Foreign-Device-Table-Ack message and send it to the originating
               device. If the BBMD is unable to perform the read of its FDT,
               it shall return a BVLC-Result message to the originating device
               with a result code of X'0040' indicating that the read attempt has
               failed. */
            if (bvlc_send_fdt(&sin) <= 0) {
                bvlc_send_result(&sin,
                    BVLC_RESULT_READ_FOREIGN_DEVICE_TABLE_NAK);
            }
            /* not an NPDU */
            npdu_len = 0;
            break;
        case BVLC_READ_FOREIGN_DEVICE_TABLE_ACK:
            debug_printf("BVLC: Received Read-FDT-Ack.\n");
            /* FIXME: complete the code for client side read */
            /* not an NPDU */
            npdu_len = 0;
            break;
        case BVLC_DELETE_FOREIGN_DEVICE_TABLE_ENTRY:
            debug_printf("BVLC: Received Delete-FDT-Entry.\n");
            /* Upon receipt of a BVLL Delete-Foreign-Device-Table-Entry
               message, a BBMD shall search its foreign device table for an entry
               corresponding to the B/IP address supplied in the message. If an
               entry is found, it shall be deleted and the BBMD shall return a
               BVLC-Result message to the originating device with a result code
               of X'0000'. Otherwise, the BBMD shall return a BVLCResult
               message to the originating device with a result code of X'0050'
               indicating that the deletion attempt has failed. */
            if (bvlc_delete_foreign_device(&npdu[4])) {
                bvlc_send_result(&sin, BVLC_RESULT_SUCCESSFUL_COMPLETION);
            } else {
                bvlc_send_result(&sin,
                    BVLC_RESULT_DELETE_FOREIGN_DEVICE_TABLE_ENTRY_NAK);
            }
            /* not an NPDU */
            npdu_len = 0;
            break;
        case BVLC_DISTRIBUTE_BROADCAST_TO_NETWORK:
            debug_printf
                ("BVLC: Received Distribute-Broadcast-to-Network from %s:%04X.\n",
                inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
            /* Upon receipt of a BVLL Distribute-Broadcast-To-Network message
               from a foreign device, the receiving BBMD shall transmit a
               BVLL Forwarded-NPDU message on its local IP subnet using the
               local B/IP broadcast address as the destination address. In
               addition, a Forwarded-NPDU message shall be sent to each entry
               in its BDT as described in the case of the receipt of a
               BVLL Original-Broadcast-NPDU as well as directly to each foreign
               device currently in the BBMD's FDT except the originating
               node. If the BBMD is unable to perform the forwarding function,
               it shall return a BVLC-Result message to the foreign device
               with a result code of X'0060' indicating that the forwarding
               attempt was unsuccessful */
            bvlc_forward_npdu(&sin, &npdu[4], npdu_len);
            bvlc_bdt_forward_npdu(&sin, &npdu[4], npdu_len, false);
            bvlc_fdt_forward_npdu(&sin, &npdu[4], npdu_len, false);
            /* not an NPDU */
            npdu_len = 0;
            break;
        case BVLC_ORIGINAL_UNICAST_NPDU:
            debug_printf("BVLC: Received Original-Unicast-NPDU.\n");
            if ((sin.sin_addr.s_addr == bip_get_addr()) &&
                (sin.sin_port == bip_get_port())) {
                /* ignore messages from me */
                npdu_len = 0;
            } else if (BVLC_NAT_Handling &&
                     (sin.sin_addr.s_addr == BVLC_Global_Address.s_addr) &&
                     (sin.sin_port == bip_get_port())) {
                /* If the BBMD is behind a NAT router, the router forwards packets from
                   global IP and BACnet port to us. */
                npdu_len = 0;
            } else {
                bvlc_internet_to_bacnet_address(src, &sin);
                if (npdu_len < max_npdu) {
                    /* shift the buffer to return a valid PDU */
                    for (i = 0; i < npdu_len; i++) {
                        npdu[i] = npdu[4 + i];
                    }
                } else {
                    /* ignore packets that are too large */
                    /* clients should check my max-apdu first */
                    npdu_len = 0;
                }
            }
            break;
        case BVLC_ORIGINAL_BROADCAST_NPDU:
            debug_printf("BVLC: Received Original-Broadcast-NPDU.\n");
            /* Upon receipt of a BVLL Original-Broadcast-NPDU message,
               a BBMD shall construct a BVLL Forwarded-NPDU message and
               send it to each IP subnet in its BDT with the exception
               of its own. The B/IP address to which the Forwarded-NPDU
               message is sent is formed by inverting the broadcast
               distribution mask in the BDT entry and logically ORing it
               with the BBMD address of the same entry. This process
               produces either the directed broadcast address of the remote
               subnet or the unicast address of the BBMD on that subnet
               depending on the contents of the broadcast distribution
               mask. See J.4.3.2.. In addition, the received BACnet NPDU
               shall be sent directly to each foreign device currently in
               the BBMD's FDT also using the BVLL Forwarded-NPDU message. */
            bvlc_internet_to_bacnet_address(src, &sin);
            if (npdu_len < max_npdu) {
                /* shift the buffer to return a valid PDU */
                for (i = 0; i < npdu_len; i++) {
                    npdu[i] = npdu[4 + i];
                }
                /* if BDT or FDT entries exist, Forward the NPDU */
                bvlc_bdt_forward_npdu(&sin, &npdu[0], npdu_len, true);
                bvlc_fdt_forward_npdu(&sin, &npdu[0], npdu_len, true);
            } else {
                /* ignore packets that are too large */
                npdu_len = 0;
            }
            break;
        default:
            break;
    }

    return npdu_len;
}

/** Send a packet out the BACnet/IP socket (Annex J)
 *
 * @param dest - destination address
 * @param npdu_data - network information
 * @param pdu - any data to be sent - may be null
 * @param pdu_len - number of bytes of data
 *
 * @return returns number of bytes sent on success, negative number on failure
 */
int bvlc_send_pdu(
    BACNET_ADDRESS * dest,
    BACNET_NPDU_DATA * npdu_data,
    uint8_t * pdu,
    unsigned pdu_len)
{
    struct sockaddr_in bvlc_dest = { 0 };
    uint8_t mtu[MAX_MPDU] = { 0 };
    uint16_t mtu_len = 0;
    /* addr and port in network format */
    struct in_addr address;
    uint16_t port = 0;
    uint16_t BVLC_length = 0;

    /* bip datalink doesn't need to know the npdu data */
    (void) npdu_data;
    mtu[0] = BVLL_TYPE_BACNET_IP;
    /* handle various broadcasts: */
    /* mac_len = 0 is a broadcast address */
    /* net = 0 indicates local, net = 65535 indicates global */
    if ((dest->net == BACNET_BROADCAST_NETWORK) || (dest->mac_len == 0)) {
        /* if we are a foreign device */
        if (Remote_BBMD.sin_port) {
            mtu[1] = BVLC_DISTRIBUTE_BROADCAST_TO_NETWORK;
            address.s_addr = Remote_BBMD.sin_addr.s_addr;
            port = Remote_BBMD.sin_port;
            debug_printf("BVLC: Sent Distribute-Broadcast-to-Network.\n");
        } else {
            address.s_addr = bip_get_broadcast_addr();
            port = bip_get_port();
            mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
            debug_printf("BVLC: Sent Original-Broadcast-NPDU.\n");
        }
    } else if ((dest->net > 0) && (dest->len == 0)) {
        /* net > 0 and net < 65535 are network specific broadcast if len = 0 */
        if (dest->mac_len == 6) {
            /* network specific broadcast */
            bvlc_decode_bip_address(&dest->mac[0], &address, &port);
        } else {
            address.s_addr = bip_get_broadcast_addr();
            port = bip_get_port();
        }
        mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
        debug_printf("BVLC: Sent Original-Broadcast-NPDU.\n");
    } else if (dest->mac_len == 6) {
        /* valid unicast */
        bvlc_decode_bip_address(&dest->mac[0], &address, &port);
        mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
        debug_printf("BVLC: Sent Original-Unicast-NPDU.\n");
    } else {
        /* invalid address */
        return -1;
    }
    bvlc_dest.sin_addr.s_addr = address.s_addr;
    bvlc_dest.sin_port = port;
    BVLC_length = (uint16_t) pdu_len + 4 /*inclusive */ ;
    mtu_len = 2;
    mtu_len += (uint16_t) encode_unsigned16(&mtu[mtu_len], BVLC_length);
    memcpy(&mtu[mtu_len], pdu, pdu_len);
    mtu_len += (uint16_t) pdu_len;
    return bvlc_send_mpdu(&bvlc_dest, mtu, mtu_len);
}
#endif


/***********************************************
 * Functions to register us as a foreign device.
 ********************************************* */

/** Encode foreign device registration message
 *
 * @param pdu - bytes for encoding the message
 *                       in network byte order.
 * @param time_to_live_seconds - Lease time to use when registering.
 * @return Number of bytes encoded) on success,
 *         or 0 if no encoding occurred.
 */
static int bvlc_encode_register_foreign_device(
    uint8_t * pdu,
    uint16_t time_to_live_seconds)
{
    int len = 0;

    if (pdu) {
        pdu[0] = BVLL_TYPE_BACNET_IP;
        pdu[1] = BVLC_REGISTER_FOREIGN_DEVICE;
        /* The 2-octet BVLC Length field is the length, in octets,
           of the entire BVLL message, including the two octets of the
           length field itself, most significant octet first. */
        encode_unsigned16(&pdu[2], 6);
        encode_unsigned16(&pdu[4], time_to_live_seconds);
        len = 6;
    }

    return len;
}

/** Register as a foreign device with the indicated BBMD.
 * @param bbmd_address - IPv4 address (long) of BBMD to register with,
 *                       in network byte order.
 * @param bbmd_port - Network port of BBMD, in network byte order
 * @param time_to_live_seconds - Lease time to use when registering.
 * @return Positive number (of bytes sent) on success,
 *         0 if no registration request is sent, or
 *         -1 if registration fails.
 */
int bvlc_register_with_bbmd(
    uint32_t bbmd_address,
    uint16_t bbmd_port,
    uint16_t time_to_live_seconds)
{
    uint8_t mtu[MAX_MPDU] = { 0 };
    uint16_t mtu_len = 0;
    int retval = 0;

    /* Store the BBMD address and port so that we
       won't broadcast locally. */
    Remote_BBMD.sin_addr.s_addr = bbmd_address;
    Remote_BBMD.sin_port = bbmd_port;
    /* In order for their broadcasts to get here,
       we need to register our address with the remote BBMD using
       Write Broadcast Distribution Table, or
       register with the BBMD as a Foreign Device */
    mtu_len =
        (uint16_t) bvlc_encode_register_foreign_device(&mtu[0],
        time_to_live_seconds);
    retval = bvlc_send_mpdu(&Remote_BBMD, &mtu[0], mtu_len);
    return retval;
}


/** Note any BVLC_RESULT code, or NAK the BVLL message in the unsupported cases.
 * Use this handler when you are not a BBMD.
 * Sets the BVLC_Function_Code in case it is needed later.
 *
 * @param sout  [in] Socket address to send any NAK back to.
 * @param npdu  [in] The received buffer.
 * @param received_bytes [in] How many bytes in npdu[].
 * @return Non-zero BVLC_RESULT_ code if we sent a response (NAK) to this
 *      BVLC message.  If zero, may need further processing.
 */
int bvlc_for_non_bbmd(
    struct sockaddr_in *sout,
    uint8_t * npdu,
    uint16_t received_bytes)
{
    uint16_t result_code = 0;   /* aka, BVLC_RESULT_SUCCESSFUL_COMPLETION */

    BVLC_Function_Code = npdu[1];       /* The BVLC function */
    switch (BVLC_Function_Code) {
        case BVLC_RESULT:
            if (received_bytes >= 6) {
                /* This is the result of our foreign device registration */
                (void) decode_unsigned16(&npdu[4], &result_code);
                BVLC_Result_Code = (BACNET_BVLC_RESULT) result_code;
                debug_printf("BVLC: Result Code=%d\n", BVLC_Result_Code);
                /* But don't send any response */
                result_code = 0;
            }
            break;
        case BVLC_WRITE_BROADCAST_DISTRIBUTION_TABLE:
            result_code = BVLC_RESULT_WRITE_BROADCAST_DISTRIBUTION_TABLE_NAK;
            break;
        case BVLC_READ_BROADCAST_DIST_TABLE:
            result_code = BVLC_RESULT_READ_BROADCAST_DISTRIBUTION_TABLE_NAK;
            break;
            /* case BVLC_READ_BROADCAST_DIST_TABLE_ACK: */
        case BVLC_REGISTER_FOREIGN_DEVICE:
            result_code = BVLC_RESULT_REGISTER_FOREIGN_DEVICE_NAK;
            break;
        case BVLC_READ_FOREIGN_DEVICE_TABLE:
            result_code = BVLC_RESULT_READ_FOREIGN_DEVICE_TABLE_NAK;
            break;
            /* case BVLC_READ_FOREIGN_DEVICE_TABLE_ACK: */
        case BVLC_DELETE_FOREIGN_DEVICE_TABLE_ENTRY:
            result_code = BVLC_RESULT_DELETE_FOREIGN_DEVICE_TABLE_ENTRY_NAK;
            break;
        case BVLC_DISTRIBUTE_BROADCAST_TO_NETWORK:
            result_code = BVLC_RESULT_DISTRIBUTE_BROADCAST_TO_NETWORK_NAK;
            break;
            /* case BVLC_FORWARDED_NPDU: */
            /* case BVLC_ORIGINAL_UNICAST_NPDU: */
            /* case BVLC_ORIGINAL_BROADCAST_NPDU: */
        default:
            break;
    }
    if (result_code > 0) {
        bvlc_send_result(sout, result_code);
        debug_printf("BVLC: NAK code=%d\n", result_code);
    }

    return result_code;
}

/** Returns the last BVLL Result we received, either as the result of a BBMD
 * request we sent, or (if not a BBMD or Client), from trying to register
 * as a foreign device.
 *
 * @return BVLC_RESULT_SUCCESSFUL_COMPLETION on success,
 * BVLC_RESULT_REGISTER_FOREIGN_DEVICE_NAK if registration failed,
 * or one of the other codes (if we are a BBMD).
 */
BACNET_BVLC_RESULT bvlc_get_last_result(
    void)
{
    return BVLC_Result_Code;
}

/** Returns the current BVLL Function Code we are processing.
 * We have to store this higher layer code for when the lower layers
 * need to know what it is, especially to differentiate between
 * BVLC_ORIGINAL_UNICAST_NPDU and BVLC_ORIGINAL_BROADCAST_NPDU.
 *
 * @return A BVLC_ code, such as BVLC_ORIGINAL_UNICAST_NPDU.
 */
BACNET_BVLC_FUNCTION bvlc_get_function_code(
    void)
{
    return BVLC_Function_Code;
}

/** Get handle to broadcast distribution table (BDT).
 *
 *  Do not modify the table using the returned pointer,
 *  use the dedicated functions instead.
 *  (For optimization the table is not copied to caller)
 *
 * @param table [out] - broadcast distribution table
 *
 * @return Number of valid entries in the table or -1 on error.
 */
int bvlc_get_bdt_local(
     const BBMD_TABLE_ENTRY** table)
{
    int count = 0;

    if(table == NULL)
        return -1;

    *table = BBMD_Table;

    for (count = 0; count < MAX_BBMD_ENTRIES; ++count) {
        if (!BBMD_Table[count].valid) {
            break;
        }
    }

    return count;
}

/** Invalidate all entries in the broadcast distribution table (BDT).
 */
void bvlc_clear_bdt_local(
    void)
{
    int i = 0;
    for (i = 0; i < MAX_BBMD_ENTRIES; ++i) {
        BBMD_Table[i].valid = false;
        BBMD_Table[i].dest_address.s_addr = 0;
        BBMD_Table[i].dest_port = 0;
        BBMD_Table[i].broadcast_mask.s_addr = 0;
    }
}

/** Add new entry to broadcast distribution table.
 *
 * @return True if the new entry was added successfully.
 */
bool bvlc_add_bdt_entry_local(
    BBMD_TABLE_ENTRY* entry)
{
    bool found = false;
    int i = 0;

    if(entry == NULL)
        return false;

    /* Find first empty slot */
    for (i = 0; i < MAX_BBMD_ENTRIES; ++i) {
        if (!BBMD_Table[i].valid) {
            found = true;
            break;
        }

        /* Make sure that we are not adding a duplicate */
        if(BBMD_Table[i].dest_address.s_addr == entry->dest_address.s_addr &&
           BBMD_Table[i].broadcast_mask.s_addr == entry->broadcast_mask.s_addr &&
           BBMD_Table[i].dest_port == entry->dest_port) {
            return false;
        }
    }

    if(!found)
        return false;

    /* Copy new entry to the empty slot */
    BBMD_Table[i] = *entry;
    BBMD_Table[i].valid = true;

    return true;
}

/** Enable NAT handling and set the global IP address
 * @param [in] - Global IP address visible to peer BBMDs and foreign devices
 */
void bvlc_set_global_address_for_nat(const struct in_addr* addr)
{
    BVLC_Global_Address = *addr;
    BVLC_NAT_Handling = true;
}

/** Disable NAT handling.
 */
void bvlc_disable_nat(void)
{
    BVLC_NAT_Handling = false;
    BVLC_Global_Address.s_addr = 0;
}


#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

/* copy the source internet address to the BACnet address */
/* FIXME: IPv6? */
static void bvlc_bacnet_to_internet_address(
    struct sockaddr_in *sin,    /* source address in network order */
    BACNET_ADDRESS * src)
{       /* returns the BACnet source address */

    if (src && sin) {
        if (src->mac_len == 6) {
            memcpy(&sin->sin_addr.s_addr, &src->mac[0], 4);
            memcpy(&sin->sin_port, &src->mac[4], 2);
        }
    }

    return;
}

void testBIPAddress(
    Test * pTest)
{
    uint8_t apdu[50] = { 0 };
    uint32_t value = 0, test_value = 0;
    int len = 0, test_len = 0;
    struct in_addr address;
    struct in_addr test_address;
    uint16_t port = 0, test_port = 0;

    address.s_addr = 42;
    len = bvlc_encode_bip_address(&apdu[0], &address, port);
    test_len = bvlc_decode_bip_address(&apdu[0], &test_address, &test_port);
    ct_test(pTest, len == test_len);
    ct_test(pTest, address.s_addr == test_address.s_addr);
    ct_test(pTest, port == test_port);
}

void testInternetAddress(
    Test * pTest)
{
    BACNET_ADDRESS src;
    BACNET_ADDRESS test_src;
    struct sockaddr_in sin = { 0 };
    struct sockaddr_in test_sin = { 0 };

    sin.sin_port = htons(0xBAC0);
    sin.sin_addr.s_addr = inet_addr("192.168.0.1");
    bvlc_internet_to_bacnet_address(&src, &sin);
    bvlc_bacnet_to_internet_address(&test_sin, &src);
    ct_test(pTest, sin.sin_port == test_sin.sin_port);
    ct_test(pTest, sin.sin_addr.s_addr == test_sin.sin_addr.s_addr);
}

#ifdef TEST_BVLC
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Virtual Link Control", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBIPAddress);
    assert(rc);
    rc = ct_addTestFunction(pTest, testInternetAddress);
    assert(rc);
    /* configure output */
    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}

#endif /* TEST_BBMD */
#endif /* TEST */
