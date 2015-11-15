/**
 * \file
 *
 * This file defines a useful set of functions for the Serial interface on AVR
 * XMEGA devices.
 *
 * Copyright (c) 2009-2012 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#ifndef _USART_SERIAL_H_
#define _USART_SERIAL_H_

#include "compiler.h"
#include "sysclk.h"
#include "status_codes.h"
#include "usart.h"

/*! \name Serial Management Configuration
 */
//! @{
#include "conf_usart_serial.h"
//! @}

typedef usart_rs232_options_t usart_serial_options_t;

typedef USART_t *usart_if;

/*! \brief Initializes the Usart in master mode.
 *
 * \param usart       Base address of the USART instance.
 * \param options     Options needed to set up RS232 communication (see \ref usart_serial_options_t).
 *
 * \retval true if the initialization was successful
 * \retval false if initialization failed (error in baud rate calculation)
 */
static inline bool usart_serial_init(usart_if usart, const
		usart_serial_options_t *options)
{
	// USART options.
	usart_rs232_options_t usart_rs232_options;
	usart_rs232_options.charlength   = options->charlength;
	usart_rs232_options.paritytype   = options->paritytype;
	usart_rs232_options.stopbits     = options->stopbits;
	usart_rs232_options.baudrate     = options->baudrate;

#ifdef USARTC0
	if((uint16_t)usart == (uint16_t)&USARTC0) {
		sysclk_enable_module(SYSCLK_PORT_C,PR_USART0_bm);
	}
#endif
#ifdef USARTC1
	if((uint16_t)usart == (uint16_t)&USARTC1) {
		sysclk_enable_module(SYSCLK_PORT_C,PR_USART1_bm);
	}
#endif
#ifdef USARTD0
	if((uint16_t)usart == (uint16_t)&USARTD0) {
		sysclk_enable_module(SYSCLK_PORT_D,PR_USART0_bm);
	}
#endif
#ifdef USARTD1
	if((uint16_t)usart == (uint16_t)&USARTD1) {
		sysclk_enable_module(SYSCLK_PORT_D,PR_USART1_bm);
	}
#endif
#ifdef USARTE0
	if((uint16_t)usart == (uint16_t)&USARTE0) {
		sysclk_enable_module(SYSCLK_PORT_E,PR_USART0_bm);
	}
#endif
#ifdef USARTE1
	if((uint16_t)usart == (uint16_t)&USARTE1) {
		sysclk_enable_module(SYSCLK_PORT_E,PR_USART1_bm);
	}
#endif
#ifdef USARTF0
	if((uint16_t)usart == (uint16_t)&USARTF0) {
		sysclk_enable_module(SYSCLK_PORT_F,PR_USART0_bm);
	}
#endif
#ifdef USARTF1
	if((uint16_t)usart == (uint16_t)&USARTF1) {
		sysclk_enable_module(SYSCLK_PORT_F,PR_USART1_bm);
	}
#endif
	if (usart_init_rs232(usart, &usart_rs232_options)) {
		return true;
	}
	else {
		return false;
	}
}

/*! \brief Sends a character with the USART.
 *
 * \param usart   Base address of the USART instance.
 * \param c       Character to write.
 *
 * \return Status code
 */
static inline enum status_code usart_serial_putchar(usart_if usart, uint8_t c)
{
	return usart_putchar(usart, c);
}
/*! \brief Waits until a character is received, and returns it.
 *
 * \param usart   Base address of the USART instance.
 * \param data   Data to read
 *
 */
static inline void usart_serial_getchar(usart_if usart, uint8_t *data)
{
	*data = usart_getchar(usart);
}

/**
 * \brief Send a sequence of bytes to USART device
 *
 * \param usart  Base address of the USART instance.
 * \param data   Data buffer to read
 * \param len    Length of data
 *
 */
extern status_code_t usart_serial_write_packet(usart_if usart, const uint8_t *data, size_t len);

/**
 * \brief Receive a sequence of bytes from USART device
 *
 * \param usart  Base address of the USART instance.
 * \param data   Data buffer to write
 * \param len    Length of data
 *
 */
extern status_code_t usart_serial_read_packet(usart_if usart, uint8_t *data, size_t len);

#endif  // _USART_SERIAL_H_
