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
#include <p18f6720.h>
#include "stdint.h"
#include "hardware.h"
#include "rs485.h"
#include "dlmstp.h"

/* from main.c */
extern volatile uint8_t Milliseconds;

void InterruptHandlerHigh(
    void);
void InterruptHandlerLow(
    void);
void Interrupt_Timer2(
    void);
void Interrupt_Timer3(
    void);
void Interrupt_Timer4(
    void);
void Interrupt_USART_Rx(
    void);
void Interrupt_USART_Tx(
    void);
void Interrupt_CCP2(
    void);
void INT0_Interrupt(
    void);

#pragma code InterruptVectorHigh = 0x08
void InterruptVectorHigh(
    void)
{
    /* jump to interrupt routine */
_asm goto InterruptHandlerHigh _endasm}
#pragma code
#pragma code InterruptVectorLow = 0x18
void InterruptVectorLow(
    void)
{
    /* jump to interrupt routine */
_asm goto InterruptHandlerLow _endasm}
#pragma code
#pragma interrupt InterruptHandlerHigh
void InterruptHandlerHigh(
    void)
{
#if 0
    /* check for USART Rx int */
    if ((PIR1bits.RCIF) && (PIE1bits.RCIE)) {
        if ((RCSTA1bits.FERR) || (RCSTA1bits.OERR)) {
            Comstat.Rx_Bufferoverrun = TRUE;
            PIE1bits.RC1IE = 0; /* Disable Interrupt on receipt */
        } else if (Comstat.Rx_Bytes++ < RX_BUFFER_SIZE - 1) {
            Rx_Buffer[Comstat.RxHead++] = RCREG1;

            /* Stick a Null on the end to let us use str functions on our
             * buffer */
            Rx_Buffer[Comstat.RxHead] = 0;
        } else {
            Comstat.Rx_Bufferoverrun = TRUE;
            PIE1bits.RC1IE = 0; /* Disable Interrupt on receipt */
        }
    }
#endif

    /* check for timer0 int */
    if ((INTCONbits.TMR0IF) && (INTCONbits.TMR0IE)) {
        INTCONbits.TMR0IF = 0;
    }
}

#pragma interruptlow InterruptHandlerLow save = PROD, section(".tmpdata"), TABLAT, TBLPTR, section \
    ("MATH_DATA")

void InterruptHandlerLow(
    void)
{
    /* check for timer2 int */
    if ((PIR1bits.TMR2IF) && (PIE1bits.TMR2IE)) {
        PIR1bits.TMR2IF = 0;
        Interrupt_Timer2();
    }

    /* check for timer3 int */
    if ((PIR2bits.TMR3IF) && (PIE2bits.TMR3IE)) {
        PIR2bits.TMR3IF = 0;
        Interrupt_Timer3();
    }

    /* check for timer4 int */
    if ((PIR3bits.TMR4IF) && (PIE3bits.TMR4IE)) {
        PIR3bits.TMR4IF = 0;
        dlmstp_millisecond_timer();
        Interrupt_Timer4();
    }

    /* check for compare int */
    if ((PIR2bits.CCP2IF) && (PIE2bits.CCP2IE)) {
        PIR2bits.CCP2IF = 0;
        Interrupt_CCP2();
    }

    /* check for USART Tx int */
    if ((PIR3bits.TX2IF) && (PIE3bits.TX2IE)) {
        RS485_Interrupt_Tx();
    }

    /* check for USART Rx int */
    if ((PIR3bits.RC2IF) && (PIE3bits.RC2IE)) {
        RS485_Interrupt_Rx();
    }

/* Unused Interrupts
  //check for timer1 int
  if ((PIR1bits.TMR1IF) && (PIE1bits.TMR1IE))
  {						
    PIR1bits.TMR1IF = 0; 
    Interrupt_Timer1();
  }

  //check for compare int
  if ((PIR1bits.CCP1IF) && (PIE1bits.CCP1IE))
  {						
    PIR1bits.CCP1IF = 0; 
    Interrupt_CCP1();
  }

  //check for compare int
  if ((PIR3bits.CCP3IF) && (PIE3bits.CCP3IE))
  {						
    PIR3bits.CCP3IF = 0; 
    Interrupt_CCP3();
  }

  //check for compare int
  if ((PIR3bits.CCP4IF) && (PIE3bits.CCP4IE))
  {						
    PIR3bits.CCP4IF = 0; 

    Interrupt_CCP4();
  }

  //check for AD int
  if ((PIR1bits.ADIF) && (PIE1bits.ADIE))
  {						
    PIR1bits.ADIF = 0;   
    Interrupt_ADC();
  }

  //check for MSSP int
  if ((PIR1bits.SSPIF) && (PIE1bits.SSPIE))
  {						
    PIR1bits.SSPIF = 0;     
    Interrupt_SSP();
  }

*/
}

void Interrupt_Timer2(
    void)
{
}

void Interrupt_Timer3(
    void)
{
}

/* Timer4 is set to go off every 1ms. This is our system tick */
void Interrupt_Timer4(
    void)
{
    /* Milisecond is our system tick */
    if (Milliseconds < 0xFF)
        ++Milliseconds;
}

void Interrupt_CCP2(
    void)
{

}
