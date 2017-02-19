/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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
#if PRINT_ENABLED
#define PRINT_ENABLED_RS485 1
#else
#define PRINT_ENABLED_RS485 0
#endif

#include <stdint.h>
#include <rtkernel.h>
#include <rtcom.h>
#include <itimer.h>
#if PRINT_ENABLED_RS485
#include <stdio.h>
#endif
#include "mstp.h"

/* note: use the RTKernel-C API so that it can use this library */

#define RS485_IO_ENABLE(p)       ModemControl(p,0,DTR);
#define RS485_TRANSMIT_ENABLE(p) ModemControl(p,1,RTS);
#define RS485_RECEIVE_ENABLE(p)  ModemControl(p,0,RTS);

/* COM port number - COM1 = 0 */
static int RS485_Port = COM2;
/* baud rate */
static long RS485_Baud = 9600;
/* io base address */
static long RS485_Base = 0;
/* hardware IRQ number */
static long RS485_IRQ_Number = 0;

#if PRINT_ENABLED_RS485
static FineTime RS485_Debug_Transmit_Timer;
#endif

#if PRINT_ENABLED_RS485
void RS485_Print_Frame(
    int port,
    FineTime timer,
    uint8_t * buffer,   /* frame to send (up to 501 bytes of data) */
    uint16_t nbytes)
{
    uint16_t i; /* byte counter */
    unsigned long duration;     /* measures the time from last output to this one */
    unsigned long seconds;
    unsigned long milliseconds;

    duration = ElapsedMilliSecs(timer);
    seconds = duration / 1000U;
    milliseconds = duration - (seconds * 1000U);
    fprintf(stderr, "%0lu.%03lu: COM%d:", seconds, milliseconds, port + 1);
    for (i = 0; i < nbytes; i++) {
        unsigned value;
        value = buffer[i];
        fprintf(stderr, " %02X", value);
    }
    fprintf(stderr, "\n");
    fflush(stderr);
}
#endif

static void RS485_Standard_Port_Settings(
    long port,
    long *pIRQ,
    long *pBase)
{
    switch (port) {
        case COM1:
            *pBase = (long) 0x3F8;
            *pIRQ = 4L;
            break;
        case COM2:
            *pBase = (long) 0x2F8;
            *pIRQ = 3L;
            break;
        case COM3:
            *pBase = (long) 0x3E8;
            *pIRQ = 4L;
            break;
        case COM4:
            *pBase = (long) 0x2E8;
            *pIRQ = 3L;
            break;
        default:
            break;
    }
}

static int TestCOMPort(
    int Base)
{       /* base address of UART */
    int i;

    for (i = 0; i < 256; i++) {
        RTOut(Base + 7, (BYTE) i);      /* write scratch register */
        RTOut(Base + 1, RTIn(Base + 1));        /* read/write IER */
        if (RTIn(Base + 7) != i)        /* check scratch register */
            return FALSE;
    }
    return TRUE;
}

static RS485_Open_Port(
    int port,   /* COM port number - COM1 = 0 */
    long baud,  /* baud rate */
    unsigned base,      /* io base address */
    int irq)
{       /* hardware IRQ number */
    if (!TestCOMPort(base))
        return;

    /* setup the COM IO */
    SetIOBase(port, base);
    SetIRQ(port, irq);

    if (irq < 8)
        RTKIRQTopPriority(irq, 9);

    InitPort(port, baud, PARITY_NONE, 1, 8);

    if (HasFIFO(port))
        EnableFIFO(port, 8);
    EnableCOMInterrupt(port, 1024 * 4);

    /* enable the 485 via the DTR pin */
    RS485_IO_ENABLE(port);
    RS485_RECEIVE_ENABLE(port);
#if PRINT_ENABLED_RS485
    fprintf(stderr, "RS485: COM%d Enabled\r\n", port + 1);
#endif

    return;
}

void RS485_Initialize(
    void)
{
#if PRINT_ENABLED_RS485
    MarkTime(&RS485_Debug_Transmit_Timer);
#endif
    RS485_Standard_Port_Settings(RS485_Port, &RS485_IRQ_Number, &RS485_Base);
    RS485_Open_Port(RS485_Port, RS485_Baud, RS485_Base, RS485_IRQ_Number);
}

void RS485_Send_Frame(
    volatile struct mstp_port_struct_t *mstp_port,      /* port specific data */
    uint8_t * buffer,   /* frame to send (up to 501 bytes of data) */
    uint16_t nbytes)
{       /* number of bytes of data (up to 501) */
    bool status = true; /* return value */

    /* fixme: wait turnaround time */
    RS485_TRANSMIT_ENABLE(RS485_Port);
    SendBlock(RS485_Port, (char *) buffer, nbytes);
    /* need to wait at least 9600 baud * 512 bytes = 54mS */
    (void) WaitSendBufferEmpty(RS485_Port, MilliSecsToTicks(200));
    while (!(LineStatus(RS485_Port) & TX_SHIFT_EMPTY))
        RTKScheduler();
    RS485_RECEIVE_ENABLE(RS485_Port);
    /* SilenceTimer is cleared by the Receive State Machine when
       activity is detected and by the SendFrame procedure as each
       octet is transmitted. */
    mstp_port->SilenceTimer = 0;
#if PRINT_ENABLED_RS485
    RS485_Print_Frame(RS485_Port, RS485_Debug_Transmit_Timer, buffer,   /* frame to send (up to 501 bytes of data) */
        nbytes);
    MarkTime(&RS485_Debug_Transmit_Timer);
#endif

    return;
}

void RS485_Check_UART_Data(
    volatile struct mstp_port_struct_t *mstp_port)
{       /* port specific data */
    COMData com_data = 0;       /* byte from COM driver */
    unsigned timeout = 1;       /* milliseconds to wait for a character */
    static Duration ticks = 0;  /* duration to wait for data */

    if (mstp_port->ReceiveError) {
        /* wait for state machine to clear this */
        RTKScheduler();
    }
    /* wait for state machine to read from the DataRegister */
    else if (!mstp_port->DataAvailable) {
        if (!ticks) {
            ticks = MilliSecsToTicks(timeout);
            if (!ticks)
                ticks = 1;
        }
        /* check for data */
        if (RTKGetTimed(ReceiveBuffer[RS485_Port], &com_data, ticks)) {
            /* if error, */
            if (com_data & (COM_OVERRUN << 8))
                mstp_port->ReceiveError = true;
            else if (com_data & (COM_FRAME << 8))
                mstp_port->ReceiveError = true;
            else {
                mstp_port->DataRegister = com_data & 0x00FF;
                mstp_port->DataAvailable = true;
            }
        }
    } else
        RTKScheduler();
}
