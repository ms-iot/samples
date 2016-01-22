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
#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacint.h"
#include "bacenum.h"
#include "bits.h"
#include "npdu.h"
#include "apdu.h"

/** @file npdu.c  Encode/Decode NPDUs - Network Protocol Data Units */

/** Copy the npdu_data structure information from src to dest.
 * @param dest [out] The 'to' structure
 * @param src   [in] The 'from' structure
 */
void npdu_copy_data(
    BACNET_NPDU_DATA * dest,
    BACNET_NPDU_DATA * src)
{
    if (dest && src) {
        dest->protocol_version = src->protocol_version;
        dest->data_expecting_reply = src->data_expecting_reply;
        dest->network_layer_message = src->network_layer_message;
        dest->priority = src->priority;
        dest->network_message_type = src->network_message_type;
        dest->vendor_id = src->vendor_id;
        dest->hop_count = src->hop_count;
    }

    return;
}

/*

The following ICI parameters are exchanged with the
various service primitives across an API:

'destination_address' (DA): the address of the device(s)
intended to receive the service primitive. Its format (device name,
network address, etc.) is a local matter. This address
may also be a multicast, local broadcast or global broadcast type.

'source_address' (SA): the address of the device from which
the service primitive was received. Its format (device name,
network address, etc.) is a local matter.

'network_priority' (NP): a four-level network priority parameter
described in 6.2.2.

'data_expecting_reply' (DER): a Boolean parameter that indicates
whether (TRUE) or not (FALSE) a reply service primitive
is expected for the service being issued.


Table 5-1. Applicability of ICI parameters for abstract service primitives
     Service Primitive         DA           SA         NP        DER
CONF_SERV.request              Yes          No         Yes       Yes
CONF_SERV.indication           Yes         Yes         Yes       Yes
CONF_SERV.response             Yes          No         Yes       Yes
CONF_SERV.confirm              Yes         Yes         Yes        No
UNCONF_SERV.request            Yes          No         Yes        No
UNCONF_SERV.indication         Yes         Yes         Yes        No
REJECT.request                 Yes          No         Yes        No
REJECT.indication              Yes         Yes         Yes        No
SEGMENT_ACK.request            Yes          No         Yes        No
SEGMENT_ACK.indication         Yes         Yes         Yes        No
ABORT.request                  Yes          No         Yes        No
ABORT.indication               Yes         Yes         Yes        No
*/


/** Encode the NPDU portion of a message to be sent, based on the npdu_data
 *  and associated data.
 *  If this is to be a Network Layer Control Message, there are probably
 *  more bytes which will need to be encoded following the ones encoded here.
 *  The Network Layer Protocol Control Information byte is described
 *  in section 6.2.2 of the BACnet standard.
 * @param npdu [out] Buffer which will hold the encoded NPDU header bytes.
 * 					 The size isn't given, but it must be at least 2 bytes
 *                   for the simplest case, and should always be at least 24
 *                   bytes to accommodate the maximal case (all fields loaded).
 * @param dest [in] The routing destination information if the message must
 *                   be routed to reach its destination.
 *                   If dest->net and dest->len are 0, there is no
 *                   routing destination information.
 * @param src  [in] The routing source information if the message was routed
 *                   from another BACnet network.
 *                   If src->net and src->len are 0, there is no
 *                   routing source information.
 *                   This src describes the original source of the message when
 *                   it had to be routed to reach this BACnet Device.
 * @param npdu_data [in] The structure which describes how the NCPI and other
 *                   NPDU bytes should be encoded.
 * @return On success, returns the number of bytes which were encoded into the
 * 		   NPDU section.
 *         If 0 or negative, there were problems with the data or encoding.
 */
