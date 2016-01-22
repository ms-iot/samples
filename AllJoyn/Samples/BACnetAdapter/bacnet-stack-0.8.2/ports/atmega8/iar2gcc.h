/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
#ifndef IAR2GCC_H
#define IAR2GCC_H

/* common embedded extensions for different compilers */

#if !defined(F_CPU)
#error You must define F_CPU - clock frequency!
#endif

#if defined (__CROSSWORKS_AVR)
#include <inavr.h>
#include <stdint.h>
#endif

/* IAR */
#if defined(__ICCAVR__)
#include <inavr.h>
#include <intrinsics.h>
#include <stdint.h>

/* inline function */
static inline void _delay_us(
    uint8_t microseconds)
{
    do {
        __delay_cycles(F_CPU / 1000000UL);
    } while (microseconds--);
}
#endif

#if defined(__GNUC__)
#include <util/delay.h>
#endif

/* adjust some definitions to common versions */
#if defined (__CROSSWORKS_AVR)
#if (__TARGET_PROCESSOR == ATmega644P)
#define PRR PRR0
#define UBRR0 UBRR0W
#define UBRR1 UBRR1W

#define PA0 PORTA0
#define PA1 PORTA1
#define PA2 PORTA2
#define PA3 PORTA3
#define PA4 PORTA4
#define PA5 PORTA5
#define PA6 PORTA6
#define PA7 PORTA7

#define PB0 PORTB0
#define PB1 PORTB1
#define PB2 PORTB2
#define PB3 PORTB3
#define PB4 PORTB4
#define PB5 PORTB5
#define PB6 PORTB6
#define PB7 PORTB7

#define PC0 PORTC0
#define PC1 PORTC1
#define PC2 PORTC2
#define PC3 PORTC3
#define PC4 PORTC4
#define PC5 PORTC5
#define PC6 PORTC6
#define PC7 PORTC7

#define PD0 PORTD0
#define PD1 PORTD1
#define PD2 PORTD2
#define PD3 PORTD3
#define PD4 PORTD4
#define PD5 PORTD5
#define PD6 PORTD6
#define PD7 PORTD7

#endif
#endif

/* Input/Output Registers */
#if defined(__GNUC__)
#include <avr/io.h>

typedef struct {
    unsigned char bit0:1;
    unsigned char bit1:1;
    unsigned char bit2:1;
    unsigned char bit3:1;
    unsigned char bit4:1;
    unsigned char bit5:1;
    unsigned char bit6:1;
    unsigned char bit7:1;
} BitRegisterType;

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#define GPIO_BITREG(port,bitnum) \
        ((volatile BitRegisterType*)_SFR_MEM_ADDR(port) \
        )->bit ## bitnum

#define PINA_Bit0  GPIO_BITREG(PINA,0)
#define PINA_Bit1  GPIO_BITREG(PINA,1)
#define PINA_Bit2  GPIO_BITREG(PINA,2)
#define PINA_Bit3  GPIO_BITREG(PINA,3)
#define PINA_Bit4  GPIO_BITREG(PINA,4)
#define PINA_Bit5  GPIO_BITREG(PINA,5)
#define PINA_Bit6  GPIO_BITREG(PINA,6)
#define PINA_Bit7  GPIO_BITREG(PINA,7)

#define PORTA_Bit0 GPIO_BITREG(PORTA,0)
#define PORTA_Bit1 GPIO_BITREG(PORTA,1)
#define PORTA_Bit2 GPIO_BITREG(PORTA,2)
#define PORTA_Bit3 GPIO_BITREG(PORTA,3)
#define PORTA_Bit4 GPIO_BITREG(PORTA,4)
#define PORTA_Bit5 GPIO_BITREG(PORTA,5)
#define PORTA_Bit6 GPIO_BITREG(PORTA,6)
#define PORTA_Bit7 GPIO_BITREG(PORTA,7)

#define PINB_Bit0  GPIO_BITREG(PINB,0)
#define PINB_Bit1  GPIO_BITREG(PINB,1)
#define PINB_Bit2  GPIO_BITREG(PINB,2)
#define PINB_Bit3  GPIO_BITREG(PINB,3)
#define PINB_Bit4  GPIO_BITREG(PINB,4)
#define PINB_Bit5  GPIO_BITREG(PINB,5)
#define PINB_Bit6  GPIO_BITREG(PINB,6)
#define PINB_Bit7  GPIO_BITREG(PINB,7)

#define PORTB_Bit0 GPIO_BITREG(PORTB,0)
#define PORTB_Bit1 GPIO_BITREG(PORTB,1)
#define PORTB_Bit2 GPIO_BITREG(PORTB,2)
#define PORTB_Bit3 GPIO_BITREG(PORTB,3)
#define PORTB_Bit4 GPIO_BITREG(PORTB,4)
#define PORTB_Bit5 GPIO_BITREG(PORTB,5)
#define PORTB_Bit6 GPIO_BITREG(PORTB,6)
#define PORTB_Bit7 GPIO_BITREG(PORTB,7)

