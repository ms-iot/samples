/*
+----------------------------------------------------+
|               Thunderbird Software                 |
+----------------------------------------------------+
| Filespec  :  Serial.c                              |
| Date      :  October 24, 1991                      |
| Time      :  15:03                                 |
| Revision  :  1.1                                   |
|     Update: August 29, 1994                        |
+----------------------------------------------------+
| Programmer:  Scott Andrews                         |
| Address   :  5358 Summit RD SW                     |
| City/State:  Pataskala, Ohio                       |
| Zip       :  43062                                 |
+----------------------------------------------------+
| Released to the Public Domain                      |
+----------------------------------------------------+
*/

/*
+----------------------------------------------------------+
|  Call open_serial to install the interrupt handler       |
|  You must call close_serial before exiting your program  |
|  or a machine crash will occur!                          |
+----------------------------------------------------------+
*/

#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include "serial.h"
#include "queue.h"

QUEUE *Serial_In_Queue;
QUEUE *Serial_Out_Queue;

OLD_COMM_PARAMS old_comm_params;
COMM_STATUS comm_status;

void (
    INTERRUPT FAR * oldvector_serial) (
    );
/* save addr for intr handler */

int ComBase;    /* Comm port address */
int IrqNum;     /* Comm interrupt request */

void CloseComPort(
    void)
{
    int status;

    /* restore UART to previous state */

    outp(ComBase + INT_EN, (unsigned char) 0);
    outp(ComBase + MODEM_CNTRL, (unsigned char) old_comm_params.modem);
    status = inp(ComBase + LINE_CNTRL);
    outp(ComBase + LINE_CNTRL, (unsigned char) status | 0x80);
    outp(ComBase + BAUD_LSB, (unsigned char) old_comm_params.baud_lsb);
    outp(ComBase + BAUD_MSB, (unsigned char) old_comm_params.baud_msb);
    outp(ComBase + LINE_CNTRL, (unsigned char) old_comm_params.line);
    outp(0x21, (unsigned char) old_comm_params.int_cntrl);

    /* restore old interrupt handler */

    setvect(IrqNum + 8, oldvector_serial);

    /* free input and output queues */

    free(Serial_In_Queue);
    free(Serial_Out_Queue);
    return;
}

int OpenComPort(
    char Port)
{       /* install int. handler */
    unsigned status;
    int retval = -1;

    /* allocate input and output queues */

    Serial_In_Queue = alloc_queue(SerInBufSize);
    if ((QUEUE *) 0 == Serial_In_Queue)
        return retval;
    Serial_Out_Queue = alloc_queue(SerOutBufSize);
    if ((QUEUE *) 0 == Serial_Out_Queue) {
        free(Serial_In_Queue);
        return retval;
    }
    retval = 0;

    /* Setup Comm base port address and IRQ number */

    switch (Port) {
        case '1':
            ComBase = 0x3F8;
            IrqNum = 4;
            break;
        case '2':
            ComBase = 0x2F8;
            IrqNum = 3;
            break;
        case '3':
            ComBase = 0x3E8;
            IrqNum = 4;
            break;
        case '4':
            ComBase = 0x2E8;
            IrqNum = 3;
            break;
        default:
            ComBase = 0x3F8;
            IrqNum = 4;
            break;
    }
    old_comm_params.int_enable = inp(ComBase + INT_EN);
    outp(ComBase + INT_EN, 0);  /* turn off comm interrupts */

    /* save old comm parameters */

    old_comm_params.line = inp(ComBase + LINE_CNTRL);
    old_comm_params.modem = inp(ComBase + MODEM_CNTRL);
    status = inp(ComBase + LINE_CNTRL);
    outp(ComBase + LINE_CNTRL, (unsigned char) status | 0x80);
    old_comm_params.baud_lsb = inp(ComBase + BAUD_LSB);
    old_comm_params.baud_msb = inp(ComBase + BAUD_MSB);
    status = inp(ComBase + LINE_CNTRL);
    outp(ComBase + LINE_CNTRL, (unsigned char) status | 0x7F);
    status = OUT2 | DTR;        /* DTR/OUT2 must be set! */
    outp(ComBase + MODEM_CNTRL, (unsigned char) status);

    /* get serial port address/vector */

    oldvector_serial = (void (INTERRUPT FAR *) (void)) getvect(IrqNum + 8);

    /* set our interrupt handler */

    setvect(IrqNum + 8, serial);

    /* save the PIC */

    old_comm_params.int_cntrl = inp(0x21);
    status = (1 << IrqNum);     /* calculate int enable bit */
    status = ~status;

    /* ok enable comm ints */

    outp(0x21,
        (unsigned char) old_comm_params.int_cntrl & (unsigned char) status);

    atexit(CloseComPort);

    return retval;
}

