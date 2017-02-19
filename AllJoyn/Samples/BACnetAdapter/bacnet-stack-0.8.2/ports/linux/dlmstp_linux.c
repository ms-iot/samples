/**************************************************************************
*
* Copyright (C) 2008 Steve Karg <skarg@users.sourceforge.net>
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
#include <time.h>
#include "bacdef.h"
#include "bacaddr.h"
#include "mstp.h"
/*#include "dlmstp.h" */
#include "dlmstp_linux.h"
#include "rs485.h"
#include "npdu.h"
#include "bits.h"
/* OS Specific include */
#include "net.h"
#include "ringbuf.h"

/** @file linux/dlmstp.c  Provides Linux-specific DataLink functions for MS/TP. */

#define BACNET_PDU_CONTROL_BYTE_OFFSET 1
#define BACNET_DATA_EXPECTING_REPLY_BIT 2
#define BACNET_DATA_EXPECTING_REPLY(control) ( (control & (1 << BACNET_DATA_EXPECTING_REPLY_BIT) ) > 0 )

#define INCREMENT_AND_LIMIT_UINT16(x) {if (x < 0xFFFF) x++;}
uint32_t Timer_Silence(
    void *poPort)
{
    struct timeval now, tmp_diff;
    SHARED_MSTP_DATA *poSharedData;
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return -1;
    }
    poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
    if (!poSharedData) {
        return -1;
    }

    int32_t res;

    gettimeofday(&now, NULL);
    timersub(&poSharedData->start, &now, &tmp_diff);
    res = ((tmp_diff.tv_sec) * 1000 + (tmp_diff.tv_usec) / 1000);

    return (res >= 0 ? res : -res);
}

void Timer_Silence_Reset(
    void *poPort)
{
    SHARED_MSTP_DATA *poSharedData;
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return;
    }
    poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
    if (!poSharedData) {
        return;
    }

    gettimeofday(&poSharedData->start, NULL);
}

void get_abstime(
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
    void *poPort)
{
    SHARED_MSTP_DATA *poSharedData;
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return;
    }
    poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
    if (!poSharedData) {
        return;
    }

    /* restore the old port settings */
    tcsetattr(poSharedData->RS485_Handle, TCSANOW,
        &poSharedData->RS485_oldtio);
    close(poSharedData->RS485_Handle);

    pthread_cond_destroy(&poSharedData->Received_Frame_Flag);
    pthread_cond_destroy(&poSharedData->Receive_Packet_Flag);
    pthread_cond_destroy(&poSharedData->Master_Done_Flag);
    pthread_mutex_destroy(&poSharedData->Received_Frame_Mutex);
    pthread_mutex_destroy(&poSharedData->Receive_Packet_Mutex);
    pthread_mutex_destroy(&poSharedData->Master_Done_Mutex);
}

/* returns number of bytes sent on success, zero on failure */
int dlmstp_send_pdu(
    void *poPort,
    BACNET_ADDRESS * dest,      /* destination address */
    uint8_t * pdu,      /* any data to be sent - may be null */
    unsigned pdu_len)
{       /* number of bytes of data */
    int bytes_sent = 0;
    struct mstp_pdu_packet *pkt;
    unsigned i = 0;
    SHARED_MSTP_DATA *poSharedData;
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return 0;
    }
    poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
    if (!poSharedData) {
        return 0;
    }

    pkt = (struct mstp_pdu_packet *) Ringbuf_Data_Peek(&poSharedData->PDU_Queue);
    if (pkt) {
        pkt->data_expecting_reply =
            BACNET_DATA_EXPECTING_REPLY(pdu[BACNET_PDU_CONTROL_BYTE_OFFSET]);
        for (i = 0; i < pdu_len; i++) {
            pkt->buffer[i] = pdu[i];
        }
        pkt->length = pdu_len;
        pkt->destination_mac = dest->mac[0];
        if (Ringbuf_Data_Put(&PDU_Queue, (uint8_t *)pkt)) {
            bytes_sent = pdu_len;
        }
    }

    return bytes_sent;
}

