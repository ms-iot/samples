/**
 * \file
 *
 * \brief USART driver for AVR XMEGA.
 *
 * This file contains basic functions for the AVR XMEGA USART, with support for all
 * modes, settings and clock speeds.
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
#ifndef _USART_H_
#define _USART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "compiler.h"
#include "pmic.h"
#include "status_codes.h"

/**
 * \defgroup usart_group USART module (USART)
 *
 * See \ref xmega_usart_quickstart.
 *
 * This is a driver for configuring, enabling, disabling and use of the on-chip
 * USART.
 *
 * \section dependencies Dependencies
 *
 * The USART module depends on the following modules:
 *  - \ref sysclk_group for peripheral clock control.
 *  - \ref port_driver_group for peripheral io port control.
 *
 * @{
 */

//! Offset in lookup table for baudrate 1200
#define USART_BAUD_1200      0x00
//! Offset in lookup table for baudrate 2400
#define USART_BAUD_2400      0x01
//! Offset in lookup table for baudrate 4800
#define USART_BAUD_4800      0x02
//! Offset in lookup table for baudrate 9600
#define USART_BAUD_9600      0x03
//! Offset in lookup table for baudrate 19200
#define USART_BAUD_19200     0x04
//! Offset in lookup table for baudrate 38400
#define USART_BAUD_38400     0x05
//! Offset in lookup table for baudrate 57600
#define USART_BAUD_57600     0x06
//! Baudrate not in lookup table
#define USART_BAUD_UNDEFINED 0xFF

//! Lookup table containing baudctrl values for CPU frequency 2 Mhz
static PROGMEM_DECLARE(uint16_t, baudctrl_2mhz[]) = {
	0xE5BC, // Baud: 1200
	0xC5AC, // Baud: 2400
	0x859C, // Baud: 4800
	0x0396, // Baud: 9600
	0xC192, // Baud: 19200
	0x2191, // Baud: 38400
	0x9690, // Baud: 57600
};

//! Lookup table containing baudctrl values for CPU frequency 32 Mhz
static PROGMEM_DECLARE(uint16_t, baudctrl_32mhz[]) = {
	0x031D, // Baud: 1200
	0x01ED, // Baud: 2400
	0xFDDC, // Baud: 4800
	0xF5CC, // Baud: 9600
	0xE5BC, // Baud: 19200
	0xC5AC, // Baud: 38400
	0x6EA8, // Baud: 57600
};
//! @}

//! Input parameters when initializing RS232 and similar modes.
typedef struct usart_rs232_options {
	//! Set baud rate of the USART (unused in slave modes).
	uint32_t baudrate;

	//! Number of bits to transmit as a character (5 to 9).
	USART_CHSIZE_t charlength;

	//! Parity type: USART_PMODE_DISABLED_gc, USART_PMODE_EVEN_gc,
	//! USART_PMODE_ODD_gc.
	USART_PMODE_t paritytype;

	//! Number of stop bits between two characters:
	//! true: 2 stop bits
	//! false: 1 stop bit
	bool stopbits;

} usart_rs232_options_t;

//! Input parameters when initializing SPI master mode.
typedef struct usart_spi_options {
	//! Set baud rate of the USART in SPI mode.
	uint32_t baudrate;

	//! SPI transmission mode.
	uint8_t spimode;

	uint8_t data_order;
} usart_spi_options_t;

//! USART interrupt levels
enum usart_int_level_t {
	USART_INT_LVL_OFF = 0x00,
	USART_INT_LVL_LO = 0x01,
	USART_INT_LVL_MED = 0x02,
	USART_INT_LVL_HI = 0x03,
};

/**
 * \brief Enable USART receiver.
 *
 * \param usart Pointer to the USART module
 */
static inline void usart_rx_enable(USART_t *usart)
{
	(usart)->CTRLB |= USART_RXEN_bm;
}

/**
 * \brief Disable USART receiver.
 *
 * \param usart Pointer to the USART module.
 */
static inline void usart_rx_disable(USART_t *usart)
{
	(usart)->CTRLB &= ~USART_RXEN_bm;
}

/**
 * \brief  Configure the USART frame format.
 *
 *  Sets the frame format, Frame Size, parity mode and number of stop bits.
 *
 *  \param usart Pointer to the USART module
 *  \param charSize The character size. Use USART_CHSIZE_t type.
 *  \param parityMode The parity Mode. Use USART_PMODE_t type.
 *  \param twoStopBits Enable two stop bit mode. Use bool type.
 */
