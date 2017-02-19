/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2008 Steve Karg

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
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "bacdef.h"
#include "mstpdef.h"
#include "dlmstp.h"
#include "rs485.h"
#include "crc.h"
#include "npdu.h"
#include "bits.h"
#include "bytes.h"
#include "bacaddr.h"
/* special optimization - I-Am response in this module */
#include "client.h"
#include "txbuf.h"

/* This file has been customized for use with small microprocessors */
/* Assumptions:
    Only one slave node MS/TP datalink layer
*/
#include "hardware.h"
#include "timer.h"

/* The state of the Receive State Machine */
static MSTP_RECEIVE_STATE Receive_State;
static struct mstp_flag_t {
    /* A Boolean flag set to TRUE by the Receive State Machine  */
    /* if an invalid frame is received.  */
    /* Set to FALSE by the main state machine. */
    unsigned ReceivedInvalidFrame:1;
    /* A Boolean flag set to TRUE by the Receive State Machine  */
    /* if a valid frame is received.  */
    /* Set to FALSE by the main state machine. */
    unsigned ReceivedValidFrame:1;
    /* A Boolean flag set TRUE by the datalink transmit if a
       frame is pending */
    unsigned TransmitPacketPending:1;
    /* A Boolean flag set TRUE by the datalink transmit if a
       pending packet is DataExpectingReply */
    unsigned TransmitPacketDER:1;
    /* A Boolean flag set TRUE by the datalink if a
       packet has been received, but not processed. */
    unsigned ReceivePacketPending:1;
} MSTP_Flag;

/* Used to store the data length of a received frame. */
static uint16_t DataLength;
/* Used to store the destination address of a received frame. */
static uint8_t DestinationAddress;
/* Used to store the frame type of a received frame. */
static uint8_t FrameType;
/* An array of octets, used to store octets as they are received. */
/* InputBuffer is indexed from 0 to InputBufferSize-1. */
/* FIXME: assign this to an actual array of bytes! */
/* Note: the buffer is designed as a pointer since some compilers
   and microcontroller architectures have limits as to places to
   hold contiguous memory. */
static uint8_t *InputBuffer;
static uint16_t InputBufferSize;
/* Used to store the Source Address of a received frame. */
static uint8_t SourceAddress;
/* "This Station," the MAC address of this node. TS is generally read from a */
/* hardware DIP switch, or from nonvolatile memory. Valid values for TS are */
/* 0 to 254. The value 255 is used to denote broadcast when used as a */
/* destination address but is not allowed as a value for TS. */
static uint8_t This_Station;
/* An array of octets, used to store octets for transmitting */
/* OutputBuffer is indexed from 0 to OutputBufferSize-1. */
/* The MAX_PDU size of a frame is MAX_APDU + MAX_NPDU octets. */
/* FIXME: assign this to an actual array of bytes! */
/* Note: the buffer is designed as a pointer since some compilers
   and microcontroller architectures have limits as to places to
   hold contiguous memory. */
static uint8_t *TransmitPacket;
static uint16_t TransmitPacketLen;
static uint8_t TransmitPacketDest;

/* The minimum time without a DataAvailable or ReceiveError event within */
/* a frame before a receiving node may discard the frame: 60 bit times. */
/* (Implementations may use larger values for this timeout, */
/* not to exceed 100 milliseconds.) */
/* At 9600 baud, 60 bit times would be about 6.25 milliseconds */
/* const uint16_t Tframe_abort = 1 + ((1000 * 60) / 9600); */
#define Tframe_abort 30

/* The maximum time a node may wait after reception of a frame that expects */
/* a reply before sending the first octet of a reply or Reply Postponed */
/* frame: 250 milliseconds. */
#define Treply_delay 250

/* we need to be able to increment without rolling over */
#define INCREMENT_AND_LIMIT_UINT8(x) {if (x < 0xFF) x++;}

bool dlmstp_init(
    char *ifname)
{
    ifname = ifname;
    /* initialize hardware */
    RS485_Initialize();

    return true;
}

