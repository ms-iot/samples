/**
 * \file *********************************************************************
 *
 * \brief  XMEGA TWI slave driver header file.
 *
 *      This file contains the function prototypes and enumerator definitions
 *      for various configuration parameters for the XMEGA TWI slave driver.
 *
 *      The driver is not intended for size and/or speed critical code, since
 *      most functions are just a few lines of code, and the function call
 *      overhead would decrease code performance. The driver is intended for
 *      rapid prototyping and documentation purposes for getting started with
 *      the XMEGA TWI slave module.
 *
 *      For size and/or speed critical code, it is recommended to copy the
 *      function contents directly into your application instead of making
 *      a function call.
 *
 * \par Application note:
 *      AVR1307: Using the XMEGA TWI
 *
 * \par Documentation
 *      For comprehensive code documentation, supported compilers, compiler
 *      settings and supported devices see readme.html
 *
 *      Atmel Corporation: http://www.atmel.com \n
 *
 * $Revision: 1569 $
 * $Date: 2008-04-22 13:03:43 +0200 (Tue, 22 Apr 2008) $  \n
 *
 * Copyright (c) 2008 Atmel Corporation. All rights reserved.
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
 *****************************************************************************/
#ifndef TWIS_H
#define TWIS_H

/**
 * \defgroup group_xmega_drivers_twi_twis TWI Slave
 *
 * \ingroup group_xmega_drivers_twi
 *
 * \{
 */

#include "compiler.h"
#include "twi_common.h"

/*! Baud register setting calculation. Formula described in datasheet. */
#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5)

/* Transaction status defines.*/
#define TWIS_STATUS_READY                0
#define TWIS_STATUS_BUSY                 1

/* Transaction result enumeration */
typedef enum TWIS_RESULT_enum
{
  TWIS_RESULT_UNKNOWN = (0x00 << 0),
  TWIS_RESULT_OK = (0x01 << 0),
  TWIS_RESULT_BUFFER_OVERFLOW = (0x02 << 0),
  TWIS_RESULT_TRANSMIT_COLLISION = (0x03 << 0),
  TWIS_RESULT_BUS_ERROR = (0x04 << 0),
  TWIS_RESULT_FAIL = (0x05 << 0),
  TWIS_RESULT_ABORTED = (0x06 << 0),
}
TWIS_RESULT_t;

/* Buffer size defines. */
#define TWIS_RECEIVE_BUFFER_SIZE         8
#define TWIS_SEND_BUFFER_SIZE            8



/*! \brief TWI slave driver struct.
 *
 *  TWI slave struct. Holds pointer to TWI module and data processing routine,
 *  buffers and necessary variables.
 */
typedef struct TWI_Slave
{
  TWI_t *interface;		/*!< Pointer to what interface to use */
  void (*Process_Data) (void);	/*!< Pointer to process data function */
  register8_t receivedData[TWIS_RECEIVE_BUFFER_SIZE];	/*!< Read data */
  register8_t sendData[TWIS_SEND_BUFFER_SIZE];	/*!< Data to write */
  register8_t bytesReceived;	/*!< Number of bytes received */
  register8_t bytesSent;	/*!< Number of bytes sent */
  register8_t status;		/*!< Status of transaction */
  register8_t result;		/*!< Result of transaction */
  bool abort;			/*!< Strobe to abort */
}
TWI_Slave_t;


void TWI_SlaveInitializeDriver (TWI_Slave_t * twi,
				TWI_t * module,
				void (*processDataFunction) (void));

void TWI_SlaveInitializeModule (TWI_Slave_t * twi,
				uint8_t address, TWI_SLAVE_INTLVL_t intLevel);

void TWI_SlaveInterruptHandler (TWI_Slave_t * twi);
void TWI_SlaveAddressMatchHandler (TWI_Slave_t * twi);
void TWI_SlaveStopHandler (TWI_Slave_t * twi);
void TWI_SlaveDataHandler (TWI_Slave_t * twi);
void TWI_SlaveReadHandler (TWI_Slave_t * twi);
void TWI_SlaveWriteHandler (TWI_Slave_t * twi);
void TWI_SlaveTransactionFinished (TWI_Slave_t * twi, uint8_t result);


/*! TWI slave interrupt service routine.
 *
 *  Interrupt service routine for the TWI slave. Copy the interrupt vector
 *  into your code if needed.
 *
     ISR(TWIC_TWIS_vect)
    {
      TWI_SlaveInterruptHandler(&twiSlaveC);
    }
 *
 */
/*! \brief Enable Slave Mode of the TWI.
 *
 * \param twi         Base address of the TWI instance.
 */
static inline void
twi_slave_enable (TWI_t * twi)
{
  twi->SLAVE.CTRLA |= TWI_SLAVE_ENABLE_bm;
}

/*! \brief Disable Slave Mode of the TWI.
 *
 * \param twi         Base address of the TWI instance.
 */
static inline void
twi_slave_disable (TWI_t * twi)
{
  twi->SLAVE.CTRLA &= (~TWI_SLAVE_ENABLE_bm);
}

/**
 * \}
 */

#endif /* TWIS_H */