static inline void usart_format_set(USART_t *usart, USART_CHSIZE_t charSize,
		USART_PMODE_t parityMode, bool twoStopBits)
{
	(usart)->CTRLC = (uint8_t)charSize | parityMode
		| (twoStopBits ? USART_SBMODE_bm : 0);
}

/**
 * \brief Enable USART transmitter.
 *
 * \param usart Pointer to the USART module.
 */
static inline void usart_tx_enable(USART_t *usart)
{
	(usart)->CTRLB |= USART_TXEN_bm;
}

/**
 * \brief Disable USART transmitter.
 *
 * \param usart Pointer to the USART module.
 */
static inline void usart_tx_disable(USART_t *usart)
{
	(usart)->CTRLB &= ~USART_TXEN_bm;
}

/**
 * \brief Set USART RXD interrupt level.
 *
 * Sets the interrupt level on RX Complete interrupt.
 *
 * \param usart Pointer to the USART module.
 * \param level Interrupt level of the RXD interrupt.
 */
static inline void usart_set_rx_interrupt_level(USART_t *usart,
		enum usart_int_level_t level)
{
	(usart)->CTRLA = ((usart)->CTRLA & ~USART_RXCINTLVL_gm) |
			(level << USART_RXCINTLVL_gp);
}

/**
 * \brief Set USART TXD interrupt level.
 *
 * Sets the interrupt level on TX Complete interrupt.
 *
 * \param usart Pointer to the USART module.
 * \param level Interrupt level of the TXD interrupt.
 */
static inline void usart_set_tx_interrupt_level(USART_t *usart,
		enum usart_int_level_t level)
{
	(usart)->CTRLA = ((usart)->CTRLA & ~USART_TXCINTLVL_gm) |
			(level << USART_TXCINTLVL_gp);
}

/**
 * \brief Set USART DRE interrupt level.
 *
 * Sets the interrupt level on Data Register interrupt.
 *
 * \param usart Pointer to the USART module.
 * \param level Interrupt level of the DRE interrupt.
 *              Use USART_DREINTLVL_t type.
 */
static inline void usart_set_dre_interrupt_level(USART_t *usart,
		enum usart_int_level_t level)
{
	(usart)->CTRLA = ((usart)->CTRLA & ~USART_DREINTLVL_gm) |
			(level << USART_DREINTLVL_gp);
}

/**
 * \brief Set the mode the USART run in.
 *
 * Set the mode the USART run in. The default mode is asynchronous mode.
 *
 * \param usart Pointer to the USART module register section.
 * \param usartmode Selects the USART mode. Use USART_CMODE_t type.
 *
 * USART modes:
 * - 0x0        : Asynchronous mode.
 * - 0x1        : Synchronous mode.
 * - 0x2        : IrDA mode.
 * - 0x3        : Master SPI mode.
 */
static inline void usart_set_mode(USART_t *usart, USART_CMODE_t usartmode)
{
	(usart)->CTRLC = ((usart)->CTRLC & (~USART_CMODE_gm)) | usartmode;
}

/**
 * \brief Check if data register empty flag is set.
 *
 * \param usart The USART module.
 */
static inline bool usart_data_register_is_empty(USART_t * usart)
{
	return (usart)->STATUS & USART_DREIF_bm;
}

/**
 * \brief Checks if the RX complete interrupt flag is set.
 *
 * Checks if the RX complete interrupt flag is set.
 *
 * \param usart The USART module.
 */
static inline bool usart_rx_is_complete(USART_t * usart)
{
	return (usart)->STATUS & USART_RXCIF_bm;
}

/**
 * \brief Checks if the TX complete interrupt flag is set.
 *
 * Checks if the TX complete interrupt flag is set.
 *
 * \param usart The USART module.
 */
static inline bool usart_tx_is_complete(USART_t * usart)
{
	return (usart)->STATUS & USART_TXCIF_bm;
}

/**
 * \brief Clear TX complete interrupt flag.
 *
 * \param usart The USART module.
 */
static inline void usart_clear_tx_complete(USART_t * usart)
{
	(usart)->STATUS = USART_TXCIF_bm;
}

/**
 * \brief Clear RX complete interrupt flag.
 *
 * \param usart The USART module.
 */
static inline void usart_clear_rx_complete(USART_t *usart)
{
	(usart)->STATUS = USART_RXCIF_bm;
}

