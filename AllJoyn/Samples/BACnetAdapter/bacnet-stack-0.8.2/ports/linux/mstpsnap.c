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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
/* OS specific include*/
#include "net.h"
#include "timer.h"
/* local includes */
#include "bytes.h"
#include "rs485.h"
#include "crc.h"
#include "mstp.h"
#include "dlmstp.h"
#include "mstptext.h"
#include "bacint.h"

/** @file linux/mstpsnap.c  Example application testing BACnet MS/TP on Linux. */

#ifndef max
#define max(a,b) (((a) (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

/* local port data - shared with RS-485 */
static volatile struct mstp_port_struct_t MSTP_Port;
/* buffers needed by mstp port struct */
static uint8_t RxBuffer[MAX_MPDU];
static uint8_t TxBuffer[MAX_MPDU];
static uint32_t Timer_Silence(
    void *pArg)
{
    uint32_t delta_time = 0;

    delta_time = timer_milliseconds(TIMER_SILENCE);
    if (delta_time > 0xFFFF) {
        delta_time = 0xFFFF;
    }

    return delta_time;
}

static void Timer_Silence_Reset(
    void *pArg)
{
    timer_reset(TIMER_SILENCE);
}

/* functions used by the MS/TP state machine to put or get data */
uint16_t MSTP_Put_Receive(
    volatile struct mstp_port_struct_t *mstp_port)
{
    (void) mstp_port;

    return 0;
}

/* for the MS/TP state machine to use for getting data to send */
/* Return: amount of PDU data */
uint16_t MSTP_Get_Send(
    volatile struct mstp_port_struct_t * mstp_port,
    unsigned timeout)
{       /* milliseconds to wait for a packet */
    (void) mstp_port;
    (void) timeout;
    return 0;
}

uint16_t MSTP_Get_Reply(
    volatile struct mstp_port_struct_t * mstp_port,
    unsigned timeout)
{       /* milliseconds to wait for a packet */
    (void) mstp_port;
    (void) timeout;
    return 0;
}

static int network_init(
    const char *name,
    int protocol)
{
    /* check to see if we are being run as root */
    if (getuid() != 0) {
        fprintf(stderr, "Requires root priveleges.\n");
        return -1;
    }

    int sockfd = socket(PF_PACKET, SOCK_RAW, htons(protocol));

    if (sockfd == -1) {
        perror("Unable to create socket");
        return sockfd;
    }

    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, name, strlen(name));

    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
        perror("Unable to get interface index");
        return -1;
    }

    struct sockaddr_ll sll;

    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol);

    if (bind(sockfd, (struct sockaddr *) &sll, sizeof(sll)) == -1) {
        perror("Unable to bind socket");
        return -1;
    }

    return sockfd;
}

static void snap_received_packet(
    volatile struct mstp_port_struct_t *mstp_port,
    int sockfd)
{
    uint16_t mtu_len = 0;       /* number of octets of packet saved in file */
    unsigned i = 0;     /* counter */
    static uint8_t mtu[1500] = { 0 };
    uint16_t max_data = 0;

    mtu[0] = 0;
    mtu[1] = 0;
    mtu[2] = 0;
    mtu[3] = 0;
    mtu[4] = 0;
    mtu[5] = mstp_port->DestinationAddress;
    mtu[6] = 0;
    mtu[7] = 0;
    mtu[8] = 0;
    mtu[9] = 0;
    mtu[10] = 0;
    mtu[11] = mstp_port->SourceAddress;
    /* length - 12, 13 */
    mtu[14] = 0xaa;     /* DSAP for SNAP */
    mtu[15] = 0xaa;     /* SSAP for SNAP */
    mtu[16] = 0x03;     /* Control Field for SNAP */
    mtu[17] = 0x00;     /* Organization Code: Cimetrics */
    mtu[18] = 0x10;     /* Organization Code: Cimetrics */
    mtu[19] = 0x90;     /* Organization Code: Cimetrics */
    mtu[20] = 0x00;     /* Protocol ID */
    mtu[21] = 0x01;     /* Protocol ID */
    mtu[22] = 0x00;     /* delta time */
    mtu[23] = 0x00;     /* delta time */
    mtu[24] = 0x80;     /* unknown byte */
    mtu[25] = mstp_port->FrameType;
    mtu[26] = mstp_port->DestinationAddress;
    mtu[27] = mstp_port->SourceAddress;
    mtu[28] = HI_BYTE(mstp_port->DataLength);
    mtu[29] = LO_BYTE(mstp_port->DataLength);
    mtu[30] = mstp_port->HeaderCRCActual;
    mtu_len = 31;
    if (mstp_port->DataLength) {
        max_data = min(mstp_port->InputBufferSize, mstp_port->DataLength);
        for (i = 0; i < max_data; i++) {
            mtu[31 + i] = mstp_port->InputBuffer[i];
        }
        mtu[31 + max_data] = mstp_port->DataCRCActualMSB;
        mtu[31 + max_data + 1] = mstp_port->DataCRCActualLSB;
        mtu_len += (max_data + 2);
    }
    /* Ethernet length is data only - not address or length bytes */
    encode_unsigned16(&mtu[12], mtu_len - 14);
    (void) write(sockfd, &mtu[0], mtu_len);
}