uint16_t dlmstp_receive(
    void *poPort,
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      /* PDU data */
    uint16_t max_pdu,   /* amount of space available in the PDU  */
    unsigned timeout)
{       /* milliseconds to wait for a packet */
    uint16_t pdu_len = 0;
    struct timespec abstime;
    int rv = 0;
    SHARED_MSTP_DATA *poSharedData;
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return 0;
    }
    poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
    if (!poSharedData) {
        return 0;
    }
    (void) max_pdu;
    /* see if there is a packet available, and a place
       to put the reply (if necessary) and process it */
    get_abstime(&abstime, timeout);
    rv = pthread_cond_timedwait(&poSharedData->Receive_Packet_Flag,
        &poSharedData->Receive_Packet_Mutex, &abstime);
    if (rv == 0) {
        if (poSharedData->Receive_Packet.ready) {
            if (poSharedData->Receive_Packet.pdu_len) {
                poSharedData->MSTP_Packets++;
                if (src) {
                    memmove(src, &poSharedData->Receive_Packet.address,
                        sizeof(poSharedData->Receive_Packet.address));
                }
                if (pdu) {
                    memmove(pdu, &poSharedData->Receive_Packet.pdu,
                        sizeof(poSharedData->Receive_Packet.pdu));
                }
                pdu_len = poSharedData->Receive_Packet.pdu_len;
            }
            poSharedData->Receive_Packet.ready = false;
        }
    }

    return pdu_len;
}

void *dlmstp_receive_fsm_task(
    void *pArg)
{
    bool received_frame;
    SHARED_MSTP_DATA *poSharedData;
    struct mstp_port_struct_t *mstp_port = (struct mstp_port_struct_t *) pArg;
    if (!mstp_port) {
        return NULL;
    }

    poSharedData =
        (SHARED_MSTP_DATA *) ((struct mstp_port_struct_t *) pArg)->UserData;
    if (!poSharedData) {
        return NULL;
    }

    for (;;) {
        /* only do receive state machine while we don't have a frame */
        if ((mstp_port->ReceivedValidFrame == false) &&
            (mstp_port->ReceivedInvalidFrame == false)) {
            do {
                RS485_Check_UART_Data(mstp_port);
                MSTP_Receive_Frame_FSM((volatile struct mstp_port_struct_t *)
                    pArg);
                received_frame = mstp_port->ReceivedValidFrame ||
                    mstp_port->ReceivedInvalidFrame;
                if (received_frame) {
                    pthread_cond_signal(&poSharedData->Received_Frame_Flag);
                    break;
                }
            } while (mstp_port->DataAvailable);
        }
    }

    return NULL;
}

