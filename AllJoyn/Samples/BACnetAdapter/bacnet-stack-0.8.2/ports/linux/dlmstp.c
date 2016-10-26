/**************************************************************************
*
* Copyright (C) 2008 Steve Karg <skarg@users.sourceforge.net>
* Updated by Nikola Jelic 2011 <nikola.jelic@euroicc.com>
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
#include <sys/time.h>
#include "bacdef.h"
#include "bacaddr.h"
#include "mstp.h"
#include "dlmstp.h"
#include "rs485.h"
#include "npdu.h"
#include "bits.h"
#include "ringbuf.h"
#include "debug.h"
/* OS Specific include */
#include "net.h"

/** @file linux/dlmstp.c  Provides Linux-specific DataLink functions for MS/TP. */

/* Number of MS/TP Packets Rx/Tx */
uint16_t MSTP_Packets = 0;

/* packet queues */
static DLMSTP_PACKET Receive_Packet;
/* mechanism to wait for a packet */
/*
static RT_COND Receive_Packet_Flag;
static RT_MUTEX Receive_Packet_Mutex;
*/
static pthread_cond_t Receive_Packet_Flag;
static pthread_mutex_t Receive_Packet_Mutex;
/* mechanism to wait for a frame in state machine */
/*
static RT_COND Received_Frame_Flag;
static RT_MUTEX Received_Frame_Mutex;
*/

static pthread_cond_t Received_Frame_Flag;
static pthread_mutex_t Received_Frame_Mutex;
static pthread_cond_t Master_Done_Flag;
static pthread_mutex_t Master_Done_Mutex;

/*RT_TASK Receive_Task, Fsm_Task;*/
/* local MS/TP port data - shared with RS-485 */
static volatile struct mstp_port_struct_t MSTP_Port;
/* buffers needed by mstp port struct */
static uint8_t TxBuffer[MAX_MPDU];
static uint8_t RxBuffer[MAX_MPDU];
/* data structure for MS/TP PDU Queue */
struct mstp_pdu_packet {
    bool data_expecting_reply;
    uint8_t destination_mac;
    uint16_t length;
    uint8_t buffer[MAX_MPDU];
};
/* count must be a power of 2 for ringbuf library */
#ifndef MSTP_PDU_PACKET_COUNT
#define MSTP_PDU_PACKET_COUNT 8
#endif
static struct mstp_pdu_packet PDU_Buffer[MSTP_PDU_PACKET_COUNT];
static RING_BUFFER PDU_Queue;
/* The minimum time without a DataAvailable or ReceiveError event */
/* that a node must wait for a station to begin replying to a */
/* confirmed request: 255 milliseconds. (Implementations may use */
/* larger values for this timeout, not to exceed 300 milliseconds.) */
static uint16_t Treply_timeout = 300;
/* The minimum time without a DataAvailable or ReceiveError event that a */
/* node must wait for a remote node to begin using a token or replying to */
/* a Poll For Master frame: 20 milliseconds. (Implementations may use */
/* larger values for this timeout, not to exceed 100 milliseconds.) */
static uint8_t Tusage_timeout = 100;
/* Timer that indicates line silence - and functions */

static struct timeval start;

static uint32_t Timer_Silence(
    void *pArg)
{
    struct timeval now, tmp_diff;
    int32_t res;

    gettimeofday(&now, NULL);
    timersub(&start, &now, &tmp_diff);
    res = ((tmp_diff.tv_sec) * 1000 + (tmp_diff.tv_usec) / 1000);

    return (res >= 0 ? res : -res);
}

static void Timer_Silence_Reset(
    void *pArg)
{
    gettimeofday(&start, NULL);
}

static void get_abstime(
    struct timespec *abstime,
    unsigned long milliseconds)
{
    struct timeval now, offset, result;

    gettimeofday(&now, NULL);
    offset.tv_sec = 0;
    offset.tv_usec = milliseconds * 1000;
    timeradd(&now, &offset, &result);
    abstime->tv_sec = result.tv_sec;
    abstime->tv_nsec = result.tv_usec * 1000;
}