void dlmstp_cleanup(
    void)
{
    /* nothing to do for static buffers */
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

/* MS/TP Frame Format */
/* All frames are of the following format: */
/* */
/* Preamble: two octet preamble: X`55', X`FF' */
/* Frame Type: one octet */
/* Destination Address: one octet address */
/* Source Address: one octet address */
/* Length: two octets, most significant octet first, of the Data field */
/* Header CRC: one octet */
/* Data: (present only if Length is non-zero) */
/* Data CRC: (present only if Length is non-zero) two octets, */
/*           least significant octet first */
/* (pad): (optional) at most one octet of padding: X'FF' */
static void MSTP_Send_Frame(
    uint8_t frame_type, /* type of frame to send - see defines */
    uint8_t destination,        /* destination address */
    uint8_t source,     /* source address */
    uint8_t * pdu,      /* any data to be sent - may be null */
    uint16_t pdu_len)
{       /* number of bytes of data (up to 501) */
    uint8_t crc8 = 0xFF;        /* used to calculate the crc value */
    uint16_t crc16 = 0xFFFF;    /* used to calculate the crc value */
    uint8_t buffer[8];  /* stores the header and crc */
    uint8_t datacrc[2]; /* stores the data crc */
    uint16_t i = 0;     /* used to calculate CRC for data */

    /* create the MS/TP header */
    buffer[0] = 0x55;
    buffer[1] = 0xFF;
    buffer[2] = frame_type;
    crc8 = CRC_Calc_Header(buffer[2], crc8);
    buffer[3] = destination;
    crc8 = CRC_Calc_Header(buffer[3], crc8);
    buffer[4] = source;
    crc8 = CRC_Calc_Header(buffer[4], crc8);

    buffer[5] = HI_BYTE(pdu_len);
    crc8 = CRC_Calc_Header(buffer[5], crc8);
    buffer[6] = LO_BYTE(pdu_len);
    crc8 = CRC_Calc_Header(buffer[6], crc8);
    buffer[7] = ~crc8;
    if (pdu_len) {
        /* calculate CRC for any data */
        for (i = 0; i < pdu_len; i++) {
            crc16 = CRC_Calc_Data(pdu[i], crc16);
        }
        crc16 = ~crc16;
        datacrc[0] = (crc16 & 0x00FF);
        datacrc[1] = ((crc16 & 0xFF00) >> 8);
    }
    /* now transmit the frame */
    RS485_Turnaround_Delay();
    RS485_Transmitter_Enable(true);
    RS485_Send_Data(buffer, 8);
    /* send any data */
    if (pdu_len) {
        RS485_Send_Data(pdu, pdu_len);
        RS485_Send_Data(datacrc, 2);
    }
    RS485_Transmitter_Enable(false);
}

static void MSTP_Receive_Frame_FSM(
    void)
{
    /* stores the latest received data octet */
    uint8_t DataRegister = 0;
    /* Used to accumulate the CRC on the data field of a frame. */
    static uint16_t DataCRC = 0;
    /* Used to accumulate the CRC on the header of a frame. */
    static uint8_t HeaderCRC = 0;
    /* Used as an index by the Receive State Machine,
       up to a maximum value of the MPDU */
    static uint16_t Index = 0;

    switch (Receive_State) {
        case MSTP_RECEIVE_STATE_IDLE:
            /* In the IDLE state, the node waits for the beginning of a frame. */
            if (RS485_ReceiveError()) {
                /* EatAnError */
                timer_silence_reset();
            } else if (RS485_DataAvailable(&DataRegister)) {
                timer_silence_reset();
                if (DataRegister == 0x55) {
                    /* Preamble1 */
                    /* receive the remainder of the frame. */
                    Receive_State = MSTP_RECEIVE_STATE_PREAMBLE;
                }
            }
            break;
        case MSTP_RECEIVE_STATE_PREAMBLE:
            /* In the PREAMBLE state, the node waits for the
               second octet of the preamble. */
            if (timer_silence_elapsed(Tframe_abort)) {
                /* Timeout */
                /* a correct preamble has not been received */
                /* wait for the start of a frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (RS485_ReceiveError()) {
                /* Error */
                timer_silence_reset();
                /* wait for the start of a frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (RS485_DataAvailable(&DataRegister)) {
                timer_silence_reset();
                if (DataRegister == 0xFF) {
                    /* Preamble2 */
                    Index = 0;
                    HeaderCRC = 0xFF;
                    /* receive the remainder of the frame. */
                    Receive_State = MSTP_RECEIVE_STATE_HEADER;
                } else if (DataRegister == 0x55) {
                    /* ignore RepeatedPreamble1 */
                    /* wait for the second preamble octet. */
                    Receive_State = MSTP_RECEIVE_STATE_PREAMBLE;
                } else {
                    /* NotPreamble */
                    /* wait for the start of a frame. */
                    Receive_State = MSTP_RECEIVE_STATE_IDLE;
                }
            }
            break;
        case MSTP_RECEIVE_STATE_HEADER:
            /* In the HEADER state, the node waits for the fixed message header. */
            if (timer_silence_elapsed(Tframe_abort)) {
                /* Timeout */
                /* indicate that an error has occurred during the reception of a frame */
                MSTP_Flag.ReceivedInvalidFrame = true;
                /* wait for the start of a frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (RS485_ReceiveError()) {
                /* Error */
                timer_silence_reset();
                /* indicate that an error has occurred during the reception of a frame */
                MSTP_Flag.ReceivedInvalidFrame = true;
                /* wait for the start of a frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (RS485_DataAvailable(&DataRegister)) {
                timer_silence_reset();
                if (Index == 0) {
                    /* FrameType */
                    HeaderCRC = CRC_Calc_Header(DataRegister, HeaderCRC);
                    FrameType = DataRegister;
                    Index = 1;
                } else if (Index == 1) {
                    /* Destination */
                    HeaderCRC = CRC_Calc_Header(DataRegister, HeaderCRC);
                    DestinationAddress = DataRegister;
                    Index = 2;
                } else if (Index == 2) {
                    /* Source */
                    HeaderCRC = CRC_Calc_Header(DataRegister, HeaderCRC);
                    SourceAddress = DataRegister;
                    Index = 3;
                } else if (Index == 3) {
                    /* Length1 */
                    HeaderCRC = CRC_Calc_Header(DataRegister, HeaderCRC);
                    DataLength = DataRegister * 256;
                    Index = 4;
                } else if (Index == 4) {
                    /* Length2 */
                    HeaderCRC = CRC_Calc_Header(DataRegister, HeaderCRC);
                    DataLength += DataRegister;
                    Index = 5;
                } else if (Index == 5) {
                    /* HeaderCRC */
                    HeaderCRC = CRC_Calc_Header(DataRegister, HeaderCRC);
                    /* In the HEADER_CRC state, the node validates the CRC
                       on the fixed  message header. */
                    if (HeaderCRC != 0x55) {
                        /* BadCRC */
                        /* indicate that an error has occurred during
                           the reception of a frame */
                        MSTP_Flag.ReceivedInvalidFrame = true;
                        /* wait for the start of the next frame. */
                        Receive_State = MSTP_RECEIVE_STATE_IDLE;
                    } else {
                        /* Note: proposed change to BACnet MSTP state machine!
                           If we don't decode data that is not for us, we could
                           get confused about the start if the Preamble 55 FF
                           is part of the data. */
                        if ((DataLength) && (DataLength <= InputBufferSize)) {
                            /* Data */
                            Index = 0;
                            DataCRC = 0xFFFF;
                            /* receive the data portion of the frame. */
                            Receive_State = MSTP_RECEIVE_STATE_DATA;
                        } else {
                            if (DataLength == 0) {
                                /* NoData */
                                if ((DestinationAddress == This_Station) ||
                                    (DestinationAddress ==
                                        MSTP_BROADCAST_ADDRESS)) {
                                    /* ForUs */
                                    /* indicate that a frame with
                                       no data has been received */
                                    MSTP_Flag.ReceivedValidFrame = true;
                                } else {
                                    /* NotForUs - drop */
                                }
                            } else {
                                /* FrameTooLong */
                                /* indicate that a frame with an illegal or  */
                                /* unacceptable data length has been received */
                                MSTP_Flag.ReceivedInvalidFrame = true;
                            }
                            /* wait for the start of the next frame. */
                            Receive_State = MSTP_RECEIVE_STATE_IDLE;
                        }
                    }
                } else {
                    /* indicate that an error has occurred during  */
                    /* the reception of a frame */
                    MSTP_Flag.ReceivedInvalidFrame = true;
                    /* wait for the start of a frame. */
                    Receive_State = MSTP_RECEIVE_STATE_IDLE;
                }
            }
            break;
        case MSTP_RECEIVE_STATE_DATA:
            /* In the DATA state, the node waits for the data portion of a frame. */
            if (timer_silence_elapsed(Tframe_abort)) {
                /* Timeout */
                /* indicate that an error has occurred during the reception of a frame */
                MSTP_Flag.ReceivedInvalidFrame = true;
                /* wait for the start of the next frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (RS485_ReceiveError()) {
                /* Error */
                timer_silence_reset();
                /* indicate that an error has occurred during
                   the reception of a frame */
                MSTP_Flag.ReceivedInvalidFrame = true;
                /* wait for the start of the next frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (RS485_DataAvailable(&DataRegister)) {
                timer_silence_reset();
                if (Index < DataLength) {
                    /* DataOctet */
                    DataCRC = CRC_Calc_Data(DataRegister, DataCRC);
                    InputBuffer[Index] = DataRegister;
                    Index++;
                } else if (Index == DataLength) {
                    /* CRC1 */
                    DataCRC = CRC_Calc_Data(DataRegister, DataCRC);
                    Index++;
                } else if (Index == (DataLength + 1)) {
                    /* CRC2 */
                    DataCRC = CRC_Calc_Data(DataRegister, DataCRC);
                    /* STATE DATA CRC - no need for new state */
                    /* indicate the complete reception of a valid frame */
                    if (DataCRC == 0xF0B8) {
                        if ((DestinationAddress == This_Station) ||
                            (DestinationAddress == MSTP_BROADCAST_ADDRESS)) {
                            /* ForUs */
                            /* indicate that a frame with no data
                               has been received */
                            MSTP_Flag.ReceivedValidFrame = true;
                        }
                    } else {
                        MSTP_Flag.ReceivedInvalidFrame = true;
                    }
                    Receive_State = MSTP_RECEIVE_STATE_IDLE;
                }
            }
            break;
        default:
            /* shouldn't get here - but if we do... */
            Receive_State = MSTP_RECEIVE_STATE_IDLE;
            break;
    }

    return;
}

