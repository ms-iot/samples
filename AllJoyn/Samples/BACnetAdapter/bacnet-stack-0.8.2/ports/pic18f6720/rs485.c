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

/* The module handles sending data out the RS-485 port */
/* and handles receiving data from the RS-485 port. */
/* Customize this file for your specific hardware */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "hardware.h"
#include "mstp.h"
#include "rs485.h"
#include "fifo.h"

/* public port info */
extern volatile struct mstp_port_struct_t MSTP_Port;

/* the baud rate is adjustable */
uint32_t RS485_Baud_Rate = 38400;

/* the FIFO structures for sending and receiving */
FIFO_BUFFER FIFO_Rx;
FIFO_BUFFER FIFO_Tx;
#pragma udata MSTPPortData
/* the buffer for receiving data (size must be a power of 2) */
volatile uint8_t RS485_Rx_Buffer[128];
/* the buffer for sending data (size must be a power of 2) */
volatile uint8_t RS485_Tx_Buffer[128];
#pragma udata

/****************************************************************************
* DESCRIPTION: Transmits a frame using the UART
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Send_Frame(
    volatile struct mstp_port_struct_t *mstp_port,      /* port specific data */
    uint8_t * buffer,   /* frame to send (up to 501 bytes of data) */
    uint16_t nbytes)
{       /* number of bytes of data (up to 501) */
    uint16_t i = 0;     /* loop counter */
    uint8_t turnaround_time;

    if (!buffer)
        return;

    while (!FIFO_Empty(&FIFO_Tx)) {
        /* buffer is not empty.  Wait for ISR to transmit. */
    };

    /* wait 40 bit times since reception */
    if (RS485_Baud_Rate == 9600)
        turnaround_time = 4;
    else if (RS485_Baud_Rate == 19200)
        turnaround_time = 2;
    else
        turnaround_time = 2;

    while (mstp_port->SilenceTimer < turnaround_time) {
        /* The line has not been silent long enough, so wait. */
    };

    if (FIFO_Add(&FIFO_Tx, buffer, nbytes)) {
        /* disable the receiver */
        PIE3bits.RC2IE = 0;
        RCSTA2bits.CREN = 0;
        /* enable the transceiver */
        RS485_TX_ENABLE = 1;
        RS485_RX_DISABLE = 1;
        /* enable the transmitter */
        TXSTA2bits.TXEN = 1;
        PIE3bits.TX2IE = 1;
        /* reset the silence timer per MSTP spec, sort of */
        mstp_port->SilenceTimer = 0;
    }

    return;
}

/****************************************************************************
* DESCRIPTION: Checks for data on the receive UART, and handles errors
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
bool RS485_Check_UART_Data(
    volatile struct mstp_port_struct_t * mstp_port)
{
    /* check for data */
    if (!FIFO_Empty(&FIFO_Rx)) {
        mstp_port->DataRegister = FIFO_Get(&FIFO_Rx);
        mstp_port->DataAvailable = TRUE;
    }

    return (!FIFO_Empty(&FIFO_Rx));
}

/* *************************************************************************
  DESCRIPTION:  Receives RS485 data stream

  RETURN: none

  ALGORITHM:  none

  NOTES:  none
 *************************************************************************** */
void RS485_Interrupt_Rx(
    void)
{
    uint8_t data_byte;

    if ((RCSTA2bits.FERR) || (RCSTA2bits.OERR)) {
        /* Clear the error */
        RCSTA2bits.CREN = 0;
        RCSTA2bits.CREN = 1;
        /* FIXME: flag the MS/TP state machine on buffer overrun */
        data_byte = RCREG2;
    } else {
        data_byte = RCREG2;
        FIFO_Put(&FIFO_Rx, data_byte);
    }
}

/* *************************************************************************
  DESCRIPTION:  Transmits a byte using the UART out the RS485 port

  RETURN: none

  ALGORITHM:  none

  NOTES:  none
 *************************************************************************** */
void RS485_Interrupt_Tx(
    void)
{
    if (!FIFO_Empty(&FIFO_Tx)) {
        TXREG2 = FIFO_Get(&FIFO_Tx);
    } else {
        /* wait for the USART to be empty */
        while (!TXSTA2bits.TRMT);
        /* disable this interrupt */
        PIE3bits.TX2IE = 0;
        /* enable the receiver */
        RS485_TX_ENABLE = 0;
        RS485_RX_DISABLE = 0;
        /* enable the this interrupt */
        PIE3bits.RC2IE = 1;
        RCSTA2bits.CREN = 1;
    }
}

