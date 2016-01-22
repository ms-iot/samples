/**************************************************************************
*
* Copyright (C) 2004 Steve Karg <skarg@users.sourceforge.net>
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
#ifndef MSTP_H
#define MSTP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "mstpdef.h"

struct mstp_port_struct_t {
    MSTP_RECEIVE_STATE receive_state;
    /* When a master node is powered up or reset, */
    /* it shall unconditionally enter the INITIALIZE state. */
    MSTP_MASTER_STATE master_state;
    /* A Boolean flag set to TRUE by the Receive State Machine  */
    /* if an error is detected during the reception of a frame.  */
    /* Set to FALSE by the Master or Slave Node state machine. */
    unsigned ReceiveError:1;
    /* There is data in the buffer */
    unsigned DataAvailable:1;
    unsigned ReceivedInvalidFrame:1;
    /* A Boolean flag set to TRUE by the Receive State Machine  */
    /* if a valid frame is received.  */
    /* Set to FALSE by the Master or Slave Node state machine. */
    unsigned ReceivedValidFrame:1;
    /* A Boolean flag set to TRUE by the Receive State Machine  */
    /* if a valid frame is received but it is not addressed to us.  */
    /* Set to FALSE by the Master or Slave Node state machine. */
    unsigned ReceivedValidFrameNotForUs:1;
    /* A Boolean flag set to TRUE by the master machine if this node is the */
    /* only known master node. */
    unsigned SoleMaster:1;
    /* stores the latest received data */
    uint8_t DataRegister;
    /* Used to accumulate the CRC on the data field of a frame. */
    uint16_t DataCRC;
    /* Used to store the actual CRC from the data field. */
    uint8_t DataCRCActualMSB;
    uint8_t DataCRCActualLSB;
    /* Used to store the data length of a received frame. */
    uint16_t DataLength;
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
    uint8_t FrameCount;
    /* Used to accumulate the CRC on the header of a frame. */
    uint8_t HeaderCRC;
    /* Used to store the actual CRC from the header. */
    uint8_t HeaderCRCActual;
    /* Used as an index by the Receive State Machine, up to a maximum value of */
    /* InputBufferSize. */
    uint32_t Index;
    /* An array of octets, used to store octets as they are received. */
    /* InputBuffer is indexed from 0 to InputBufferSize-1. */
    /* The maximum size of a frame is 501 octets. */
    /* FIXME: assign this to an actual array of bytes! */
    /* Note: the buffer is designed as a pointer since some compilers
       and microcontroller architectures have limits as to places to
       hold contiguous memory. */
    uint8_t *InputBuffer;
    uint16_t InputBufferSize;
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
    /* Note: done here as functions - put into timer task or ISR
       so that you can be atomic on 8 bit microcontrollers */
             uint32_t(
        *SilenceTimer) (
        void *pArg);
    void (
        *SilenceTimerReset) (
        void *pArg);

    /* A timer used to measure and generate Reply Postponed frames.  It is */
    /* incremented by a timer process and is cleared by the Master Node State */
    /* Machine when a Data Expecting Reply Answer activity is completed. */
    /* note: we always send a reply postponed since a message other than
       the reply may be in the transmit queue */
    /*    uint16_t ReplyPostponedTimer; */

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
    uint8_t Nmax_info_frames;

    /* This parameter represents the value of the Max_Master property of the */
    /* node's Device object. The value of Max_Master specifies the highest */
    /* allowable address for master nodes. The value of Max_Master shall be */
    /* less than or equal to 127. If Max_Master is not writable in a node, */
    /* its value shall be 127. */
    uint8_t Nmax_master;

    /* An array of octets, used to store octets for transmitting */
    /* OutputBuffer is indexed from 0 to OutputBufferSize-1. */
    /* The maximum size of a frame is 501 octets. */
    /* FIXME: assign this to an actual array of bytes! */
    /* Note: the buffer is designed as a pointer since some compilers
       and microcontroller architectures have limits as to places to
       hold contiguous memory. */
    uint8_t *OutputBuffer;
    uint16_t OutputBufferSize;

    /*Platform-specific port data */
    void *UserData;

};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void MSTP_Init(
        volatile struct mstp_port_struct_t *mstp_port);
    void MSTP_Receive_Frame_FSM(
        volatile struct mstp_port_struct_t
        *mstp_port);
    bool MSTP_Master_Node_FSM(
        volatile struct mstp_port_struct_t
        *mstp_port);
    void MSTP_Slave_Node_FSM(
        volatile struct mstp_port_struct_t *mstp_port);

    /* returns true if line is active */
    bool MSTP_Line_Active(
        volatile struct mstp_port_struct_t *mstp_port);

    uint16_t MSTP_Create_Frame(
        uint8_t * buffer,       /* where frame is loaded */
        uint16_t buffer_len,    /* amount of space available */
        uint8_t frame_type,     /* type of frame to send - see defines */
        uint8_t destination,    /* destination address */
        uint8_t source, /* source address */
        uint8_t * data, /* any data to be sent - may be null */
        uint16_t data_len);     /* number of bytes of data (up to 501) */

    void MSTP_Create_And_Send_Frame(
        volatile struct mstp_port_struct_t *mstp_port,  /* port to send from */
        uint8_t frame_type,     /* type of frame to send - see defines */
        uint8_t destination,    /* destination address */
        uint8_t source, /* source address */
        uint8_t * data, /* any data to be sent - may be null */
        uint16_t data_len);

    void MSTP_Fill_BACnet_Address(
        BACNET_ADDRESS * src,
        uint8_t mstp_address);

    /* functions used by the MS/TP state machine to put or get data */
    /* FIXME: developer must implement these in their DLMSTP module */
    uint16_t MSTP_Put_Receive(
        volatile struct mstp_port_struct_t *mstp_port);

    /* for the MS/TP state machine to use for getting data to send */
    /* Return: amount of PDU data */
    uint16_t MSTP_Get_Send(
        volatile struct mstp_port_struct_t *mstp_port,
        unsigned timeout);      /* milliseconds to wait for a packet */
    /* for the MS/TP state machine to use for getting the reply for
       Data-Expecting-Reply Frame */
    /* Return: amount of PDU data */
    uint16_t MSTP_Get_Reply(
        volatile struct mstp_port_struct_t *mstp_port,
        unsigned timeout);      /* milliseconds to wait for a packet */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
