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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef __BORLANDC__
#include <process.h>
#endif
#include "bacdef.h"
#include "bacaddr.h"
#include "mstp.h"
#include "dlmstp.h"
#include "rs485.h"
#include "npdu.h"
#include "bits.h"
#include "ringbuf.h"
#include "timer.h"

#define WIN32_LEAN_AND_MEAN
#define STRICT 1
#include <windows.h>

/* Number of MS/TP Packets Rx/Tx */
uint16_t MSTP_Packets = 0;

/* packet queues */
static DLMSTP_PACKET Receive_Packet;
static HANDLE Receive_Packet_Flag;
/* mechanism to wait for a frame in state machine */
HANDLE Received_Frame_Flag;
static DLMSTP_PACKET Transmit_Packet;
/* local MS/TP port data - shared with RS-485 */
volatile struct mstp_port_struct_t MSTP_Port;
/* buffers needed by mstp port struct */
static uint8_t TxBuffer[MAX_MPDU];
static uint8_t RxBuffer[MAX_MPDU];
/* The minimum time without a DataAvailable or ReceiveError event */
/* that a node must wait for a station to begin replying to a */
/* confirmed request: 255 milliseconds. (Implementations may use */
/* larger values for this timeout, not to exceed 300 milliseconds.) */
static uint16_t Treply_timeout = 260;
/* The minimum time without a DataAvailable or ReceiveError event that a */
/* node must wait for a remote node to begin using a token or replying to */
/* a Poll For Master frame: 20 milliseconds. (Implementations may use */
/* larger values for this timeout, not to exceed 100 milliseconds.) */
static uint8_t Tusage_timeout = 50;

/* Timer that indicates line silence - and functions */
static uint32_t Timer_Silence(
    void *pArg)
{
    return timer_milliseconds(TIMER_SILENCE);
}

static void Timer_Silence_Reset(
    void *pArg)
{
    timer_reset(TIMER_SILENCE);
}