void dlmstp_cleanup(
    void)
{
    pthread_cond_destroy(&Received_Frame_Flag);
    pthread_cond_destroy(&Receive_Packet_Flag);
    pthread_cond_destroy(&Master_Done_Flag);
    pthread_mutex_destroy(&Received_Frame_Mutex);
    pthread_mutex_destroy(&Receive_Packet_Mutex);
    pthread_mutex_destroy(&Master_Done_Mutex);
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
    unsigned i = 0;

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
            /* mac_len = 0 is a broadcast address */
            pkt->destination_mac = MSTP_BROADCAST_ADDRESS;
        }
        if (Ringbuf_Data_Put(&PDU_Queue, (uint8_t *)pkt)) {
            bytes_sent = pdu_len;
        }
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
    struct timespec abstime;

    (void) max_pdu;
    /* see if there is a packet available, and a place
       to put the reply (if necessary) and process it */
    pthread_mutex_lock(&Receive_Packet_Mutex);
    get_abstime(&abstime, timeout);
    pthread_cond_timedwait(&Receive_Packet_Flag, &Receive_Packet_Mutex,
        &abstime);
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
    pthread_mutex_unlock(&Receive_Packet_Mutex);

    return pdu_len;
}

static void *dlmstp_master_fsm_task(
    void *pArg)
{
    uint32_t silence = 0;
    bool run_master = false;

    (void) pArg;
    for (;;) {
        if (MSTP_Port.ReceivedValidFrame == false &&
            MSTP_Port.ReceivedInvalidFrame == false) {
            RS485_Check_UART_Data(&MSTP_Port);
            MSTP_Receive_Frame_FSM(&MSTP_Port);
        }
        if (MSTP_Port.ReceivedValidFrame || MSTP_Port.ReceivedInvalidFrame) {
            run_master = true;
        } else {
            silence = MSTP_Port.SilenceTimer(NULL);
            switch (MSTP_Port.master_state) {
                case MSTP_MASTER_STATE_IDLE:
                    if (silence >= Tno_token)
                        run_master = true;
                    break;
                case MSTP_MASTER_STATE_WAIT_FOR_REPLY:
                    if (silence >= Treply_timeout)
                        run_master = true;
                    break;
                case MSTP_MASTER_STATE_POLL_FOR_MASTER:
                    if (silence >= Tusage_timeout)
                        run_master = true;
                    break;
                default:
                    run_master = true;
                    break;
            }
        }
        if (run_master) {
            if (MSTP_Port.This_Station <= 127) {
                while (MSTP_Master_Node_FSM(&MSTP_Port)) {
                    /* do nothing while immediate transitioning */
                }
            } else if (MSTP_Port.This_Station < 255) {
                MSTP_Slave_Node_FSM(&MSTP_Port);
            }
        }
    }

    return NULL;
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

    pthread_mutex_lock(&Receive_Packet_Mutex);
    if (Receive_Packet.ready) {
        debug_printf("MS/TP: Dropped! Not Ready.\n");
    } else {
        /* bounds check - maybe this should send an abort? */
        pdu_len = mstp_port->DataLength;
        if (pdu_len > sizeof(Receive_Packet.pdu)) {
            pdu_len = sizeof(Receive_Packet.pdu);
        }
        if (pdu_len == 0) {
            debug_printf("MS/TP: PDU Length is 0!\n");
        }
        memmove((void *) &Receive_Packet.pdu[0],
            (void *) &mstp_port->InputBuffer[0], pdu_len);
        dlmstp_fill_bacnet_address(&Receive_Packet.address,
            mstp_port->SourceAddress);
        Receive_Packet.pdu_len = mstp_port->DataLength;
        Receive_Packet.ready = true;
        pthread_cond_signal(&Receive_Packet_Flag);
    }
    pthread_mutex_unlock(&Receive_Packet_Mutex);

    return pdu_len;
}

