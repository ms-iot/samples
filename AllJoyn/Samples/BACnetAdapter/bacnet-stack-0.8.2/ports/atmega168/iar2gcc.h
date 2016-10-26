/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2007 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/
#ifndef IAR2GCC_H
#define IAR2GCC_H

#if !defined(F_CPU)
#define F_CPU (7372800)
#endif

/* IAR */
#if defined(__IAR_SYSTEMS_ICC__) || defined(__IAR_SYSTEMS_ASM__)
#include <inavr.h>
#include <ioavr.h>
/* BitValue is used alot in GCC examples */
#ifndef _BV
#define _BV(bit_num) (1 << (bit_num))
#endif

/* inline function */
static inline void _delay_us(
    uint8_t microseconds)
{
    do {
        __delay_cycles(F_CPU / 1000000UL);
    } while (microseconds--);
}
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
#define ISR(vec) PRAGMA( vector=vec ) __interrupt void handler_##vec(void)
#endif
#if defined(__GNUC__)
#include <avr/interrupt.h>
#endif

/* Flash */
#if defined(__ICCAVR__)
#define FLASH_DECLARE(x) __flash x
#endif
#if defined(__GNUC__)
#define FLASH_DECLARE(x) x __attribute__((__progmem__))
#endif

/* EEPROM */
#if defined(__ICCAVR__)
#define EEPROM_DECLARE(x) __eeprom x
#endif
#if defined(__GNUC__)
#include <avr/eeprom.h>
#define EEPROM_DECLARE(x) x __attribute__((section (".eeprom")))
#endif

/* IAR intrinsic routines */
#if defined(__GNUC__)
    /* FIXME: intrinsic routines: map to assembler for size/speed */
#define __multiply_unsigned(x,y) ((x)*(y))
    /* FIXME: __root means to not optimize or strip */
#define __root
#endif

#endif
