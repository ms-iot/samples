/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#if PRINT_ENABLED
#include <stdio.h>
#endif
#include "bacdef.h"
#include "mstp.h"
#include "dlmstp.h"
#include "rs485.h"
#include "npdu.h"

/* receive buffer */
static DLMSTP_PACKET Receive_Buffer;
/* temp buffer for NPDU insertion */
static uint8_t PDU_Buffer[MAX_MPDU];
/* local MS/TP port data */
static volatile struct mstp_port_struct_t MSTP_Port;

void dlmstp_init(
    char *ifname)
{
    (void) ifname;
    /* initialize buffer */
    Receive_Buffer.ready = false;
    Receive_Buffer.pdu_len = 0;
    /* initialize hardware */
    RS485_Initialize();
    MSTP_Init(&MSTP_Port, MSTP_Port.This_Station);
}

void dlmstp_cleanup(
    void)
{
    /* nothing to do for static buffers */
}

/* returns number of bytes sent on success, zero on failure */
int dlmstp_send_pdu(
    BACNET_ADDRESS * dest,      /* destination address */
    BACNET_NPDU_DATA * npdu_data,       /* network information */
    uint8_t * pdu,      /* any data to be sent - may be null */
    unsigned pdu_len)
{       /* number of bytes of data */
    int bytes_sent = 0;
    uint8_t frame_type = 0;
    uint8_t destination = 0;    /* destination address */
    BACNET_ADDRESS src;
    unsigned mtu_len = 0;

    if (MSTP_Port.TxReady == false) {
        if (npdu_data->confirmed_message) {
            MSTP_Port.TxFrameType = FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY;
        } else {
            MSTP_Port.TxFrameType = FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY;
        }
        /* load destination MAC address */
        if (dest->mac_len) {
            destination = dest->mac[0];
        } else {
            /* mac_len = 0 is a broadcast address */
            destination = MSTP_BROADCAST_ADDRESS;
        }
        /* header len */
        mtu_len = MAX_HEADER - 2 /* data crc */ ;
        if ((MAX_HEADER + pdu_len) > MAX_MPDU) {
#if PRINT_ENABLED
            fprintf(stderr, "mstp: PDU is too big to send!\n");
#endif
            return -4;
        }
        memmove(&PDU_Buffer[mtu_len], pdu, pdu_len);
        mtu_len += pdu_len;
        bytes_sent =
            MSTP_Create_Frame((uint8_t *) & MSTP_Port.TxBuffer[0],
            sizeof(MSTP_Port.TxBuffer), MSTP_Port.TxFrameType, destination,
            MSTP_Port.This_Station, &PDU_Buffer[0], mtu_len);
        MSTP_Port.TxLength = bytes_sent;
        MSTP_Port.TxReady = true;
    }

    return bytes_sent;
}

/* called about once a millisecond */
void dlmstp_millisecond_timer(
    void)
{
    MSTP_Millisecond_Timer(&MSTP_Port);
}

/* returns the number of octets in the PDU, or zero on failure */
/* This function is expecting to be polled. */
uint16_t dlmstp_receive(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* PDU data */
    uint16_t max_pdu,   /* amount of space available in the PDU  */
    unsigned timeout)
{
    uint16_t pdu_len = 0;

    (void) timeout;
    /* only do receive state machine while we don't have a frame */
    if ((MSTP_Port.ReceivedValidFrame == false) &&
        (MSTP_Port.ReceivedInvalidFrame == false)) {
        RS485_Check_UART_Data(&MSTP_Port);
        MSTP_Receive_Frame_FSM(&MSTP_Port);
    }
    /* only do master state machine while rx is idle */
    if (MSTP_Port.receive_state == MSTP_RECEIVE_STATE_IDLE) {
        if (This_Station <= DEFAULT_MAX_MASTER) {
            while (MSTP_Master_Node_FSM(&MSTP_Port)) {
            };
        }
    }
    /* see if there is a packet available */
    if (Receive_Buffer.ready) {
        memmove(src, &Receive_Buffer.address, sizeof(Receive_Buffer.address));
        pdu_len = Receive_Buffer.pdu_len;
        memmove(&pdu[0], &Receive_Buffer.pdu[0], max_pdu);
        Receive_Buffer.ready = false;
    }

    return pdu_len;
}

