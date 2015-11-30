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
 Boston, MA  02111-1307
 USA.

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
#ifndef DLMSTP_H
#define DLMSTP_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bacdef.h"
#include "npdu.h"

/* defines specific to MS/TP */
#define MAX_HEADER (2+1+1+1+2+1)
#define MAX_MPDU (MAX_HEADER+MAX_PDU)

typedef struct dlmstp_packet {
    bool ready; /* true if ready to be sent or received */
    BACNET_ADDRESS address;     /* source address */
    uint8_t frame_type; /* type of message */
    unsigned pdu_len;   /* packet length */
    uint8_t pdu[MAX_MPDU];      /* packet */
} DLMSTP_PACKET;

/* number of MS/TP tx/rx packets */
extern uint16_t MSTP_Packets;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void dlmstp_reinit(
        void);
    void dlmstp_init(
        void);
    void dlmstp_cleanup(
        void);
    void dlmstp_millisecond_timer(
        void);
    void dlmstp_task(
        void);

    /* returns number of bytes sent on success, negative on failure */
    int dlmstp_send_pdu(
        BACNET_ADDRESS * dest,  /* destination address */
        BACNET_NPDU_DATA * npdu_data,   /* network information */
        uint8_t * pdu,  /* any data to be sent - may be null */
        unsigned pdu_len);      /* number of bytes of data */

    /* This parameter represents the value of the Max_Info_Frames property of */
    /* the node's Device object. The value of Max_Info_Frames specifies the */
    /* maximum number of information frames the node may send before it must */
    /* pass the token. Max_Info_Frames may have different values on different */
    /* nodes. This may be used to allocate more or less of the available link */
    /* bandwidth to particular nodes. If Max_Info_Frames is not writable in a */
    /* node, its value shall be 1. */
    void dlmstp_set_max_info_frames(
        uint8_t max_info_frames);
    unsigned dlmstp_max_info_frames(
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

    /* MAC address for MS/TP */
    void dlmstp_set_my_address(
        uint8_t my_address);
    uint8_t dlmstp_my_address(
        void);

    /* BACnet address used in datalink */
    void dlmstp_get_my_address(
        BACNET_ADDRESS * my_address);
    void dlmstp_get_broadcast_address(
        BACNET_ADDRESS * dest); /* destination address */

    /* MS/TP state machine functions */
    uint16_t dlmstp_put_receive(
        uint8_t src,    /* source MS/TP address */
        uint8_t * pdu,  /* PDU data */
        uint16_t pdu_len);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