/**
 * \brief Write a data to the USART data register.
 *
 * \param usart The USART module.
 * \param txdata The data to be transmitted.
 */
static inline void usart_put(USART_t * usart, uint8_t txdata)
{
	(usart)->DATA = txdata;
}

/**
 * \brief  Read a data to the USART data register.
 *
 * \param usart The USART module.
 *
 * \return The received data
 */
static inline uint8_t usart_get(USART_t * usart)
{
	return (usart)->DATA;
}

/**
 * \brief Performs a data transfer on the USART in SPI mode.
 *
 * \param usart The USART module.
 * \param txdata The data to be transmitted.
 *
 * \return The received data
 */
static inline uint8_t usart_spi_transmit(USART_t * usart,
		uint8_t txdata)
{
	while (usart_data_register_is_empty(usart) == false);
	usart_put(usart, txdata);
	while (!usart_tx_is_complete(usart));
	usart_clear_tx_complete(usart);
	return usart_get(usart);
}

bool usart_init_rs232(USART_t *usart, const usart_rs232_options_t *opt);
void usart_init_spi(USART_t * usart, const usart_spi_options_t * opt);

status_code_t usart_putchar(USART_t * usart, uint8_t c);
uint8_t usart_getchar(USART_t * usart);

void usart_set_bsel_bscale_value(USART_t *usart, uint16_t bsel, uint8_t bscale);
void usart_set_baudrate_precalculated(USART_t *usart, uint32_t baud,
		uint32_t cpu_hz);
bool usart_set_baudrate(USART_t *usart, uint32_t baud, uint32_t cpu_hz);
void usart_spi_set_baudrate(USART_t * usart, uint32_t baud, uint32_t cpu_hz);
//! @}

#ifdef __cplusplus
}
#endif

/**
 * \page xmega_usart_quickstart Quick start guide for USART module
 *
 * This is the quick start guide for the \ref usart_group "USART module", with
 * step-by-step instructions on how to configure and use the driver in a
 * selection of use cases.
 *
 * The use cases contain several code fragments. The code fragments in the
 * steps for setup can be copied into a custom initialization function, while
 * the steps for usage can be copied into, e.g., the main application function.
 *
 * \section usart_basic_use_case Basic use case
 * \section usart_use_cases USART use cases
 * - \ref usart_basic_use_case
 * - \subpage usart_use_case_1
 *
 * \section usart_basic_use_case Basic use case - transmit a character
 * In this use case, the USART module is configured for:
 * - Using USARTD0
 * - Baudrate: 9600
 * - Character length: 8 bit
 * - Parity mode: Disabled
 * - Stop bit: None
 * - RS232 mode
 *
 * \section usart_basic_use_case_setup Setup steps
 *
 * \subsection usart_basic_use_case_setup_prereq Prerequisites
 * -# \ref sysclk_group
 * \subsection usart_basic_use_case_setup_code Example code
 * The following configuration must be added to the project (typically to a 
 * conf_usart.h file, but it can also be added to your main application file.)
 * \code
 *    #define USART_SERIAL                     &USARTD0
 *    #define USART_SERIAL_BAUDRATE            9600
 *    #define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
 *    #define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
 *    #define USART_SERIAL_STOP_BIT            false
 * \endcode
 *
 * Add to application initialization:
 * \code
 *    sysclk_init();
 *    static usart_rs232_options_t USART_SERIAL_OPTIONS = {
 *       .baudrate = USART_SERIAL_BAUDRATE,
 *       .charlength = USART_SERIAL_CHAR_LENGTH,
 *       .paritytype = USART_SERIAL_PARITY,
 *       .stopbits = USART_SERIAL_STOP_BIT
 *    };
 *   sysclk_enable_module(SYSCLK_PORT_D, PR_USART0_bm);
 *   usart_init_rs232(USART_SERIAL, &USART_SERIAL_OPTIONS);
 * \endcode
 *
 * \subsection usart_basic_use_case_setup_flow Workflow
 * -# Initialize system clock:
 *   - \code sysclk_init(); \endcode
 *   - \note Not always required, but since the \ref usart_group driver is 
 *           dependent on \ref sysclk_group it is good practise to initialize
 *           this module. 
 * -# Create USART options struct:
 *   - \code
 *       static usart_rs232_options_t USART_SERIAL_OPTIONS = {
 *          .baudrate = USART_SERIAL_BAUDRATE,
 *          .charlength = USART_SERIAL_CHAR_LENGTH,
 *          .paritytype = USART_SERIAL_PARITY,
 *          .stopbits = USART_SERIAL_STOP_BIT
 *       };
 *     \endcode
 * -# Enable the clock for the USART module:
 *   - \code sysclk_enable_module(SYSCLK_PORT_D, PR_USART0_bm); \endcode
 * -# Initialize in RS232 mode:
 *   - \code usart_init_rs232(USART_SERIAL, &USART_SERIAL_OPTIONS);
 *     \endcode
 *
 * \section usart_basic_use_case_usage Usage steps
 *
 * \subsection usart_basic_use_case_usage_code Example code
 * Add to application C-file:
 * \code
 *    usart_putchar(USART_SERIAL, 'a');
 * \endcode
 *
 * \subsection usart_basic_use_case_usage_flow Workflow
 * -# Send an 'a' character via USART
 *   - \code usart_putchar(USART_SERIAL, 'a'); \endcode
 */

