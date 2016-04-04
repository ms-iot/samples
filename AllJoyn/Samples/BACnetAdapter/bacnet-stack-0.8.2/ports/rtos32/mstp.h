/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2004 Steve Karg

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

#ifndef MSTP_H
#define MSTP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "bacdef.h"
#include "mstpdef.h"
#include "dlmstp.h"

struct mstp_port_struct_t {
    MSTP_RECEIVE_STATE receive_state;
    /* When a master node is powered up or reset, */
    /* it shall unconditionally enter the INITIALIZE state. */
    MSTP_MASTER_STATE master_state;
    /* A Boolean flag set to TRUE by the Receive State Machine  */
    /* if an error is detected during the reception of a frame.  */
    /* Set to FALSE by the main state machine. */
    unsigned ReceiveError:1;
    /* There is data in the buffer */
    unsigned DataAvailable:1;
    unsigned FramingError:1;    /* TRUE if we got a framing error */
    unsigned ReceivedInvalidFrame:1;
    /* A Boolean flag set to TRUE by the Receive State Machine  */
    /* if a valid frame is received.  */
    /* Set to FALSE by the main state machine. */
    unsigned ReceivedValidFrame:1;
    /* A Boolean flag set to TRUE by the master machine if this node is the */
    /* only known master node. */
    unsigned SoleMaster:1;
    /* After receiving a frame this value will be TRUE until Tturnaround */
    /* has expired */
    unsigned Turn_Around_Waiting:1;
    /* stores the latest received data */
    uint8_t DataRegister;
    /* Used to accumulate the CRC on the data field of a frame. */
    uint16_t DataCRC;
    /* Used to store the data length of a received frame. */
    unsigned DataLength;
    /* Used to store the destination address of a received frame. */
    uint8_t DestinationAddress;
    /* Used to count the number of received octets or errors. */
    /* This is used in the detection of link activity. */
    /* Compared to Nmin_octets */
    uint8_t EventCount;
    /* Used to store the frame type of a received frame. */
    uint8_t FrameType;
    /* The number of frames sent by this node during a single token hold. */
    /* When this counter reaches the value Nmax_info_frames, the node must */
    /* pass the token. */
    unsigned FrameCount;
    /* Used to accumulate the CRC on the header of a frame. */
    uint8_t HeaderCRC;
    /* Used as an index by the Receive State Machine, up to a maximum value of */
    /* InputBufferSize. */
    unsigned Index;
    /* An array of octets, used to store octets as they are received. */
    /* InputBuffer is indexed from 0 to InputBufferSize-1. */
    /* The maximum size of a frame is 501 octets. */
    uint8_t InputBuffer[MAX_MPDU];
    /* "Next Station," the MAC address of the node to which This Station passes */
    /* the token. If the Next_Station is unknown, Next_Station shall be equal to */
    /* This_Station. */
    uint8_t Next_Station;
    /* "Poll Station," the MAC address of the node to which This Station last */
    /* sent a Poll For Master. This is used during token maintenance. */
    uint8_t Poll_Station;
    /* A counter of transmission retries used for Token and Poll For Master */
    /* transmission. */
    unsigned RetryCount;
    /* A timer with nominal 5 millisecond resolution used to measure and */
    /* generate silence on the medium between octets. It is incremented by a */
    /* timer process and is cleared by the Receive State Machine when activity */
    /* is detected and by the SendFrame procedure as each octet is transmitted. */
    /* Since the timer resolution is limited and the timer is not necessarily */
    /* synchronized to other machine events, a timer value of N will actually */
    /* denote intervals between N-1 and N */
    uint16_t SilenceTimer;

    /* A timer used to measure and generate Reply Postponed frames.  It is */
    /* incremented by a timer process and is cleared by the Master Node State */
    /* Machine when a Data Expecting Reply Answer activity is completed. */
    uint16_t ReplyPostponedTimer;

    /* Used to store the Source Address of a received frame. */
    uint8_t SourceAddress;

    /* The number of tokens received by this node. When this counter reaches the */
    /* value Npoll, the node polls the address range between TS and NS for */
    /* additional master nodes. TokenCount is set to zero at the end of the */
    /* polling process. */
    unsigned TokenCount;

    /* "This Station," the MAC address of this node. TS is generally read from a */
    /* hardware DIP switch, or from nonvolatile memory. Valid values for TS are */
    /* 0 to 254. The value 255 is used to denote broadcast when used as a */
    /* destination address but is not allowed as a value for TS. */
    uint8_t This_Station;

    /* This parameter represents the value of the Max_Info_Frames property of */
    /* the node's Device object. The value of Max_Info_Frames specifies the */
    /* maximum number of information frames the node may send before it must */
    /* pass the token. Max_Info_Frames may have different values on different */
    /* nodes. This may be used to allocate more or less of the available link */
    /* bandwidth to particular nodes. If Max_Info_Frames is not writable in a */
    /* node, its value shall be 1. */
    unsigned Nmax_info_frames;

    /* This parameter represents the value of the Max_Master property of the */
    /* node's Device object. The value of Max_Master specifies the highest */
    /* allowable address for master nodes. The value of Max_Master shall be */
    /* less than or equal to 127. If Max_Master is not writable in a node, */
    /* its value shall be 127. */
    unsigned Nmax_master;

    /* An array of octets, used to store PDU octets prior to being transmitted. */
    /* This array is only used for APDU messages */
    uint8_t TxBuffer[MAX_MPDU];
    unsigned TxLength;
    bool TxReady;       /* true if ready to be sent or received */
    uint8_t TxFrameType;        /* type of message - needed by MS/TP */
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void MSTP_Init(
        volatile struct mstp_port_struct_t *mstp_port,
        uint8_t this_station_mac);
    void MSTP_Millisecond_Timer(
        volatile struct mstp_port_struct_t
        *mstp_port);
    void MSTP_Receive_Frame_FSM(
        volatile struct mstp_port_struct_t
        *mstp_port);
    bool MSTP_Master_Node_FSM(
        volatile struct mstp_port_struct_t
        *mstp_port);

    /* returns true if line is active */
    bool MSTP_Line_Active(
        volatile struct mstp_port_struct_t *mstp_port);

    unsigned MSTP_Create_Frame(
        uint8_t * buffer,       /* where frame is loaded */
        unsigned buffer_len,    /* amount of space available */
        uint8_t frame_type,     /* type of frame to send - see defines */
        uint8_t destination,    /* destination address */
        uint8_t source, /* source address */
        uint8_t * data, /* any data to be sent - may be null */
        unsigned data_len);     /* number of bytes of data (up to 501) */


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