static void MSTP_Slave_Node_FSM(
    void)
{
    if (MSTP_Flag.ReceivedValidFrame) {
        MSTP_Flag.ReceivedValidFrame = false;
        switch (FrameType) {
            case FRAME_TYPE_TOKEN:
                break;
            case FRAME_TYPE_POLL_FOR_MASTER:
                break;
            case FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY:
                break;
            case FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY:
                /* indicate successful reception to the higher layers  */
                MSTP_Flag.ReceivePacketPending = true;
                break;
            case FRAME_TYPE_TEST_REQUEST:
                MSTP_Send_Frame(FRAME_TYPE_TEST_RESPONSE, SourceAddress,
                    This_Station, &InputBuffer[0], DataLength);
                break;
            case FRAME_TYPE_TEST_RESPONSE:
            default:
                break;
        }
    } else if (MSTP_Flag.TransmitPacketPending) {
        /* Reply */
        /* If a reply is available from the higher layers  */
        /* within Treply_delay after the reception of the  */
        /* final octet of the requesting frame  */
        /* (the mechanism used to determine this is a local matter), */
        /* then call MSTP_Send_Frame to transmit the reply frame  */
        /* and enter the IDLE state to wait for the next frame. */
        /* Note: optimized such that we are never a client */
        MSTP_Send_Frame(FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY,
            TransmitPacketDest, This_Station, (uint8_t *) & TransmitPacket[0],
            TransmitPacketLen);
        MSTP_Flag.TransmitPacketPending = false;
        MSTP_Flag.ReceivePacketPending = false;
    }
}

