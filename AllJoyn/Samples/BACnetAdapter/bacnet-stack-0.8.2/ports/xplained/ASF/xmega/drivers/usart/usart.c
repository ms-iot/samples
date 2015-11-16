/**
 * \file
 *
 * \brief USART driver for AVR XMEGA.
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
#include <stdint.h>
#include "compiler.h"
#include "usart.h"
#include "sysclk.h"
#include "ioport.h"
#include "status_codes.h"

/*
 * Fix XMEGA header files
 * USART.CTRLC  bit masks and bit positions
 */
#ifndef USART_UCPHA_bm
#  define USART_UCPHA_bm 0x02
#endif
#ifndef USART_DORD_bm
#  define USART_DORD_bm 0x04
#endif

/**
 * \brief Initialize USART in RS232 mode.
 *
 * This function initializes the USART module in RS232 mode using the
 * usart_rs232_options_t configuration structure and CPU frequency.
 *
 * \param usart The USART module.
 * \param opt The RS232 configuration option.
 *
 * \retval true if the initialization was successfull
 * \retval false if the initialization failed (error in baud rate calculation)
 */
bool usart_init_rs232(USART_t *usart, const usart_rs232_options_t *opt)
{
	bool result;
	sysclk_enable_peripheral_clock(usart);
	usart_set_mode(usart, USART_CMODE_ASYNCHRONOUS_gc);
	usart_format_set(usart, opt->charlength, opt->paritytype,
			opt->stopbits);
	result = usart_set_baudrate(usart, opt->baudrate, sysclk_get_per_hz());
	usart_tx_enable(usart);
	usart_rx_enable(usart);
	
	return result;
}

/**
 * \brief Initialize USART in SPI master mode.
 *
 * This function initializes the USART module in SPI master mode using the
 * usart_spi_options_t configuration structure and CPU frequency.
 *
 * \param usart The USART module.
 * \param opt The RS232 configuration option.
 */
void usart_init_spi(USART_t *usart, const usart_spi_options_t *opt)
{
	ioport_pin_t sck_pin;
	bool invert_sck;

	sysclk_enable_peripheral_clock(usart);

	usart_rx_disable(usart);

	/* configure Clock polarity using INVEN bit of the correct SCK I/O port **/
	invert_sck = (opt->spimode == 2) || (opt->spimode == 3);
	UNUSED(invert_sck);

#ifdef USARTC0
	if ((uint16_t)usart == (uint16_t)&USARTC0) {
#  ifdef PORT_USART0_bm
		if (PORTC.REMAP & PORT_USART0_bm) {
			sck_pin = IOPORT_CREATE_PIN(PORTC, 5);
		} else {
			sck_pin = IOPORT_CREATE_PIN(PORTC, 1);
		}
#  else
		sck_pin = IOPORT_CREATE_PIN(PORTC, 1);
#  endif
	}
#endif
#ifdef USARTC1
	if ((uint16_t)usart == (uint16_t)&USARTC1) {
		sck_pin = IOPORT_CREATE_PIN(PORTC, 5);
	}
#endif
#ifdef USARTD0
	if ((uint16_t)usart == (uint16_t)&USARTD0) {
#  ifdef PORT_USART0_bm
		if (PORTD.REMAP & PORT_USART0_bm) {
			sck_pin = IOPORT_CREATE_PIN(PORTD, 5);
		} else {
			sck_pin = IOPORT_CREATE_PIN(PORTD, 1);
		}
#  else
		sck_pin = IOPORT_CREATE_PIN(PORTD, 1);
#  endif
	}
#endif
#ifdef USARTD1
	if ((uint16_t)usart == (uint16_t)&USARTD1) {
		sck_pin = IOPORT_CREATE_PIN(PORTD, 5);
	}
#endif
#ifdef USARTE0
	if ((uint16_t)usart == (uint16_t)&USARTE0) {
#  ifdef PORT_USART0_bm
		if(PORTE.REMAP & PORT_USART0_bm) {
			sck_pin = IOPORT_CREATE_PIN(PORTE, 5);
		} else {
			sck_pin = IOPORT_CREATE_PIN(PORTE, 1);
		}
#  else
		sck_pin = IOPORT_CREATE_PIN(PORTE, 1);
#  endif
	}
#endif
#ifdef USARTE1
	if ((uint16_t)usart == (uint16_t)&USARTE1) {
		sck_pin = IOPORT_CREATE_PIN(PORTE, 5);
	}
#endif
#ifdef USARTF0
	if ((uint16_t)usart == (uint16_t)&USARTF0) {
#  ifdef PORT_USART0_bm
		if(PORTF.REMAP & PORT_USART0_bm) {
			sck_pin = IOPORT_CREATE_PIN(PORTF, 5);
		} else {
			sck_pin = IOPORT_CREATE_PIN(PORTF, 1);
		}
#  else
		sck_pin = IOPORT_CREATE_PIN(PORTF, 1);
# endif
	}
#endif
#ifdef USARTF1
	if ((uint16_t)usart == (uint16_t)&USARTF1) {
		sck_pin = IOPORT_CREATE_PIN(PORTF, 5);
	}
#endif

	/* Configure the USART output pin */
	ioport_set_pin_dir(sck_pin, IOPORT_DIR_OUTPUT);
	ioport_set_pin_mode(sck_pin,
			IOPORT_MODE_TOTEM | (invert_sck? IOPORT_MODE_INVERT_PIN : 0));
	ioport_set_pin_level(sck_pin, IOPORT_PIN_LEVEL_HIGH);

	usart_set_mode(usart, USART_CMODE_MSPI_gc);

	if (opt->spimode == 1 || opt->spimode == 3) {
		usart->CTRLC |= USART_UCPHA_bm;
	} else {
		usart->CTRLC &= ~USART_UCPHA_bm;
	}
	if (opt->data_order) {
		(usart)->CTRLC |= USART_DORD_bm;
	} else {
		(usart)->CTRLC &= ~USART_DORD_bm;
	}

	usart_spi_set_baudrate(usart, opt->baudrate, sysclk_get_per_hz());
	usart_tx_enable(usart);
	usart_rx_enable(usart);
}