int npdu_encode_pdu(
    uint8_t * npdu,
    BACNET_ADDRESS * dest,
    BACNET_ADDRESS * src,
    BACNET_NPDU_DATA * npdu_data)
{
    int len = 0;        /* return value - number of octets loaded in this function */
    uint8_t i = 0;      /* counter  */


    if (npdu && npdu_data) {
        /* protocol version */
        npdu[0] = npdu_data->protocol_version;
        /* initialize the control octet */
        npdu[1] = 0;
        /* Bit 7: 1 indicates that the NSDU conveys a network layer message. */
        /*          Message Type field is present. */
        /*        0 indicates that the NSDU contains a BACnet APDU. */
        /*          Message Type field is absent. */
        if (npdu_data->network_layer_message)
            npdu[1] |= BIT7;
        /*Bit 6: Reserved. Shall be zero. */
        /*Bit 5: Destination specifier where: */
        /* 0 = DNET, DLEN, DADR, and Hop Count absent */
        /* 1 = DNET, DLEN, and Hop Count present */
        /* DLEN = 0 denotes broadcast MAC DADR and DADR field is absent */
        /* DLEN > 0 specifies length of DADR field */
        if (dest && dest->net)
            npdu[1] |= BIT5;
        /* Bit 4: Reserved. Shall be zero. */
        /* Bit 3: Source specifier where: */
        /* 0 =  SNET, SLEN, and SADR absent */
        /* 1 =  SNET, SLEN, and SADR present */
        /* SLEN = 0 Invalid */
        /* SLEN > 0 specifies length of SADR field */
        if (src && src->net && src->len)
            npdu[1] |= BIT3;
        /* Bit 2: The value of this bit corresponds to the */
        /* data_expecting_reply parameter in the N-UNITDATA primitives. */
        /* 1 indicates that a BACnet-Confirmed-Request-PDU, */
        /* a segment of a BACnet-ComplexACK-PDU, */
        /* or a network layer message expecting a reply is present. */
        /* 0 indicates that other than a BACnet-Confirmed-Request-PDU, */
        /* a segment of a BACnet-ComplexACK-PDU, */
        /* or a network layer message expecting a reply is present. */
        if (npdu_data->data_expecting_reply)
            npdu[1] |= BIT2;
        /* Bits 1,0: Network priority where: */
        /* B'11' = Life Safety message */
        /* B'10' = Critical Equipment message */
        /* B'01' = Urgent message */
        /* B'00' = Normal message */
        npdu[1] |= (npdu_data->priority & 0x03);
        len = 2;
        if (dest && dest->net) {
            len += encode_unsigned16(&npdu[len], dest->net);
            npdu[len++] = dest->len;
            /* DLEN = 0 denotes broadcast MAC DADR and DADR field is absent */
            /* DLEN > 0 specifies length of DADR field */
            if (dest->len) {
                for (i = 0; i < dest->len; i++) {
                    npdu[len++] = dest->adr[i];
                }
            }
        }
        if (src && src->net && src->len) {      /* Only insert if valid */
            len += encode_unsigned16(&npdu[len], src->net);
            npdu[len++] = src->len;
            /* SLEN = 0 denotes broadcast MAC SADR and SADR field is absent */
            /* SLEN > 0 specifies length of SADR field */
            if (src->len) {
                for (i = 0; i < src->len; i++) {
                    npdu[len++] = src->adr[i];
                }
            }
        }
        /* The Hop Count field shall be present only if the message is */
        /* destined for a remote network, i.e., if DNET is present. */
        /* This is a one-octet field that is initialized to a value of 0xff. */
        if (dest && dest->net) {
            npdu[len] = npdu_data->hop_count;
            len++;
        }
        if (npdu_data->network_layer_message) {
            npdu[len] = npdu_data->network_message_type;
            len++;
            /* Message Type field contains a value in the range 0x80 - 0xFF, */
            /* then a Vendor ID field shall be present */
            if (npdu_data->network_message_type >= 0x80)
                len += encode_unsigned16(&npdu[len], npdu_data->vendor_id);
        }
    }

    return len;
}

/* Configure the NPDU portion of the packet for an APDU */
/* This function does not handle the network messages, just APDUs. */
/* From BACnet 5.1:
Applicability of ICI parameters for abstract service primitives
Service Primitive      DA  SA  NP  DER
-----------------      --- --- --- ---
CONF_SERV.request      Yes No  Yes Yes
CONF_SERV.indication   Yes Yes Yes Yes
CONF_SERV.response     Yes No  Yes Yes
CONF_SERV.confirm      Yes Yes Yes No
UNCONF_SERV.request    Yes No  Yes No
UNCONF_SERV.indication Yes Yes Yes No
REJECT.request         Yes No  Yes No
REJECT.indication      Yes Yes Yes No
SEGMENT_ACK.request    Yes No  Yes No
SEGMENT_ACK.indication Yes Yes Yes No
ABORT.request          Yes No  Yes No
ABORT.indication       Yes Yes Yes No

Where:
'destination_address' (DA): the address of the device(s) intended
to receive the service primitive. Its format (device name,
network address, etc.) is a local matter. This address may
also be a multicast, local broadcast or global broadcast type.
'source_address' (SA): the address of the device from which
the service primitive was received. Its format (device name,
network address, etc.) is a local matter.
'network_priority' (NP): a four-level network priority parameter
described in 6.2.2.
'data_expecting_reply' (DER): a Boolean parameter that indicates
whether (TRUE) or not (FALSE) a reply service primitive
is expected for the service being issued.
*/