/* returns number of bytes sent on success, zero on failure */
int dlmstp_send_pdu(
    BACNET_ADDRESS * dest,      /* destination address */
    BACNET_NPDU_DATA * npdu_data,       /* network information */
    uint8_t * pdu,      /* any data to be sent - may be null */
    unsigned pdu_len)
{       /* number of bytes of data */
    int bytes_sent = 0;

    if (MSTP_Flag.TransmitPacketPending == false) {
        MSTP_Flag.TransmitPacketDER = npdu_data->data_expecting_reply;
        TransmitPacket = pdu;
        TransmitPacketLen = pdu_len;
        bytes_sent = pdu_len;
        TransmitPacketDest = dest->mac[0];
        MSTP_Flag.TransmitPacketPending = true;
    }

    return bytes_sent;
}

/* Return the length of the packet */
uint16_t dlmstp_receive(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* PDU data */
    uint16_t max_pdu,   /* amount of space available in the PDU  */
    unsigned timeout)
{       /* milliseconds to wait for a packet */
    uint16_t pdu_len = 0;       /* return value */

    /* dummy - unused parameter */
    timeout = timeout;
    /* set the input buffer to the same data storage for zero copy */
    if (!InputBuffer) {
        InputBuffer = pdu;
        InputBufferSize = max_pdu;
    }
    /* only do receive state machine while we don't have a frame */
    if ((MSTP_Flag.ReceivedValidFrame == false) &&
        (MSTP_Flag.ReceivedInvalidFrame == false) &&
        (MSTP_Flag.ReceivePacketPending == false)) {
        for (;;) {
            MSTP_Receive_Frame_FSM();
            if (MSTP_Flag.ReceivedValidFrame || MSTP_Flag.ReceivedInvalidFrame)
                break;
            /* if we are not idle, then we are
               receiving a frame or timing out */
            if (Receive_State == MSTP_RECEIVE_STATE_IDLE)
                break;
        }
    }
    /* only do master state machine while rx is idle */
    if (Receive_State == MSTP_RECEIVE_STATE_IDLE) {
        MSTP_Slave_Node_FSM();
    }
    /* if there is a packet that needs processed, do it now. */
    if (MSTP_Flag.ReceivePacketPending) {
        MSTP_Flag.ReceivePacketPending = false;
        pdu_len = DataLength;
        src->mac_len = 1;
        src->mac[0] = SourceAddress;
        /* data is already in the pdu pointer */
    }

    return pdu_len;
}

void dlmstp_set_mac_address(
    uint8_t mac_address)
{
    /* Master Nodes can only have address 0-127 */
    if (mac_address <= 127) {
        This_Station = mac_address;
        /* FIXME: implement your data storage */
        /* I2C_Write_Byte(
           EEPROM_DEVICE_ADDRESS,
           mac_address,
           EEPROM_MSTP_MAC_ADDR); */
    }

    return;
}

uint8_t dlmstp_mac_address(
    void)
{
    return This_Station;
}

void dlmstp_get_my_address(
    BACNET_ADDRESS * my_address)
{
    int i = 0;  /* counter */

    my_address->mac_len = 1;
    my_address->mac[0] = This_Station;
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
        dest->len = 0;  /* always zero when DNET is broadcast */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            dest->adr[i] = 0;
        }
    }

    return;
}