void *dlmstp_master_fsm_task(
    void *pArg)
{
    uint32_t silence = 0;
    bool run_master = false;
    SHARED_MSTP_DATA *poSharedData;
    struct mstp_port_struct_t *mstp_port = (struct mstp_port_struct_t *) pArg;
    if (!mstp_port) {
        return NULL;
    }

    poSharedData =
        (SHARED_MSTP_DATA *) ((struct mstp_port_struct_t *) pArg)->UserData;
    if (!poSharedData) {
        return NULL;
    }

    for (;;) {
        if (mstp_port->ReceivedValidFrame == false &&
            mstp_port->ReceivedInvalidFrame == false) {
            RS485_Check_UART_Data(mstp_port);
            MSTP_Receive_Frame_FSM(mstp_port);
        }
        if (mstp_port->ReceivedValidFrame || mstp_port->ReceivedInvalidFrame) {
            run_master = true;
        } else {
            silence = mstp_port->SilenceTimer(NULL);
            switch (mstp_port->master_state) {
                case MSTP_MASTER_STATE_IDLE:
                    if (silence >= Tno_token)
                        run_master = true;
                    break;
                case MSTP_MASTER_STATE_WAIT_FOR_REPLY:
                    if (silence >= poSharedData->Treply_timeout)
                        run_master = true;
                    break;
                case MSTP_MASTER_STATE_POLL_FOR_MASTER:
                    if (silence >= poSharedData->Tusage_timeout)
                        run_master = true;
                    break;
                default:
                    run_master = true;
                    break;
            }
        }
        if (run_master) {
            if (mstp_port->This_Station <= DEFAULT_MAX_MASTER) {
                while (MSTP_Master_Node_FSM(mstp_port)) {
                    /* do nothing while immediate transitioning */
                }
            } else if (mstp_port->This_Station < 255) {
                MSTP_Slave_Node_FSM(mstp_port);
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
    SHARED_MSTP_DATA *poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;

    if (!poSharedData) {
        return 0;
    }

    if (!poSharedData->Receive_Packet.ready) {
        /* bounds check - maybe this should send an abort? */
        pdu_len = mstp_port->DataLength;
        if (pdu_len > sizeof(poSharedData->Receive_Packet.pdu))
            pdu_len = sizeof(poSharedData->Receive_Packet.pdu);
        memmove((void *) &poSharedData->Receive_Packet.pdu[0],
            (void *) &mstp_port->InputBuffer[0], pdu_len);
        dlmstp_fill_bacnet_address(&poSharedData->Receive_Packet.address,
            mstp_port->SourceAddress);
        poSharedData->Receive_Packet.pdu_len = mstp_port->DataLength;
        poSharedData->Receive_Packet.ready = true;
        pthread_cond_signal(&poSharedData->Receive_Packet_Flag);
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
    uint8_t frame_type = 0;
    struct mstp_pdu_packet *pkt;
    SHARED_MSTP_DATA *poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;

    if (!poSharedData) {
        return 0;
    }

    (void) timeout;
    if (Ringbuf_Empty(&poSharedData->PDU_Queue)) {
        return 0;
    }
    pkt = (struct mstp_pdu_packet *) Ringbuf_Peek(&poSharedData->PDU_Queue);
    if (pkt->data_expecting_reply) {
        frame_type = FRAME_TYPE_BACNET_DATA_EXPECTING_REPLY;
    } else {
        frame_type = FRAME_TYPE_BACNET_DATA_NOT_EXPECTING_REPLY;
    }
    /* convert the PDU into the MSTP Frame */
    pdu_len = MSTP_Create_Frame(&mstp_port->OutputBuffer[0],    /* <-- loading this */
        mstp_port->OutputBufferSize, frame_type, pkt->destination_mac,
        mstp_port->This_Station, (uint8_t *) & pkt->buffer[0], pkt->length);
    (void) Ringbuf_Pop(&poSharedData->PDU_Queue, NULL);

    return pdu_len;
}

bool dlmstp_compare_data_expecting_reply(
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
    SHARED_MSTP_DATA *poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;

    if (!poSharedData) {
        return 0;
    }

    if (Ringbuf_Empty(&poSharedData->PDU_Queue)) {
        return 0;
    }
    pkt = (struct mstp_pdu_packet *) Ringbuf_Peek(&poSharedData->PDU_Queue);
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
    (void) Ringbuf_Pop(&poSharedData->PDU_Queue, NULL);

    return pdu_len;
}

void dlmstp_set_mac_address(
    void *poPort,
    uint8_t mac_address)
{
/*	SHARED_MSTP_DATA * poSharedData; */
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return;
    }
/*
	poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
	if(!poSharedData)
	{
		return;
	}
*/
    /* Master Nodes can only have address 0-127 */
    if (mac_address <= 127) {
        mstp_port->This_Station = mac_address;
        /* FIXME: implement your data storage */
        /* I2C_Write_Byte(
           EEPROM_DEVICE_ADDRESS,
           mac_address,
           EEPROM_MSTP_MAC_ADDR); */
        if (mac_address > mstp_port->Nmax_master)
            dlmstp_set_max_master(mstp_port, mac_address);
    }

    return;
}

uint8_t dlmstp_mac_address(
    void *poPort)
{
/*	SHARED_MSTP_DATA * poSharedData; */
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return 0;
    }
/*	poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
	if(!poSharedData)
	{
		return 0;
	}
*/

    return mstp_port->This_Station;
}

/* This parameter represents the value of the Max_Info_Frames property of */
/* the node's Device object. The value of Max_Info_Frames specifies the */
/* maximum number of information frames the node may send before it must */
/* pass the token. Max_Info_Frames may have different values on different */
/* nodes. This may be used to allocate more or less of the available link */
/* bandwidth to particular nodes. If Max_Info_Frames is not writable in a */
/* node, its value shall be 1. */
void dlmstp_set_max_info_frames(
    void *poPort,
    uint8_t max_info_frames)
{
/*	SHARED_MSTP_DATA * poSharedData; */
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return;
    }
/*
	poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
	if(!poSharedData)
	{
		return;
	}
*/
    if (max_info_frames >= 1) {
        mstp_port->Nmax_info_frames = max_info_frames;
        /* FIXME: implement your data storage */
        /* I2C_Write_Byte(
           EEPROM_DEVICE_ADDRESS,
           (uint8_t)max_info_frames,
           EEPROM_MSTP_MAX_INFO_FRAMES_ADDR); */
    }

    return;
}

uint8_t dlmstp_max_info_frames(
    void *poPort)
{
/*	SHARED_MSTP_DATA * poSharedData; */
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return 0;
    }
/*
	poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
	if(!poSharedData)
	{
		return 0;
	}
*/
    return mstp_port->Nmax_info_frames;
}