/**
 * \brief Send a data with the USART module
 *
 * This function outputs a data using the USART module.
 *
 * \param usart The USART module.
 * \param c The data to send.
 *
 * \return STATUS_OK
 */
status_code_t usart_putchar(USART_t *usart, uint8_t c)
{
	while (usart_data_register_is_empty(usart) == false) {
	}
	
	(usart)->DATA = c;
	return STATUS_OK;
}

/**
 * \brief Receive a data with the USART module
 *
 * This function returns the received data from the USART module.
 *
 * \param usart The USART module.
 *
 * \return The received data.
 */
uint8_t usart_getchar(USART_t *usart)
{
	while (usart_rx_is_complete(usart) == false) {
	}
	
	return ((uint8_t)(usart)->DATA);
}

/**
 * \brief Get the offset for lookup in the baudrate table
 *
 * \param baud The requested baudrate
 *
 * \return The baudrate offset in PROGMEM table
 * \retval USART_BAUD_UNDEFINED for baudrates not in lookup table
 */
static uint8_t usart_get_baud_offset(uint32_t baud)
{
	switch (baud) {
	case 1200:
		return (uint8_t)USART_BAUD_1200;

	case 2400:
		return (uint8_t)USART_BAUD_2400;

	case 4800:
		return (uint8_t)USART_BAUD_4800;

	case 9600:
		return (uint8_t)USART_BAUD_9600;

	case 19200:
		return (uint8_t)USART_BAUD_19200;

	case 38400:
		return (uint8_t)USART_BAUD_38400;

	case 57600:
		return (uint8_t)USART_BAUD_57600;

	default:
		return (uint8_t)USART_BAUD_UNDEFINED;
	}
}

/**
 * \brief Set the baudrate by setting the BSEL and BSCALE values in the USART
 *
 * This function sets the selected BSEL and BSCALE value in the BAUDCTRL
 * registers with BSCALE 0. For calculation options, see table 21-1 in XMEGA A
 * manual.
 *
 * \param usart  The USART module.
 * \param bsel   Calculated BSEL value.
 * \param bscale Calculated BSEL value.
 *
 */
void usart_set_bsel_bscale_value(USART_t *usart, uint16_t bsel, uint8_t bscale)
{
	(usart)->BAUDCTRLA = (uint8_t)(bsel);
	(usart)->BAUDCTRLB = (uint8_t)(((bsel >> 8) & 0X0F) | (bscale << 4));
}

/**
 * \brief Set the baudrate using precalculated BAUDCTRL values from PROGMEM
 *
 * \note This function only works for cpu_hz 2Mhz or 32Mhz and baudrate values
 * 1200, 2400, 4800, 9600, 19200, 38400 and 57600.
 *
 * \param usart  The USART module.
 * \param baud   The baudrate.
 * \param cpu_hz The CPU frequency.
 *
 */
void usart_set_baudrate_precalculated(USART_t *usart, uint32_t baud,
		uint32_t cpu_hz)
{
	uint8_t baud_offset;
	uint16_t baudctrl = 0;

	baud_offset = usart_get_baud_offset(baud);

	if (cpu_hz == 2000000UL) {
		baudctrl = PROGMEM_READ_WORD(baudctrl_2mhz + baud_offset);
	} else if (cpu_hz == 32000000UL) {
		baudctrl = PROGMEM_READ_WORD(baudctrl_32mhz + baud_offset);
	} else {
		/* Error, system clock speed or USART baud rate is not supported
		 * by the look-up table */
		Assert(false);
	}

	if (baud_offset != USART_BAUD_UNDEFINED) {
		(usart)->BAUDCTRLB = (uint8_t)((uint16_t)baudctrl);
		(usart)->BAUDCTRLA = (uint8_t)((uint16_t)baudctrl >> 8);
	}
}

