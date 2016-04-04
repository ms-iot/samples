/**
 * \file
 *
 * \brief XMEGA TWI master source file.
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


#include "twim.h"


/* Master Transfer Descriptor */

static struct {
    TWI_t *bus; // Bus register interface
    twi_package_t *pkg; // Bus message descriptor
    int addr_count;     // Bus transfer address data counter
    unsigned int data_count;    // Bus transfer payload data counter
    bool read;  // Bus transfer direction
    bool locked;        // Bus busy or unavailable
    volatile status_code_t status;      // Transfer status

} transfer;


/**
 * \internal
 *
 * \brief TWI Master Interrupt Vectors
 *
 * The TWI master interrupt request entry points are conditionally compiled
 * for the TWI interfaces supported by the XMEGA MCU variant.  All of these
 * entry points call a common service function, twim_interrupt_handler(),
 * to handle bus events.  This handler uses the bus interface and message
 * parameters specified in the global \c transfer structure.
 */
static void twim_interrupt_handler(void);

#ifdef TWIC
ISR(TWIC_TWIM_vect)
{
    twim_interrupt_handler();
}
#endif
#ifdef TWID
ISR(TWID_TWIM_vect)
{
    twim_interrupt_handler();
}
#endif
#ifdef TWIE
ISR(TWIE_TWIM_vect)
{
    twim_interrupt_handler();
}
#endif
#ifdef TWIF
ISR(TWIF_TWIM_vect)
{
    twim_interrupt_handler();
}
#endif

/**
 * \internal
 *
 * \brief Test for an idle bus state.
 *
 *  Software can determine the TWI master bus state (unknown, idle, owner, or
 *  busy) by reading the bus master status register:
 *
 *          TWI_MASTER_BUSSTATE_UNKNOWN_gc Bus state is unknown.
 *          TWI_MASTER_BUSSTATE_IDLE_gc    Bus state is idle.
 *          TWI_MASTER_BUSSTATE_OWNER_gc   Bus state is owned by the master.
 *          TWI_MASTER_BUSSTATE_BUSY_gc    Bus state is busy.
 *
 * \param   twi     Base address of the TWI (i.e. &TWI_t).
 *
 * \retval  true    The bus is currently idle.
 * \retval  false   The bus is currently busy.
 */
static inline bool twim_idle(const TWI_t * twi)
{

    return ((twi->MASTER.STATUS & TWI_MASTER_BUSSTATE_gm)
        == TWI_MASTER_BUSSTATE_IDLE_gc);
}

/**
 * \internal
 *
 * \brief Get exclusive access to global TWI resources.
 *
 * Wait to acquire bus hardware interface and ISR variables.
 *
 * \param no_wait  Set \c true to return instead of doing busy-wait (spin-lock).
 *
 * \return STATUS_OK if the bus is acquired, else ERR_BUSY.
 */
static inline status_code_t twim_acquire(bool no_wait)
{
    while (transfer.locked) {

        if (no_wait) {
            return ERR_BUSY;
        }
    }

    irqflags_t const flags = cpu_irq_save();

    transfer.locked = true;
    transfer.status = OPERATION_IN_PROGRESS;

    cpu_irq_restore(flags);

    return STATUS_OK;
}

/**
 * \internal
 *
 * \brief Release exclusive access to global TWI resources.
 *
 * Release bus hardware interface and ISR variables previously locked by
 * a call to \ref twim_acquire().  This function will busy-wait for
 * pending driver operations to complete.
 *
 * \return  status_code_t
 *      - STATUS_OK if the transfer completes
 *      - ERR_BUSY to indicate an unavailable bus
 *      - ERR_IO_ERROR to indicate a bus transaction error
 *      - ERR_NO_MEMORY to indicate buffer errors
 *      - ERR_PROTOCOL to indicate an unexpected bus state
 */
static inline status_code_t twim_release(void)
{
    /* First wait for the driver event handler to indicate something
     * other than a transfer in-progress, then test the bus interface
     * for an Idle bus state.
     */
    while (OPERATION_IN_PROGRESS == transfer.status);

    while (!twim_idle(transfer.bus)) {
        barrier();
    }

    status_code_t const status = transfer.status;

    transfer.locked = false;

    return status;
}

/**
 * \internal
 *
 * \brief TWI master write interrupt handler.
 *
 *  Handles TWI transactions (master write) and responses to (N)ACK.
 */
