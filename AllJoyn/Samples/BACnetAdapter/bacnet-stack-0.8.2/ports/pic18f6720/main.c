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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>     /* for memmove */
#include <p18f6720.h>
#include <stdlib.h>
#include <string.h>
#include "stdint.h"
#include "hardware.h"
/* BACnet */
#include "apdu.h"
#include "datalink.h"
#include "dcc.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#include "rs485.h"

/* chip configuration data */
/* define this to enable ICD */
/* #define USE_ICD */

/* Configuration Bits  */
#pragma config OSC = HS, OSCS = OFF
#pragma config PWRT = ON
#pragma config BOR = ON, BORV = 27
#pragma config CCP2MUX = ON
#pragma config STVR = ON
#pragma config LVP = OFF
#pragma config CP0 = OFF
#pragma config CP1 = OFF
#pragma config CP2 = OFF
#pragma config CP3 = OFF
#pragma config CP4 = OFF
#pragma config CP5 = OFF
#pragma config CP6 = OFF
#pragma config CP7 = OFF
#pragma config CPB = OFF
#pragma config CPD = OFF
#pragma config WRT0 = OFF
#pragma config WRT1 = OFF
#pragma config WRT2 = OFF
#pragma config WRT3 = OFF
#pragma config WRT4 = OFF
#pragma config WRT5 = OFF
#pragma config WRT6 = OFF
#pragma config WRT7 = OFF
#pragma config WRTB = OFF
#pragma config WRTC = OFF
#pragma config WRTD = OFF
#pragma config EBTR0 = OFF
#pragma config EBTR1 = OFF
#pragma config EBTR2 = OFF
#pragma config EBTR3 = OFF
#pragma config EBTR4 = OFF
#pragma config EBTR5 = OFF
#pragma config EBTR6 = OFF
#pragma config EBTR7 = OFF
#pragma config EBTRB = OFF

#ifdef USE_ICD
#pragma config WDT = OFF, WDTPS = 128
#pragma config DEBUG = ON
#else
#pragma config WDT = ON, WDTPS = 128
#pragma config DEBUG = OFF
#endif /* USE_ICD */

volatile uint8_t Milliseconds = 0;
volatile uint8_t Zero_Cross_Timeout = 0;

void Reinitialize(
    void)
{
    uint8_t i;
    char name = 0;

    _asm reset _endasm return;
}

void Global_Int(
    enum INT_STATE state)
{
    static uint8_t intstate = 0;

    switch (state) {
        case INT_DISABLED:
            intstate >>= 2;
            intstate |= (INTCON & 0xC0);
            break;
        case INT_ENABLED:
            INTCONbits.GIE = 1;
            INTCONbits.PEIE = 1;
            intstate <<= 2;
            break;
        case INT_RESTORE:
            INTCON |= (intstate & 0xC0);
            intstate <<= 2;
            break;
        default:
            break;
    }
}

