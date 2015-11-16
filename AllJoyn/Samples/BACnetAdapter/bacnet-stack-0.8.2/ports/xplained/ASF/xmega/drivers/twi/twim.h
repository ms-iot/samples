/**
 * \file
 *
 * \brief TWI driver for AVR.
 *
 * This file defines a useful set of functions for the TWI interface on AVR
 * devices.
 *
 * Copyright (c) 2010-2012 Atmel Corporation. All rights reserved.
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
#ifndef _TWIM_H_
#define _TWIM_H_

/**
 * \defgroup group_xmega_drivers_twi_twim TWI Master
 *
 * \ingroup group_xmega_drivers_twi
 *
 * \{
 */

#ifdef __cplusplus
extern "C"
{
#endif


#include <compiler.h>
#include <status_codes.h>

#include "conf_twim.h"
#include "twi_common.h"


/*! \brief Error Codes for the Module
 *
 * \deprecated
 * This definition is provided for compatibility with existing ASF example
 * applications.  This module uses the \ref status_code_t values that will
 * replace module-specific error codes in ASF drivers.
 */
#define TWI_SUCCESS (STATUS_OK)


/*! Baud register setting calculation. Formula described in datasheet. */
#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)


/*! \brief Initialize the twi master module
 *
 * \param twi       Base address of the TWI (i.e. &TWIC).
 * \param *opt      Options for initializing the twi module
 *                  (see \ref twi_options_t)
 * \retval STATUS_OK        Transaction is successful
 * \retval ERR_INVALID_ARG  Invalid arguments in \c opt.
 */
  status_code_t twi_master_init (TWI_t * twi, const twi_options_t * opt);

/*! \brief Perform a TWI master write or read transfer.
 *
 * This function is a TWI Master write or read transaction.
 *
 * \param twi       Base address of the TWI (i.e. &TWI_t).
 * \param package   Package information and data
 *                  (see \ref twi_package_t)
 * \param read      Selects the transfer direction
 *
 * \return  status_code_t
 *      - STATUS_OK if the transfer completes
 *      - ERR_BUSY to indicate an unavailable bus
 *      - ERR_IO_ERROR to indicate a bus transaction error
 *      - ERR_NO_MEMORY to indicate buffer errors
 *      - ERR_PROTOCOL to indicate an unexpected bus state
 *      - ERR_INVALID_ARG to indicate invalid arguments.
 */
  status_code_t twi_master_transfer (TWI_t * twi,
				     const twi_package_t * package,
				     bool read);

/*! \brief Read multiple bytes from a TWI compatible slave device
 *
 * \param twi       Base address of the TWI (i.e. &TWI_t).
 * \param package   Package information and data
 *                  (see \ref twi_package_t)
 * \return STATUS_OK   If all bytes were read, error code otherwise
 */
  static inline status_code_t twi_master_read (TWI_t * twi,
					       const twi_package_t * package)
  {
    return twi_master_transfer (twi, package, true);
  }

/*! \brief Write multiple bytes to a TWI compatible slave device
 *
 * \param twi       Base address of the TWI (i.e. &TWI_t).
 * \param package   Package information and data
 *                  (see \ref twi_package_t)
 * \return STATUS_OK   If all bytes were written, error code otherwise
 */
  static inline status_code_t twi_master_write (TWI_t * twi,
						const twi_package_t * package)
  {
    return twi_master_transfer (twi, package, false);
  }

/*! \brief Enable Master Mode of the TWI.
 *
 * \param twi       Base address of the TWI instance.
 */
  static inline void twi_master_enable (TWI_t * twi)
  {
    twi->MASTER.CTRLA |= TWI_MASTER_ENABLE_bm;
  }

/*! \brief Disable Master Mode of the TWI.
 *
 * \param twi       Base address of the TWI instance.
 */
  static inline void twi_master_disable (TWI_t * twi)
  {
    twi->MASTER.CTRLA &= (~TWI_MASTER_ENABLE_bm);
  }


#ifdef __cplusplus
}
#endif

/**
 * \}
 */

#endif // _TWIM_H_
