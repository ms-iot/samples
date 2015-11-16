/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2009 Steve Karg

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
#include "dlmstp.h"
#include "rs485.h"
#include "crc.h"
#include "npdu.h"
#include "bits.h"
#include "bacaddr.h"
#include "timer.h"
#include "ringbuf.h"
#include "mstpdef.h"
#include "automac.h"
#include "device.h"

/* This file has been customized for use with small microprocessors */
/* Assumptions:
    Only one MS/TP datalink layer
*/

/* The state of the Receive State Machine */
static MSTP_RECEIVE_STATE Receive_State;
/* When a master node is powered up or reset, */
/* it shall unconditionally enter the INITIALIZE state. */
static MSTP_MASTER_STATE Master_State;
/* bit-sized boolean flags */
static struct mstp_flag_t {
    /* A Boolean flag set to TRUE by the Receive State Machine  */
    /* if an invalid frame is received.  */
    /* Set to FALSE by the main state machine. */
    unsigned ReceivedInvalidFrame:1;
    /* A Boolean flag set to TRUE by the Receive State Machine  */
    /* if a valid frame is received.  */
    /* Set to FALSE by the main state machine. */
    unsigned ReceivedValidFrame:1;
    /* set to TRUE when we get a frame not for us */
    unsigned ReceivedValidFrameNotForUs:1;
    /* A Boolean flag set to TRUE by the master machine if this node is the */
    /* only known master node. */
    unsigned SoleMaster:1;
    /* A Boolean flag set TRUE by the datalink if a
       packet has been received, but not processed. */
    unsigned ReceivePacketPending:1;
} MSTP_Flag;

/* Used to store the data length of a received frame. */
static uint32_t DataLength;
/* Used to store the destination address of a received frame. */
static uint8_t DestinationAddress;
/* Used to count the number of received octets or errors. */
/* This is used in the detection of link activity. */
/* Compared to Nmin_octets */
static uint8_t EventCount;
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
/* 0 to 254. The value 255 is used to denote AutoMAC addressing */
static uint8_t This_Station = 255;
/* This parameter represents the value of the Max_Master property of the */
/* node's Device object. The value of Max_Master specifies the highest */
/* allowable address for master nodes. The value of Max_Master shall be */
/* less than or equal to 127. If Max_Master is not writable in a node, */
/* its value shall be 127. */
static uint8_t Nmax_master = 127;

/* The time without a DataAvailable or ReceiveError event before declaration */
/* of loss of token: 500 milliseconds. */
#define Tno_token 500

/* The minimum time without a DataAvailable or ReceiveError event */
/* that a node must wait for a station to begin replying to a */
/* confirmed request: 255 milliseconds. (Implementations may use */
/* larger values for this timeout, not to exceed 300 milliseconds.) */
#define Treply_timeout 260

/* The minimum time without a DataAvailable or ReceiveError event that a */
/* node must wait for a remote node to begin using a token or replying to */
/* a Poll For Master frame: 20 milliseconds. (Implementations may use */
/* larger values for this timeout, not to exceed 100 milliseconds.) */
#define Tusage_timeout 60

/* The number of tokens received or used before a Poll For Master cycle */
/* is executed: 50. */
#define Npoll 50

/* The number of retries on sending Token: 1. */
#define Nretry_token 1

/* The minimum number of DataAvailable or ReceiveError events that must be */
/* seen by a receiving node in order to declare the line "active": 4. */
#define Nmin_octets 4

/* The minimum time without a DataAvailable or ReceiveError event within */
/* a frame before a receiving node may discard the frame: 60 bit times. */
/* (Implementations may use larger values for this timeout, */
/* not to exceed 100 milliseconds.) */
/* At 9600 baud, 60 bit times would be about 6.25 milliseconds */
/* const uint16_t Tframe_abort = 1 + ((1000 * 60) / 9600); */
#define Tframe_abort 30

/* The maximum idle time a sending node may allow to elapse between octets */
/* of a frame the node is transmitting: 20 bit times. */
#define Tframe_gap 20

/* The maximum time after the end of the stop bit of the final */
/* octet of a transmitted frame before a node must disable its */
/* EIA-485 driver: 15 bit times. */
#define Tpostdrive 15

/* The maximum time a node may wait after reception of a frame that expects */
/* a reply before sending the first octet of a reply or Reply Postponed */
/* frame: 250 milliseconds. */
#define Treply_delay 250

/* The width of the time slot within which a node may generate a token: */
/* 10 milliseconds. */
#define Tslot 10

/* The maximum time a node may wait after reception of the token or */
/* a Poll For Master frame before sending the first octet of a frame: */
/* 15 milliseconds. */
#define Tusage_delay 15

/* we need to be able to increment without rolling over */
#define INCREMENT_AND_LIMIT_UINT8(x) {if (x < 0xFF) x++;}

/* data structure for MS/TP transmit packet */
struct mstp_tx_packet {
    uint16_t length;
    uint16_t index;
    uint8_t buffer[MAX_MPDU];
};
/* count must be a power of 2 for ringbuf library */
#ifndef MSTP_TRANSMIT_PACKET_COUNT
#define MSTP_TRANSMIT_PACKET_COUNT 1
#endif
static struct mstp_tx_packet Transmit_Buffer[MSTP_TRANSMIT_PACKET_COUNT];
static RING_BUFFER Transmit_Queue;

/* data structure for MS/TP PDU Queue */
struct mstp_pdu_packet {
    bool data_expecting_reply;
    uint8_t destination_mac;
    uint16_t length;
    uint8_t buffer[MAX_MPDU];
};
/* count must be a power of 2 for ringbuf library */
#ifndef MSTP_PDU_PACKET_COUNT
#define MSTP_PDU_PACKET_COUNT 2
#endif
static struct mstp_pdu_packet PDU_Buffer[MSTP_PDU_PACKET_COUNT];
static RING_BUFFER PDU_Queue;
/* This parameter represents the value of the Max_Info_Frames property of */
/* the node's Device object. The value of Max_Info_Frames specifies the */
/* maximum number of information frames the node may send before it must */
/* pass the token. Max_Info_Frames may have different values on different */
/* nodes. This may be used to allocate more or less of the available link */
/* bandwidth to particular nodes. If Max_Info_Frames is not writable in a */
/* node, its value shall be 1. */
static uint8_t Nmax_info_frames = MSTP_PDU_PACKET_COUNT;

void dlmstp_automac_hander(
    void);