void Hardware_Initialize(
    void)
{
    /* PORTA.0 Input - Photocell PORTA.1 Output - LED Row6 PORTA.2 Output
     * - LED Row5 PORTA.3 Output - LED Row4 PORTA.4 Input - Square Wave
     * input from RTC PORTA.5 Output - LCD RW */
    TRISA = 0xD1;

    /* PORTB.0 Input - Zero Cross PORTB.1 Input - USB RXF# PORTB.2 Input
     * USB TXE# PORTB.3 Output - Keypad Row Enable (74HC373 Output Control)
     * PORTB.4 Output Keypad Row Gate (74HC373 Gate) PORTB.5 Output Switch
     * Input Latch & Keypad Column Gate (74HC373 Gate) PORTB.6 Input - ICD
     * connection PORTB.7 Input - ICD connection */
    TRISB = 0xC7;

    /* PORTC.0 Output - Pilot Latch PORTC.1 Output - Pilot Output Enable
     * (low) PORTC.2 I/O - Piezo PORTC.3 Input - I2C clock PORTC.4 Input
     * I2C data PORTC.5 Output RS232 enable (low) PORTC.6 Output - RS232 Tx
     * PORTC.7 Input - RS232 Rx */
    TRISC = 0x9C;

    /* PORTD.0 I/O - Data bus PORTD.1 I/O - Data bus PORTD.2 I/O - Data
     * bus PORTD.3 I/O - Data bus PORTD.4 I/O - Data bus PORTD.5 I/O - Data
     * bus PORTD.6 I/O - Data bus PORTD.7 I/O - Data bus */
    TRISD = 0xFF;

    /* PORTE.0 Input - USB RD PORTE.1 Input - USB WR PORTE.2 Output - LCD
     * RS PORTE.3 Output - 485 transmit enable PORTE.4 Output - Relay data
     * latch PORTE.5 Output Switch Input Clock PORTE.6 Output - Switch
     * Input High/Low PORTE.7 Input Switch Input Data */
    TRISE = 0x83;

    /* PORTF.0 Output - LED Row2 PORTF.1 Output - LED Row1 PORTF.2 Output
     * - LED Col5 PORTF.3 Output - LED Col4 PORTF.4 Output - LED Col3
     * PORTF.5 Output - LED Col2 PORTF.6 Output - LED Col1 PORTF.7 Output
     * LED Col0 */
    TRISF = 0x00;

    /* PORTG.0 Output - 485 receive enable PORTG.1 Output - 485 Tx PORTG.2
     * Input 485 Rx PORTG.3 Output - LCD E PORTG.4 Output - LED Row0 */
    TRISG = 0xE6;

    /* The initial state of the keypad enables and latches */
    KEYPAD_ROW_ENABLE = 1;
    KEYPAD_ROW_LATCH = 0;
    KEYPAD_COL_LATCH = 1;

    RELAY_LATCH = 0;

    /* Setup to read the switch inputs */
    SWITCH_COM = 1;

    /* Enable the RS232 transmitter */
    RS232_ENABLE = 0;

    /* Turn all leds off. These are the hardware pins */
    LED_ROW1 = 1;
    LED_ROW2 = 1;
    LED_ROW3 = 1;
    LED_ROW4 = 1;
    LED_ROW5 = 1;
    LED_ROW6 = 1;
    LEDPORT = 0x03;

    /* The initial values for the signals to the LCD */
    LCD_E = 1;
    LCD_RW = 1;
    LCD_RS = 1;

    /* The following gives us a PWM frequency of 1.990KHz with a 50% duty
     * cycle It also serves to multiplex the LEDs. */
    PIEZO_OFF();
    CCPR1L = 0x4E;
    CCP1CON = 0x2F;
    setup_timer2(6, 156, 2);
    PIE1bits.TMR2IE = 1;

    /* We will use Timer4 as our system tick timer. Our system tick is set
     * to 1ms. Hold off on enabling the int. */
    setup_timer4(5, 250, 5);

    /* Setup our interrupt priorities */
    RCONbits.IPEN = 1;
    IPR1 = 0;
    IPR2 = 0;
    IPR3 = 0;

    /* Setup TMR0 to be high priority */
    INTCON2 = 0xFC;
    INTCON3 = 0;

    /* USART 1 high priority */
    IPR1bits.RC1IP = 1;
    IPR1bits.TX1IP = 1;

    /* Finally enable our ints */
    Global_Int(INT_ENABLED);
}

void Initialize_Variables(
    void)
{
    /* Check to see if we need to initialize our eeproms */
    ENABLE_TIMER4_INT();
    /* interrupts must be enabled before we read our inputs */
    Global_Int(INT_ENABLED);
    /* Start our time from now */
    Milliseconds = 0;
}

void MainTasks(
    void)
{
    static uint16_t millisecond_counter = 0;
    /* Handle our millisecond counters */
    while (Milliseconds) {
        millisecond_counter++;
        --Milliseconds;
    }
    /* Handle our seconds counters */
    if (millisecond_counter > 1000) {
        millisecond_counter -= 1000;
        dcc_timer_seconds(1);
    }
}

void main(
    void)
{
    RCONbits.NOT_POR = 1;
    RCONbits.NOT_RI = 1;
    Hardware_Initialize();
    Initialize_Variables();
    /* initialize BACnet Data Link Layer */
    dlmstp_set_my_address(42);
    dlmstp_set_max_info_frames(1);
    dlmstp_set_max_master(127);
    RS485_Set_Baud_Rate(38400);
    dlmstp_init();
    /* Handle anything that needs to be done on powerup */
    /* Greet the BACnet world! */
    Send_I_Am(&Handler_Transmit_Buffer[0]);
    /* Main loop */
    while (TRUE) {
        RESTART_WDT();
        dlmstp_task();
        MainTasks();
        Global_Int(INT_ENABLED);
        ENABLE_TIMER4_INT();
    }
}
