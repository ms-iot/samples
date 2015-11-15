/**************************************************************************
*
* Copyright (C) 2011 Steve Karg <skarg@users.sourceforge.net>
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
* Module Description:
* Handle the configuration and operation of the RS485 bus.
**************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware.h"
#include "timer.h"
#include "bits.h"
#include "fifo.h"
#include "led.h"
#include "rs485.h"

/* buffer for storing received bytes - size must be power of two */
static uint8_t Receive_Buffer_Data[512];
static FIFO_BUFFER Receive_Buffer;
/* amount of silence on the wire */
static struct etimer Silence_Timer;
/* baud rate */
static uint32_t Baud_Rate = 38400;

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

/*************************************************************************
* Description: Reset the silence on the wire timer.
* Returns: nothing
* Notes: none
**************************************************************************/
void rs485_silence_reset(
    void)
{
    timer_elapsed_start(&Silence_Timer);
}

/*************************************************************************
* Description: Determine the amount of silence on the wire from the timer.
* Returns: true if the amount of time has elapsed
* Notes: none
**************************************************************************/
bool rs485_silence_elapsed(
    uint32_t interval)
{
    return timer_elapsed_milliseconds(&Silence_Timer, interval);
}

/*************************************************************************
* Description: Baud rate determines turnaround time.
* Returns: amount of milliseconds
* Notes: none
**************************************************************************/
static uint16_t rs485_turnaround_time(
    void)
{
    /* delay after reception before transmitting - per MS/TP spec */
    /* wait a minimum  40 bit times since reception */
    /* at least 2 ms for errors: rounding, clock tick */
    return (2 + ((Tturnaround * 1000UL) / Baud_Rate));
}

/*************************************************************************
* Description: Use the silence timer to determine turnaround time.
* Returns: true if turnaround time has expired.
* Notes: none
**************************************************************************/
bool rs485_turnaround_elapsed(
    void)
{
    return timer_elapsed_milliseconds(&Silence_Timer, rs485_turnaround_time());
}


/*************************************************************************
* Description: Determines if an error occured while receiving
* Returns: true an error occurred.
* Notes: none
**************************************************************************/
bool rs485_receive_error(
    void)
{
    return false;
}

/*********************************************************************//**
 * @brief        USARTx interrupt handler sub-routine
 * @param[in]    None
 * @return         None
 **********************************************************************/
void USART2_IRQHandler(
    void)
{
    uint8_t data_byte;


    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        /* Read one byte from the receive data register */
        data_byte = USART_ReceiveData(USART2);
        (void) FIFO_Put(&Receive_Buffer, data_byte);
    }
}

/*************************************************************************
* DESCRIPTION: Return true if a byte is available
* RETURN:      true if a byte is available, with the byte in the parameter
* NOTES:       none
**************************************************************************/
bool rs485_byte_available(
    uint8_t * data_register)
{
    bool data_available = false;        /* return value */

    if (!FIFO_Empty(&Receive_Buffer)) {
        if (data_register) {
            *data_register = FIFO_Get(&Receive_Buffer);
        }
        timer_elapsed_start(&Silence_Timer);
        data_available = true;
        led_rx_on_interval(10);
    }

    return data_available;
}

/*************************************************************************
* DESCRIPTION: Sends a byte of data
* RETURN:      nothing
* NOTES:       none
**************************************************************************/
void rs485_byte_send(
    uint8_t tx_byte)
{
    led_tx_on_interval(10);
    USART_SendData(USART2, tx_byte);
    timer_elapsed_start(&Silence_Timer);
}

/*************************************************************************
* Description: Determines if a byte in the USART has been shifted from
*   register
* Returns: true if the USART register is empty
* Notes: none
**************************************************************************/
bool rs485_byte_sent(
    void)
{
    return USART_GetFlagStatus(USART2, USART_FLAG_TXE);
}

/*************************************************************************
* Description: Determines if the entire frame is sent from USART FIFO
* Returns: true if the USART FIFO is empty
* Notes: none
**************************************************************************/
bool rs485_frame_sent(
    void)
{
    return USART_GetFlagStatus(USART2, USART_FLAG_TC);
}

/*************************************************************************
* DESCRIPTION: Send some data and wait until it is sent
* RETURN:      true if a collision or timeout occurred
* NOTES:       none
**************************************************************************/
void rs485_bytes_send(
    uint8_t * buffer,   /* data to send */
    uint16_t nbytes)
{       /* number of bytes of data */
    uint8_t tx_byte;

    while (nbytes) {
        /* Send the data byte */
        tx_byte = *buffer;
        /* Send one byte */
        USART_SendData(USART2, tx_byte);
        while (!rs485_byte_sent()) {
            /* do nothing - wait until Tx buffer is empty */
        }
        buffer++;
        nbytes--;
    }
    /* was the frame sent? */
    while (!rs485_frame_sent()) {
        /* do nothing - wait until the entire frame in the
           Transmit Shift Register has been shifted out */
    }
    timer_elapsed_start(&Silence_Timer);

    return;
}

/*************************************************************************
* Description: Configures the baud rate of the USART
* Returns: nothing
* Notes: none
**************************************************************************/
static void rs485_baud_rate_configure(
    void)
{
    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = Baud_Rate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Configure USARTx */
    USART_Init(USART2, &USART_InitStructure);
}

/*************************************************************************
* Description: Sets the baud rate to non-volatile storeage and configures USART
* Returns: true if a value baud rate was saved
* Notes: none
**************************************************************************/
bool rs485_baud_rate_set(
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
            rs485_baud_rate_configure();
            break;
        default:
            valid = false;
            break;
    }

    return valid;
}

/*************************************************************************
* Description: Determines the baud rate in bps
* Returns: baud rate in bps
* Notes: none
**************************************************************************/
uint32_t rs485_baud_rate(
    void)
{
    return Baud_Rate;
}

/*************************************************************************
* Description: Enable the Request To Send (RTS) aka Transmit Enable pin
* Returns: nothing
* Notes: none
**************************************************************************/
void rs485_rts_enable(
    bool enable)
{
    if (enable) {
        GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);
    } else {
        GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);
    }
}

/*************************************************************************
* Description: Initialize the room network USART
* Returns: nothing
* Notes: none
**************************************************************************/
void rs485_init(
    void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);
    /* Configure USARTx Rx as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Configure USARTx Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Configure the Request To Send (RTS) aka Transmit Enable pin */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Enable USARTx Clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    /*RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);*/
    /* Enable GPIO Clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* Enable the USART Pins Software Remapping for this pair
    of pins and peripheral functions:
    USART3 Full remap (TX/PD8, RX/PD9, CK/PD10, CTS/PD11, RTS/PD12) */
    /*GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);*/
    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
    /* Enable the USARTx Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* enable the USART to generate interrupts */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    rs485_baud_rate_set(Baud_Rate);

    USART_Cmd(USART2, ENABLE);

    FIFO_Init(&Receive_Buffer, &Receive_Buffer_Data[0],
        (unsigned) sizeof(Receive_Buffer_Data));
    timer_elapsed_start(&Silence_Timer);
}