/**
 * \brief Set the baudrate value in the USART module
 *
 * This function sets the baudrate register with scaling regarding the CPU
 * frequency and makes sure the baud rate is supported by the hardware.
 * The function can be used if you don't want to calculate the settings
 * yourself or changes to baudrate at runtime is required.
 *
 * \param usart The USART module.
 * \param baud The baudrate.
 * \param cpu_hz The CPU frequency.
 *
 * \retval true if the hardware supports the baud rate
 * \retval false if the hardware does not support the baud rate (i.e. it's
 *               either too high or too low.)
 */
bool usart_set_baudrate(USART_t *usart, uint32_t baud, uint32_t cpu_hz)
{
	int8_t exp;
	uint32_t div;
	uint32_t limit;
	uint32_t ratio;
	uint32_t min_rate;
	uint32_t max_rate;

	/*
	 * Check if the hardware supports the given baud rate
	 */
	/* 8 = (2^0) * 8 * (2^0) = (2^BSCALE_MIN) * 8 * (BSEL_MIN) */
	max_rate = cpu_hz / 8;
	/* 4194304 = (2^7) * 8 * (2^12) = (2^BSCALE_MAX) * 8 * (BSEL_MAX+1) */
	min_rate = cpu_hz / 4194304;

	if (!((usart)->CTRLB & USART_CLK2X_bm)) {
		max_rate /= 2;
		min_rate /= 2;
	}

	if ((baud > max_rate) || (baud < min_rate)) {
		return false;
	}

	/* Check if double speed is enabled. */
	if (!((usart)->CTRLB & USART_CLK2X_bm)) {
		baud *= 2;
	}

	/* Find the lowest possible exponent. */
	limit = 0xfffU >> 4;
	ratio = cpu_hz / baud;

	for (exp = -7; exp < 7; exp++) {
		if (ratio < limit) {
			break;
		}

		limit <<= 1;

		if (exp < -3) {
			limit |= 1;
		}
	}

	/*
	 * Depending on the value of exp, scale either the input frequency or
	 * the target baud rate. By always scaling upwards, we never introduce
	 * any additional inaccuracy.
	 *
	 * We are including the final divide-by-8 (aka. right-shift-by-3) in
	 * this operation as it ensures that we never exceeed 2**32 at any
	 * point.
	 *
	 * The formula for calculating BSEL is slightly different when exp is
	 * negative than it is when exp is positive.
	 */
	if (exp < 0) {
		/* We are supposed to subtract 1, then apply BSCALE. We want to
		 * apply BSCALE first, so we need to turn everything inside the
		 * parenthesis into a single fractional expression.
		 */
		cpu_hz -= 8 * baud;

		/* If we end up with a left-shift after taking the final
		 * divide-by-8 into account, do the shift before the divide.
		 * Otherwise, left-shift the denominator instead (effectively
		 * resulting in an overall right shift.)
		 */
		if (exp <= -3) {
			div = ((cpu_hz << (-exp - 3)) + baud / 2) / baud;
		} else {
			baud <<= exp + 3;
			div = (cpu_hz + baud / 2) / baud;
		}
	} else {
		/* We will always do a right shift in this case, but we need to
		 * shift three extra positions because of the divide-by-8.
		 */
		baud <<= exp + 3;
		div = (cpu_hz + baud / 2) / baud - 1;
	}

	(usart)->BAUDCTRLB = (uint8_t)(((div >> 8) & 0X0F) | (exp << 4));
	(usart)->BAUDCTRLA = (uint8_t)div;

	return true;
}

/**
 * \brief Set the baudrate value in the USART_SPI module
 *
 * This function sets the baudrate register regarding the CPU frequency.
 *
 * \param usart The USART(SPI) module.
 * \param baud The baudrate.
 * \param cpu_hz The CPU frequency.
 */
void usart_spi_set_baudrate(USART_t *usart, uint32_t baud, uint32_t cpu_hz)
{
	uint16_t bsel_value;

	/* Check if baudrate is less than the maximim limit specified in
	 * datasheet */
	if (baud < (cpu_hz / 2)) {
		bsel_value = (cpu_hz / (baud * 2)) - 1;
	} else {
		/* If baudrate is not within the specfication in datasheet,
		 * assign maximum baudrate possible for the current CPU frequency */
		bsel_value = 0;
	}

	(usart)->BAUDCTRLB = (uint8_t)((~USART_BSCALE_gm) & (bsel_value >> 8));
	(usart)->BAUDCTRLA = (uint8_t)(bsel_value);
}
