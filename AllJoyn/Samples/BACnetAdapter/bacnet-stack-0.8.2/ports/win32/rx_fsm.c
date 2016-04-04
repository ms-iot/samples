/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2007 Steve Karg

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
#include <conio.h>
#include <time.h>

/* Windows includes */
#define WIN32_LEAN_AND_MEAN
#define STRICT 1
#include <windows.h>

/* local includes */
#include "bytes.h"
#include "rs485.h"
#include "mstp.h"
#include "mstptext.h"
#include "crc.h"

#ifndef max
#define max(a,b) (((a) (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

/* file format for libpcap/winpcap */
/* from http://wiki.wireshark.org/Development/LibpcapFileFormat */
typedef struct pcap_hdr_s {
    uint32_t magic_number;      /* magic number */
    uint16_t version_major;     /* major version number */
    uint16_t version_minor;     /* minor version number */
    int32_t thiszone;   /* GMT to local correction */
    uint32_t sigfigs;   /* accuracy of timestamps */
    uint32_t snaplen;   /* max length of captured packets, in octets */
    uint32_t network;   /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
    uint32_t ts_sec;    /* timestamp seconds */
    uint32_t ts_usec;   /* timestamp microseconds */
    uint32_t incl_len;  /* number of octets of packet saved in file */
    uint32_t orig_len;  /* actual length of packet */
} pcaprec_hdr_t;

/* local port data - shared with RS-485 */
volatile struct mstp_port_struct_t MSTP_Port;
static uint8_t RxBuffer[MAX_MPDU];
static uint8_t TxBuffer[MAX_MPDU];
static uint16_t SilenceTime;
#define INCREMENT_AND_LIMIT_UINT16(x) {if (x < 0xFFFF) x++;}
static uint16_t Timer_Silence(
    void)
{
    return SilenceTime;
}

static void Timer_Silence_Reset(
    void)
{
    SilenceTime = 0;
}

static void dlmstp_millisecond_timer(
    void)
{
    INCREMENT_AND_LIMIT_UINT16(SilenceTime);
}

