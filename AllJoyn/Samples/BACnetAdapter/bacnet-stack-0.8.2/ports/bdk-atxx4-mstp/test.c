/**************************************************************************
*
* Copyright (C) 2010 Steve Karg <skarg@users.sourceforge.net>
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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "hardware.h"
#include "timer.h"
#include "serial.h"
#include "input.h"
#include "bo.h"
#include "rs485.h"
#include "dlmstp.h"
#include "seeprom.h"
#include "nvdata.h"
/* me */
#include "test.h"

#ifndef BDK_VERSION
#define BDK_VERSION 4
#endif

#define BINARY_STRING_MAX 3
const char * const binary_string[BINARY_STRING_MAX] = {
    "INACTIVE", "ACTIVE", "RELINQUISH"
};

/* timer for test task */
static struct itimer Test_Timer;
/* MAC Address of MS/TP */
static uint8_t MSTP_MAC_Address;

void test_init(
    void)
{
#ifdef MSTP_MONITOR
    serial_baud_rate_set(115200);
#else
    serial_baud_rate_set(9600);
#endif
    timer_interval_start_seconds(&Test_Timer, 1);
    /* configure a port pin as output */
#if (BDK_VERSION==4)
    BIT_SET(DDRD, DDB5);
#else
    BIT_SET(DDRB, DDB0);
#endif
}

/**
* @brief send a string of text to the RS-232 port
* @param text - C string of text (null terminated)
*/
static void test_write_string(const char * text)
{
    size_t len = 0;

    if (text) {
        len = strlen(text);
        serial_bytes_send((uint8_t *)text, len);
    }
}


/*************************************************************************
* Description: Turn on a pin
* Returns: none
* Notes: none
*************************************************************************/
static inline void test_pin_on(
    void)
{
#if (BDK_VERSION==4)
    BIT_SET(PORTD, PD5);
#else
    BIT_SET(PORTB, PB0);
#endif
}

/*************************************************************************
* Description: Turn off a pin
* Returns: none
* Notes: none
*************************************************************************/
static inline void test_pin_off(
    void)
{
#if (BDK_VERSION==4)
    BIT_CLEAR(PORTD, PD5);
#else
    BIT_CLEAR(PORTB, PB0);
#endif
}

/*************************************************************************
* Description: Get the state of the test pin
* Returns: true if on, false if off.
* Notes: none
*************************************************************************/
static inline bool test_pin_state(
    void)
{
#if (BDK_VERSION==4)
    return (BIT_CHECK(PIND, PD5));
#else
    return (BIT_CHECK(PINB, PB0));
#endif
}

/*************************************************************************
* Description: Toggle the test pin
* Returns: none
* Notes: none
*************************************************************************/
static inline void test_pin_toggle(
    void)
{
    if (test_pin_state()) {
        test_pin_off();
    } else {
        test_pin_on();
    }
}

#ifdef MSTP_MONITOR
void test_task(
    void)
{
    if (timer_interval_expired(&Test_Timer)) {
        timer_interval_reset(&Test_Timer);
        MSTP_MAC_Address = MSTP_MAC_Address;
    }
}
#else
char Send_Buffer[32];

void test_task(
    void)
{
    uint8_t data_register = 0;
    uint16_t id = 0;

    if (timer_interval_expired(&Test_Timer)) {
        timer_interval_reset(&Test_Timer);
        sprintf(Send_Buffer, "BACnet: 0000000\r\n");
        MSTP_MAC_Address = input_address();
        Send_Buffer[8] = (MSTP_MAC_Address & BIT0) ? '1' : '0';
        Send_Buffer[9] = (MSTP_MAC_Address & BIT1) ? '1' : '0';
        Send_Buffer[10] = (MSTP_MAC_Address & BIT2) ? '1' : '0';
        Send_Buffer[11] = (MSTP_MAC_Address & BIT3) ? '1' : '0';
        Send_Buffer[12] = (MSTP_MAC_Address & BIT4) ? '1' : '0';
        Send_Buffer[13] = (MSTP_MAC_Address & BIT5) ? '1' : '0';
        Send_Buffer[14] = (MSTP_MAC_Address & BIT6) ? '1' : '0';
        serial_bytes_send((uint8_t *) Send_Buffer, 17);
    }
    if (serial_byte_get(&data_register)) {
        /* echo the character */
        serial_byte_send(data_register);
        switch (data_register) {
            case '0':
                Binary_Output_Present_Value_Set(0, BINARY_INACTIVE, 0);
                Binary_Output_Present_Value_Set(1, BINARY_INACTIVE, 0);
                test_write_string(binary_string[0]);
                break;
            case '1':
                Binary_Output_Present_Value_Set(0, BINARY_ACTIVE, 0);
                Binary_Output_Present_Value_Set(1, BINARY_ACTIVE, 0);
                test_write_string(binary_string[1]);
                break;
            case '2':
                Binary_Output_Present_Value_Set(0, BINARY_NULL, 0);
                Binary_Output_Present_Value_Set(1, BINARY_NULL, 0);
                test_write_string(binary_string[2]);
                break;
            case '3':
                rs485_baud_rate_set(38400);
                break;
            case '5':
                rs485_baud_rate_set(57600);
                break;
            case '7':
                rs485_baud_rate_set(76800);
                break;
            case '9':
                rs485_baud_rate_set(9600);
                break;
            case 'e':
                seeprom_bytes_read(NV_SEEPROM_TYPE_0, (uint8_t *) & id, 2);
                sprintf(Send_Buffer, "\r\n%04X", id);
                serial_bytes_send((uint8_t *) Send_Buffer,
                    strlen(Send_Buffer));
                break;
            case 'b':
                sprintf(Send_Buffer, "\r\n%lubps",
                    (unsigned long) rs485_baud_rate());
                serial_bytes_send((uint8_t *) Send_Buffer,
                    strlen(Send_Buffer));
                break;
            case 'm':
                sprintf(Send_Buffer, "\r\nMax:%u",
                    (unsigned) dlmstp_max_master());
                serial_bytes_send((uint8_t *) Send_Buffer,
                    strlen(Send_Buffer));
                break;
            default:
                break;
        }
        serial_byte_send('\r');
        serial_byte_send('\n');
        serial_byte_transmit_complete();
    }
    test_pin_toggle();
}
#endif