/**
 * \page usart_use_case_1 USART receive character and echo back
 *
 * In this use case, the USART module is configured for:
 * - Using USARTD0
 * - Baudrate: 9600
 * - Character length: 8 bit
 * - Parity mode: Disabled
 * - Stop bit: None
 * - RS232 mode
 *
 * The use case waits for a received character on the configured USART and
 * echoes the character back to the same USART.
 *
 * \section usart_use_case_1_setup Setup steps
 *
 * \subsection usart_use_case_1_setup_prereq Prerequisites
 * -# \ref sysclk_group
 *
 * \subsection usart_use_case_1_setup_code Example code
 * -# The following configuration must be added to the project (typically to a 
 * conf_usart.h file, but it can also be added to your main application file.):
 * \code
 *    #define USART_SERIAL                     &USARTD0
 *    #define USART_SERIAL_BAUDRATE            9600
 *    #define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
 *    #define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
 *    #define USART_SERIAL_STOP_BIT            false
 * \endcode
 *
 * A variable for the received byte must be added:
 * \code uint8_t received_byte; \endcode
 *
 * Add to application initialization:
 * \code
 *    sysclk_init();
 *    static usart_rs232_options_t USART_SERIAL_OPTIONS = {
 *       .baudrate = USART_SERIAL_BAUDRATE,
 *       .charlength = USART_SERIAL_CHAR_LENGTH,
 *       .paritytype = USART_SERIAL_PARITY,
 *       .stopbits = USART_SERIAL_STOP_BIT
 *    };
 *   sysclk_enable_module(SYSCLK_PORT_D, PR_USART0_bm);
 *   usart_init_rs232(USART_SERIAL, &USART_SERIAL_OPTIONS);
 * \endcode
 *
 * \subsection usart_use_case_1_setup_flow Workflow
 * -# Initialize system clock:
 *   - \code sysclk_init(); \endcode
 *   - \note Not always required, but since the \ref usart_group driver is 
 *           dependent on \ref sysclk_group it is good practise to initialize
 *           this module. 
 * -# Create USART options struct:
 *   - \code
 *       static usart_rs232_options_t USART_SERIAL_OPTIONS = {
 *          .baudrate = USART_SERIAL_BAUDRATE,
 *          .charlength = USART_SERIAL_CHAR_LENGTH,
 *          .paritytype = USART_SERIAL_PARITY,
 *          .stopbits = USART_SERIAL_STOP_BIT
 *       };
 *     \endcode
 * -# Enable the clock for the USART module:
 *   - \code sysclk_enable_module(SYSCLK_PORT_D, PR_USART0_bm); \endcode
 * -# Initialize in RS232 mode:
 *   - \code usart_init_rs232(USART_SERIAL, &USART_SERIAL_OPTIONS);
 *     \endcode
 *
 * \section usart_use_case_1_usage Usage steps
 *
 * \subsection usart_use_case_1_usage_code Example code
 * Add to, e.g., main loop in application C-file:
 * \code
 *       received_byte = usart_getchar(USART_SERIAL);
 *       usart_putchar(USART_SERIAL, received_byte);
 * \endcode
 *
 * \subsection usart_use_case_1_usage_flow Workflow
 * -# Wait for reception of a character:
 *   - \code received_byte = usart_getchar(USART_SERIAL); \endcode
 * -# Echo the character back:
 *   - \code usart_putchar(USART_SERIAL, received_byte); \endcode
 */

#endif // _USART_H_
