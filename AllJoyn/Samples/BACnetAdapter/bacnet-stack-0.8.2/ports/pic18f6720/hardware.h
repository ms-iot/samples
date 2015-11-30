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
#ifndef HARDWARE_H
#define HARDWARE_H

#include <p18f6720.h>
#include <stdint.h>
#include <stdbool.h>

/* PORTA.0 Photocell Input PORTA.1 LED Row6 PORTA.2 LED Row5 PORTA.3 LED
 * Row4 PORTA.4 Square Wave input from RTC PORTA.5 LCD RW PORTB.0 Zero
 * Cross PORTB.1 USB RXF# PORTB.2 USB TXE# PORTB.3 Keypad Row Enable
 * (74HC373 Output Control) PORTB.4 Keypad Row Gate (74HC373 Gate)
 * PORTB.5 Switch Input Latch & Keypad Column Gate (74HC373 Gate) PORTB.6
 * ICD connection PORTB.7 ICD connection PORTC.0 Pilot Latch PORTC.1
 * Pilot Output Enable (low) PORTC.2 Piezo PORTC.3 I2C clock PORTC.4 I2C
 * data PORTC.5 RS232 enable (low) PORTC.6 RS232 Tx PORTC.7 RS232 Rx
 * PORTD.0 Data bus PORTD.1 Data bus PORTD.2 Data bus PORTD.3 Data bus
 * PORTD.4 Data bus PORTD.5 Data bus PORTD.6 Data bus PORTD.7 Data bus
 * PORTE.0 USB RD PORTE.1 USB WR PORTE.2 LCD RS PORTE.3 485 transmit
 * enable PORTE.4 Relay data latch PORTE.5 Switch Input Clock PORTE.6
 * Switch Input High/Low PORTE.7 Switch Input Data PORTF.0 LED Row2
 * PORTF.1 LED Row1 PORTF.2 LED Col5 PORTF.3 LED Col4 PORTF.4 LED Col3
 * PORTF.5 LED Col2 PORTF.6 LED Col1 PORTF.7 LED Col0 PORTG.0 485 receive
 * enable PORTG.1 485 Tx PORTG.2 485 Rx PORTG.3 LCD E PORTG.4 LED Row0 */
#define LCD_BUSY                PORTDbits.RD7
#define LCD_E                   PORTGbits.RG3
#define LCD_RW                  PORTAbits.RA5
#define LCD_RS                  PORTEbits.RE2
#define LCD_DATA                PORTD
#define LCD_TRIS                TRISD

#define PILOT_LATCH             PORTCbits.RC0
#define PILOT_ENABLE            PORTCbits.RC1
#define PILOT_PORT              PORTD
#define PILOT_PORT_TRIS         TRISD

#define PIEZO                   PORTCbits.RC2
#define PIEZO_ON()              TRISCbits.TRISC2 = 0
#define PIEZO_OFF()             TRISCbits.TRISC2 = 1

#define RS232_ENABLE            PORTCbits.RC5

#define RS485_TX_ENABLE         PORTEbits.RE3
#define RS485_RX_DISABLE        PORTGbits.RG0

#define SWITCH_LOAD             PORTBbits.RB5
#define SWITCH_CLK              PORTEbits.RE5
#define SWITCH_COM              PORTEbits.RE6
#define SWITCH_DATA             PORTEbits.RE7

#define LEDPORT                 PORTF
#define LEDTRIS                 TRISF
#define LED_ROW1                PORTGbits.RG4
#define LED_ROW2                PORTFbits.RF1
#define LED_ROW3                PORTFbits.RF0
#define LED_ROW4                PORTAbits.RA3
#define LED_ROW5                PORTAbits.RA2
#define LED_ROW6                PORTAbits.RA1

#define RELAY_PORT              PORTD
#define RELAY_PORT_TRIS         TRISD
#define RELAY_LATCH             PORTEbits.RE4

#define KEYPAD_DATA             PORTD
#define KEYPAD_TRIS             TRISD
#define KEYPAD_COL_LATCH        PORTBbits.RB5
#define KEYPAD_ROW_ENABLE       PORTBbits.RB3
#define KEYPAD_ROW_LATCH        PORTBbits.RB4
#define KEYPAD_ROW1             0 b00000001
#define KEYPAD_ROW2             0 b00000010
#define KEYPAD_ROW3             0 b00000100
#define KEYPAD_ROW4             0 b00001000
#define KEYPAD_ROW5             0 b00010000
#define KEYPAD_ROW6             0 b00100000

#define USB_RD_EMPTY            PORTBbits.RB1
#define USB_WR_FULL             PORTBbits.RB2
#define USB_RD                  PORTEbits.RE0
#define USB_WR                  PORTEbits.RE1
#define USB_PORT                PORTD
#define USB_PORT_TRIS           TRISD

#define ZERO_CROSS              PORTBbits.RB0

#define PORT_A_TRIS_MASK        0x11    /* 0b00010001 */
#define PORT_B_TRIS_MASK        0xC7    /* 0b11000111 */
#define PORT_C_TRIS_MASK        0x9C    /* 0b10011100 */
#define PORT_D_TRIS_MASK        0xFF    /* 0b11111111 */
#define PORT_E_TRIS_MASK        0x88    /* 0b10001000 */
#define PORT_F_TRIS_MASK        0x00    /* 0b00000000 */
#define PORT_G_TRIS_MASK        0x04    /* 0b00000100 */