static void cleanup(
    void)
{
}

#if (!defined(_WIN32))
static void sig_int(
    int signo)
{
    (void) signo;

    cleanup();
    exit(0);
}

void signal_init(
    void)
{
    signal(SIGINT, sig_int);
    signal(SIGHUP, sig_int);
    signal(SIGTERM, sig_int);
}
#endif

/* simple test to packetize the data and print it */
int main(
    int argc,
    char *argv[])
{
    volatile struct mstp_port_struct_t *mstp_port;
    long my_baud = 38400;
    uint32_t packet_count = 0;
    int sockfd = -1;
    char *my_interface = "eth0";

    /* mimic our pointer in the state machine */
    mstp_port = &MSTP_Port;
    if ((argc > 1) && (strcmp(argv[1], "--help") == 0)) {
        printf("mstsnap [serial] [baud] [network]\r\n"
            "Captures MS/TP packets from a serial interface\r\n"
            "and sends them to a network interface using SNAP \r\n"
            "protocol packets (mimics Cimetrics U+4 packet).\r\n" "\r\n"
            "Command line options:\r\n" "[serial] - serial interface.\r\n"
            "    defaults to /dev/ttyUSB0.\r\n"
            "[baud] - baud rate.  9600, 19200, 38400, 57600, 115200\r\n"
            "    defaults to 38400.\r\n" "[network] - network interface.\r\n"
            "    defaults to eth0.\r\n" "");
        return 0;
    }
    /* initialize our interface */
    if (argc > 1) {
        RS485_Set_Interface(argv[1]);
    }
    if (argc > 2) {
        my_baud = strtol(argv[2], NULL, 0);
    }
    if (argc > 3) {
        my_interface = argv[3];
    }
    sockfd = network_init(my_interface, ETH_P_ALL);
    if (sockfd == -1) {
        return 1;
    }
    RS485_Set_Baud_Rate(my_baud);
    RS485_Initialize();
    MSTP_Port.InputBuffer = &RxBuffer[0];
    MSTP_Port.InputBufferSize = sizeof(RxBuffer);
    MSTP_Port.OutputBuffer = &TxBuffer[0];
    MSTP_Port.OutputBufferSize = sizeof(TxBuffer);
    MSTP_Port.This_Station = 127;
    MSTP_Port.Nmax_info_frames = 1;
    MSTP_Port.Nmax_master = 127;
    MSTP_Port.SilenceTimer = Timer_Silence;
    MSTP_Port.SilenceTimerReset = Timer_Silence_Reset;
    MSTP_Init(mstp_port);
    fprintf(stdout, "mstpcap: Using %s for capture at %ld bps.\n",
        RS485_Interface(), (long) RS485_Get_Baud_Rate());
    atexit(cleanup);
#if defined(_WIN32)
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_PROCESSED_INPUT);
    SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlCHandler, TRUE);
#else
    signal_init();
#endif
    /* run forever */
    for (;;) {
        RS485_Check_UART_Data(mstp_port);
        MSTP_Receive_Frame_FSM(mstp_port);
        /* process the data portion of the frame */
        if (mstp_port->ReceivedValidFrame) {
            mstp_port->ReceivedValidFrame = false;
            snap_received_packet(mstp_port, sockfd);
            packet_count++;
        } else if (mstp_port->ReceivedInvalidFrame) {
            mstp_port->ReceivedInvalidFrame = false;
            fprintf(stderr, "ReceivedInvalidFrame\n");
            snap_received_packet(mstp_port, sockfd);
            packet_count++;
        }
        if (!(packet_count % 100)) {
            fprintf(stdout, "\r%hu packets", packet_count);
        }
    }

    return 0;
}