/* This parameter represents the value of the Max_Master property of the */
/* node's Device object. The value of Max_Master specifies the highest */
/* allowable address for master nodes. The value of Max_Master shall be */
/* less than or equal to 127. If Max_Master is not writable in a node, */
/* its value shall be 127. */
void dlmstp_set_max_master(
    void *poPort,
    uint8_t max_master)
{
/*	SHARED_MSTP_DATA * poSharedData; */
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return;
    }
/*
	poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
	if(!poSharedData)
	{
		return;
	}
*/
    if (max_master <= 127) {
        if (mstp_port->This_Station <= max_master) {
            mstp_port->Nmax_master = max_master;
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
    void *poPort)
{
/*	SHARED_MSTP_DATA * poSharedData; */
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return 0;
    }
/*
	poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
	if(!poSharedData)
	{
		return 0;
	}
*/
    return mstp_port->Nmax_master;
}

/* RS485 Baud Rate 9600, 19200, 38400, 57600, 115200 */
void dlmstp_set_baud_rate(
    void *poPort,
    uint32_t baud)
{
    SHARED_MSTP_DATA *poSharedData;
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return;
    }
    poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
    if (!poSharedData) {
        return;
    }

    switch (baud) {
        case 9600:
            poSharedData->RS485_Baud = B9600;
            break;
        case 19200:
            poSharedData->RS485_Baud = B19200;
            break;
        case 38400:
            poSharedData->RS485_Baud = B38400;
            break;
        case 57600:
            poSharedData->RS485_Baud = B57600;
            break;
        case 115200:
            poSharedData->RS485_Baud = B115200;
            break;
        default:
            break;
    }
}

uint32_t dlmstp_baud_rate(
    void *poPort)
{
    SHARED_MSTP_DATA *poSharedData;
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return false;
    }
    poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
    if (!poSharedData) {
        return false;
    }

    switch (poSharedData->RS485_Baud) {
        case B19200:
            return 19200;
        case B38400:
            return 38400;
        case B57600:
            return 57600;
        case B115200:
            return 115200;
        default:
        case B9600:
            return 9600;
    }
}

