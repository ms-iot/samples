/**************************************************************************
*
* Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
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
/*#include "mstp.h" */

/* This file has been customized for use with ATMEGA168 */
#include "hardware.h"
#include "timer.h"

/* Timers for turning off the TX,RX LED indications */
static uint8_t LED1_Off_Timer;
static uint8_t LED3_Off_Timer;

/* baud rate */
static uint32_t RS485_Baud = 9600;

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
    /* enable Transmit and Receive */
    UCSR0B = _BV(TXEN0) | _BV(RXEN0);

    /* Set USART Control and Status Register n C */
    /* Asynchronous USART 8-bit data, No parity, 1 stop */
    /* Set USART Mode Select: UMSELn1 UMSELn0 = 00 for Asynchronous USART */
    /* Set Parity Mode:  UPMn1 UPMn0 = 00 for Parity Disabled */
    /* Set Stop Bit Select: USBSn = 0 for 1 stop bit */
    /* Set Character Size: UCSZn2 UCSZn1 UCSZn0 = 011 for 8-bit */
    /* Clock Polarity: UCPOLn = 0 when asynchronous mode is used. */
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    /* Clear Power Reduction USART0 */
    BIT_CLEAR(PRR, PRUSART0);
    /* Use port PD2 for RTS - enable and disable of Transceiver Tx/Rx */
    /* Set port bit as Output - initially receiving */
    BIT_CLEAR(PORTD, PD2);
    BIT_SET(DDRD, DDD2);
    /* Configure Transmit and Receive LEDs - initially off */
    BIT_SET(PORTD, PD6);
    BIT_SET(PORTD, PD7);
    BIT_SET(DDRD, DDD6);
    BIT_SET(DDRD, DDD7);

    return;
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
* RETURN:      true if valid baud rate
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
            /* 2x speed mode */
            BIT_SET(UCSR0A, U2X0);
            /* configure baud rate */
            UBRR0 = (F_CPU / (8UL * RS485_Baud)) - 1;
            /* FIXME: store the baud rate */
            break;
        default:
            valid = false;
            break;
    }

    return valid;
}

/****************************************************************************
* DESCRIPTION: Enable or disable the transmitter
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Transmitter_Enable(
    bool enable)
{
    if (enable) {
        BIT_SET(PORTD, PD2);
    } else {
        BIT_CLEAR(PORTD, PD2);
    }
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
    uint8_t nbytes = 4;

    RS485_Transmitter_Enable(false);
    while (nbytes) {
        while (!BIT_CHECK(UCSR0A, UDRE0)) {
            /* do nothing - wait until Tx buffer is empty */
        }
        /* Send the data byte */
        UDR0 = 0xff;
        nbytes--;
    }
    /* was the frame sent? */
    while (!BIT_CHECK(UCSR0A, TXC0)) {
        /* do nothing - wait until the entire frame in the
           Transmit Shift Register has been shifted out */
    }
    /* Clear the Transmit Complete flag by writing a one to it. */
    BIT_SET(UCSR0A, TXC0);
}

/****************************************************************************
* DESCRIPTION: Timers for delaying the LED indicators going off
* RETURN:      none
* ALGORITHM:   none
* NOTES:       expected to be called once a millisecond
*****************************************************************************/
void RS485_LED_Timers(
    void)
{
    if (LED1_Off_Timer) {
        LED1_Off_Timer--;
        if (LED1_Off_Timer == 0) {
            BIT_SET(PORTD, PD6);
        }
    }
    if (LED3_Off_Timer) {
        LED3_Off_Timer--;
        if (LED3_Off_Timer == 0) {
            BIT_SET(PORTD, PD7);
        }
    }
}

/****************************************************************************
* DESCRIPTION: Turn on the LED, and set the off timer to turn it off
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
static void RS485_LED1_On(
    void)
{
    BIT_CLEAR(PORTD, PD6);
    LED1_Off_Timer = 20;
}

/****************************************************************************
* DESCRIPTION: Turn on the LED, and set the off timer to turn it off
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
static void RS485_LED3_On(
    void)
{
    BIT_CLEAR(PORTD, PD7);
    LED3_Off_Timer = 20;
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
    RS485_LED3_On();
    while (nbytes) {
        while (!BIT_CHECK(UCSR0A, UDRE0)) {
            /* do nothing - wait until Tx buffer is empty */
        }
        /* Send the data byte */
        UDR0 = *buffer;
        buffer++;
        nbytes--;
    }
    /* was the frame sent? */
    while (!BIT_CHECK(UCSR0A, TXC0)) {
        /* do nothing - wait until the entire frame in the
           Transmit Shift Register has been shifted out */
    }
    /* Clear the Transmit Complete flag by writing a one to it. */
    BIT_SET(UCSR0A, TXC0);
    /* per MSTP spec, sort of */
    Timer_Silence_Reset();
}

/****************************************************************************
* DESCRIPTION: Return true if a framing or overrun error is present
* RETURN:      true if error
* ALGORITHM:   autobaud - if there are a lot of errors, switch baud rate
* NOTES:       Clears any error flags.
*****************************************************************************/
bool RS485_ReceiveError(
    void)
{
    bool ReceiveError = false;
    uint8_t dummy_data;

    /* check for framing error */
#if 0
    if (BIT_CHECK(UCSR0A, FE0)) {
        /* FIXME: how do I clear the error flags? */
        BITMASK_CLEAR(UCSR0A, (_BV(FE0) | _BV(DOR0) | _BV(UPE0)));
        ReceiveError = true;
    }
#endif
    /* check for overrun error */
    if (BIT_CHECK(UCSR0A, DOR0)) {
        /* flush the receive buffer */
        do {
            dummy_data = UDR0;
        } while (BIT_CHECK(UCSR0A, RXC0));
        ReceiveError = true;
    }
    if (ReceiveError) {
        RS485_LED1_On();
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
    uint8_t * data)
{
    bool DataAvailable = false;

    /* check for data */
    if (BIT_CHECK(UCSR0A, RXC0)) {
        *data = UDR0;
        DataAvailable = true;
        RS485_LED1_On();
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
#endif /* TEST_RS485 */
