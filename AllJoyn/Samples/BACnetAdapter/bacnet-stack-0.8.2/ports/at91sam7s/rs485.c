/**************************************************************************
*
* Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
* RS-485 initialization on AT91SAM7S inspired by Keil Eletronik serial.c
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
#include <stdlib.h>
#include <stdio.h>
#include "timer.h"

/* This file has been customized for use with UART0
   on the AT91SAM7S-EK */
#include "board.h"

/* UART */
static volatile AT91S_USART *RS485_Interface = AT91C_BASE_US0;
/* baud rate */
static int RS485_Baud = 38400;

/* The minimum time after the end of the stop bit of the final octet of a */
/* received frame before a node may enable its EIA-485 driver: 40 bit times. */
/* At 9600 baud, 40 bit times would be about 4.166 milliseconds */
/* At 19200 baud, 40 bit times would be about 2.083 milliseconds */
/* At 38400 baud, 40 bit times would be about 1.041 milliseconds */
/* At 57600 baud, 40 bit times would be about 0.694 milliseconds */
/* At 76800 baud, 40 bit times would be about 0.520 milliseconds */
/* At 115200 baud, 40 bit times would be about 0.347 milliseconds */
/* 40 bits is 4 octets including a start and stop bit with each octet */
#define Tturnaround  (40UL)
/* turnaround_time_milliseconds = (Tturnaround*1000UL)/RS485_Baud; */

/****************************************************************************
* DESCRIPTION: Initializes the RS485 hardware and variables, and starts in
*              receive mode.
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Initialize(
    void)
{
    unsigned int pcsr;
    /* Enable the USART0 clock in the Power Management Controller */
    volatile AT91PS_PMC pPMC = AT91C_BASE_PMC;

    pcsr = pPMC->PMC_PCSR;
    pPMC->PMC_PCER = pcsr | (1 << AT91C_ID_US0);

    /* Disable and clear USART0 interrupt
       in AIC Interrupt Disable Command Register */
    volatile AT91PS_AIC pAIC = AT91C_BASE_AIC;
    pAIC->AIC_IDCR = (1 << AT91C_ID_US0);
    pAIC->AIC_ICCR = (1 << AT91C_ID_US0);

    /* enable the peripheral by disabling the pin in the PIO controller */
    *AT91C_PIOA_PDR = AT91C_PA5_RXD0 | AT91C_PA6_TXD0 | AT91C_PA7_RTS0;

    RS485_Interface->US_CR = AT91C_US_RSTRX |   /* Reset Receiver      */
        AT91C_US_RSTTX |        /* Reset Transmitter   */
        AT91C_US_RSTSTA |       /* Clear status register */
        AT91C_US_RXDIS |        /* Receiver Disable    */
        AT91C_US_TXDIS; /* Transmitter Disable */

    RS485_Interface->US_MR = AT91C_US_USMODE_RS485 |    /* RS-485 Mode - RTS auto assert */
        AT91C_US_CLKS_CLOCK |   /* Clock = MCK */
        AT91C_US_CHRL_8_BITS |  /* 8-bit Data  */
        AT91C_US_PAR_NONE |     /* No Parity   */
        AT91C_US_NBSTOP_1_BIT;  /* 1 Stop Bit  */

    /* set the Time Guard to release RTS after x bit times */
    RS485_Interface->US_TTGR = 1;

    /* Receiver Time-out disabled */
    RS485_Interface->US_RTOR = 0;

    /* baud rate */
    RS485_Interface->US_BRGR = MCK / 16 / RS485_Baud;

    RS485_Interface->US_CR = AT91C_US_RXEN |    /* Receiver Enable     */
        AT91C_US_TXEN;  /* Transmitter Enable  */

    return;
}

void RS485_Cleanup(
    void)
{

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
    return RS485_Baud;
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
            RS485_Baud = baud;
            RS485_Interface->US_BRGR = MCK / 16 / baud;
            /* FIXME: store the baud rate */
            break;
        default:
            valid = false;
            break;
    }

    return valid;
}