void dlmstp_get_my_address(
    void *poPort,
    BACNET_ADDRESS * my_address)
{
    int i = 0;  /* counter */
    SHARED_MSTP_DATA *poSharedData;
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return;
    }
    poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
    if (!poSharedData) {
        return;
    }
    my_address->mac_len = 1;
    my_address->mac[0] = mstp_port->This_Station;
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
    void *poPort,
    char *ifname)
{
    unsigned long hThread = 0;
    int rv = 0;
    SHARED_MSTP_DATA *poSharedData;
    struct mstp_port_struct_t *mstp_port =
        (struct mstp_port_struct_t *) poPort;
    if (!mstp_port) {
        return false;
    }

    poSharedData = (SHARED_MSTP_DATA *) ((struct mstp_port_struct_t *)
        mstp_port)->UserData;
    if (!poSharedData) {
        return false;
    }

    poSharedData->RS485_Port_Name = ifname;
    /* initialize PDU queue */
    Ringbuf_Init(&poSharedData->PDU_Queue,
        (uint8_t *) & poSharedData->PDU_Buffer, sizeof(struct mstp_pdu_packet),
        MSTP_PDU_PACKET_COUNT);
    /* initialize packet queue */
    poSharedData->Receive_Packet.ready = false;
    poSharedData->Receive_Packet.pdu_len = 0;
    rv = pthread_cond_init(&poSharedData->Receive_Packet_Flag, NULL);
    if (rv != 0) {
        fprintf(stderr,
            "MS/TP Interface: %s\n cannot allocate PThread Condition.\n",
            ifname);
        exit(1);
    }
    rv = pthread_mutex_init(&poSharedData->Receive_Packet_Mutex, NULL);
    if (rv != 0) {
        fprintf(stderr,
            "MS/TP Interface: %s\n cannot allocate PThread Mutex.\n", ifname);
        exit(1);
    }

    struct termios newtio;
    printf("RS485: Initializing %s", poSharedData->RS485_Port_Name);
    /*
       Open device for reading and writing.
       Blocking mode - more CPU effecient
     */
    poSharedData->RS485_Handle =
        open(poSharedData->RS485_Port_Name,
        O_RDWR | O_NOCTTY | O_NONBLOCK /*| O_NDELAY */ );
    if (poSharedData->RS485_Handle < 0) {
        perror(poSharedData->RS485_Port_Name);
        exit(-1);
    }
#if 0
    /* non blocking for the read */
    fcntl(poSharedData->RS485_Handle, F_SETFL, FNDELAY);
#else
    /* efficient blocking for the read */
    fcntl(poSharedData->RS485_Handle, F_SETFL, 0);
#endif
    /* save current serial port settings */
    tcgetattr(poSharedData->RS485_Handle, &poSharedData->RS485_oldtio);
    /* clear struct for new port settings */
    bzero(&newtio, sizeof(newtio));
    /*
       BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
       CRTSCTS : output hardware flow control (only used if the cable has
       all necessary lines. See sect. 7 of Serial-HOWTO)
       CLOCAL  : local connection, no modem contol
       CREAD   : enable receiving characters
     */
    newtio.c_cflag =
        poSharedData->RS485_Baud | poSharedData->RS485MOD | CLOCAL | CREAD;
    /* Raw input */
    newtio.c_iflag = 0;
    /* Raw output */
    newtio.c_oflag = 0;
    /* no processing */
    newtio.c_lflag = 0;
    /* activate the settings for the port after flushing I/O */
    tcsetattr(poSharedData->RS485_Handle, TCSAFLUSH, &newtio);
    /* flush any data waiting */
    usleep(200000);
    tcflush(poSharedData->RS485_Handle, TCIOFLUSH);
    /* ringbuffer */
    FIFO_Init(&poSharedData->Rx_FIFO, poSharedData->Rx_Buffer,
        sizeof(poSharedData->Rx_Buffer));
    printf("=success!\n");
    mstp_port->InputBuffer = &poSharedData->RxBuffer[0];
    mstp_port->InputBufferSize = sizeof(poSharedData->RxBuffer);
    mstp_port->OutputBuffer = &poSharedData->TxBuffer[0];
    mstp_port->OutputBufferSize = sizeof(poSharedData->TxBuffer);
    gettimeofday(&poSharedData->start, NULL);
    mstp_port->SilenceTimer = Timer_Silence;
    mstp_port->SilenceTimerReset = Timer_Silence_Reset;
    MSTP_Init(mstp_port);
#if PRINT_ENABLED
    fprintf(stderr, "MS/TP MAC: %02X\n", mstp_port->This_Station);
    fprintf(stderr, "MS/TP Max_Master: %02X\n", mstp_port->Nmax_master);
    fprintf(stderr, "MS/TP Max_Info_Frames: %u\n",
        mstp_port->Nmax_info_frames);
#endif

    rv = pthread_create(&hThread, NULL, dlmstp_master_fsm_task, mstp_port);
    if (rv != 0) {
        fprintf(stderr, "Failed to start Master Node FSM task\n");
    }

    return true;
}
