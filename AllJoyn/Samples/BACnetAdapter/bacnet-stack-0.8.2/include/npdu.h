/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#ifndef NPDU_H
#define NPDU_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacenum.h"

/** Hop count default is required by BTL to be maximum */
#ifndef HOP_COUNT_DEFAULT
#define HOP_COUNT_DEFAULT 255
#endif

/* an NPDU structure keeps the parameter stack to a minimum */
typedef struct bacnet_npdu_data_t {
    uint8_t protocol_version;
    /* parts of the control octet: */
    bool data_expecting_reply;
    bool network_layer_message; /* false if APDU */
    BACNET_MESSAGE_PRIORITY priority;
    /* optional network message info */
    BACNET_NETWORK_MESSAGE_TYPE network_message_type;   /* optional */
    uint16_t vendor_id; /* optional, if net message type is > 0x80 */
    uint8_t hop_count;
} BACNET_NPDU_DATA;

struct router_port_t;
/** The info[] string has no agreed-upon purpose, hence it is useless.
 * Keeping it short here. This size could be 0-255. */
#define ROUTER_PORT_INFO_LEN 2
/** Port Info structure used by Routers for their routing table. */
typedef struct router_port_t {
    uint16_t dnet;              /**< The DNET number that identifies this port. */
    uint8_t id;                 /**< Either 0 or some ill-defined, meaningless value. */
    uint8_t info[ROUTER_PORT_INFO_LEN];  /**< Info like 'modem dialing string' */
    uint8_t info_len;   /**< Length of info[]. */
    struct router_port_t *next;         /**< Point to next in linked list */
} BACNET_ROUTER_PORT;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    uint8_t npdu_encode_max_seg_max_apdu(
        int max_segs,
        int max_apdu);

    int npdu_encode_pdu(
        uint8_t * npdu,
        BACNET_ADDRESS * dest,
        BACNET_ADDRESS * src,
        BACNET_NPDU_DATA * npdu_data);

    void npdu_encode_npdu_data(
        BACNET_NPDU_DATA * npdu,
        bool data_expecting_reply,
        BACNET_MESSAGE_PRIORITY priority);

    void npdu_copy_data(
        BACNET_NPDU_DATA * dest,
        BACNET_NPDU_DATA * src);

    int npdu_decode(
        uint8_t * npdu,
        BACNET_ADDRESS * dest,
        BACNET_ADDRESS * src,
        BACNET_NPDU_DATA * npdu_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