/* for the MS/TP state machine to use for getting data to send */
/* Return: amount of PDU data */
uint16_t MSTP_Get_Send(
    volatile struct mstp_port_struct_t * mstp_port,
    unsigned timeout)
{       /* milliseconds to wait for a packet */
    uint16_t pdu_len = 0;
    uint8_t frame_type = 0;
    struct mstp_pdu_packet *pkt;

    (void) timeout;
    if (Ringbuf_Empty(&PDU_Queue)) {
        return 0;
    }
    pkt = (struct mstp_pdu_packet *) Ringbuf_Peek(&PDU_Queue);
    if (pkt->data_expecting_reply) {
        frame_type = FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY;
    } else {
        frame_type = FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY;
    }
    /* convert the PDU into the MSTP Frame */
    pdu_len = MSTP_Create_Frame(&mstp_port->OutputBuffer[0],    /* <-- loading this */
        mstp_port->OutputBufferSize, frame_type, pkt->destination_mac,
        mstp_port->This_Station, (uint8_t *) & pkt->buffer[0], pkt->length);
    (void) Ringbuf_Pop(&PDU_Queue, NULL);

    return pdu_len;
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

    /* unused parameters */
    request_pdu_len = request_pdu_len;
    reply_pdu_len = reply_pdu_len;
    /* decode the request data */
    request.address.mac[0] = src_address;
    request.address.mac_len = 1;
    offset =
        npdu_decode(&request_pdu[0], NULL, &request.address,
        &request.npdu_data);
    if (request.npdu_data.network_layer_message) {
#if PRINT_ENABLED
        fprintf(stderr,
            "DLMSTP: DER Compare failed: " "Request is Network message.\n");
#endif
        return false;
    }
    request.pdu_type = request_pdu[offset] & 0xF0;
    if (request.pdu_type != PDU_TYPE_CONFIRMED_SERVICE_REQUEST) {
#if PRINT_ENABLED
        fprintf(stderr,
            "DLMSTP: DER Compare failed: " "Not Confirmed Request.\n");
#endif
        return false;
    }
    request.invoke_id = request_pdu[offset + 2];
    /* segmented message? */
    if (request_pdu[offset] & BIT3) {
        request.service_choice = request_pdu[offset + 5];
    } else {
        request.service_choice = request_pdu[offset + 3];
    }
    /* decode the reply data */
    reply.address.mac[0] = dest_address;
    reply.address.mac_len = 1;
    offset =
        npdu_decode(&reply_pdu[0], &reply.address, NULL, &reply.npdu_data);
    if (reply.npdu_data.network_layer_message) {
#if PRINT_ENABLED
        fprintf(stderr,
            "DLMSTP: DER Compare failed: " "Reply is Network message.\n");
#endif
        return false;
    }
    /* reply could be a lot of things:
       confirmed, simple ack, abort, reject, error */
    reply.pdu_type = reply_pdu[offset] & 0xF0;
    switch (reply.pdu_type) {
        case PDU_TYPE_CONFIRMED_SERVICE_REQUEST:
            reply.invoke_id = reply_pdu[offset + 2];
            /* segmented message? */
            if (reply_pdu[offset] & BIT3) {
                reply.service_choice = reply_pdu[offset + 5];
            } else {
                reply.service_choice = reply_pdu[offset + 3];
            }
            break;
        case PDU_TYPE_SIMPLE_ACK:
            reply.invoke_id = reply_pdu[offset + 1];
            reply.service_choice = reply_pdu[offset + 2];
            break;
        case PDU_TYPE_COMPLEX_ACK:
            reply.invoke_id = reply_pdu[offset + 1];
            /* segmented message? */
            if (reply_pdu[offset] & BIT3) {
                reply.service_choice = reply_pdu[offset + 4];
            } else {
                reply.service_choice = reply_pdu[offset + 2];
            }
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
#if PRINT_ENABLED
            fprintf(stderr,
                "DLMSTP: DER Compare failed: " "Invoke ID mismatch.\n");
#endif
            return false;
        }
    } else {
        if (request.invoke_id != reply.invoke_id) {
#if PRINT_ENABLED
            fprintf(stderr,
                "DLMSTP: DER Compare failed: " "Invoke ID mismatch.\n");
#endif
            return false;
        }
        if (request.service_choice != reply.service_choice) {
#if PRINT_ENABLED
            fprintf(stderr,
                "DLMSTP: DER Compare failed: " "Service choice mismatch.\n");
#endif
            return false;
        }
    }
    if (request.npdu_data.protocol_version != reply.npdu_data.protocol_version) {
#if PRINT_ENABLED
        fprintf(stderr,
            "DLMSTP: DER Compare failed: "
            "NPDU Protocol Version mismatch.\n");
#endif
        return false;
    }
    if (request.npdu_data.priority != reply.npdu_data.priority) {
#if PRINT_ENABLED
        fprintf(stderr,
            "DLMSTP: DER Compare failed: " "NPDU Priority mismatch.\n");
#endif
        return false;
    }
    if (!bacnet_address_same(&request.address, &reply.address)) {
#if PRINT_ENABLED
        fprintf(stderr,
            "DLMSTP: DER Compare failed: " "BACnet Address mismatch.\n");
#endif
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
    bool matched = false;
    uint8_t frame_type = 0;
    struct mstp_pdu_packet *pkt;

    (void) timeout;
    if (Ringbuf_Empty(&PDU_Queue)) {
        return 0;
    }
    pkt = (struct mstp_pdu_packet *) Ringbuf_Peek(&PDU_Queue);
    /* is this the reply to the DER? */
    matched =
        dlmstp_compare_data_expecting_reply(&mstp_port->InputBuffer[0],
        mstp_port->DataLength, mstp_port->SourceAddress,
        (uint8_t *) & pkt->buffer[0], pkt->length, pkt->destination_mac);
    if (!matched) {
        return 0;
    }
    if (pkt->data_expecting_reply) {
        frame_type = FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY;
    } else {
        frame_type = FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY;
    }
    /* convert the PDU into the MSTP Frame */
    pdu_len = MSTP_Create_Frame(&mstp_port->OutputBuffer[0],    /* <-- loading this */
        mstp_port->OutputBufferSize, frame_type, pkt->destination_mac,
        mstp_port->This_Station, (uint8_t *) & pkt->buffer[0], pkt->length);
    (void) Ringbuf_Pop(&PDU_Queue, NULL);

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
    int rv = 0;

    /* initialize PDU queue */
    Ringbuf_Init(&PDU_Queue, (uint8_t *) & PDU_Buffer,
        sizeof(struct mstp_pdu_packet), MSTP_PDU_PACKET_COUNT);
    /* initialize packet queue */
    Receive_Packet.ready = false;
    Receive_Packet.pdu_len = 0;
    rv = pthread_cond_init(&Receive_Packet_Flag, NULL);
    if (rv != 0) {
        fprintf(stderr,
            "MS/TP Interface: %s\n cannot allocate PThread Condition.\n",
            ifname);
        exit(1);
    }
    rv = pthread_mutex_init(&Receive_Packet_Mutex, NULL);
    if (rv != 0) {
        fprintf(stderr,
            "MS/TP Interface: %s\n cannot allocate PThread Mutex.\n", ifname);
        exit(1);
    }
    /* initialize hardware */
    if (ifname) {
        RS485_Set_Interface(ifname);
#if PRINT_ENABLED
        fprintf(stderr, "MS/TP Interface: %s\n", ifname);
#endif
    }
    RS485_Initialize();
    MSTP_Port.InputBuffer = &RxBuffer[0];
    MSTP_Port.InputBufferSize = sizeof(RxBuffer);
    MSTP_Port.OutputBuffer = &TxBuffer[0];
    MSTP_Port.OutputBufferSize = sizeof(TxBuffer);
    gettimeofday(&start, NULL);
    MSTP_Port.SilenceTimer = Timer_Silence;
    MSTP_Port.SilenceTimerReset = Timer_Silence_Reset;
    MSTP_Init(&MSTP_Port);
#if PRINT_ENABLED
    fprintf(stderr, "MS/TP MAC: %02X\n", MSTP_Port.This_Station);
    fprintf(stderr, "MS/TP Max_Master: %02X\n", MSTP_Port.Nmax_master);
    fprintf(stderr, "MS/TP Max_Info_Frames: %u\n", MSTP_Port.Nmax_info_frames);
#endif
    /* start the threads */
    /*    rv = pthread_create(&hThread, NULL, dlmstp_receive_fsm_task, NULL); */
    /*    if (rv != 0) {
       fprintf(stderr, "Failed to start recive FSM task\n");
       } */
    rv = pthread_create(&hThread, NULL, dlmstp_master_fsm_task, NULL);
    if (rv != 0) {
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
        pdu_len = dlmstp_receive(NULL, NULL, 0, UINT_MAX);
        MSTP_Create_And_Send_Frame(&MSTP_Port, FRAME_TYPE_TEST_REQUEST,
            MSTP_Port.SourceAddress, MSTP_Port.This_Station, NULL, 0);
    }

    return 0;
}
#endif