/****************************************************************************
* DESCRIPTION: Returns the baud rate that we are currently running at
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
uint32_t RS485_Get_Baud_Rate(
    void)
{
    return RS485_Baud_Rate;
}

/****************************************************************************
* DESCRIPTION: Sets the baud rate for the chip USART
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
bool RS485_Set_Baud_Rate(
    uint32_t baud)
{
    bool valid = true;

    switch (baud) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 76800:
        case 115200:
            RS485_Baud_Rate = baud;
            break;
        default:
            valid = false;
            break;
    }

    if (valid) {
        /* FIXME: store the baud rate */
        /* I2C_Write_Block(
           EEPROM_DEVICE_ADDRESS,
           (char *)&RS485_Baud_Rate,
           sizeof(RS485_Baud_Rate),
           EEPROM_MSTP_BAUD_RATE_ADDR); */
    }

    return valid;
}

/****************************************************************************
* DESCRIPTION: Initializes the RS485 hardware and variables, and starts in
*              receive mode.
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Initialize_Port(
    void)
{

    /* Reset USART registers to POR state */
    TXSTA2 = 0;
    RCSTA2 = 0;
    /* configure USART for receiving */
    /* since the TX will handle setting up for transmit */
    RCSTA2bits.CREN = 1;
    /* Interrupt on receipt */
    PIE3bits.RC2IE = 1;
    /* enable the transmitter, disable its interrupt */
    TXSTA2bits.TXEN = 1;
    PIE3bits.TX2IE = 0;
    /* setup USART Baud Rate Generator */
    /* see BAUD RATES FOR ASYNCHRONOUS MODE in Data Book */
    /* Fosc=20MHz
       BRGH=1              BRGH=0
       Rate    SPBRG       Rate    SPBRG
       ------- -----       ------- -----
       9615     129          9469    32
       19230     64         19530    15
       37878     32         78130     3
       56818     21        104200     2
       113630    10        312500     0
       250000     4
       625000     1
       1250000    0
     */
    switch (RS485_Baud_Rate) {
        case 19200:
            SPBRG2 = 64;
            TXSTA2bits.BRGH = 1;
            break;
        case 38400:
            SPBRG2 = 32;
            TXSTA2bits.BRGH = 1;
            break;
        case 57600:
            SPBRG2 = 21;
            TXSTA2bits.BRGH = 1;
            break;
        case 76800:
            SPBRG2 = 3;
            TXSTA2bits.BRGH = 0;
            break;
        case 115200:
            SPBRG2 = 10;
            TXSTA2bits.BRGH = 1;
            break;
        case 9600:
            SPBRG2 = 129;
            TXSTA2bits.BRGH = 1;
            break;
        default:
            SPBRG2 = 129;
            TXSTA2bits.BRGH = 1;
            RS485_Set_Baud_Rate(9600);
            break;
    }
    /* select async mode */
    TXSTA2bits.SYNC = 0;
    /* enable transmitter */
    TXSTA2bits.TXEN = 1;
    /* serial port enable */
    RCSTA2bits.SPEN = 1;
    /* since we are using RS485,
       we need to explicitly say
       transmit enable or not */
    RS485_RX_DISABLE = 0;
    RS485_TX_ENABLE = 0;
}

/****************************************************************************
* DESCRIPTION: Disables the RS485 hardware
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Disable_Port(
    void)
{
    RCSTA2 &= 0x4F;     /* Disable the receiver */
    TXSTA2bits.TXEN = 0;        /* and transmitter */
    PIE3 &= 0xCF;       /* Disable both interrupts */
}

/****************************************************************************
* DESCRIPTION: Reinitializes the port
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Reinit(
    void)
{
    RS485_Set_Baud_Rate(38400);
}

/****************************************************************************
* DESCRIPTION: Initializes the data and the port
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Initialize(
    void)
{
    /* Init the Rs485 buffers */
    FIFO_Init(&FIFO_Rx, RS485_Rx_Buffer, sizeof(RS485_Rx_Buffer));
    FIFO_Init(&FIFO_Tx, RS485_Tx_Buffer, sizeof(RS485_Tx_Buffer));

    /* FIXME: read the stored baud rate */
    /* I2C_Read_Block(
       EEPROM_DEVICE_ADDRESS,
       (char *)&RS485_Baud_Rate,
       sizeof(RS485_Baud_Rate),
       EEPROM_MSTP_BAUD_RATE_ADDR); */

    RS485_Initialize_Port();
}
