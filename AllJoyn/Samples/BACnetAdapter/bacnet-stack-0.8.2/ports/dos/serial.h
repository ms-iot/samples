/*
+----------------------------------------------------+
|               Thunderbird Software                 |
+----------------------------------------------------+
| Filespec  :  Serial.c                              |
| Date      :  October 24, 1991                      |
| Time      :  15:03                                 |
| Revision  :  1.1                                   |
| Update    : August 29, 1994                        |
| Update    : March 12, 1995 by Bob Stout            |
+----------------------------------------------------+
| Programmer:  Scott Andrews                         |
| Address   :  5358 Summit RD SW                     |
| City/State:  Pataskala, Ohio                       |
| Zip       :  43062                                 |
+----------------------------------------------------+
| Released to the Public Domain                      |
+----------------------------------------------------+
*/

#ifndef SERIAL__H
#define SERIAL__H

#include "extkword.h"
#include "pchwio.h"

#define  SerInBufSize  4096     /* Size of input buffer          */
#define  SerOutBufSize 512      /* Size of output buffer         */

/* 8250 registers */

#define  REC             0      /* Uart receive reg.             */
#define  XMIT            0      /* Uart transmit reg.            */
#define  INT_EN          1      /* Uart int. enable reg.         */
#define  INT_ID          2      /* Uart int. ident. reg.         */
#define  LINE_CNTRL      3      /* Uart line control reg.        */
#define  MODEM_CNTRL     4      /* Uart modem control reg.       */
#define  LINE_STATUS     5      /* Uart line status reg.         */
#define  MODEM_STATUS    6      /* Uart modem status reg.        */
#define  BAUD_LSB        0      /* Uart baud divisor reg.        */
#define  BAUD_MSB        1      /* Uart baud divisor reg.        */

#define  NONE            0      /* Handshake param none          */
#define  HDW             1      /* Handshake param hardware      */
#define  XON             2      /* Handshake param software      */

/* Interrupt enable register  */

#define  RX_INT          0x01   /* Receive interrupt mask        */
#define  TBE_INT         0x02   /* Transmit buffer empty mask    */
#define  ERR_INT         0x04   /* Error interrupt mask          */
#define  RS_INT          0x08   /* Line interrupt mask           */

/* Interrupt id register      */

#define  OUT2            0x08   /* Out 2 line                    */
#define  DTR             0x01   /* DTR high                      */
#define  RTS             0x02   /* RTS high                      */
#define  CTS             0x10
#define  DSR             0x20
#define  XMTRDY          0x20
#define  TXR             0      /*  Transmit register (WRITE)          */

#if !defined TRUE       /* Define boolean true/false     */
#define  FALSE  0
#define  TRUE   !FALSE
#endif

extern void (
    INTERRUPT FAR * oldvector_serial) (
    void);

extern int ComBase;     /* Comm port address             */
extern int IrqNum;      /* Comm interrupt request        */

typedef struct {        /* Save existing comm params     */
    int int_enable;     /* old interrupt enable reg value */
    int line;   /*  "  line control      "    "  */
    int modem;  /* old modem control     "    "  */
    int baud_lsb;       /* old baud rate divisor LSD     */
    int baud_msb;       /*  "   "    "      "    MSD     */
    int int_cntrl;      /* old PIC interrupt reg value   */
} OLD_COMM_PARAMS;
extern OLD_COMM_PARAMS old_comm_params;

typedef struct {
    int line;   /* Uart line status reg.         */
    int modem;  /* Uart mode status reg.         */
    int intrupt;        /* Uart interrupt reg.           */
    int handshake;      /* Handshake status              */
} COMM_STATUS;  /* status, updated, handler      */
extern COMM_STATUS comm_status;

int OpenComPort(
    char Port); /*setup comm for usage         */
void InitComPort(
    char Baud[],
    char Databits,
    char Parity,
    char Stop);
void CloseComPort(
    void);      /* Restore comm port           */
void DropDtr(
    void);      /* Lower DTR                   */
void RaiseDtr(
    void);      /* Raise DTR                   */
int ComRecChar(
    void);      /* Fetch character from rcv buf */

int ComSendChar(
    char character);    /* Put char into xmit buffer   */
int ComSendString(
    char *string);
int ComSendData(
    char *buffer,
    unsigned buffer_length);
int ComStatus(
    void);      /* Fetch comm status           */
void INTERRUPT FAR serial(
    void);      /* interrupt handler           */

/* End of Serial.H */

#endif /* SERIAL__H */
