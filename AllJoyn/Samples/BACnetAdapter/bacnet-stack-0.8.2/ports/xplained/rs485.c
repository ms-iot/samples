/*
 * @file
 *
 * @brief RS-485 Interface
 *
 * Copyright (C) 2013 Steve Karg <skarg@users.sourceforge.net>
 *
 * @page License
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
 */
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "board.h"
#include "usart.h"
#include "ioport.h"
#include "sysclk.h"
#include "fifo.h"
#include "timer.h"
#include "led.h"
#include "mstpdef.h"
/* me! */
#include "rs485.h"

#ifdef CONF_BOARD_ENABLE_RS485_XPLAINED
#define RS485_RE       IOPORT_CREATE_PIN(PORTC, 1)
#define RS485_DE       IOPORT_CREATE_PIN(PORTC, 0)
#define RS485_TXD      IOPORT_CREATE_PIN(PORTC, 3)
#define RS485_RXD      IOPORT_CREATE_PIN(PORTC, 2)
#define RS485_USART    USARTC0
#define RS485_TXC_vect USARTC0_TXC_vect
#define RS485_RXC_vect USARTC0_RXC_vect
#else
#define RS485_RE    IOPORT_CREATE_PIN(PORTE, 0)
#define RS485_DE    IOPORT_CREATE_PIN(PORTE, 0)
#define RS485_TXD   IOPORT_CREATE_PIN(PORTE, 3)
#define RS485_RXD   IOPORT_CREATE_PIN(PORTE, 2)
#define RS485_USART USARTE0
#define RS485_TXC_vect USARTE0_TXC_vect
#define RS485_RXC_vect USARTE0_RXC_vect
#endif

/* buffer for storing received bytes - size must be power of two */
/* BACnet MAX_APDU for MS/TP is 480 bytes */
static uint8_t Receive_Queue_Data[512];
static FIFO_BUFFER Receive_Queue;
/* buffer for storing bytes to transmit - size must be power of two */
/* BACnet MAX_APDU for MS/TP is 480 bytes */
static uint8_t Transmit_Queue_Data[512];
static FIFO_BUFFER Transmit_Queue;
/* baud rate of the UART interface */
static uint32_t Baud_Rate;
/* timer for measuring line silence */
static struct etimer Silence_Timer;
/* flag to track RTS status */
static volatile bool RTS_Status;

/**
 *  Resets the silence timer
 */
void rs485_silence_reset(void)
{
    timer_elapsed_start(&Silence_Timer);
}

/**
 * Determine the amount of silence on the wire from the timer.
 *
 * @param interval - amount of time in milliseconds that line could be silent
 *
 * @return true if the line has been silent for the interval
 */
bool rs485_silence_elapsed(uint32_t interval)
{
    return timer_elapsed_milliseconds(&Silence_Timer, interval);
}

/**
 *  enable the transmit-enable line on the RS-485 transceiver
 *
 * @param enable - true to enable RTS, false to disable RTS
 */
void rs485_rts_enable(bool enable)
{
    if (enable) {
        /* Turn Tx enable on */
        ioport_set_value(RS485_RE, 1);
        ioport_set_value(RS485_DE, 1);
        led_on(LED_RS485_TX);
        RTS_Status = true;
    } else {
        /* Turn Tx enable off */
        ioport_set_value(RS485_RE, 0);
        ioport_set_value(RS485_DE, 0);
        led_off_delay(LED_RS485_TX, 10);
        RTS_Status = false;
    }
}

/**
 *  Determine the status of the transmit-enable line on the RS-485 transceiver
 *
 * @return true if RTS is enabled, false if RTS is disabled
 */
bool rs485_rts_enabled(void)
{
    return RTS_Status;
}

/**
 * Baud rate determines turnaround time.
 * The minimum time after the end of the stop bit of the final octet of a
 * received frame before a node may enable its EIA-485 driver: 40 bit times.
 * At 9600 baud, 40 bit times would be about 4.166 milliseconds
 * At 19200 baud, 40 bit times would be about 2.083 milliseconds
 * At 38400 baud, 40 bit times would be about 1.041 milliseconds
 * At 57600 baud, 40 bit times would be about 0.694 milliseconds
 * At 76800 baud, 40 bit times would be about 0.520 milliseconds
 * At 115200 baud, 40 bit times would be about 0.347 milliseconds
 * 40 bits is 4 octets including a start and stop bit with each octet
 *
 * @return: amount of milliseconds to wait
 */
static uint16_t rs485_turnaround_time(void)
{
    /* delay after reception before transmitting - per MS/TP spec */
    /* wait a minimum  40 bit times since reception */
    /* at least 2 ms for errors: rounding, clock tick */
    return (2 + ((Tturnaround * 1000) / Baud_Rate));
}

/**
 * Use the silence timer to determine turnaround time.
 *
 * @return true if the line has been silent for the turnaround interval
 */
bool rs485_turnaround_elapsed(void)
{
    return timer_elapsed_milliseconds(&Silence_Timer, rs485_turnaround_time());
}

