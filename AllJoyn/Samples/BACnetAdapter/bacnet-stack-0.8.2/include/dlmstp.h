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
#ifndef DLMSTP_H
#define DLMSTP_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bacdef.h"
#include "npdu.h"

/* defines specific to MS/TP */
/* preamble+type+dest+src+len+crc8+crc16 */
#define MAX_HEADER (2+1+1+1+2+1+2)
#define MAX_MPDU (MAX_HEADER+MAX_PDU)

typedef struct dlmstp_packet {
    bool ready; /* true if ready to be sent or received */
    BACNET_ADDRESS address;     /* source address */
    uint8_t frame_type; /* type of message */
    uint16_t pdu_len;   /* packet length */
    uint8_t pdu[MAX_MPDU];      /* packet */
} DLMSTP_PACKET;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    bool dlmstp_init(
        char *ifname);
    void dlmstp_reset(
        void);
    void dlmstp_cleanup(
        void);

    /* returns number of bytes sent on success, negative on failure */
    int dlmstp_send_pdu(
        BACNET_ADDRESS * dest,  /* destination address */
        BACNET_NPDU_DATA * npdu_data,   /* network information */
        uint8_t * pdu,  /* any data to be sent - may be null */
        unsigned pdu_len);      /* number of bytes of data */

    /* returns the number of octets in the PDU, or zero on failure */
    uint16_t dlmstp_receive(
        BACNET_ADDRESS * src,   /* source address */
        uint8_t * pdu,  /* PDU data */
        uint16_t max_pdu,       /* amount of space available in the PDU  */
        unsigned timeout);      /* milliseconds to wait for a packet */

    /* This parameter represents the value of the Max_Info_Frames property of */
    /* the node's Device object. The value of Max_Info_Frames specifies the */
    /* maximum number of information frames the node may send before it must */
    /* pass the token. Max_Info_Frames may have different values on different */
    /* nodes. This may be used to allocate more or less of the available link */
    /* bandwidth to particular nodes. If Max_Info_Frames is not writable in a */
    /* node, its value shall be 1. */
    void dlmstp_set_max_info_frames(
        uint8_t max_info_frames);
    uint8_t dlmstp_max_info_frames(
        void);

    /* This parameter represents the value of the Max_Master property of the */
    /* node's Device object. The value of Max_Master specifies the highest */
    /* allowable address for master nodes. The value of Max_Master shall be */
    /* less than or equal to 127. If Max_Master is not writable in a node, */
    /* its value shall be 127. */
    void dlmstp_set_max_master(
        uint8_t max_master);
    uint8_t dlmstp_max_master(
        void);

    /* MAC address 0-127 */
    void dlmstp_set_mac_address(
        uint8_t my_address);
    uint8_t dlmstp_mac_address(
        void);

    void dlmstp_get_my_address(
        BACNET_ADDRESS * my_address);
    void dlmstp_get_broadcast_address(
        BACNET_ADDRESS * dest); /* destination address */

    /* RS485 Baud Rate 9600, 19200, 38400, 57600, 115200 */
    void dlmstp_set_baud_rate(
        uint32_t baud);
    uint32_t dlmstp_baud_rate(
        void);

    void dlmstp_fill_bacnet_address(
        BACNET_ADDRESS * src,
        uint8_t mstp_address);

    bool dlmstp_sole_master(
        void);
    bool dlmstp_send_pdu_queue_empty(void);
    bool dlmstp_send_pdu_queue_full(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