/** Initialize an npdu_data structure to good defaults.
 * The name is a misnomer, as it doesn't do any actual encoding here.
 * @see npdu_encode_npdu_network if you need to set a network layer msg.
 *
 * @param npdu_data [out] Returns a filled-out structure with information
 * 					 provided by the other arguments and good defaults.
 * @param data_expecting_reply [in] True if message should have a reply.
 * @param priority [in] One of the 4 priorities defined in section 6.2.2,
 *                      like B'11' = Life Safety message
 */
void npdu_encode_npdu_data(
    BACNET_NPDU_DATA * npdu_data,
    bool data_expecting_reply,
    BACNET_MESSAGE_PRIORITY priority)
{
    if (npdu_data) {
        npdu_data->data_expecting_reply = data_expecting_reply;
        npdu_data->protocol_version = BACNET_PROTOCOL_VERSION;
        npdu_data->network_layer_message = false;       /* false if APDU */
        npdu_data->network_message_type = NETWORK_MESSAGE_INVALID;      /* optional */
        npdu_data->vendor_id = 0;       /* optional, if net message type is > 0x80 */
        npdu_data->priority = priority;
        npdu_data->hop_count = HOP_COUNT_DEFAULT;
    }
}

/** Decode the NPDU portion of a received message, particularly the NCPI byte.
 *  The Network Layer Protocol Control Information byte is described
 *  in section 6.2.2 of the BACnet standard.
 * @param npdu [in] Buffer holding the received NPDU header bytes (must be at least 2)
 * @param dest [out] Returned with routing destination information if the NPDU
 *                   has any and if this points to non-null storage for it.
 *                   If dest->net and dest->len are 0 on return, there is no
 *                   routing destination information.
 * @param src  [out] Returned with routing source information if the NPDU
 *                   has any and if this points to non-null storage for it.
 *                   If src->net and src->len are 0 on return, there is no
 *                   routing source information.
 *                   This src describes the original source of the message when
 *                   it had to be routed to reach this BACnet Device.
 * @param npdu_data [out] Returns a filled-out structure with information
 * 					 decoded from the NCPI and other NPDU bytes.
 * @return On success, returns the number of bytes which were decoded from the
 * 		   NPDU section; if this is a  network layer message, there may be more
 *         bytes left in the NPDU; if not a network msg, the APDU follows.
 *         If 0 or negative, there were problems with the data or arguments.
 */