/**
 * Checks for data on the receive UART, and handles errors
 *
 * @param data register to store the byte, if available (can be NULL)
 *
 * @return true if a byte is available
 */
bool rs485_byte_available(uint8_t * data_register)
{
    bool data_available = false;        /* return value */

    if (FIFO_Empty(&Receive_Queue)) {
        led_off_delay(LED_RS485_RX, 2);
    } else {
        led_on(LED_RS485_RX);
        if (data_register) {
            *data_register = FIFO_Get(&Receive_Queue);
        }
        data_available = true;
    }

    return data_available;
}

/**
 * returns an error indication if errors are enabled
 *
 * @return returns true if error is detected and errors are enabled
 */
bool rs485_receive_error(void)
{
    return false;
}

/**
 * Determines if the entire frame is sent from USART FIFO
 *
 * @return true if the USART FIFO is empty
 */
bool rs485_frame_sent(void)
{
    return usart_tx_is_complete(&RS485_USART);
}

/**
*  Transmit one or more bytes on RS-485. Can be called while transmitting to add
*  additional bytes to transmit queue.
*
* @param buffer - array of one or more bytes to transmit
* @param nbytes - number of bytes to transmit
*/
bool rs485_bytes_send(uint8_t * buffer,
    uint16_t nbytes)
{
    bool status = false;
    bool start_required = false;
    uint8_t ch = 0;

    if (buffer && (nbytes > 0)) {
        if (FIFO_Empty(&Transmit_Queue)) {
            start_required = true;
        }
        status = FIFO_Add(&Transmit_Queue, buffer, nbytes);
        if (start_required && status) {
            rs485_rts_enable(true);
            timer_elapsed_start(&Silence_Timer);
            ch = FIFO_Get(&Transmit_Queue);
            usart_clear_tx_complete(&RS485_USART);
            usart_set_tx_interrupt_level(&RS485_USART, USART_INT_LVL_LO);
            usart_putchar(&RS485_USART, ch);
        }
    }

    return status;
}

/**
* RS485 RX interrupt
*/
ISR(RS485_RXC_vect)
{
    unsigned char ch;

    ch = usart_getchar(&RS485_USART);
    FIFO_Put(&Receive_Queue, ch);
    usart_clear_rx_complete(&RS485_USART);
}

/**
* RS485 TX interrupt
*/
ISR(RS485_TXC_vect)
{
    uint8_t ch;

    if (FIFO_Empty(&Transmit_Queue)) {
        /* end of packet */
        usart_set_tx_interrupt_level(&RS485_USART, USART_INT_LVL_OFF);
        rs485_rts_enable(false);
    } else {
        rs485_rts_enable(true);
        ch = FIFO_Get(&Transmit_Queue);
        usart_putchar(&RS485_USART, ch);
    }
}

/**
 * Return the RS-485 baud rate
 *
 * @return baud - RS-485 baud rate in bits per second (bps)
 */
uint32_t rs485_baud_rate(
    void)
{
    return Baud_Rate;
}

/**
 * Initialize the RS-485 baud rate
 *
 * @param baud - RS-485 baud rate in bits per second (bps)
 *
 * @return true if set and valid
 */
bool rs485_baud_rate_set(
    uint32_t baud)
{
    bool valid = true;
    unsigned long frequency;

    switch (baud) {
        case 9600:
        case 19200:
        case 38400:
        case 57600:
        case 76800:
        case 115200:
            frequency = sysclk_get_peripheral_bus_hz(&RS485_USART);
            valid = usart_set_baudrate (&RS485_USART, baud, frequency);
            if (valid) {
                Baud_Rate = baud;
            }
            break;
        default:
            valid = false;
            break;
    }

    return valid;
}


/**
 *  Initialize the RS-485 UART interface, receive interrupts enabled
 */
void rs485_init(void)
{
    usart_rs232_options_t option;

    /* initialize the Rx and Tx byte queues */
    FIFO_Init(&Receive_Queue, &Receive_Queue_Data[0],
        (unsigned) sizeof(Receive_Queue_Data));
    FIFO_Init(&Transmit_Queue, &Transmit_Queue_Data[0],
        (unsigned) sizeof(Transmit_Queue_Data));
    /* initialize the silence timer */
    timer_elapsed_start(&Silence_Timer);
    /* configure the TX pin */
    ioport_configure_pin(RS485_TXD,
        IOPORT_DIR_OUTPUT | IOPORT_INIT_HIGH);
    /* configure the RX pin */
    ioport_configure_pin(RS485_RXD, 
		IOPORT_DIR_INPUT);
    /* configure the RTS pins */
    ioport_configure_pin(RS485_RE,
        IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
    ioport_configure_pin(RS485_DE,
        IOPORT_DIR_OUTPUT | IOPORT_INIT_LOW);
    option.baudrate = Baud_Rate;
    option.charlength = USART_CHSIZE_8BIT_gc;
    option.paritytype = USART_PMODE_DISABLED_gc;
    option.stopbits = false;
    usart_init_rs232(&RS485_USART, &option);
    usart_set_rx_interrupt_level(&RS485_USART, USART_INT_LVL_HI);
}