/****************************************************************************
* DESCRIPTION: Waits on the SilenceTimer for 40 bits.
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Turnaround_Delay(
    void)
{
    uint16_t turnaround_time;

    /* delay after reception before trasmitting - per MS/TP spec */
    /* wait a minimum  40 bit times since reception */
    /* at least 1 ms for errors: rounding, clock tick */
    turnaround_time = 2 + ((Tturnaround * 1000UL) / RS485_Baud);
    while (Timer_Silence() < turnaround_time) {
        /* do nothing - wait for timer to increment */
    };
}

/****************************************************************************
* DESCRIPTION: Enable or disable the transmitter
* RETURN:      none
* ALGORITHM:   none
* NOTES:       The Atmel ARM7 has an automatic enable/disable in RS485 mode.
*****************************************************************************/
void RS485_Transmitter_Enable(
    bool enable)
{
    (void) enable;
}

/****************************************************************************
* DESCRIPTION: Send some data and wait until it is sent
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Send_Data(
    uint8_t * buffer,   /* data to send */
    uint16_t nbytes)
{       /* number of bytes of data */
    /* LED on send */
    volatile AT91PS_PIO pPIO = AT91C_BASE_PIOA;
    /* LED ON */
    pPIO->PIO_CODR = LED1;
    /* send all the bytes */
    while (nbytes) {
        while (!(RS485_Interface->US_CSR & AT91C_US_TXRDY)) {
            /* do nothing - wait until Tx buffer is empty */
        }
        RS485_Interface->US_THR = *buffer;
        buffer++;
        nbytes--;
    }
    while (!(RS485_Interface->US_CSR & AT91C_US_TXRDY)) {
        /* do nothing - wait until Tx buffer is empty */
    }
    /* per MSTP spec */
    Timer_Silence_Reset();
}

/****************************************************************************
* DESCRIPTION: Return true if a framing or overrun error is present
* RETURN:      true if error
* ALGORITHM:   none
* NOTES:       Clears any error flags.
*****************************************************************************/
bool RS485_ReceiveError(
    void)
{
    bool ReceiveError = false;
    /* LED on send */
    volatile AT91PS_PIO pPIO = AT91C_BASE_PIOA;

    /* check for data or error */
    if (RS485_Interface->US_CSR & (AT91C_US_OVRE | AT91C_US_FRAME)) {
        /* clear the error flag */
        RS485_Interface->US_CR = AT91C_US_RSTSTA;
        ReceiveError = true;
        /* LED ON */
        pPIO->PIO_CODR = LED2;
    }

    return ReceiveError;
}

/****************************************************************************
* DESCRIPTION: Return true if data is available
* RETURN:      true if data is available, with the data in the parameter set
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
bool RS485_DataAvailable(
    uint8_t * DataRegister)
{
    bool DataAvailable = false;
    /* LED on send */
    volatile AT91PS_PIO pPIO = AT91C_BASE_PIOA;

    if (RS485_Interface->US_CSR & AT91C_US_RXRDY) {
        /* data is available */
        if (DataRegister) {
            *DataRegister = RS485_Interface->US_RHR;
        }
        DataAvailable = true;
        /* LED ON */
        pPIO->PIO_CODR = LED2;
    }

    return DataAvailable;
}

#ifdef TEST_RS485
int main(
    void)
{
    unsigned i = 0;
    uint8_t DataRegister;

    RS485_Set_Baud_Rate(38400);
    RS485_Initialize();
    /* receive task */
    for (;;) {
        if (RS485_ReceiveError()) {
            fprintf(stderr, "ERROR ");
        } else if (RS485_DataAvailable(&DataRegister)) {
            fprintf(stderr, "%02X ", DataRegister);
        }
    }
}
#endif