void dlmstp_cleanup(
    void)
{
    /* nothing to do for static buffers */
    if (Received_Frame_Flag) {
        CloseHandle(Received_Frame_Flag);
    }
    if (Receive_Packet_Flag) {
        CloseHandle(Receive_Packet_Flag);
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
    unsigned i = 0;

    if (!Transmit_Packet.ready) {
        if (npdu_data->data_expecting_reply) {
            Transmit_Packet.frame_type =
                FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY;
        } else {
            Transmit_Packet.frame_type =
                FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY;
        }
        Transmit_Packet.pdu_len = (uint16_t) pdu_len;
        for (i = 0; i < pdu_len; i++) {
            Transmit_Packet.pdu[i] = pdu[i];
        }
        bacnet_address_copy(&Transmit_Packet.address, dest);
        bytes_sent = pdu_len + MAX_HEADER;
        Transmit_Packet.ready = true;
    }

    return bytes_sent;
}

uint16_t dlmstp_receive(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* PDU data */
    uint16_t max_pdu,   /* amount of space available in the PDU  */
    unsigned timeout)
{       /* milliseconds to wait for a packet */
    uint16_t pdu_len = 0;
    DWORD wait_status = 0;

    (void) max_pdu;
    /* see if there is a packet available, and a place
       to put the reply (if necessary) and process it */
    wait_status = WaitForSingleObject(Receive_Packet_Flag, timeout);
    if (wait_status == WAIT_OBJECT_0) {
        if (Receive_Packet.ready) {
            if (Receive_Packet.pdu_len) {
                MSTP_Packets++;
                if (src) {
                    memmove(src, &Receive_Packet.address,
                        sizeof(Receive_Packet.address));
                }
                if (pdu) {
                    memmove(pdu, &Receive_Packet.pdu,
                        sizeof(Receive_Packet.pdu));
                }
                pdu_len = Receive_Packet.pdu_len;
            }
            Receive_Packet.ready = false;
        }
    }

    return pdu_len;
}

static void dlmstp_receive_fsm_task(
    void *pArg)
{
    bool received_frame;

    (void) pArg;
    (void) SetThreadPriority(GetCurrentThread(),
        THREAD_PRIORITY_TIME_CRITICAL);
    for (;;) {
        /* only do receive state machine while we don't have a frame */
        if ((MSTP_Port.ReceivedValidFrame == false) &&
            (MSTP_Port.ReceivedInvalidFrame == false)) {
            do {
                RS485_Check_UART_Data(&MSTP_Port);
                MSTP_Receive_Frame_FSM(&MSTP_Port);
                received_frame = MSTP_Port.ReceivedValidFrame ||
                    MSTP_Port.ReceivedInvalidFrame;
                if (received_frame) {
                    ReleaseSemaphore(Received_Frame_Flag, 1, NULL);
                    break;
                }
            } while (MSTP_Port.DataAvailable);
        }
    }
}

static void dlmstp_master_fsm_task(
    void *pArg)
{
    DWORD dwMilliseconds = 0;

    (void) pArg;
    (void) SetThreadPriority(GetCurrentThread(),
        THREAD_PRIORITY_TIME_CRITICAL);
    for (;;) {
        switch (MSTP_Port.master_state) {
            case MSTP_MASTER_STATE_IDLE:
                dwMilliseconds = Tno_token;
                break;
            case MSTP_MASTER_STATE_WAIT_FOR_REPLY:
                dwMilliseconds = Treply_timeout;
                break;
            case MSTP_MASTER_STATE_POLL_FOR_MASTER:
                dwMilliseconds = Tusage_timeout;
                break;
            default:
                dwMilliseconds = 0;
                break;
        }
        if (dwMilliseconds)
            WaitForSingleObject(Received_Frame_Flag, dwMilliseconds);
        MSTP_Master_Node_FSM(&MSTP_Port);
    }
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
uint16_t MSTP_Put_Receive(
    volatile struct mstp_port_struct_t *mstp_port)
{
    uint16_t pdu_len = 0;
    BOOL rc;

    if (!Receive_Packet.ready) {
        /* bounds check - maybe this should send an abort? */
        pdu_len = mstp_port->DataLength;
        if (pdu_len > sizeof(Receive_Packet.pdu))
            pdu_len = sizeof(Receive_Packet.pdu);
        memmove((void *) &Receive_Packet.pdu[0],
            (void *) &mstp_port->InputBuffer[0], pdu_len);
        dlmstp_fill_bacnet_address(&Receive_Packet.address,
            mstp_port->SourceAddress);
        Receive_Packet.pdu_len = mstp_port->DataLength;
        Receive_Packet.ready = true;
        rc = ReleaseSemaphore(Receive_Packet_Flag, 1, NULL);
        (void) rc;
    }

    return pdu_len;
}

/* for the MS/TP state machine to use for getting data to send */
/* Return: amount of PDU data */
uint16_t MSTP_Get_Send(
    volatile struct mstp_port_struct_t * mstp_port,
    unsigned timeout)
{       /* milliseconds to wait for a packet */
    uint16_t pdu_len = 0;
    uint8_t destination = 0;    /* destination address */

    (void) timeout;
    if (!Transmit_Packet.ready) {
        return 0;
    }
    /* load destination MAC address */
    if (Transmit_Packet.address.mac_len) {
        destination = Transmit_Packet.address.mac[0];
    } else {
        destination = MSTP_BROADCAST_ADDRESS;
    }
    if ((MAX_HEADER + Transmit_Packet.pdu_len) > MAX_MPDU) {
        return 0;
    }
    /* convert the PDU into the MSTP Frame */
    pdu_len = MSTP_Create_Frame(&mstp_port->OutputBuffer[0],    /* <-- loading this */
        mstp_port->OutputBufferSize, Transmit_Packet.frame_type, destination,
        mstp_port->This_Station, &Transmit_Packet.pdu[0],
        Transmit_Packet.pdu_len);
    Transmit_Packet.ready = false;

    return pdu_len;
}

static bool dlmstp_compare_data_expecting_reply(
    uint8_t * request_pdu,
    uint16_t request_pdu_len,
    uint8_t src_address,
    uint8_t * reply_pdu,
    uint16_t reply_pdu_len,
    BACNET_ADDRESS * dest_address)
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

    /* unused parameters */
    request_pdu_len = request_pdu_len;
    reply_pdu_len = reply_pdu_len;
    /* decode the request data */
    request.address.mac[0] = src_address;
    request.address.mac_len = 1;
    offset =
        (uint16_t) npdu_decode(&request_pdu[0], NULL, &request.address,
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
    bacnet_address_copy(&reply.address, dest_address);
    offset =
        (uint16_t) npdu_decode(&reply_pdu[0], &reply.address, NULL,
        &reply.npdu_data);
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
    /* these don't have service choice included */
    if ((reply.pdu_type == PDU_TYPE_REJECT) ||
        (reply.pdu_type == PDU_TYPE_ABORT)) {
        if (request.invoke_id != reply.invoke_id) {
            return false;
        }
    } else {
        if (request.invoke_id != reply.invoke_id) {
            return false;
        }
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

/* Get the reply to a DATA_EXPECTING_REPLY frame, or nothing */
uint16_t MSTP_Get_Reply(
    volatile struct mstp_port_struct_t * mstp_port,
    unsigned timeout)
{       /* milliseconds to wait for a packet */
    uint16_t pdu_len = 0;       /* return value */
    uint8_t destination = 0;    /* destination address */
    bool matched = false;

    (void) timeout;
    if (!Transmit_Packet.ready) {
        return 0;
    }
    /* load destination MAC address */
    if (Transmit_Packet.address.mac_len == 1) {
        destination = Transmit_Packet.address.mac[0];
    } else {
        return 0;
    }
    if ((MAX_HEADER + Transmit_Packet.pdu_len) > MAX_MPDU) {
        return 0;
    }
    /* is this the reply to the DER? */
    matched =
        dlmstp_compare_data_expecting_reply(&mstp_port->InputBuffer[0],
        mstp_port->DataLength, mstp_port->SourceAddress,
        &Transmit_Packet.pdu[0], Transmit_Packet.pdu_len,
        &Transmit_Packet.address);
    if (!matched)
        return 0;
    /* convert the PDU into the MSTP Frame */
    pdu_len = MSTP_Create_Frame(&mstp_port->OutputBuffer[0],    /* <-- loading this */
        mstp_port->OutputBufferSize, Transmit_Packet.frame_type, destination,
        mstp_port->This_Station, &Transmit_Packet.pdu[0],
        Transmit_Packet.pdu_len);
    Transmit_Packet.ready = false;

    return pdu_len;
}

void dlmstp_set_mac_address(
    uint8_t mac_address)
{
    /* Master Nodes can only have address 0-127 */
    if (mac_address <= 127) {
        MSTP_Port.This_Station = mac_address;
        /* FIXME: implement your data storage */
        /* I2C_Write_Byte(
           EEPROM_DEVICE_ADDRESS,
           mac_address,
           EEPROM_MSTP_MAC_ADDR); */
        if (mac_address > MSTP_Port.Nmax_master)
            dlmstp_set_max_master(mac_address);
    }

    return;
}

uint8_t dlmstp_mac_address(
    void)
{
    return MSTP_Port.This_Station;
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
        MSTP_Port.Nmax_info_frames = max_info_frames;
        /* FIXME: implement your data storage */
        /* I2C_Write_Byte(
           EEPROM_DEVICE_ADDRESS,
           (uint8_t)max_info_frames,
           EEPROM_MSTP_MAX_INFO_FRAMES_ADDR); */
    }

    return;
}

uint8_t dlmstp_max_info_frames(
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
    if (max_master <= 127) {
        if (MSTP_Port.This_Station <= max_master) {
            MSTP_Port.Nmax_master = max_master;
            /* FIXME: implement your data storage */
            /* I2C_Write_Byte(
               EEPROM_DEVICE_ADDRESS,
               max_master,
               EEPROM_MSTP_MAX_MASTER_ADDR); */
        }
    }

    return;
}

uint8_t dlmstp_max_master(
    void)
{
    return MSTP_Port.Nmax_master;
}

/* RS485 Baud Rate 9600, 19200, 38400, 57600, 115200 */
void dlmstp_set_baud_rate(
    uint32_t baud)
{
    RS485_Set_Baud_Rate(baud);
}

uint32_t dlmstp_baud_rate(
    void)
{
    return RS485_Get_Baud_Rate();
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
        dest->len = 0;  /* always zero when DNET is broadcast */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            dest->adr[i] = 0;
        }
    }

    return;
}

bool dlmstp_init(
    char *ifname)
{
    unsigned long hThread = 0;
    uint32_t arg_value = 0;

    /* initialize packet queue */
    Receive_Packet.ready = false;
    Receive_Packet.pdu_len = 0;
    Receive_Packet_Flag = CreateSemaphore(NULL, 0, 1, "dlmstpReceivePacket");
    if (Receive_Packet_Flag == NULL)
        exit(1);
    Received_Frame_Flag = CreateSemaphore(NULL, 0, 1, "dlsmtpReceiveFrame");
    if (Received_Frame_Flag == NULL) {
        CloseHandle(Receive_Packet_Flag);
        exit(1);
    }
    /* initialize hardware */
    timer_init();
    if (ifname) {
        RS485_Set_Interface(ifname);
    }
    RS485_Initialize();
    MSTP_Port.InputBuffer = &RxBuffer[0];
    MSTP_Port.InputBufferSize = sizeof(RxBuffer);
    MSTP_Port.OutputBuffer = &TxBuffer[0];
    MSTP_Port.OutputBufferSize = sizeof(TxBuffer);
    MSTP_Port.SilenceTimer = Timer_Silence;
    MSTP_Port.SilenceTimerReset = Timer_Silence_Reset;
    MSTP_Init(&MSTP_Port);
#if 0
    uint8_t data;

    /* FIXME: implement your data storage */
    data = 64;  /* I2C_Read_Byte(
                   EEPROM_DEVICE_ADDRESS,
                   EEPROM_MSTP_MAC_ADDR); */
    if (data <= 127)
        MSTP_Port.This_Station = data;
    else
        dlmstp_set_my_address(DEFAULT_MAC_ADDRESS);
    /* FIXME: implement your data storage */
    data = 127; /* I2C_Read_Byte(
                   EEPROM_DEVICE_ADDRESS,
                   EEPROM_MSTP_MAX_MASTER_ADDR); */
    if ((data <= 127) && (data >= MSTP_Port.This_Station))
        MSTP_Port.Nmax_master = data;
    else
        dlmstp_set_max_master(DEFAULT_MAX_MASTER);
    /* FIXME: implement your data storage */
    data = 1;
    /* I2C_Read_Byte(
       EEPROM_DEVICE_ADDRESS,
       EEPROM_MSTP_MAX_INFO_FRAMES_ADDR); */
    if (data >= 1)
        MSTP_Port.Nmax_info_frames = data;
    else
        dlmstp_set_max_info_frames(DEFAULT_MAX_INFO_FRAMES);
#endif
#if PRINT_ENABLED
    fprintf(stderr, "MS/TP MAC: %02X\n", MSTP_Port.This_Station);
    fprintf(stderr, "MS/TP Max_Master: %02X\n", MSTP_Port.Nmax_master);
    fprintf(stderr, "MS/TP Max_Info_Frames: %u\n",
        (unsigned) MSTP_Port.Nmax_info_frames);
#endif
    hThread = _beginthread(dlmstp_receive_fsm_task, 4096, &arg_value);
    if (hThread == 0) {
        fprintf(stderr, "Failed to start recive FSM task\n");
    }
    hThread = _beginthread(dlmstp_master_fsm_task, 4096, &arg_value);
    if (hThread == 0) {
        fprintf(stderr, "Failed to start Master Node FSM task\n");
    }

    return true;
}

#ifdef TEST_DLMSTP
#include <stdio.h>

void apdu_handler(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * apdu,     /* APDU data */
    uint16_t pdu_len)
{       /* for confirmed messages */
    (void) src;
    (void) apdu;
    (void) pdu_len;
}

/* returns a delta timestamp */
uint32_t timestamp_ms(
    void)
{
    DWORD ticks = 0, delta_ticks = 0;
    static DWORD last_ticks = 0;

    ticks = GetTickCount();
    delta_ticks =
        (ticks >= last_ticks ? ticks - last_ticks : MAXDWORD - last_ticks);
    last_ticks = ticks;

    return delta_ticks;
}

static char *Network_Interface = NULL;

int main(
    int argc,
    char *argv[])
{
    uint16_t pdu_len = 0;

    /* argv has the "COM4" or some other device */
    if (argc > 1) {
        Network_Interface = argv[1];
    }
    dlmstp_set_baud_rate(38400);
    dlmstp_set_mac_address(0x05);
    dlmstp_set_max_info_frames(DEFAULT_MAX_INFO_FRAMES);
    dlmstp_set_max_master(DEFAULT_MAX_MASTER);
    dlmstp_init(Network_Interface);
    /* forever task */
    for (;;) {
        pdu_len = dlmstp_receive(NULL, NULL, 0, INFINITE);
#if 0
        MSTP_Create_And_Send_Frame(&MSTP_Port, FRAME_TYPE_TEST_REQUEST,
            MSTP_Port.SourceAddress, MSTP_Port.This_Station, NULL, 0);
#endif
    }

    return 0;
}
#endif