#define PINC_Bit0  GPIO_BITREG(PINC,0)
#define PINC_Bit1  GPIO_BITREG(PINC,1)
#define PINC_Bit2  GPIO_BITREG(PINC,2)
#define PINC_Bit3  GPIO_BITREG(PINC,3)
#define PINC_Bit4  GPIO_BITREG(PINC,4)
#define PINC_Bit5  GPIO_BITREG(PINC,5)
#define PINC_Bit6  GPIO_BITREG(PINC,6)
#define PINC_Bit7  GPIO_BITREG(PINC,7)

#define PORTC_Bit0 GPIO_BITREG(PORTC,0)
#define PORTC_Bit1 GPIO_BITREG(PORTC,1)
#define PORTC_Bit2 GPIO_BITREG(PORTC,2)
#define PORTC_Bit3 GPIO_BITREG(PORTC,3)
#define PORTC_Bit4 GPIO_BITREG(PORTC,4)
#define PORTC_Bit5 GPIO_BITREG(PORTC,5)
#define PORTC_Bit6 GPIO_BITREG(PORTC,6)
#define PORTC_Bit7 GPIO_BITREG(PORTC,7)

#define PIND_Bit0  GPIO_BITREG(PIND,0)
#define PIND_Bit1  GPIO_BITREG(PIND,1)
#define PIND_Bit2  GPIO_BITREG(PIND,2)
#define PIND_Bit3  GPIO_BITREG(PIND,3)
#define PIND_Bit4  GPIO_BITREG(PIND,4)
#define PIND_Bit5  GPIO_BITREG(PIND,5)
#define PIND_Bit6  GPIO_BITREG(PIND,6)
#define PIND_Bit7  GPIO_BITREG(PIND,7)

#define PORTD_Bit0 GPIO_BITREG(PORTD,0)
#define PORTD_Bit1 GPIO_BITREG(PORTD,1)
#define PORTD_Bit2 GPIO_BITREG(PORTD,2)
#define PORTD_Bit3 GPIO_BITREG(PORTD,3)
#define PORTD_Bit4 GPIO_BITREG(PORTD,4)
#define PORTD_Bit5 GPIO_BITREG(PORTD,5)
#define PORTD_Bit6 GPIO_BITREG(PORTD,6)
#define PORTD_Bit7 GPIO_BITREG(PORTD,7)

#define GPIOR0_Bit0 GPIO_BITREG(GPIOR0,0)
#define GPIOR0_Bit1 GPIO_BITREG(GPIOR0,1)
#define GPIOR0_Bit2 GPIO_BITREG(GPIOR0,2)
#define GPIOR0_Bit3 GPIO_BITREG(GPIOR0,3)
#define GPIOR0_Bit4 GPIO_BITREG(GPIOR0,4)
#define GPIOR0_Bit5 GPIO_BITREG(GPIOR0,5)
#define GPIOR0_Bit6 GPIO_BITREG(GPIOR0,6)
#define GPIOR0_Bit7 GPIO_BITREG(GPIOR0,7)

#define GPIOR1_Bit0 GPIO_BITREG(GPIOR1,0)
#define GPIOR1_Bit1 GPIO_BITREG(GPIOR1,1)
#define GPIOR1_Bit2 GPIO_BITREG(GPIOR1,2)
#define GPIOR1_Bit3 GPIO_BITREG(GPIOR1,3)
#define GPIOR1_Bit4 GPIO_BITREG(GPIOR1,4)
#define GPIOR1_Bit5 GPIO_BITREG(GPIOR1,5)
#define GPIOR1_Bit6 GPIO_BITREG(GPIOR1,6)
#define GPIOR1_Bit7 GPIO_BITREG(GPIOR1,7)

#define GPIOR2_Bit0 GPIO_BITREG(GPIOR2,0)
#define GPIOR2_Bit1 GPIO_BITREG(GPIOR2,1)
#define GPIOR2_Bit2 GPIO_BITREG(GPIOR2,2)
#define GPIOR2_Bit3 GPIO_BITREG(GPIOR2,3)
#define GPIOR2_Bit4 GPIO_BITREG(GPIOR2,4)
#define GPIOR2_Bit5 GPIO_BITREG(GPIOR2,5)
#define GPIOR2_Bit6 GPIO_BITREG(GPIOR2,6)
#define GPIOR2_Bit7 GPIO_BITREG(GPIOR2,7)

#endif

/* Global Interrupts */
#if defined(__GNUC__)
#define __enable_interrupt() sei()
#define __disable_interrupt() cli()
#endif

/* Interrupts */
#if defined(__ICCAVR__)
#define PRAGMA(x) _Pragma( #x )
#define ISR(vec) \
    /* function prototype for use with "require protoptypes" option.  */ \
    PRAGMA( vector=vec ) __interrupt void handler_##vec(void); \
    PRAGMA( vector=vec ) __interrupt void handler_##vec(void)