#define TURN_OFF_COMPARATORS()  CMCON = 0x07

#define SHORT_BEEP              50
#define LONG_BEEP               250

#define CLICK()                 Hardware_Sound_Piezo(SHORT_BEEP);
#define BEEP()                  Hardware_Sound_Piezo(LONG_BEEP);

typedef union {
    struct {
        uint8_t:1;
        uint8_t:1;
        uint8_t Thursday:1;
        uint8_t Wednesday:1;
        uint8_t Tuesday:1;
        uint8_t Monday:1;
        uint8_t Program:1;
        uint8_t Run:1;

                uint8_t:1;
                uint8_t:1;
        uint8_t Input1:1;
        uint8_t Input2:1;
        uint8_t Input3:1;
        uint8_t Input4:1;
        uint8_t Input5:1;
        uint8_t Input6:1;

                uint8_t:1;
                uint8_t:1;
                uint8_t:1;
        uint8_t Input7:1;
                uint8_t:1;
                uint8_t:1;
        uint8_t Input8:1;
        uint8_t Photocell:1;

                uint8_t:1;
                uint8_t:1;
                uint8_t:1;
                uint8_t:1;
                uint8_t:1;
                uint8_t:1;
        uint8_t Remote:1;
        uint8_t Relay8:1;

                uint8_t:1;
                uint8_t:1;
                uint8_t:1;
        uint8_t Relay7:1;
        uint8_t Relay6:1;
        uint8_t Relay5:1;
        uint8_t Relay4:1;
        uint8_t Relay3:1;

                uint8_t:1;
                uint8_t:1;
        uint8_t Relay2:1;
        uint8_t Relay1:1;
        uint8_t Holiday:1;
        uint8_t Sunday:1;
        uint8_t Saturday:1;
        uint8_t Friday:1;
    };
    struct {
        uint8_t row1;
        uint8_t row2;
        uint8_t row3;
        uint8_t row4;
        uint8_t row5;
        uint8_t row6;
    };
} LED_REGS;

union SWITCH_REGS {
    struct {
        uint8_t All_On:1;
        uint8_t All_Off:1;
        uint8_t Addr:4;
        uint8_t Pilot_Fault:1;
        uint8_t Master:1;
    };
    uint8_t Sw_Byte;
};

enum INT_STATE { INT_DISABLED, INT_ENABLED, INT_RESTORE };

#define RESTART_WDT()       { _asm CLRWDT _endasm }

/* *************************************************************************
  define ENABLE_GLOBAL_INT() INTCONbits.GIE = 1 £
  #define DISABLE_GLOBAL_INT() INTCONbits.GIE = 0 £
  #define ENABLE_PERIPHERAL_INT() INTCONbits.PEIE = 1 £
  #define DISABLE_PERIPHERAL_INT() INTCONbits.PEIE = 0
 *************************************************************************** */
#define ENABLE_HIGH_INT()     INTCONbits.GIE = 1
#define DISABLE_HIGH_INT()    INTCONbits.GIE = 0

#define ENABLE_LOW_INT()      INTCONbits.PEIE = 1
#define DISABLE_LOW_INT()     INTCONbits.PEIE = 0

#define ENABLE_TIMER0_INT()   INTCONbits.TMR0IE = 1
#define DISABLE_TIMER0_INT()  INTCONbits.TMR0IE = 0

#define ENABLE_TIMER2_INT()   PIE1bits.TMR2IE = 1
#define DISABLE_TIMER2_INT()  PIE1bits.TMR2IE = 0

#define ENABLE_TIMER4_INT()   PIE3bits.TMR4IE = 1
#define DISABLE_TIMER4_INT()  PIE3bits.TMR4IE = 0

#define ENABLE_CCP2_INT()     PIE2bits.CCP2IE = 1
#define DISABLE_CCP2_INT()    PIE2bits.CCP2IE = 0

#define ENABLE_CCP1_INT()     PIE1bits.CCP1IE = 1
#define DISABLE_CCP1_INT()    PIE1bits.CCP1IE = 0

#define ENABLE_ABUS_INT()     PIE1bits.SSPIE = 1
#define DISABLE_ABUS_INT()    PIE1bits.SSPIE = 0
#define CLEAR_ABUS_FLAG()     PIR1bits.SSPIF = 0

#define SETUP_CCP1(x)         CCP1CON = x
#define SETUP_CCP2(x)         CCP2CON = x

#define DISABLE_RX_INT()      PIE1bits.RCIE = 0
#define ENABLE_RX_INT()       PIE1bits.RCIE = 1

#define DISABLE_TX_INT()      PIE1bits.TXIE = 0
#define ENABLE_TX_INT()       PIE1bits.TXIE = 1

#if CLOCKSPEED == 20
#define DELAY_US(x)           { _asm                 \
                                  MOVLW x            \
                                LOOP:                \
                                  NOP                \
                                  NOP                \
                                  DECFSZ WREG, 1, 0  \
                                  BRA LOOP           \
                                _endasm }
#endif

#define setup_timer4(mode, period, postscale) \
  T4CON = (mode | (postscale - 1) << 3); \
  PR4 = period

#define setup_timer2(mode, period, postscale) \
  T2CON = (mode | (postscale - 1) << 3); \
  PR2 = period


/* Global Vars */
extern volatile LED_REGS LEDS;
extern volatile LED_REGS Blink;
extern uint8_t Piezo_Timer;
extern volatile bool DataPortLocked;

#endif /* HARDWARE_H */