static inline void twim_write_handler(void)
{
    TWI_t *const bus = transfer.bus;
    twi_package_t *const pkg = transfer.pkg;

    if (transfer.addr_count < pkg->addr_length) {

        const uint8_t *const data = pkg->addr;
        bus->MASTER.DATA = data[transfer.addr_count++];

    } else if (transfer.data_count < pkg->length) {

        if (transfer.read) {

            /* Send repeated START condition (Address|R/W=1). */

            bus->MASTER.ADDR |= 0x01;

        } else {
            const uint8_t *const data = pkg->buffer;
            bus->MASTER.DATA = data[transfer.data_count++];
        }

    } else {

        /* Send STOP condition to complete the transaction. */

        bus->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        transfer.status = STATUS_OK;
    }
}

/**
 * \internal
 *
 * \brief TWI master read interrupt handler.
 *
 *  This is the master read interrupt handler that takes care of
 *  reading bytes from the TWI slave.
 */
static inline void twim_read_handler(void)
{
    TWI_t *const bus = transfer.bus;
    twi_package_t *const pkg = transfer.pkg;

    if (transfer.data_count < pkg->length) {

        uint8_t *const data = pkg->buffer;
        data[transfer.data_count++] = bus->MASTER.DATA;

        /* If there is more to read, issue ACK and start a byte read.
         * Otherwise, issue NACK and STOP to complete the transaction.
         */
        if (transfer.data_count < pkg->length) {

            bus->MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;

        } else {

            bus->MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
            transfer.status = STATUS_OK;
        }

    } else {

        /* Issue STOP and buffer overflow condition. */

        bus->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        transfer.status = ERR_NO_MEMORY;
    }
}

/**
 * \internal
 *
 * \brief Common TWI master interrupt service routine.
 *
 *  Check current status and calls the appropriate handler.
 */
static void twim_interrupt_handler(void)
{
    uint8_t const master_status = transfer.bus->MASTER.STATUS;

    if (master_status & TWI_MASTER_ARBLOST_bm) {

        transfer.bus->MASTER.STATUS = master_status | TWI_MASTER_ARBLOST_bm;
        transfer.bus->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        transfer.status = ERR_BUSY;

    } else if ((master_status & TWI_MASTER_BUSERR_bm) ||
        (master_status & TWI_MASTER_RXACK_bm)) {

        transfer.bus->MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        transfer.status = ERR_IO_ERROR;

    } else if (master_status & TWI_MASTER_WIF_bm) {

        twim_write_handler();

    } else if (master_status & TWI_MASTER_RIF_bm) {

        twim_read_handler();

    } else {

        transfer.status = ERR_PROTOCOL;
    }
}

/**
 * \brief Initialize the twi master module
 *
 * \param twi       Base address of the TWI (i.e. &TWIC).
 * \param *opt      Options for initializing the twi module
 *                  (see \ref twi_options_t)
 * \retval STATUS_OK        Transaction is successful
 * \retval ERR_INVALID_ARG  Invalid arguments in \c opt.
 */
status_code_t twi_master_init(TWI_t * twi,
    const twi_options_t * opt)
{
    uint8_t const ctrla =
        CONF_TWIM_INTLVL | TWI_MASTER_RIEN_bm | TWI_MASTER_WIEN_bm |
        TWI_MASTER_ENABLE_bm;

    twi->MASTER.BAUD = opt->speed_reg;
    twi->MASTER.CTRLA = ctrla;
    twi->MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;

    transfer.locked = false;
    transfer.status = STATUS_OK;

    /* Enable configured PMIC interrupt level. */

    PMIC.CTRL |= CONF_PMIC_INTLVL;

    cpu_irq_enable();

    return STATUS_OK;
}

/**
 * \brief Perform a TWI master write or read transfer.
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
status_code_t twi_master_transfer(TWI_t * twi,
    const twi_package_t * package,
    bool read)
{
    /* Do a sanity check on the arguments. */

    if ((twi == NULL) || (package == NULL)) {
        return ERR_INVALID_ARG;
    }

    /* Initiate a transaction when the bus is ready. */

    status_code_t status = twim_acquire(package->no_wait);

    if (STATUS_OK == status) {
        transfer.bus = (TWI_t *) twi;
        transfer.pkg = (twi_package_t *) package;
        transfer.addr_count = 0;
        transfer.data_count = 0;
        transfer.read = read;

        uint8_t const chip = (package->chip) << 1;

        if (package->addr_length || (false == read)) {
            transfer.bus->MASTER.ADDR = chip;
        } else if (read) {
            transfer.bus->MASTER.ADDR = chip | 0x01;
        }

        status = twim_release();
    }

    return status;
}