bool dlmstp_init(
    char *ifname)
{
    ifname = ifname;
    Ringbuf_Init(&Transmit_Queue, (uint8_t *) & Transmit_Buffer,
        sizeof(struct mstp_tx_packet), MSTP_TRANSMIT_PACKET_COUNT);
    Ringbuf_Init(&PDU_Queue, (uint8_t *) & PDU_Buffer,
        sizeof(struct mstp_pdu_packet), MSTP_PDU_PACKET_COUNT);
    rs485_init();
    automac_init();

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

static bool dlmstp_compare_data_expecting_reply(
    uint8_t * request_pdu,
    uint16_t request_pdu_len,
    uint8_t src_address,
    uint8_t * reply_pdu,
    uint16_t reply_pdu_len,
    uint8_t dest_address)
{
    uint16_t offset;
    /* One way to check the message is to compare NPDU
       src, dest, along with the APDU type, invoke id.
       Seems a bit overkill */
    struct DER_compare_t {
        BACNET_NPDU_DATA npdu_data;
        BACNET_ADDRESS address;
        uint8_t pdu_type;
        uint8_t invoke_id;
        uint8_t service_choice;
    };
    struct DER_compare_t request;
    struct DER_compare_t reply;

    /* decode the request data */
    request.address.mac[0] = src_address;
    request.address.mac_len = 1;
    offset =
        npdu_decode(&request_pdu[0], NULL, &request.address,
        &request.npdu_data);
    if (request.npdu_data.network_layer_message) {
        return false;
    }
    request.pdu_type = request_pdu[offset] & 0xF0;
    if (request.pdu_type != PDU_TYPE_CONFIRMED_SERVICE_REQUEST) {
        return false;
    }
    request.invoke_id = request_pdu[offset + 2];
    /* segmented message? */
    if (request_pdu[offset] & BIT3)
        request.service_choice = request_pdu[offset + 5];
    else
        request.service_choice = request_pdu[offset + 3];
    /* decode the reply data */
    reply.address.mac[0] = dest_address;
    reply.address.mac_len = 1;
    offset =
        npdu_decode(&reply_pdu[0], &reply.address, NULL, &reply.npdu_data);
    if (reply.npdu_data.network_layer_message) {
        return false;
    }
    /* reply could be a lot of things:
       confirmed, simple ack, abort, reject, error */
    reply.pdu_type = reply_pdu[offset] & 0xF0;
    switch (reply.pdu_type) {
        case PDU_TYPE_CONFIRMED_SERVICE_REQUEST:
            reply.invoke_id = reply_pdu[offset + 2];
            /* segmented message? */
            if (reply_pdu[offset] & BIT3)
                reply.service_choice = reply_pdu[offset + 5];
            else
                reply.service_choice = reply_pdu[offset + 3];
            break;
        case PDU_TYPE_SIMPLE_ACK:
            reply.invoke_id = reply_pdu[offset + 1];
            reply.service_choice = reply_pdu[offset + 2];
            break;
        case PDU_TYPE_COMPLEX_ACK:
            reply.invoke_id = reply_pdu[offset + 1];
            /* segmented message? */
            if (reply_pdu[offset] & BIT3)
                reply.service_choice = reply_pdu[offset + 4];
            else
                reply.service_choice = reply_pdu[offset + 2];
            break;
        case PDU_TYPE_ERROR:
            reply.invoke_id = reply_pdu[offset + 1];
            reply.service_choice = reply_pdu[offset + 2];
            break;
        case PDU_TYPE_REJECT:
        case PDU_TYPE_ABORT:
            reply.invoke_id = reply_pdu[offset + 1];
            break;
        default:
            return false;
    }
    if (request.invoke_id != reply.invoke_id) {
        return false;
    }
    /* these services don't have service choice included */
    if ((reply.pdu_type != PDU_TYPE_REJECT) &&
        (reply.pdu_type != PDU_TYPE_ABORT)) {
        if (request.service_choice != reply.service_choice) {
            return false;
        }
    }
    if (request.npdu_data.protocol_version != reply.npdu_data.protocol_version) {
        return false;
    }
    if (request.npdu_data.priority != reply.npdu_data.priority) {
        return false;
    }
    if (!bacnet_address_same(&request.address, &reply.address)) {
        return false;
    }

    return true;
}

typedef enum {
    MSTP_TX_STATE_IDLE,
    MSTP_TX_STATE_SILENCE_WAIT,
    MSTP_TX_STATE_SEND_WAIT,
    MSTP_TX_STATE_STOP
} MSTP_TX_STATE;
static bool MSTP_Transmit_FSM(
    void)
{
    static MSTP_TX_STATE state = MSTP_TX_STATE_IDLE;
    static struct mstp_tx_packet *pkt;

  MSTP_TX_START:
    switch (state) {
        case MSTP_TX_STATE_IDLE:
            if (!Ringbuf_Empty(&Transmit_Queue)) {
                /* get the packet - but don't remove it from queue */
                pkt = (struct mstp_tx_packet *)
                    Ringbuf_Peek(&Transmit_Queue);
                state = MSTP_TX_STATE_SILENCE_WAIT;
            }
            break;
        case MSTP_TX_STATE_SILENCE_WAIT:
            if (rs485_turnaround_elapsed()) {
                rs485_rts_enable(true);
                pkt->index = 0;
                rs485_byte_send(pkt->buffer[pkt->index]);
                state = MSTP_TX_STATE_SEND_WAIT;
                /* optimize a little - for slower CPUs */
                goto MSTP_TX_START;
            }
            break;
        case MSTP_TX_STATE_SEND_WAIT:
            if (rs485_byte_sent()) {
                pkt->index++;
                if (pkt->index < pkt->length) {
                    /* send next byte */
                    rs485_byte_send(pkt->buffer[pkt->index]);
                    /* optimize a little - for slower CPUs */
                    goto MSTP_TX_START;
                } else {
                    state = MSTP_TX_STATE_STOP;
                }
            }
            break;
        case MSTP_TX_STATE_STOP:
            if (rs485_byte_sent() && rs485_frame_sent()) {
                rs485_rts_enable(false);
                /* remove the packet from the queue */
                (void) Ringbuf_Pop(&Transmit_Queue, NULL);
                state = MSTP_TX_STATE_IDLE;
            }
            break;
        default:
            state = MSTP_TX_STATE_IDLE;
            break;
    }

    return (state != MSTP_TX_STATE_IDLE);
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
    uint8_t * data,     /* any data to be sent - may be null */
    uint16_t data_len)
{       /* number of bytes of data (up to 501) */
    uint8_t crc8 = 0xFF;        /* used to calculate the crc value */
    uint16_t crc16 = 0xFFFF;    /* used to calculate the crc value */
    struct mstp_tx_packet *pkt;
    uint16_t i = 0;     /* used to calculate CRC for data */

    pkt = (struct mstp_tx_packet *) Ringbuf_Data_Peek(&Transmit_Queue);
    if (pkt) {
        /* create the MS/TP header */
        pkt->buffer[0] = 0x55;
        pkt->buffer[1] = 0xFF;
        pkt->buffer[2] = frame_type;
        crc8 = CRC_Calc_Header(pkt->buffer[2], crc8);
        pkt->buffer[3] = destination;
        crc8 = CRC_Calc_Header(pkt->buffer[3], crc8);
        pkt->buffer[4] = source;
        crc8 = CRC_Calc_Header(pkt->buffer[4], crc8);
        pkt->buffer[5] = data_len / 256;
        crc8 = CRC_Calc_Header(pkt->buffer[5], crc8);
        pkt->buffer[6] = data_len % 256;
        crc8 = CRC_Calc_Header(pkt->buffer[6], crc8);
        pkt->buffer[7] = ~crc8;
        pkt->length = 8;
        if (data_len) {
            /* calculate CRC for any data */
            for (i = 0; i < data_len; i++) {
                crc16 = CRC_Calc_Data(data[i], crc16);
                pkt->buffer[8 + i] = data[i];
            }
            crc16 = ~crc16;
            pkt->buffer[8 + data_len] = (crc16 & 0x00FF);
            pkt->buffer[8 + data_len + 1] = ((crc16 & 0xFF00) >> 8);
            pkt->length += data_len;
            pkt->length += 2;
        }
        Ringbuf_Data_Put(&Transmit_Queue, (uint8_t *)pkt);
    } else {
        pkt = NULL;
    }
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
            /* In the IDLE state, the node waits
               for the beginning of a frame. */
            if (rs485_receive_error()) {
                /* EatAnError */
                rs485_silence_reset();
                INCREMENT_AND_LIMIT_UINT8(EventCount);
            } else if (rs485_byte_available(&DataRegister)) {
                rs485_silence_reset();
                INCREMENT_AND_LIMIT_UINT8(EventCount);
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
            if (rs485_silence_elapsed(Tframe_abort)) {
                /* Timeout */
                /* a correct preamble has not been received */
                /* wait for the start of a frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (rs485_receive_error()) {
                /* Error */
                rs485_silence_reset();
                INCREMENT_AND_LIMIT_UINT8(EventCount);
                /* wait for the start of a frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (rs485_byte_available(&DataRegister)) {
                rs485_silence_reset();
                INCREMENT_AND_LIMIT_UINT8(EventCount);
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
            /* In the HEADER state, the node waits
               for the fixed message header. */
            if (rs485_silence_elapsed(Tframe_abort)) {
                /* Timeout */
                /* indicate that an error has occurred
                   during the reception of a frame */
                MSTP_Flag.ReceivedInvalidFrame = true;
                /* wait for the start of a frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (rs485_receive_error()) {
                /* Error */
                rs485_silence_reset();
                INCREMENT_AND_LIMIT_UINT8(EventCount);
                /* indicate that an error has occurred
                   during the reception of a frame */
                MSTP_Flag.ReceivedInvalidFrame = true;
                /* wait for the start of a frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (rs485_byte_available(&DataRegister)) {
                rs485_silence_reset();
                INCREMENT_AND_LIMIT_UINT8(EventCount);
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
                                /* NotForUs */
                                MSTP_Flag.ReceivedValidFrameNotForUs = true;
                            }
                            /* wait for the start of the next frame. */
                            Receive_State = MSTP_RECEIVE_STATE_IDLE;
                        } else {
                            /* receive the data portion of the frame. */
                            if ((DestinationAddress == This_Station) ||
                                (DestinationAddress ==
                                    MSTP_BROADCAST_ADDRESS)) {
                                if (DataLength <= InputBufferSize) {
                                    /* Data */
                                    Receive_State = MSTP_RECEIVE_STATE_DATA;
                                } else {
                                    /* FrameTooLong */
                                    Receive_State =
                                        MSTP_RECEIVE_STATE_SKIP_DATA;
                                }
                            } else {
                                /* NotForUs */
                                Receive_State = MSTP_RECEIVE_STATE_SKIP_DATA;
                            }
                            Index = 0;
                            DataCRC = 0xFFFF;
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
        case MSTP_RECEIVE_STATE_SKIP_DATA:
            /* In the DATA state, the node waits
               for the data portion of a frame. */
            if (rs485_silence_elapsed(Tframe_abort)) {
                /* Timeout */
                /* indicate that an error has occurred
                   during the reception of a frame */
                MSTP_Flag.ReceivedInvalidFrame = true;
                /* wait for the start of the next frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (rs485_receive_error()) {
                /* Error */
                rs485_silence_reset();
                INCREMENT_AND_LIMIT_UINT8(EventCount);
                /* indicate that an error has occurred during
                   the reception of a frame */
                MSTP_Flag.ReceivedInvalidFrame = true;
                /* wait for the start of the next frame. */
                Receive_State = MSTP_RECEIVE_STATE_IDLE;
            } else if (rs485_byte_available(&DataRegister)) {
                rs485_silence_reset();
                INCREMENT_AND_LIMIT_UINT8(EventCount);
                if (Index < DataLength) {
                    /* DataOctet */
                    DataCRC = CRC_Calc_Data(DataRegister, DataCRC);
                    if (Index < InputBufferSize) {
                        InputBuffer[Index] = DataRegister;
                    }
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
                        if (Receive_State == MSTP_RECEIVE_STATE_DATA) {
                            /* ForUs */
                            MSTP_Flag.ReceivedValidFrame = true;
                        } else {
                            /* NotForUs */
                            MSTP_Flag.ReceivedValidFrameNotForUs = true;
                        }
                    } else {
                        MSTP_Flag.ReceivedInvalidFrame = true;
                    }
                    Receive_State = MSTP_RECEIVE_STATE_IDLE;
                } else {
                    MSTP_Flag.ReceivedInvalidFrame = true;
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

/* returns true if we need to transition immediately */
static bool MSTP_Master_Node_FSM(
    void)
{
    /* The number of frames sent by this node during a single token hold. */
    /* When this counter reaches the value Nmax_info_frames, the node must */
    /* pass the token. */
    static uint8_t FrameCount;
    /* "Next Station," the MAC address of the node to which This Station
       passes the token. If the Next_Station is unknown, Next_Station shall
       be equal to This_Station. */
    static uint8_t Next_Station;
    /* "Poll Station," the MAC address of the node to which This Station last */
    /* sent a Poll For Master. This is used during token maintenance. */
    static uint8_t Poll_Station;
    /* A counter of transmission retries used for Token and Poll For Master */
    /* transmission. */
    static unsigned RetryCount;
    /* The number of tokens received by this node. When this counter reaches */
    /* the value Npoll, the node polls the address range between TS and NS */
    /* for additional master nodes. TokenCount is set to zero at the end of */
    /* the polling process. */
    static unsigned TokenCount;
    /* next-x-station calculations */
    uint8_t next_poll_station = 0;
    uint8_t next_this_station = 0;
    uint8_t next_next_station = 0;
    /* timeout values */
    uint16_t my_timeout = 10, ns_timeout = 0;
    bool matched = false;
    /* transition immediately to the next state */
    bool transition_now = false;
    /* packet from the PDU Queue */
    struct mstp_pdu_packet *pkt;

    /* auto mode is active */
    if (This_Station == 255) {
        Master_State = MSTP_MASTER_STATE_INITIALIZE;
    }
    /* some calculations that several states need */
    next_poll_station = (Poll_Station + 1) % (Nmax_master + 1);
    next_this_station = (This_Station + 1) % (Nmax_master + 1);
    next_next_station = (Next_Station + 1) % (Nmax_master + 1);
    switch (Master_State) {
        case MSTP_MASTER_STATE_INITIALIZE:
            if (This_Station == 255) {
                dlmstp_automac_hander();
                if (This_Station != 255) {
                    Next_Station = automac_next_station(This_Station);
                    if (Next_Station == 255) {
                        /* indicate that the next station is unknown */
                        Next_Station = This_Station;
                        Poll_Station = This_Station;
                        TokenCount = Npoll;
                        EventCount = 0;
                        MSTP_Flag.SoleMaster = true;
                        Master_State = MSTP_MASTER_STATE_POLL_FOR_MASTER;
                    } else {
                        Poll_Station = This_Station;
                        TokenCount = 1;
                        EventCount = 0;
                        RetryCount = 0;
                        MSTP_Flag.SoleMaster = false;
                        Master_State = MSTP_MASTER_STATE_PASS_TOKEN;
                    }
                }
            } else {
                /* DoneInitializing */
                /* indicate that the next station is unknown */
                Next_Station = This_Station;
                Poll_Station = This_Station;
                /* cause a Poll For Master to be sent when this node first */
                /* receives the token */
                TokenCount = Npoll;
                MSTP_Flag.SoleMaster = false;
                Master_State = MSTP_MASTER_STATE_IDLE;
                transition_now = true;
            }
            break;
        case MSTP_MASTER_STATE_IDLE:
            /* In the IDLE state, the node waits for a frame. */
            if (rs485_silence_elapsed(Tno_token)) {
                /* LostToken */
                /* assume that the token has been lost */
                EventCount = 0; /* Addendum 135-2004d-8 */
                /* set the receive frame flags to false in case we received
                   some bytes and had a timeout for some reason */
                MSTP_Flag.ReceivedValidFrame = false;
                MSTP_Flag.ReceivedInvalidFrame = false;
                MSTP_Flag.ReceivedValidFrameNotForUs = false;
                Master_State = MSTP_MASTER_STATE_NO_TOKEN;
                transition_now = true;
            } else if (MSTP_Flag.ReceivedInvalidFrame == true) {
                /* ReceivedInvalidFrame */
                /* invalid frame was received */
                MSTP_Flag.ReceivedInvalidFrame = false;
                /* wait for the next frame - remain in IDLE */
            } else if (MSTP_Flag.ReceivedValidFrame == true) {
                switch (FrameType) {
                    case FRAME_TYPE_TOKEN:
                        /* ReceivedToken */
                        /* tokens can't be broadcast */
                        if (DestinationAddress == MSTP_BROADCAST_ADDRESS)
                            break;
                        MSTP_Flag.ReceivedValidFrame = false;
                        FrameCount = 0;
                        MSTP_Flag.SoleMaster = false;
                        Master_State = MSTP_MASTER_STATE_USE_TOKEN;
                        transition_now = true;
                        break;
                    case FRAME_TYPE_POLL_FOR_MASTER:
                        /* ReceivedPFM */
                        MSTP_Send_Frame(FRAME_TYPE_REPLY_TO_POLL_FOR_MASTER,
                            SourceAddress, This_Station, NULL, 0);
                        break;
                    case FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY:
                        /* indicate successful reception to the higher layers */
                        MSTP_Flag.ReceivePacketPending = true;
                        break;
                    case FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY:
                        /* indicate successful reception to higher layers */
                        MSTP_Flag.ReceivePacketPending = true;
                        /* broadcast DER just remains IDLE */
                        if (DestinationAddress != MSTP_BROADCAST_ADDRESS) {
                            Master_State =
                                MSTP_MASTER_STATE_ANSWER_DATA_REQUEST;
                        }
                        break;
                    case FRAME_TYPE_TEST_REQUEST:
                        MSTP_Send_Frame(FRAME_TYPE_TEST_RESPONSE,
                            SourceAddress, This_Station, &InputBuffer[0],
                            DataLength);
                        break;
                    case FRAME_TYPE_TEST_RESPONSE:
                    default:
                        break;
                }
                /* For DATA_EXPECTING_REPLY, we will keep the Rx Frame for
                   reference, and the flag will be cleared in the next state */
                if (Master_State != MSTP_MASTER_STATE_ANSWER_DATA_REQUEST) {
                    MSTP_Flag.ReceivedValidFrame = false;
                }
            }
            break;
            /* In the USE_TOKEN state, the node is allowed to send one or  */
            /* more data frames. These may be BACnet Data frames or */
            /* proprietary frames. */
        case MSTP_MASTER_STATE_USE_TOKEN:
            /* Note: We could wait for up to Tusage_delay */
            if (Ringbuf_Empty(&PDU_Queue)) {
                /* NothingToSend */
                FrameCount = Nmax_info_frames;
                Master_State = MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                transition_now = true;
            } else {
                uint8_t frame_type;
                pkt = (struct mstp_pdu_packet *) Ringbuf_Peek(&PDU_Queue);
                if (pkt->data_expecting_reply) {
                    frame_type = FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY;
                } else {
                    frame_type = FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY;
                }
                MSTP_Send_Frame(frame_type, pkt->destination_mac, This_Station,
                    (uint8_t *) & pkt->buffer[0], pkt->length);
                FrameCount++;
                switch (frame_type) {
                    case FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY:
                        /* SendAndWait */
                        if (pkt->destination_mac == MSTP_BROADCAST_ADDRESS)
                            Master_State = MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                        else
                            Master_State = MSTP_MASTER_STATE_WAIT_FOR_REPLY;
                        break;
                    case FRAME_TYPE_TEST_REQUEST:
                        Master_State = MSTP_MASTER_STATE_WAIT_FOR_REPLY;
                        break;
                    case FRAME_TYPE_TEST_RESPONSE:
                    case FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY:
                    default:
                        /* SendNoWait */
                        Master_State = MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                        break;
                }
                (void) Ringbuf_Pop(&PDU_Queue, NULL);
            }
            break;
        case MSTP_MASTER_STATE_WAIT_FOR_REPLY:
            /* In the WAIT_FOR_REPLY state, the node waits for  */
            /* a reply from another node. */
            if (rs485_silence_elapsed(Treply_timeout)) {
                /* ReplyTimeout */
                /* assume that the request has failed */
                FrameCount = Nmax_info_frames;
                Master_State = MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                /* Any retry of the data frame shall await the next entry */
                /* to the USE_TOKEN state. */
                /* (Because of the length of the timeout,  */
                /* this transition will cause the token to be */
                /* passed regardless */
                /* of the initial value of FrameCount.) */
                transition_now = true;
            } else {
                if (MSTP_Flag.ReceivedInvalidFrame == true) {
                    /* InvalidFrame */
                    /* error in frame reception */
                    MSTP_Flag.ReceivedInvalidFrame = false;
                    Master_State = MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                    transition_now = true;
                } else if (MSTP_Flag.ReceivedValidFrame == true) {
                    if (DestinationAddress == This_Station) {
                        /* What did we receive? */
                        switch (FrameType) {
                            case FRAME_TYPE_REPLY_POSTPONED:
                                /* ReceivedReplyPostponed */
                                Master_State =
                                    MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                                break;
                            case FRAME_TYPE_TEST_RESPONSE:
                                Master_State =
                                    MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                                break;
                            case FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY:
                                /* ReceivedReply */
                                /* or a proprietary type that indicates
                                   a reply */
                                /* indicate successful reception to
                                   the higher layers */
                                MSTP_Flag.ReceivePacketPending = true;
                                Master_State =
                                    MSTP_MASTER_STATE_DONE_WITH_TOKEN;
                                break;
                            default:
                                /* if proprietary frame was expected, you might
                                   need to transition to DONE WITH TOKEN */
                                Master_State = MSTP_MASTER_STATE_IDLE;
                                break;
                        }
                    } else {
                        /* ReceivedUnexpectedFrame */
                        /* an unexpected frame was received */
                        /* This may indicate the presence of multiple tokens */
                        /* or a device that didn't see activity after passing */
                        /* a token (how lame!). */
                        /* Synchronize with the network. */
                        /* This action drops the token. */
                        Master_State = MSTP_MASTER_STATE_IDLE;
                    }
                    MSTP_Flag.ReceivedValidFrame = false;
                    transition_now = true;
                }
            }
            break;
            /* The DONE_WITH_TOKEN state either sends another data frame,  */
            /* passes the token, or initiates a Poll For Master cycle. */
        case MSTP_MASTER_STATE_DONE_WITH_TOKEN:
            /* SendAnotherFrame */
            if (FrameCount < Nmax_info_frames) {
                /* then this node may send another information frame  */
                /* before passing the token.  */
                Master_State = MSTP_MASTER_STATE_USE_TOKEN;
                transition_now = true;
            } else if ((MSTP_Flag.SoleMaster == false) &&
                (Next_Station == This_Station)) {
                /* NextStationUnknown - added in Addendum 135-2008v-1 */
                /*  then the next station to which the token
                   should be sent is unknown - so PollForMaster */
                Poll_Station = next_this_station;
                MSTP_Send_Frame(FRAME_TYPE_POLL_FOR_MASTER, Poll_Station,
                    This_Station, NULL, 0);
                RetryCount = 0;
                Master_State = MSTP_MASTER_STATE_POLL_FOR_MASTER;
            }
            /* Npoll changed in Errata SSPC-135-2004 */
            else if (TokenCount < (Npoll - 1)) {
                if ((MSTP_Flag.SoleMaster == true) &&
                    (Next_Station != next_this_station)) {
                    /* SoleMaster */
                    /* there are no other known master nodes to */
                    /* which the token may be sent
                       (true master-slave operation).  */
                    FrameCount = 0;
                    TokenCount++;
                    Master_State = MSTP_MASTER_STATE_USE_TOKEN;
                    transition_now = true;
                } else {
                    /* SendToken */
                    /* Npoll changed in Errata SSPC-135-2004 */
                    /* The comparison of NS and TS+1
                       eliminates the Poll For Master
                       if there are no addresses between
                       TS and NS, since there is no
                       address at which a new master node
                       may be found in that case. */
                    TokenCount++;
                    /* transmit a Token frame to NS */
                    MSTP_Send_Frame(FRAME_TYPE_TOKEN, Next_Station,
                        This_Station, NULL, 0);
                    RetryCount = 0;
                    EventCount = 0;
                    Master_State = MSTP_MASTER_STATE_PASS_TOKEN;
                }
            } else if (next_poll_station == Next_Station) {
                if (MSTP_Flag.SoleMaster == true) {
                    /* SoleMasterRestartMaintenancePFM */
                    Poll_Station = next_next_station;
                    MSTP_Send_Frame(FRAME_TYPE_POLL_FOR_MASTER, Poll_Station,
                        This_Station, NULL, 0);
                    /* no known successor node */
                    Next_Station = This_Station;
                    RetryCount = 0;
                    TokenCount = 1;     /* changed in Errata SSPC-135-2004 */
                    /* EventCount = 0; removed in Addendum 135-2004d-8 */
                    /* find a new successor to TS */
                    Master_State = MSTP_MASTER_STATE_POLL_FOR_MASTER;
                } else {
                    /* ResetMaintenancePFM */
                    Poll_Station = This_Station;
                    /* transmit a Token frame to NS */
                    MSTP_Send_Frame(FRAME_TYPE_TOKEN, Next_Station,
                        This_Station, NULL, 0);
                    RetryCount = 0;
                    TokenCount = 1;     /* changed in Errata SSPC-135-2004 */
                    EventCount = 0;
                    Master_State = MSTP_MASTER_STATE_PASS_TOKEN;
                }
            } else {
                /* SendMaintenancePFM */
                Poll_Station = next_poll_station;
                MSTP_Send_Frame(FRAME_TYPE_POLL_FOR_MASTER, Poll_Station,
                    This_Station, NULL, 0);
                RetryCount = 0;
                Master_State = MSTP_MASTER_STATE_POLL_FOR_MASTER;
            }
            break;
            /* The PASS_TOKEN state listens for a successor to begin using */
            /* the token that this node has just attempted to pass. */
        case MSTP_MASTER_STATE_PASS_TOKEN:
            if (rs485_silence_elapsed(Tusage_timeout)) {
                if (RetryCount < Nretry_token) {
                    /* RetrySendToken */
                    RetryCount++;
                    /* Transmit a Token frame to NS */
                    MSTP_Send_Frame(FRAME_TYPE_TOKEN, Next_Station,
                        This_Station, NULL, 0);
                    EventCount = 0;
                    /* re-enter the current state to listen for NS  */
                    /* to begin using the token. */
                } else {
                    /* FindNewSuccessor */
                    /* Assume that NS has failed.  */
                    /* note: if NS=TS-1, this node could send PFM to self! */
                    Poll_Station = next_next_station;
                    /* Transmit a Poll For Master frame to PS. */
                    MSTP_Send_Frame(FRAME_TYPE_POLL_FOR_MASTER, Poll_Station,
                        This_Station, NULL, 0);
                    /* no known successor node */
                    Next_Station = This_Station;
                    RetryCount = 0;
                    TokenCount = 0;
                    /* EventCount = 0; removed in Addendum 135-2004d-8 */
                    /* find a new successor to TS */
                    Master_State = MSTP_MASTER_STATE_POLL_FOR_MASTER;
                }
            } else {
                if (EventCount > Nmin_octets) {
                    /* SawTokenUser */
                    /* Assume that a frame has been sent by
                       the new token user.  */
                    /* Enter the IDLE state to process the frame. */
                    Master_State = MSTP_MASTER_STATE_IDLE;
                    transition_now = true;
                }
            }
            break;
            /* The NO_TOKEN state is entered if Silence Timer
               becomes greater than Tno_token, indicating that
               there has been no network activity for that period
               of time. The timeout is continued to determine
               whether or not this node may create a token. */
        case MSTP_MASTER_STATE_NO_TOKEN:
            my_timeout = Tno_token + (Tslot * This_Station);
            if (rs485_silence_elapsed(my_timeout)) {
                ns_timeout = Tno_token + (Tslot * (This_Station + 1));
                if (rs485_silence_elapsed(ns_timeout)) {
                    /* should never get here unless timer resolution is bad */
                    rs485_silence_reset();
                    Master_State = MSTP_MASTER_STATE_IDLE;
                } else {
                    /* GenerateToken */
                    /* Assume that this node is the lowest numerical address  */
                    /* on the network and is empowered to create a token.  */
                    Poll_Station = next_this_station;
                    /* Transmit a Poll For Master frame to PS. */
                    MSTP_Send_Frame(FRAME_TYPE_POLL_FOR_MASTER, Poll_Station,
                        This_Station, NULL, 0);
                    /* indicate that the next station is unknown */
                    Next_Station = This_Station;
                    RetryCount = 0;
                    TokenCount = 0;
                    /* EventCount = 0; removed Addendum 135-2004d-8 */
                    /* enter the POLL_FOR_MASTER state
                       to find a new successor to TS. */
                    Master_State = MSTP_MASTER_STATE_POLL_FOR_MASTER;
                }
            } else {
                if (EventCount > Nmin_octets) {
                    /* SawFrame */
                    /* Some other node exists at a lower address.  */
                    /* Enter the IDLE state to receive and
                       process the incoming frame. */
                    Master_State = MSTP_MASTER_STATE_IDLE;
                    transition_now = true;
                }
            }
            break;
            /* In the POLL_FOR_MASTER state, the node listens for a reply to */
            /* a previously sent Poll For Master frame in order to find  */
            /* a successor node. */
        case MSTP_MASTER_STATE_POLL_FOR_MASTER:
            if (MSTP_Flag.ReceivedValidFrame == true) {
                if ((DestinationAddress == This_Station)
                    && (FrameType == FRAME_TYPE_REPLY_TO_POLL_FOR_MASTER)) {
                    /* ReceivedReplyToPFM */
                    MSTP_Flag.SoleMaster = false;
                    Next_Station = SourceAddress;
                    EventCount = 0;
                    /* Transmit a Token frame to NS */
                    MSTP_Send_Frame(FRAME_TYPE_TOKEN, Next_Station,
                        This_Station, NULL, 0);
                    Poll_Station = This_Station;
                    TokenCount = 0;
                    RetryCount = 0;
                    Master_State = MSTP_MASTER_STATE_PASS_TOKEN;
                } else {
                    /* ReceivedUnexpectedFrame */
                    /* An unexpected frame was received.  */
                    /* This may indicate the presence of multiple tokens. */
                    /* enter the IDLE state to synchronize with the network.  */
                    /* This action drops the token. */
                    Master_State = MSTP_MASTER_STATE_IDLE;
                    transition_now = true;
                }
                MSTP_Flag.ReceivedValidFrame = false;
            } else if ((rs485_silence_elapsed(Tusage_timeout)) ||
                (MSTP_Flag.ReceivedInvalidFrame == true)) {
                if (MSTP_Flag.SoleMaster == true) {
                    /* SoleMaster */
                    /* There was no valid reply to the periodic poll  */
                    /* by the sole known master for other masters. */
                    FrameCount = 0;
                    /* TokenCount++; removed in 2004 */
                    Master_State = MSTP_MASTER_STATE_USE_TOKEN;
                    transition_now = true;
                } else {
                    if (Next_Station != This_Station) {
                        /* DoneWithPFM */
                        /* There was no valid reply to the maintenance  */
                        /* poll for a master at address PS.  */
                        EventCount = 0;
                        /* transmit a Token frame to NS */
                        MSTP_Send_Frame(FRAME_TYPE_TOKEN, Next_Station,
                            This_Station, NULL, 0);
                        RetryCount = 0;
                        Master_State = MSTP_MASTER_STATE_PASS_TOKEN;
                    } else {
                        if (next_poll_station != This_Station) {
                            /* SendNextPFM */
                            Poll_Station = next_poll_station;
                            /* Transmit a Poll For Master frame to PS. */
                            MSTP_Send_Frame(FRAME_TYPE_POLL_FOR_MASTER,
                                Poll_Station, This_Station, NULL, 0);
                            RetryCount = 0;
                            /* Re-enter the current state. */
                        } else {
                            /* DeclareSoleMaster */
                            /* to indicate that this station
                               is the only master */
                            MSTP_Flag.SoleMaster = true;
                            FrameCount = 0;
                            Master_State = MSTP_MASTER_STATE_USE_TOKEN;
                            transition_now = true;
                        }
                    }
                }
                MSTP_Flag.ReceivedInvalidFrame = false;
            }
            break;
            /* The ANSWER_DATA_REQUEST state is entered when a  */
            /* BACnet Data Expecting Reply, a Test_Request, or  */
            /* a proprietary frame that expects a reply is received. */
        case MSTP_MASTER_STATE_ANSWER_DATA_REQUEST:
            pkt = (struct mstp_pdu_packet *) Ringbuf_Peek(&PDU_Queue);
            if (pkt != NULL) {
                matched =
                    dlmstp_compare_data_expecting_reply(&InputBuffer[0],
                    DataLength, SourceAddress, &pkt->buffer[0], pkt->length,
                    pkt->destination_mac);
            } else {
                matched = false;
            }
            if (matched) {
                /* Reply */
                /* If a reply is available from the higher layers  */
                /* within Treply_delay after the reception of the  */
                /* final octet of the requesting frame  */
                /* (the mechanism used to determine this is a local matter), */
                /* then call MSTP_Send_Frame to transmit the reply frame  */
                /* and enter the IDLE state to wait for the next frame. */
                uint8_t frame_type;
                if (pkt->data_expecting_reply) {
                    frame_type = FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY;
                } else {
                    frame_type = FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY;
                }
                MSTP_Send_Frame(frame_type, pkt->destination_mac, This_Station,
                    (uint8_t *) & pkt->buffer[0], pkt->length);
                Master_State = MSTP_MASTER_STATE_IDLE;
                /* clear our flag we were holding for comparison */
                MSTP_Flag.ReceivedValidFrame = false;
                /* clear the queue */
                (void) Ringbuf_Pop(&PDU_Queue, NULL);
            } else if (rs485_silence_elapsed(Treply_delay) || (pkt != NULL)) {
                /* DeferredReply */
                /* If no reply will be available from the higher layers */
                /* within Treply_delay after the reception of the */
                /* final octet of the requesting frame (the mechanism */
                /* used to determine this is a local matter), */
                /* then an immediate reply is not possible. */
                /* Any reply shall wait until this node receives the token. */
                /* Call MSTP_Send_Frame to transmit a Reply Postponed frame, */
                /* and enter the IDLE state. */
                MSTP_Send_Frame(FRAME_TYPE_REPLY_POSTPONED, SourceAddress,
                    This_Station, NULL, 0);
                Master_State = MSTP_MASTER_STATE_IDLE;
                /* clear our flag we were holding for comparison */
                MSTP_Flag.ReceivedValidFrame = false;
            }
            break;
        default:
            Master_State = MSTP_MASTER_STATE_IDLE;
            break;
    }

    return transition_now;
}

/* returns number of bytes sent on success, zero on failure */
int dlmstp_send_pdu(
    BACNET_ADDRESS * dest,      /* destination address */
    BACNET_NPDU_DATA * npdu_data,       /* network information */
    uint8_t * pdu,      /* any data to be sent - may be null */
    unsigned pdu_len)
{       /* number of bytes of data */
    int bytes_sent = 0;
    struct mstp_pdu_packet *pkt;
    uint16_t i = 0;

    pkt = (struct mstp_pdu_packet *) Ringbuf_Data_Peek(&PDU_Queue);
    if (pkt) {
        pkt->data_expecting_reply = npdu_data->data_expecting_reply;
        for (i = 0; i < pdu_len; i++) {
            pkt->buffer[i] = pdu[i];
        }
        pkt->length = pdu_len;
        if (dest && dest->mac_len) {
            pkt->destination_mac = dest->mac[0];
        } else {
            pkt->destination_mac = MSTP_BROADCAST_ADDRESS;
        }
        if (Ringbuf_Data_Put(&PDU_Queue, (uint8_t *)pkt)) {
            bytes_sent = pdu_len;
        }
    }

    return bytes_sent;
}

/* master node FSM states */
typedef enum {
    AUTOMAC_STATE_IDLE = 0,
    AUTOMAC_STATE_PFM = 1,
    AUTOMAC_STATE_TOKEN = 2,
    AUTOMAC_STATE_TESTING = 3,
    AUTOMAC_STATE_CONFIRM = 4
} AUTOMAC_STATE;
/* buffer used to send and validate a response - size is min APDU size */
static uint8_t AutoMAC_Test_Buffer[50];
void dlmstp_automac_hander(
    void)
{
    static AUTOMAC_STATE state = AUTOMAC_STATE_IDLE;
    uint8_t mac = 0;
    uint32_t serial_number = 0;
    uint16_t vendor_id = 0;
    bool take_address = false;
    bool start_over = false;

    switch (state) {
        case AUTOMAC_STATE_IDLE:
            if ((MSTP_Flag.ReceivedValidFrame) ||
                (MSTP_Flag.ReceivedValidFrameNotForUs)) {
                MSTP_Flag.ReceivedValidFrame = false;
                MSTP_Flag.ReceivedValidFrameNotForUs = false;
                /* store stats until we get a MAC */
                automac_emitter_set(SourceAddress);
                switch (FrameType) {
                    case FRAME_TYPE_TOKEN:
                        automac_token_set(SourceAddress);
                        break;
                    case FRAME_TYPE_POLL_FOR_MASTER:
                        automac_pfm_set(DestinationAddress);
                        break;
                    default:
                        break;
                }
            } else if (MSTP_Flag.ReceivedInvalidFrame) {
                MSTP_Flag.ReceivedInvalidFrame = false;
            } else if (automac_pfm_cycle_complete()) {
                mac = automac_free_address_random();
                if (automac_free_address_valid(mac)) {
                    automac_address_set(mac);
                    state = AUTOMAC_STATE_PFM;
                } else {
                    /* start over again */
                    automac_init();
                    state = AUTOMAC_STATE_IDLE;
                }
            } else if (rs485_silence_elapsed(automac_time_slot())) {
                /*  long silence indicates we are alone or
                   with other silent devices */
                SourceAddress = automac_address();
                state = AUTOMAC_STATE_TESTING;
            }
            break;
        case AUTOMAC_STATE_PFM:
            if ((MSTP_Flag.ReceivedValidFrame) ||
                (MSTP_Flag.ReceivedValidFrameNotForUs)) {
                MSTP_Flag.ReceivedValidFrame = false;
                MSTP_Flag.ReceivedValidFrameNotForUs = false;
                /* store stats until we get a MAC */
                switch (FrameType) {
                    case FRAME_TYPE_POLL_FOR_MASTER:
                        mac = automac_address();
                        if (mac == SourceAddress) {
                            /* start over again */
                            automac_init();
                            state = AUTOMAC_STATE_IDLE;
                        } else if (mac == DestinationAddress) {
                            MSTP_Send_Frame
                                (FRAME_TYPE_REPLY_TO_POLL_FOR_MASTER,
                                SourceAddress, mac, NULL, 0);
                            state = AUTOMAC_STATE_TOKEN;
                        }
                        break;
                    case FRAME_TYPE_TEST_REQUEST:
                        mac = automac_address();
                        if ((mac == DestinationAddress) ||
                            (mac == SourceAddress)) {
                            /* start over again */
                            automac_init();
                            state = AUTOMAC_STATE_IDLE;
                        }
                        break;
                    case FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY:
                    case FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY:
                    case FRAME_TYPE_TEST_RESPONSE:
                    case FRAME_TYPE_TOKEN:
                        mac = automac_address();
                        if (mac == SourceAddress) {
                            /* start over again */
                            automac_init();
                            state = AUTOMAC_STATE_IDLE;
                        }
                        break;
                    default:
                        break;
                }
            } else if (MSTP_Flag.ReceivedInvalidFrame) {
                MSTP_Flag.ReceivedInvalidFrame = false;
            } else if (rs485_silence_elapsed(automac_time_slot())) {
                /* start over again */
                automac_init();
                state = AUTOMAC_STATE_IDLE;
            }
            break;
        case AUTOMAC_STATE_TOKEN:
            if ((MSTP_Flag.ReceivedValidFrame) ||
                (MSTP_Flag.ReceivedValidFrameNotForUs)) {
                MSTP_Flag.ReceivedValidFrame = false;
                MSTP_Flag.ReceivedValidFrameNotForUs = false;
                switch (FrameType) {
                    case FRAME_TYPE_TOKEN:
                        mac = automac_address();
                        if (mac == SourceAddress) {
                            /* start over again */
                            automac_init();
                            state = AUTOMAC_STATE_IDLE;
                        } else if (mac == DestinationAddress) {
                            state = AUTOMAC_STATE_TESTING;
                        }
                        break;
                    default:
                        /* start over again */
                        automac_init();
                        state = AUTOMAC_STATE_IDLE;
                        break;
                }
            } else if (MSTP_Flag.ReceivedInvalidFrame) {
                MSTP_Flag.ReceivedInvalidFrame = false;
            } else if (rs485_silence_elapsed(automac_time_slot())) {
                /* start over again */
                automac_init();
                state = AUTOMAC_STATE_IDLE;
            }
            break;
        case AUTOMAC_STATE_TESTING:
            /* I have the token - confirm my MAC with a quick test */
            mac = automac_address();
            vendor_id = Device_Vendor_Identifier();
            encode_unsigned16(&AutoMAC_Test_Buffer[0], vendor_id);
            serial_number = Device_Object_Instance_Number();
            encode_unsigned32(&AutoMAC_Test_Buffer[2], serial_number);
            MSTP_Send_Frame(FRAME_TYPE_TEST_REQUEST, SourceAddress, mac,
                &AutoMAC_Test_Buffer[0], 6);
            state = AUTOMAC_STATE_CONFIRM;
            break;
        case AUTOMAC_STATE_CONFIRM:
            /* we may timeout if our chosen MAC is unique */
            if (MSTP_Flag.ReceivedInvalidFrame) {
                MSTP_Flag.ReceivedInvalidFrame = false;
                start_over = true;
            } else if ((MSTP_Flag.ReceivedValidFrame) ||
                (MSTP_Flag.ReceivedValidFrameNotForUs)) {
                MSTP_Flag.ReceivedValidFrame = false;
                MSTP_Flag.ReceivedValidFrameNotForUs = false;
                mac = automac_address();
                if ((mac == DestinationAddress) && (DataLength >= 6)) {
                    decode_unsigned16(&InputBuffer[0], &vendor_id);
                    decode_unsigned32(&InputBuffer[2], &serial_number);
                    if ((vendor_id == Device_Vendor_Identifier()) &&
                        (serial_number == Device_Object_Instance_Number())) {
                        take_address = true;
                    } else {
                        start_over = true;
                    }
                } else {
                    start_over = true;
                }
            } else if (rs485_silence_elapsed(300)) {
                /* use maximum possible value for Treply_timeout */
                /* in case validating device doesn't support Test Request */
                /* no response and no collission */
                take_address = true;
            }
            if (take_address) {
                /* take the address */
                This_Station = automac_address();
                DestinationAddress = automac_next_station(This_Station);
                if (DestinationAddress < 128) {
                    MSTP_Send_Frame(FRAME_TYPE_TOKEN, DestinationAddress,
                        This_Station, NULL, 0);
                }
                state = AUTOMAC_STATE_IDLE;
            } else if (start_over) {
                /* start over again */
                automac_init();
                state = AUTOMAC_STATE_IDLE;
            }
            break;
        default:
            break;
    }
}

/* Return the length of the packet */
uint16_t dlmstp_receive(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* PDU data */
    uint16_t max_pdu,   /* amount of space available in the PDU  */
    unsigned timeout)
{       /* milliseconds to wait for a packet */
    uint16_t pdu_len = 0;       /* return value */
    bool transmitting = false;

    /* set the input buffer to the same data storage for zero copy */
    if (!InputBuffer) {
        InputBuffer = pdu;
        InputBufferSize = max_pdu;
    }
    if (This_Station == 255) {
        automac_enabled_set(true);
    }
    if (Receive_State == MSTP_RECEIVE_STATE_IDLE) {
        transmitting = MSTP_Transmit_FSM();
    }
    if (transmitting == false) {
        while ((MSTP_Flag.ReceivedValidFrame == false) &&
            (MSTP_Flag.ReceivedValidFrameNotForUs == false) &&
            (MSTP_Flag.ReceivedInvalidFrame == false)) {
            /* only do receive state machine while we don't have a frame */
            MSTP_Receive_Frame_FSM();
            /* process another byte, if available */
            if (!rs485_byte_available(NULL)) {
                break;
            }
        }
    }
    /* only do master state machine while rx is idle */
    if ((Receive_State == MSTP_RECEIVE_STATE_IDLE) && (transmitting == false)) {
        if ((This_Station != 255) && (MSTP_Flag.ReceivedValidFrameNotForUs)) {
            MSTP_Flag.ReceivedValidFrameNotForUs = false;
            if ((SourceAddress == This_Station) && automac_enabled()) {
                /* duplicate MAC on the wire */
                automac_init();
                This_Station = 255;
            }
        } else {
            while (MSTP_Master_Node_FSM()) {
                /* do nothing while some states fast transition */
            };
        }
    }
    if (This_Station != 255) {
        /* if there is a packet that needs processed, do it now. */
        if (MSTP_Flag.ReceivePacketPending) {
            MSTP_Flag.ReceivePacketPending = false;
            pdu_len = DataLength;
            src->mac_len = 1;
            src->mac[0] = SourceAddress;
            /* data is already in the pdu pointer */
        }
    }

    return pdu_len;
}

void dlmstp_set_mac_address(
    uint8_t mac_address)
{
    /* Master Nodes can only have address 0-127 */
    if (mac_address <= 127) {
        This_Station = mac_address;
        if (mac_address > Nmax_master) {
            dlmstp_set_max_master(127);
        }
    } else if (mac_address == 255) {
        /* auto-MAC provision */
        This_Station = mac_address;
    }

    return;
}

uint8_t dlmstp_mac_address(
    void)
{
    return This_Station;
}

/* This parameter represents the value of the Max_Info_Frames property of */
/* the node's Device object. The value of Max_Info_Frames specifies the */
/* maximum number of information frames the node may send before it must */
/* pass the token. Max_Info_Frames may have different values on different */
/* nodes. This may be used to allocate more or less of the available link */
/* bandwidth to particular nodes. If Max_Info_Frames is not writable in a */
/* node, its value shall be 1. */
void dlmstp_set_max_info_frames(
    uint8_t max_info_frames)
{
    if (max_info_frames >= 1) {
        Nmax_info_frames = max_info_frames;
    }

    return;
}

uint8_t dlmstp_max_info_frames(
    void)
{
    return Nmax_info_frames;
}

/* This parameter represents the value of the Max_Master property of the */
/* node's Device object. The value of Max_Master specifies the highest */
/* allowable address for master nodes. The value of Max_Master shall be */
/* less than or equal to 127. If Max_Master is not writable in a node, */
/* its value shall be 127. */
void dlmstp_set_max_master(
    uint8_t max_master)
{
    if (max_master <= 127) {
        if ((This_Station == 255) || (This_Station <= max_master)) {
            Nmax_master = max_master;
        }
    }

    return;
}

uint8_t dlmstp_max_master(
    void)
{
    return Nmax_master;
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

bool dlmstp_sole_master(
    void)
{
    if (MSTP_Flag.SoleMaster) {
        return true;
    }

    return false;
}