void InitComPort(
    char Baud[],
    char Databits,
    char Parity,
    char Stopbits)
{
    int status;
    unsigned divisor;
    long baudrate;

    /* set baud rate */

    status = inp(ComBase + LINE_CNTRL);
    outp(ComBase + LINE_CNTRL, (unsigned char) status | 0x80);
    baudrate = atol(Baud);
    if (baudrate == 0)
        baudrate = 2400L;
    divisor = (unsigned) (115200L / baudrate);
    outp(ComBase + BAUD_LSB, (unsigned char) (divisor & 0x00FF));
    outp(ComBase + BAUD_MSB, (unsigned char) ((divisor >> 8) & 0x00FF));
    status = 0x00;

    /* set parity */

    switch (Parity) {   /* set parity value     */
        case 'O':      /* odd parity           */
        case 'o':
            status = 0x08;
            break;

        case 'E':      /* even parity          */
        case 'e':
            status = 0x18;
            break;

        case 'S':      /* stick parity         */
        case 's':
            status = 0x28;
            break;

        case 'N':      /* no parity            */
        case 'n':
        default:
            status = 0x00;
    }

    /* set number data bits */

    switch (Databits) {
        case '5':
            break;

        case '6':
            status = status | 0x01;
            break;

        case '7':
            status = status | 0x02;
            break;

        case '8':
        default:
            status = status | 0x03;
    }

    /* set number stop bits */

    switch (Stopbits) {
        case '2':
            status = status | 0x04;
            break;

        case '1':
        default:
            ;
    }
    outp(ComBase + LINE_CNTRL, (unsigned char) status);
    status = OUT2 | DTR;        /* DTR/OUT2 must be set! */
    outp(ComBase + MODEM_CNTRL, (unsigned char) status);

    /* enable serial interrupts */

    outp(ComBase + INT_EN, RX_INT | ERR_INT | RS_INT);
    return;
}

void DropDtr(
    void)
{
    int status;

    status = inp(ComBase + MODEM_CNTRL);
    status &= 0xFE;     /* turn off DTR bit */
    outp(ComBase + MODEM_CNTRL, (unsigned char) status);
    return;
}

void RaiseDtr(
    void)
{
    int status;

    status = inp(ComBase + MODEM_CNTRL);
    status |= 0x01;     /* turn on DTR bit */
    outp(ComBase + MODEM_CNTRL, (unsigned char) status);
    return;
}

int ComRecChar(
    void)
{
    return de_queue(Serial_In_Queue);
}

int ComSendString(
    char *string)
{
    int retval;
    char *pointer;
    pointer = string;

    while (*pointer) {
        retval = en_queue(Serial_Out_Queue, *pointer);
        pointer++;
    }
    if (0x0 == (comm_status.modem & 0x40))
        RaiseDtr();
    outp(ComBase + INT_EN, RX_INT | TBE_INT | ERR_INT | RS_INT);
    return retval;
}

int ComSendData(
    char *buffer,
    unsigned buffer_length)
{
    int retval;
    char *pointer;
    pointer = buffer;
    unsigned i;

    for (i = 0; i < buffer_length; i++) {
        retval = en_queue(Serial_Out_Queue, *pointer);
        pointer++;
    }
    if (0x0 == (comm_status.modem & 0x40))
        RaiseDtr();
    outp(ComBase + INT_EN, RX_INT | TBE_INT | ERR_INT | RS_INT);
    return retval;
}

int ComSendChar(
    char character)
{
    int retval;

    /* interrupt driven send */

    if (0x0 == (comm_status.modem & 0x40))
        RaiseDtr();
    retval = en_queue(Serial_Out_Queue, character);
    if (-1 != retval)
        outp(ComBase + INT_EN, RX_INT | TBE_INT | ERR_INT | RS_INT);
    return retval;
}

int ComStatus(
    void)
{
    unsigned status;
    unsigned retval;

    retval = inp(ComBase + LINE_STATUS);
    retval = retval << 8;
    status = inp(ComBase + MODEM_STATUS);
    retval = retval | status;
    if (queue_empty(Serial_In_Queue))
        retval &= 0xFEFF;
    else
        retval |= 0x0100;
    return (int) retval;
}

void INTERRUPT FAR serial(
    void)
{       /* interrupt handler */
    int temp;

    disable();
    while (1) {
        comm_status.intrupt = inp(ComBase + INT_ID);
        comm_status.intrupt &= 0x0f;
        switch (comm_status.intrupt) {
            case 0x00: /* modem interrupt */
                comm_status.modem = inp(ComBase + MODEM_STATUS);
                break;

            case 0x02: /* xmit interrupt */
                if (queue_empty(Serial_Out_Queue))
                    outp(ComBase + INT_EN, RX_INT | ERR_INT | RS_INT);
                else {
                    temp = de_queue(Serial_Out_Queue);
                    if (-1 != temp)
                        outp(ComBase + XMIT, temp);
                }
                break;

            case 0x04: /* receive interrupt */
                en_queue(Serial_In_Queue, (char) inp(ComBase + REC));
                break;

            case 0x06: /* line interrupt */
                comm_status.line = inp(ComBase + LINE_STATUS);
                (void) inp(ComBase + REC);
                en_queue(Serial_In_Queue, '!');
                break;

            default:   /* No Mo` Left */
                comm_status.modem = inp(ComBase + MODEM_STATUS);
                outp(0x20, 0x20);
                enable();
                return;
        }       /* switch */
    }   /* while */
}

/* End of Serial.C */