void dlmstp_fill_bacnet_address(
    BACNET_ADDRESS * src,
    uint8_t mstp_address)
{
    int i = 0;

    if (mstp_address == MSTP_BROADCAST_ADDRESS) {
        /* mac_len = 0 if broadcast address */
        src->mac_len = 0;
        src->mac[0] = 0;
    } else {
        src->mac_len = 1;
        src->mac[0] = mstp_address;
    }
    /* fill with 0's starting with index 1; index 0 filled above */
    for (i = 1; i < MAX_MAC_LEN; i++) {
        src->mac[i] = 0;
    }
    src->net = 0;
    src->len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        src->adr[i] = 0;
    }
}

/* for the MS/TP state machine to use for putting received data */
uint16_t dlmstp_put_receive(
    uint8_t src,        /* source MS/TP address */
    uint8_t * pdu,      /* PDU data */
    uint16_t pdu_len)
{
    if (Receive_Buffer.ready) {
        /* FIXME: what to do when we miss a message? */
        pdu_len = 0;
    } else if (pdu_len < sizeof(Receive_Buffer.pdu)) {
        dlmstp_fill_bacnet_address(&Receive_Buffer.address, src);
        Receive_Buffer.pdu_len = pdu_len;
        memmove(Receive_Buffer.pdu, pdu, pdu_len);
        Receive_Buffer.ready = true;
    } else {
        /* FIXME: message too large? */
        pdu_len = 0;
    }

    return pdu_len;
}

void dlmstp_set_my_address(
    uint8_t mac_address)
{
    /* FIXME: Master Nodes can only have address 1-127 */
    MSTP_Port.This_Station = mac_address;

    return;
}

/* This parameter represents the value of the Max_Info_Frames property of */
/* the node's Device object. The value of Max_Info_Frames specifies the */
/* maximum number of information frames the node may send before it must */
/* pass the token. Max_Info_Frames may have different values on different */
/* nodes. This may be used to allocate more or less of the available link */
/* bandwidth to particular nodes. If Max_Info_Frames is not writable in a */
/* node, its value shall be 1. */
void dlmstp_set_max_info_frames(
    unsigned max_info_frames)
{
    MSTP_Port.Nmax_info_frames = max_info_frames;

    return;
}

unsigned dlmstp_max_info_frames(
    void)
{
    return MSTP_Port.Nmax_info_frames;
}

/* This parameter represents the value of the Max_Master property of the */
/* node's Device object. The value of Max_Master specifies the highest */
/* allowable address for master nodes. The value of Max_Master shall be */
/* less than or equal to 127. If Max_Master is not writable in a node, */
/* its value shall be 127. */
void dlmstp_set_max_master(
    uint8_t max_master)
{
    MSTP_Port.Nmax_master = max_master;

    return;
}

uint8_t dlmstp_max_master(
    void)
{
    return MSTP_Port.Nmax_master;
}

void dlmstp_get_my_address(
    BACNET_ADDRESS * my_address)
{
    int i = 0;  /* counter */

    my_address->mac_len = 1;
    my_address->mac[0] = MSTP_Port.This_Station;
    my_address->net = 0;        /* local only, no routing */
    my_address->len = 0;
    for (i = 0; i < MAX_MAC_LEN; i++) {
        my_address->adr[i] = 0;
    }

    return;
}

void dlmstp_get_broadcast_address(
    BACNET_ADDRESS * dest)
{       /* destination address */
    int i = 0;  /* counter */

    if (dest) {
        dest->mac_len = 1;
        dest->mac[0] = MSTP_BROADCAST_ADDRESS;
        dest->net = BACNET_BROADCAST_NETWORK;
        dest->len = 0;  /* len=0 denotes broadcast address  */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            dest->adr[i] = 0;
        }
    }

    return;
}