void *milliseconds_task(
    void *pArg)
{
    (void) pArg;
    for (;;) {
        Sleep(1);
        dlmstp_millisecond_timer();
    }

    /*return NULL; */
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

#if 0
static void print_received_packet(
    volatile struct mstp_port_struct_t *mstp_port)
{
    unsigned i;

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
    fprintf(stderr, "55 FF %02X %02X %02X %02X %02X %02X ",
        mstp_port->FrameType, mstp_port->DestinationAddress,
        mstp_port->SourceAddress, HI_BYTE(mstp_port->DataLength),
        LO_BYTE(mstp_port->DataLength), mstp_port->HeaderCRCActual);
    if (mstp_port->DataLength) {
        max_data = min(mstp_port->InputBufferSize, mstp_port->DataLength);
        for (i = 0; i < max_data; i++) {
            fprintf(stderr, "%02X ", mstp_port->InputBuffer[i]);
        }
        fprintf(stderr, "%02X %02X ", mstp_port->DataCRCActualMSB,
            mstp_port->DataCRCActualLSB);
    }
    fprintf(stderr, "%s", mstptext_frame_type(mstp_port->FrameType));
    fprintf(stderr, "\n");
}
#endif

/* returns a delta timestamp */
void timestamp(
    uint32_t * ts_sec,  /* timestamp seconds since epoch (Unix) */
    uint32_t * ts_usec)
{       /* timestamp microseconds (unix) */
    DWORD ticks = 0;
    static DWORD initial_ticks = 0;
    static time_t initial_seconds = 0;
    time_t seconds = 0;
    time_t milliseconds = 0;
    time_t elapsed_ticks = 0;

    if (initial_seconds == 0) {
        initial_seconds = time(NULL);
        initial_ticks = GetTickCount();
    }
    ticks = GetTickCount();
    /* how much total time has passed? */
    elapsed_ticks = ticks - initial_ticks;
    seconds = elapsed_ticks / 1000;
    milliseconds = elapsed_ticks - (seconds * 1000);
    *ts_sec = initial_seconds + seconds;
    *ts_usec = milliseconds * 1000;

    return;
}

static const char *Capture_Filename = "mstp.cap";
static FILE *pFile = NULL;      /* stream pointer */

/* write packet to file in libpcap format */
static void write_global_header(
    void)
{
    uint32_t magic_number = 0xa1b2c3d4; /* magic number */
    uint16_t version_major = 2; /* major version number */
    uint16_t version_minor = 4; /* minor version number */
    int32_t thiszone = 0;       /* GMT to local correction */
    uint32_t sigfigs = 0;       /* accuracy of timestamps */
    uint32_t snaplen = 65535;   /* max length of captured packets, in octets */
    uint32_t network = 165;     /* data link type */

    /* create a new file. */
    pFile = fopen(Capture_Filename, "wb");
    if (pFile) {
        fwrite(&magic_number, sizeof(magic_number), 1, pFile);
        fwrite(&version_major, sizeof(version_major), 1, pFile);
        fwrite(&version_minor, sizeof(version_minor), 1, pFile);
        fwrite(&thiszone, sizeof(thiszone), 1, pFile);
        fwrite(&sigfigs, sizeof(sigfigs), 1, pFile);
        fwrite(&snaplen, sizeof(snaplen), 1, pFile);
        fwrite(&network, sizeof(network), 1, pFile);
    } else {
        fprintf(stderr, "rx_fsm: failed to open %s: %s\n", Capture_Filename,
            strerror(errno));
    }
}

static void write_received_packet(
    volatile struct mstp_port_struct_t *mstp_port)
{
    uint32_t ts_sec;    /* timestamp seconds */
    uint32_t ts_usec;   /* timestamp microseconds */
    uint32_t incl_len;  /* number of octets of packet saved in file */
    uint32_t orig_len;  /* actual length of packet */
    uint8_t header[8];  /* MS/TP header */

    if (pFile) {
        timestamp(&ts_sec, &ts_usec);
        fwrite(&ts_sec, sizeof(ts_sec), 1, pFile);
        fwrite(&ts_usec, sizeof(ts_usec), 1, pFile);
        if (mstp_port->DataLength) {
            max_data = min(mstp_port->InputBufferSize, mstp_port->DataLength);
            incl_len = orig_len = 8 + max_data + 2;
        } else {
            incl_len = orig_len = 8;
        }
        fwrite(&incl_len, sizeof(incl_len), 1, pFile);
        fwrite(&orig_len, sizeof(orig_len), 1, pFile);
        header[0] = 0x55;
        header[1] = 0xFF;
        header[2] = mstp_port->FrameType;
        header[3] = mstp_port->DestinationAddress;
        header[4] = mstp_port->SourceAddress;
        header[5] = HI_BYTE(mstp_port->DataLength);
        header[6] = LO_BYTE(mstp_port->DataLength);
        header[7] = mstp_port->HeaderCRCActual;
        fwrite(header, sizeof(header), 1, pFile);
        if (mstp_port->DataLength) {
            fwrite(&(mstp_port->DataCRCActualMSB), 1, 1, pFile);
            fwrite(&(mstp_port->DataCRCActualLSB), 1, 1, pFile);
        }
    } else {
        fprintf(stderr, "rx_fsm: failed to open %s: %s\n", Capture_Filename,
            strerror(errno));
    }
}

static char *Network_Interface = NULL;

static void cleanup(
    void)
{
    if (pFile) {
        fflush(pFile);  /* stream pointer */
        fclose(pFile);  /* stream pointer */
    }
    pFile = NULL;
}

/* simple test to packetize the data and print it */
int main(
    int argc,
    char *argv[])
{
    volatile struct mstp_port_struct_t *mstp_port;
    int rc = 0;
    unsigned long hThread = 0;
    uint32_t arg_value = 0;
    long my_mac = 127;
    long my_baud = 38400;

    /* mimic our pointer in the state machine */
    mstp_port = &MSTP_Port;
    /* argv has the "COM4" or some other device */
    if (argc > 1) {
        Network_Interface = argv[1];
    }
    if (argc > 2) {
        my_baud = strtol(argv[2], NULL, 0);
    }
    if (argc > 3) {
        my_mac = strtol(argv[3], NULL, 0);
        if (my_mac > 127)
            my_mac = 127;
    }
    /* initialize our interface */
    RS485_Set_Interface(Network_Interface);
    RS485_Set_Baud_Rate(my_baud);
    RS485_Initialize();
    MSTP_Port.InputBuffer = &RxBuffer[0];
    MSTP_Port.InputBufferSize = sizeof(RxBuffer);
    MSTP_Port.OutputBuffer = &TxBuffer[0];
    MSTP_Port.OutputBufferSize = sizeof(TxBuffer);
    MSTP_Port.This_Station = my_mac;
    MSTP_Port.Nmax_info_frames = 1;
    MSTP_Port.Nmax_master = 127;
    MSTP_Port.SilenceTimer = Timer_Silence;
    MSTP_Port.SilenceTimerReset = Timer_Silence_Reset;
    MSTP_Init(&MSTP_Port);
    mstp_port->Lurking = true;
    /* start our MilliSec task */
    hThread = _beginthread(milliseconds_task, 4096, &arg_value);
    if (hThread == 0) {
        fprintf(stderr, "Failed to start timer task\n");
    }
    atexit(cleanup);
    write_global_header();
    (void) SetThreadPriority(GetCurrentThread(),
        THREAD_PRIORITY_TIME_CRITICAL);
    /* run forever */
    for (;;) {
        RS485_Check_UART_Data(mstp_port);
        MSTP_Receive_Frame_FSM(mstp_port);
        /* process the data portion of the frame */
        if (mstp_port->ReceivedValidFrame) {
            mstp_port->ReceivedValidFrame = false;
#if 0
            print_received_packet(mstp_port);
#endif
            write_received_packet(mstp_port);
        } else if (mstp_port->ReceivedInvalidFrame) {
            mstp_port->ReceivedInvalidFrame = false;
            fprintf(stderr, "ReceivedInvalidFrame\n");
#if 0
            print_received_packet(mstp_port);
#endif
            write_received_packet(mstp_port);
        }
    }

    /*return 0; */
}
