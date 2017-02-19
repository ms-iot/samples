/**************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware.h"
#include "fifo.h"
#include "serial.h"

/* baud rate */
static uint32_t Baud_Rate = 9600;

/* buffer for storing received bytes - size must be power of two */
static uint8_t Receive_Buffer_Data[128];
static FIFO_BUFFER Receive_Buffer;

static void serial_receiver_enable(
    void)
{
    UCSR1B = _BV(TXEN1) | _BV(RXEN1) | _BV(RXCIE1);
}

ISR(USART1_RX_vect)
{
    uint8_t data_byte;

    if (BIT_CHECK(UCSR1A, RXC1)) {
        /* data is available */
        data_byte = UDR1;
        (void) FIFO_Put(&Receive_Buffer, data_byte);
    }
}

bool serial_byte_get(
    uint8_t * data_register)
{
    bool data_available = false;        /* return value */

    if (!FIFO_Empty(&Receive_Buffer)) {
        *data_register = FIFO_Get(&Receive_Buffer);
        data_available = true;
    }

    return data_available;
}

bool serial_byte_peek(
    uint8_t * data_register)
{
    bool data_available = false;        /* return value */

    if (!FIFO_Empty(&Receive_Buffer)) {
        *data_register = FIFO_Peek(&Receive_Buffer);
        data_available = true;
    }

    return data_available;
}

void serial_bytes_send(
    uint8_t * buffer,   /* data to send */
    uint16_t nbytes)
{       /* number of bytes of data */
    while (!BIT_CHECK(UCSR1A, UDRE1)) {
        /* do nothing - wait until Tx buffer is empty */
    }
    while (nbytes) {
        /* Send the data byte */
        UDR1 = *buffer;
        while (!BIT_CHECK(UCSR1A, UDRE1)) {
            /* do nothing - wait until Tx buffer is empty */
        }
        buffer++;
        nbytes--;
    }
    /* was the frame sent? */
    while (!BIT_CHECK(UCSR1A, TXC1)) {
        /* do nothing - wait until the entire frame in the
           Transmit Shift Register has been shifted out */
    }
    /* Clear the Transmit Complete flag by writing a one to it. */
    BIT_SET(UCSR1A, TXC1);

    return;
}

void serial_byte_send(
    uint8_t ch)
{
    while (!BIT_CHECK(UCSR1A, UDRE1)) {
        /* do nothing - wait until Tx buffer is empty */
    }
    /* Send the data byte */
    UDR1 = ch;

    return;
}

void serial_byte_transmit_complete(
    void)
{
    /* was the frame sent? */
    while (!BIT_CHECK(UCSR1A, TXC1)) {
        /* do nothing - wait until the entire frame in the
           Transmit Shift Register has been shifted out */
    }
    /* Clear the Transmit Complete flag by writing a one to it. */
    BIT_SET(UCSR1A, TXC1);
}

uint32_t serial_baud_rate(
    void)
{
    return Baud_Rate;
}

bool serial_baud_rate_set(
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
            Baud_Rate = baud;
            /* 2x speed mode */
            BIT_SET(UCSR1A, U2X1);
            /* configure baud rate */
            UBRR1 = (F_CPU / (8UL * Baud_Rate)) - 1;
            /* FIXME: store the baud rate */
            break;
        default:
            valid = false;
            break;
    }

    return valid;
}

static void serial_usart_init(
    void)
{
    /* enable the internal pullup on RXD1 */
    BIT_CLEAR(DDRD, DDD2);
    BIT_SET(PORTD, PD2);
    /* enable Transmit and Receive */
    UCSR1B = _BV(TXEN1) | _BV(RXEN1);
    /* Set USART Control and Status Register n C */
    /* Asynchronous USART 8-bit data, No parity, 1 stop */
    /* Set USART Mode Select: UMSELn1 UMSELn0 = 00 for Asynchronous USART */
    /* Set Parity Mode:  UPMn1 UPMn0 = 00 for Parity Disabled */
    /* Set Stop Bit Select: USBSn = 0 for 1 stop bit */
    /* Set Character Size: UCSZn2 UCSZn1 UCSZn0 = 011 for 8-bit */
    /* Clock Polarity: UCPOLn = 0 when asynchronous mode is used. */
    UCSR1C = _BV(UCSZ11) | _BV(UCSZ10);
    power_usart1_enable();
}

void serial_init(
    void)
{
    FIFO_Init(&Receive_Buffer, &Receive_Buffer_Data[0],
        (unsigned) sizeof(Receive_Buffer_Data));
    serial_usart_init();
    serial_baud_rate_set(Baud_Rate);
    serial_receiver_enable();
}
