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
#ifndef DLMSTP_LINUX_H
#define DLMSTP_LINUX_H

#include "mstp.h"
/*#include "dlmstp.h" */
#include "bits/pthreadtypes.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "bacdef.h"
#include "npdu.h"
#include <termios.h>
#include "fifo.h"
#include "ringbuf.h"
/* defines specific to MS/TP */
/* preamble+type+dest+src+len+crc8+crc16 */
#define MAX_HEADER (2+1+1+1+2+1+2)
#define MAX_MPDU (MAX_HEADER+MAX_PDU)

/* count must be a power of 2 for ringbuf library */
#ifndef MSTP_PDU_PACKET_COUNT
#define MSTP_PDU_PACKET_COUNT 8
#endif

typedef struct dlmstp_packet {
    bool ready; /* true if ready to be sent or received */
    BACNET_ADDRESS address;     /* source address */
    uint8_t frame_type; /* type of message */
    uint16_t pdu_len;   /* packet length */
    uint8_t pdu[MAX_MPDU];      /* packet */
} DLMSTP_PACKET;

/* data structure for MS/TP PDU Queue */
struct mstp_pdu_packet {
    bool data_expecting_reply;
    uint8_t destination_mac;
    uint16_t length;
    uint8_t buffer[MAX_MPDU];
};

typedef struct shared_mstp_data {
    /* Number of MS/TP Packets Rx/Tx */
    uint16_t MSTP_Packets;

    /* packet queues */
    DLMSTP_PACKET Receive_Packet;
    DLMSTP_PACKET Transmit_Packet;
    /*
       RT_COND Receive_Packet_Flag;
       RT_MUTEX Receive_Packet_Mutex;
     */
    pthread_cond_t Receive_Packet_Flag;
    pthread_mutex_t Receive_Packet_Mutex;
    /* mechanism to wait for a frame in state machine */
    /*
       RT_COND Received_Frame_Flag;
       RT_MUTEX Received_Frame_Mutex;
     */
    pthread_cond_t Received_Frame_Flag;
    pthread_mutex_t Received_Frame_Mutex;
    pthread_cond_t Master_Done_Flag;
    pthread_mutex_t Master_Done_Mutex;
    /* buffers needed by mstp port struct */
    uint8_t TxBuffer[MAX_MPDU];
    uint8_t RxBuffer[MAX_MPDU];
    /* The minimum time without a DataAvailable or ReceiveError event */
    /* that a node must wait for a station to begin replying to a */
    /* confirmed request: 255 milliseconds. (Implementations may use */
    /* larger values for this timeout, not to exceed 300 milliseconds.) */
    uint16_t Treply_timeout;
    /* The minimum time without a DataAvailable or ReceiveError event that a */
    /* node must wait for a remote node to begin using a token or replying to */
    /* a Poll For Master frame: 20 milliseconds. (Implementations may use */
    /* larger values for this timeout, not to exceed 100 milliseconds.) */
    uint8_t Tusage_timeout;
    /* Timer that indicates line silence - and functions */
    uint16_t SilenceTime;

    /* handle returned from open() */
    int RS485_Handle;
    /* baudrate settings are defined in <asm/termbits.h>, which is
       included by <termios.h> */
    unsigned int RS485_Baud;
    /* serial port name, /dev/ttyS0,
       /dev/ttyUSB0 for USB->RS485 from B&B Electronics USOPTL4 */
    char *RS485_Port_Name;
    /* serial I/O settings */
    struct termios RS485_oldtio;
    /* some terminal I/O have RS-485 specific functionality */
    tcflag_t RS485MOD;
    /* Ring buffer for incoming bytes, in order to speed up the receiving. */
    FIFO_BUFFER Rx_FIFO;
    /* buffer size needs to be a power of 2 */
    uint8_t Rx_Buffer[4096];
    struct timeval start;

    RING_BUFFER PDU_Queue;

    struct mstp_pdu_packet PDU_Buffer[MSTP_PDU_PACKET_COUNT];

} SHARED_MSTP_DATA;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    bool dlmstp_init(
        void *poShared,
        char *ifname);
    void dlmstp_reset(
        void *poShared);
    void dlmstp_cleanup(
        void *poShared);

    /* returns number of bytes sent on success, negative on failure */
    int dlmstp_send_pdu(
        void *poShared,
        BACNET_ADDRESS * dest,  /* destination address */
        uint8_t * pdu,  /* any data to be sent - may be null */
        unsigned pdu_len);      /* number of bytes of data */

    /* returns the number of octets in the PDU, or zero on failure */
    uint16_t dlmstp_receive(
        void *poShared,
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
        void *poShared,
        uint8_t max_info_frames);
    uint8_t dlmstp_max_info_frames(
        void *poShared);

    /* This parameter represents the value of the Max_Master property of the */
    /* node's Device object. The value of Max_Master specifies the highest */
    /* allowable address for master nodes. The value of Max_Master shall be */
    /* less than or equal to 127. If Max_Master is not writable in a node, */
    /* its value shall be 127. */
    void dlmstp_set_max_master(
        void *poShared,
        uint8_t max_master);
    uint8_t dlmstp_max_master(
        void *poShared);

    /* MAC address 0-127 */
    void dlmstp_set_mac_address(
        void *poShared,
        uint8_t my_address);
    uint8_t dlmstp_mac_address(
        void *poShared);

    void dlmstp_get_my_address(
        void *poShared,
        BACNET_ADDRESS * my_address);
    void dlmstp_get_broadcast_address(
        BACNET_ADDRESS * dest); /* destination address */

    /* RS485 Baud Rate 9600, 19200, 38400, 57600, 115200 */
    void dlmstp_set_baud_rate(
        void *poShared,
        uint32_t baud);
    uint32_t dlmstp_baud_rate(
        void *poShared);

    void dlmstp_fill_bacnet_address(
        BACNET_ADDRESS * src,
        uint8_t mstp_address);

    bool dlmstp_sole_master(
        void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*DLMSTP_LINUX_H */