int npdu_decode(
    uint8_t * npdu,
    BACNET_ADDRESS * dest,
    BACNET_ADDRESS * src,
    BACNET_NPDU_DATA * npdu_data)
{
    int len = 0;        /* return value - number of octets loaded in this function */
    uint8_t i = 0;      /* counter */
    uint16_t src_net = 0;
    uint16_t dest_net = 0;
    uint8_t address_len = 0;
    uint8_t mac_octet = 0;

    if (npdu && npdu_data) {
        /* Protocol Version */
        npdu_data->protocol_version = npdu[0];
        /* control octet */
        /* Bit 7: 1 indicates that the NSDU conveys a network layer message. */
        /*          Message Type field is present. */
        /*        0 indicates that the NSDU contains a BACnet APDU. */
        /*          Message Type field is absent. */
        npdu_data->network_layer_message = (npdu[1] & BIT7) ? true : false;
        /*Bit 6: Reserved. Shall be zero. */
        /* Bit 4: Reserved. Shall be zero. */
        /* Bit 2: The value of this bit corresponds to data expecting reply */
        /* parameter in the N-UNITDATA primitives. */
        /* 1 indicates that a BACnet-Confirmed-Request-PDU, */
        /* a segment of a BACnet-ComplexACK-PDU, */
        /* or a network layer message expecting a reply is present. */
        /* 0 indicates that other than a BACnet-Confirmed-Request-PDU, */
        /* a segment of a BACnet-ComplexACK-PDU, */
        /* or a network layer message expecting a reply is present. */
        npdu_data->data_expecting_reply = (npdu[1] & BIT2) ? true : false;
        /* Bits 1,0: Network priority where: */
        /* B'11' = Life Safety message */
        /* B'10' = Critical Equipment message */
        /* B'01' = Urgent message */
        /* B'00' = Normal message */
        npdu_data->priority = (BACNET_MESSAGE_PRIORITY) (npdu[1] & 0x03);
        /* set the offset to where the optional stuff starts */
        len = 2;
        /*Bit 5: Destination specifier where: */
        /* 0 = DNET, DLEN, DADR, and Hop Count absent */
        /* 1 = DNET, DLEN, and Hop Count present */
        /* DLEN = 0 denotes broadcast MAC DADR and DADR field is absent */
        /* DLEN > 0 specifies length of DADR field */
        if (npdu[1] & BIT5) {
            len += decode_unsigned16(&npdu[len], &dest_net);
            /* DLEN = 0 denotes broadcast MAC DADR and DADR field is absent */
            /* DLEN > 0 specifies length of DADR field */
            address_len = npdu[len++];
            if (dest) {
                dest->net = dest_net;
                dest->len = address_len;
            }
            if (address_len) {
                if (address_len > MAX_MAC_LEN) {
                    /* address is too large could be a malformed message */
                    return -1;
                }

                for (i = 0; i < address_len; i++) {
                    mac_octet = npdu[len++];
                    if (dest)
                        dest->adr[i] = mac_octet;
                }
            }
        }
        /* zero out the destination address */
        else if (dest) {
            dest->net = 0;
            dest->len = 0;
            for (i = 0; i < MAX_MAC_LEN; i++) {
                dest->adr[i] = 0;
            }
        }
        /* Bit 3: Source specifier where: */
        /* 0 =  SNET, SLEN, and SADR absent */
        /* 1 =  SNET, SLEN, and SADR present */
        /* SLEN = 0 Invalid todo Steve: immediately below src->len == broadcast case, surely then this comment is incorrect? */
        /* SLEN > 0 specifies length of SADR field */
        if (npdu[1] & BIT3) {
            len += decode_unsigned16(&npdu[len], &src_net);
            /* SLEN = 0 denotes broadcast MAC SADR and SADR field is absent */
            /* SLEN > 0 specifies length of SADR field */
            address_len = npdu[len++];
            if (src) {
                src->net = src_net;
                src->len = address_len;
            }
            if (address_len) {
                if (address_len > MAX_MAC_LEN) {
                    /* address is too large could be a malformed message */
                    return -1;
                }

                for (i = 0; i < address_len; i++) {
                    mac_octet = npdu[len++];
                    if (src)
                        src->adr[i] = mac_octet;
                }
            }
        } else if (src) {
            /* Clear the net number, with one exception: if the receive() 
             * function set it to BACNET_BROADCAST_NETWORK, (eg, for 
             * BVLC_ORIGINAL_BROADCAST_NPDU) then don't stomp on that.
             */
            if (src->net != BACNET_BROADCAST_NETWORK)
                src->net = 0;
            src->len = 0;
            for (i = 0; i < MAX_MAC_LEN; i++) {
                src->adr[i] = 0;
            }
        }
        /* The Hop Count field shall be present only if the message is */
        /* destined for a remote network, i.e., if DNET is present. */
        /* This is a one-octet field that is initialized to a value of 0xff. */
        if (dest_net) {
            npdu_data->hop_count = npdu[len++];
        } else {
            npdu_data->hop_count = 0;
        }
        /* Indicates that the NSDU conveys a network layer message. */
        /* Message Type field is present. */
        if (npdu_data->network_layer_message) {
            npdu_data->network_message_type =
                (BACNET_NETWORK_MESSAGE_TYPE) npdu[len++];
            /* Message Type field contains a value in the range 0x80 - 0xFF, */
            /* then a Vendor ID field shall be present */
            if (npdu_data->network_message_type >= 0x80)
                len += decode_unsigned16(&npdu[len], &npdu_data->vendor_id);
        } else {
            /* Since npdu_data->network_layer_message is false,
             * it doesn't much matter what we set here; this is safe: */
            npdu_data->network_message_type = NETWORK_MESSAGE_INVALID;
        }
    }

    return len;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

void testNPDU2(
    Test * pTest)
{
    uint8_t pdu[480] = { 0 };
    BACNET_ADDRESS dest = { 0 };
    BACNET_ADDRESS src = { 0 };
    BACNET_ADDRESS npdu_dest = { 0 };
    BACNET_ADDRESS npdu_src = { 0 };
    int len = 0;
    bool data_expecting_reply = true;
    BACNET_MESSAGE_PRIORITY priority = MESSAGE_PRIORITY_NORMAL;
    BACNET_NPDU_DATA npdu_data = { 0 };
    int i = 0;  /* counter */
    int npdu_len = 0;
    bool network_layer_message = false; /* false if APDU */
    BACNET_NETWORK_MESSAGE_TYPE network_message_type = 0;       /* optional */
    uint16_t vendor_id = 0;     /* optional, if net message type is > 0x80 */

    dest.mac_len = 6;
    for (i = 0; i < dest.mac_len; i++) {
        dest.mac[i] = i;
    }
    /* DNET,DLEN,DADR */
    dest.net = 1;
    dest.len = 6;
    for (i = 0; i < dest.len; i++) {
        dest.adr[i] = i * 10;
    }
    src.mac_len = 1;
    for (i = 0; i < src.mac_len; i++) {
        src.mac[i] = 0x80;
    }
    /* SNET,SLEN,SADR */
    src.net = 2;
    src.len = 1;
    for (i = 0; i < src.len; i++) {
        src.adr[i] = 0x40;
    }
    npdu_encode_npdu_data(&npdu_data, true, priority);
    len = npdu_encode_pdu(&pdu[0], &dest, &src, &npdu_data);
    ct_test(pTest, len != 0);
    /* can we get the info back? */
    npdu_len = npdu_decode(&pdu[0], &npdu_dest, &npdu_src, &npdu_data);
    ct_test(pTest, npdu_len != 0);
    ct_test(pTest, npdu_data.data_expecting_reply == data_expecting_reply);
    ct_test(pTest, npdu_data.network_layer_message == network_layer_message);
    if (npdu_data.network_layer_message) {
        ct_test(pTest, npdu_data.network_message_type == network_message_type);
    }
    ct_test(pTest, npdu_data.vendor_id == vendor_id);
    ct_test(pTest, npdu_data.priority == priority);
    /* DNET,DLEN,DADR */
    ct_test(pTest, npdu_dest.net == dest.net);
    ct_test(pTest, npdu_dest.len == dest.len);
    for (i = 0; i < dest.len; i++) {
        ct_test(pTest, npdu_dest.adr[i] == dest.adr[i]);
    }
    /* SNET,SLEN,SADR */
    ct_test(pTest, npdu_src.net == src.net);
    ct_test(pTest, npdu_src.len == src.len);
    for (i = 0; i < src.len; i++) {
        ct_test(pTest, npdu_src.adr[i] == src.adr[i]);
    }
}

void testNPDU1(
    Test * pTest)
{
    uint8_t pdu[480] = { 0 };
    BACNET_ADDRESS dest = { 0 };
    BACNET_ADDRESS src = { 0 };
    BACNET_ADDRESS npdu_dest = { 0 };
    BACNET_ADDRESS npdu_src = { 0 };
    int len = 0;
    bool data_expecting_reply = false;
    BACNET_MESSAGE_PRIORITY priority = MESSAGE_PRIORITY_NORMAL;
    BACNET_NPDU_DATA npdu_data = { 0 };
    int i = 0;  /* counter */
    int npdu_len = 0;
    bool network_layer_message = false; /* false if APDU */
    BACNET_NETWORK_MESSAGE_TYPE network_message_type = 0;       /* optional */
    uint16_t vendor_id = 0;     /* optional, if net message type is > 0x80 */

    /* mac_len = 0 if global address */
    dest.mac_len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        dest.mac[i] = 0;
    }
    /* DNET,DLEN,DADR */
    dest.net = 0;
    dest.len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        dest.adr[i] = 0;
    }
    src.mac_len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        src.mac[i] = 0;
    }
    /* SNET,SLEN,SADR */
    src.net = 0;
    src.len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        src.adr[i] = 0;
    }
    npdu_encode_npdu_data(&npdu_data, false, priority);
    len = npdu_encode_pdu(&pdu[0], &dest, &src, &npdu_data);
    ct_test(pTest, len != 0);
    /* can we get the info back? */
    npdu_len = npdu_decode(&pdu[0], &npdu_dest, &npdu_src, &npdu_data);
    ct_test(pTest, npdu_len != 0);
    ct_test(pTest, npdu_data.data_expecting_reply == data_expecting_reply);
    ct_test(pTest, npdu_data.network_layer_message == network_layer_message);
    if (npdu_data.network_layer_message) {
        ct_test(pTest, npdu_data.network_message_type == network_message_type);
    }
    ct_test(pTest, npdu_data.vendor_id == vendor_id);
    ct_test(pTest, npdu_data.priority == priority);
    ct_test(pTest, npdu_dest.mac_len == src.mac_len);
    ct_test(pTest, npdu_src.mac_len == dest.mac_len);
}

#ifdef TEST_NPDU
/* dummy stub for testing */
void tsm_free_invoke_id(
    uint8_t invokeID)
{
    (void) invokeID;
}

void iam_handler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
    (void) service_request;
    (void) service_len;
    (void) src;
}

int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet NPDU", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testNPDU1);
    assert(rc);
    rc = ct_addTestFunction(pTest, testNPDU2);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_NPDU */
#endif /* TEST */