#elif defined(__GNUC__)
#include <avr/interrupt.h>
#elif defined (__CROSSWORKS_AVR)
#define ISR(vec) void handler_##vec(void) __interrupt[vec]
#else
#error ISR() not defined!
#endif

/* Flash */
#if defined(__ICCAVR__)
#define FLASH_DECLARE(x) __flash x
#elif defined(__GNUC__)
#define FLASH_DECLARE(x) x __attribute__((__progmem__))
#elif defined (__CROSSWORKS_AVR)
#define FLASH_DECLARE (x) const __code x
#endif

/* EEPROM */
#if defined(__ICCAVR__)
#define EEPROM_DECLARE(x) __eeprom x
#elif defined(__GNUC__)
#include <avr/eeprom.h>
#define EEPROM_DECLARE(x) x __attribute__((section (".eeprom")))
#if ((__GNUC__ < 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ < 3)) || \
     ((__GNUC__ == 4) && (__GNUC_MINOR__ == 3) && (__GNUC_PATCHLEVEL__ <= 3)))
    /* bug in WinAVR - not quite IAR compatible */
#ifndef __EEPUT
#define __EEPUT _EEPUT
#endif
#ifndef __EEGET
#define __EEGET _EEGET
#endif
#endif
#elif defined (__CROSSWORKS_AVR)
/* use functions defined in crt0.s to mimic IAR macros */
void __uint8_eeprom_store(
    unsigned char byte,
    unsigned addr);
unsigned char __uint8_eeprom_load(
    unsigned addr);
#define __EEPUT(addr, var) \
    __uint8_eeprom_store((unsigned char)(var), (unsigned)(addr))
#define __EEGET(var, addr) \
   (var) = __uint8_eeprom_load((unsigned)(addr))
#endif

/* IAR intrinsic routines */
#if defined(__GNUC__)
    /* FIXME: intrinsic routines: map to assembler for size/speed */
#define __multiply_unsigned(x,y) ((x)*(y))
    /* FIXME: __root means to not optimize or strip */
#define __root
#endif

/* watchdog defines in GCC */
#if defined(__ICCAVR__) || defined(__CROSSWORKS_AVR)
#define WDTO_15MS   0
#define WDTO_30MS   1
#define WDTO_60MS   2
#define WDTO_120MS  3
#define WDTO_250MS  4
#define WDTO_500MS  5
#define WDTO_1S     6
#define WDTO_2S     7
#endif

/* power macros in GCC-AVR */
#if (defined(__ICCAVR__) && (defined(__ATmega644P__))) || \
    (defined(__CROSSWORKS_AVR) && (__TARGET_PROCESSOR == ATmega644P))
#define power_adc_enable()      (PRR &= (uint8_t)~(1 << PRADC))
#define power_spi_enable()      (PRR &= (uint8_t)~(1 << PRSPI))
#define power_usart0_enable()   (PRR &= (uint8_t)~(1 << PRUSART0))
#define power_usart1_enable()   (PRR &= (uint8_t)~(1 << PRUSART1))
#define power_timer0_enable()   (PRR &= (uint8_t)~(1 << PRTIM0))
#define power_timer1_enable()   (PRR &= (uint8_t)~(1 << PRTIM1))
#define power_timer2_enable()   (PRR &= (uint8_t)~(1 << PRTIM2))
#endif
#if (defined(__ICCAVR__) && (defined(__ATmega1284P__))) || \
    (defined(__CROSSWORKS_AVR) && (__TARGET_PROCESSOR == ATmega1284P))
#define power_adc_enable()      (PRR0 &= (uint8_t)~(1 << PRADC))
#define power_spi_enable()      (PRR0 &= (uint8_t)~(1 << PRSPI))
#define power_usart0_enable()   (PRR0 &= (uint8_t)~(1 << PRUSART0))
#define power_usart1_enable()   (PRR0 &= (uint8_t)~(1 << PRUSART1))
#define power_timer0_enable()   (PRR0 &= (uint8_t)~(1 << PRTIM0))
#define power_timer1_enable()   (PRR0 &= (uint8_t)~(1 << PRTIM1))
#define power_timer2_enable()   (PRR0 &= (uint8_t)~(1 << PRTIM2))
#endif
#if (defined(__GNUC__) && ((__GNUC__ == 4) && (__GNUC_MINOR__ < 5)))
#if defined(__AVR_ATmega644P__)
    /* bug in WinAVR - fixed in later versions */
#define power_usart1_enable()   (PRR &= (uint8_t)~(1 << PRUSART1))
#elif defined(__AVR_ATmega1284P__)
#define power_usart1_enable()   (PRR0 &= (uint8_t)~(1 << PRUSART1))
#endif
#endif

#if defined(__CROSSWORKS_AVR)
#define inline
#endif

#endif
