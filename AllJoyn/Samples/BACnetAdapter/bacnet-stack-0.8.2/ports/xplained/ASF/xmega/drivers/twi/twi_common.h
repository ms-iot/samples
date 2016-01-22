/**
 * \file
 *
 * \brief AVR XMEGA TWI driver common definitions
 *
 * Copyright (c) 2011-2013 Atmel Corporation. All rights reserved.
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

#ifndef TWI_COMMON_H
#define TWI_COMMON_H

/* Fix header error in iox32e5.h */
#ifndef TWI_BRIDGEEN_bm
#define TWI_BRIDGEEN_bm  0x80	/* Bridge Enable bit mask. */
#endif

#ifndef TWI_BRIDGEEN_bp
#define TWI_BRIDGEEN_bp  7	/* Bridge Enable bit position. */
#endif

#ifndef TWI_SFMPEN_bm
#define TWI_SFMPEN_bm  0x40	/* Slave Fast Mode Plus Enable bit mask. */
#endif

#ifndef TWI_SFMPEN_bp
#define TWI_SFMPEN_bp  6	/* Slave Fast Mode Plus Enable bit position. */
#endif
/* End of: Fix header error in iox32e5.h */

/**
 * \defgroup group_xmega_drivers_twi TWI - Two-Wire Interface
 *
 * See \ref xmega_twi_quickstart
 *
 * Driver for the Two-Wire Interface (TWI).
 * Provides functions for configuring and using the TWI in both master and
 * slave mode.
 *
 * \section xmega_twi_quickstart_guide Quick start guide
 * See \ref xmega_twi_quickstart
 *
 * \{
 */

/*!
 * \brief Input parameters when initializing the twi module mode
 */
typedef struct
{
  //! The baudrate of the TWI bus.
  unsigned long speed;
  //! The baudrate register value of the TWI bus.
  unsigned long speed_reg;
  //! The desired address.
  char chip;
}
twi_options_t;

/*!
 * \brief Information concerning the data transmission
 */
typedef struct
{
  //! TWI chip address to communicate with.
  char chip;
  //! TWI address/commands to issue to the other chip (node).
  uint8_t addr[3];
  //! Length of the TWI data address segment (1-3 bytes).
  int addr_length;
  //! Where to find the data to be written.
  void *buffer;
  //! How many bytes do we want to write.
  unsigned int length;
  //! Whether to wait if bus is busy (false) or return immediately (true)
  bool no_wait;
}
twi_package_t;

/**
 * \}
 */

/**
 * \page xmega_twi_quickstart Quick start guide for XMEGA TWI driver
 *
 * This is the quick start guide for the
 *\ref group_xmega_drivers_twi "TWI Driver", with step-by-step instructions on
 * how to configure and use the driver for specific use cases.
 *
 * The section described below can be compiled into e.g. the main application
 * loop or any other function that might use the TWI functionality.
 *
 * \section xmega_twi_quickstart_basic Basic use case of the TWI driver
 * In our basic use case, the TWI driver is used to set up internal
 * communication between two TWI modules on the XMEGA A1 Xplained board, since
 * this is the most simple way to show functionality without external
 * dependencies. TWIC is set up in master mode, and TWIF is set up in slave
 * mode, and these are connected together on the board by placing a connection
 * between SDA/SCL on J1 to SDA/SCL on J4.
 *
 * \section xmega_twi_qs_use_cases Specific use case for XMEGA E devices
 * - \subpage xmega_twi_xmegae
 *
 * \section xmega_twi_quickstart_prereq Prerequisites
 * The \ref sysclk_group module is required to enable the clock to the TWI
 * modules. The \ref group_xmega_drivers_twi_twim "TWI Master" driver and
 * \ref group_xmega_drivers_twi_twis "TWI Slave" driver must also be included.
 *
 * \section xmega_twi_quickstart_setup Setup
 * When the \ref sysclk_group module has been included, it must be initialized:
 * \code
 * sysclk_init();
 * \endcode
 *
 * \section xmega_twi_quickstart_use_case Use case
 *
 * \subsection xmega_twi_quickstart_use_case_example_code Example code
 *
 * \code
 * #define TWI_MASTER       TWIC
 * #define TWI_MASTER_PORT  PORTC
 * #define TWI_SLAVE        TWIF
 * #define TWI_SPEED        50000
 * #define TWI_MASTER_ADDR  0x50
 * #define TWI_SLAVE_ADDR   0x60
 *
 * #define DATA_LENGTH     8
 *
 * TWI_Slave_t slave;
 *
 * uint8_t data[DATA_LENGTH] = {
 *     0x0f, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f
 * };
 *
 * uint8_t recv_data[DATA_LENGTH] = {
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
 * };
 *
 * twi_options_t m_options = {
 *     .speed     = TWI_SPEED,
 *     .chip      = TWI_MASTER_ADDR,
 *     .speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI_SPEED)
 * };
 *
 * static void slave_process(void) {
 *     int i;
 *
 *     for(i = 0; i < DATA_LENGTH; i++) {
 *         recv_data[i] = slave.receivedData[i];
 *     }
 * }
 *
 * ISR(TWIF_TWIS_vect) {
 *     TWI_SlaveInterruptHandler(&slave);
 * }
 *
 * void send_and_recv_twi()
 * {
 *     twi_package_t packet = {
 *         .addr_length = 0,
 *         .chip        = TWI_SLAVE_ADDR,
 *         .buffer      = (void *)data,
 *         .length      = DATA_LENGTH,
 *         .no_wait     = false
 *     };
 *
 *     uint8_t i;
 *
 *     TWI_MASTER_PORT.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;
 *     TWI_MASTER_PORT.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;
 *
 *     irq_initialize_vectors();
 *
 *     sysclk_enable_peripheral_clock(&TWI_MASTER);
 *     twi_master_init(&TWI_MASTER, &m_options);
 *     twi_master_enable(&TWI_MASTER);
 *
 *     sysclk_enable_peripheral_clock(&TWI_SLAVE);
 *     TWI_SlaveInitializeDriver(&slave, &TWI_SLAVE, *slave_process);
 *     TWI_SlaveInitializeModule(&slave, TWI_SLAVE_ADDR,
 *             TWI_SLAVE_INTLVL_MED_gc);
 *
 *     for (i = 0; i < TWIS_SEND_BUFFER_SIZE; i++) {
 *         slave.receivedData[i] = 0;
 *     }
 *
 *     cpu_irq_enable();
 *
 *     twi_master_write(&TWI_MASTER, &packet);
 *
 *     do {
 *         // Nothing
 *     } while(slave.result != TWIS_RESULT_OK);
 * }
 * \endcode
 *
 * \subsection xmega_twi_quickstart_use_case_workflow Workflow
 * We first create some definitions. TWI master and slave, speed, and
 * addresses:
 * \code
 * #define TWI_MASTER       TWIC
 * #define TWI_MASTER_PORT  PORTC
 * #define TWI_SLAVE        TWIF
 * #define TWI_SPEED        50000
 * #define TWI_MASTER_ADDR  0x50
 * #define TWI_SLAVE_ADDR   0x60
 *
 * #define DATA_LENGTH     8
 * \endcode
 *
 * We create a handle to contain information about the slave module:
 * \code
 * TWI_Slave_t slave;
 * \endcode
 *
 * We create two variables, one which contains data that will be transmitted,
 * and one which will contain the received data:
 * \code
 * uint8_t data[DATA_LENGTH] = {
 *     0x0f, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f
 * };
 *
 * uint8_t recv_data[DATA_LENGTH] = {
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
 * };
 * \endcode
 *
 * Options for the TWI module initialization procedure are given below:
 * \code
 * twi_options_t m_options = {
 *     .speed     = TWI_SPEED,
 *     .chip      = TWI_MASTER_ADDR,
 *     .speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI_SPEED)
 * };
 * \endcode
 *
 * The TWI slave will fire an interrupt when it has received data, and the
 * function below will be called, which will copy the data from the driver
 * to our recv_data buffer:
 * \code
 * static void slave_process(void) {
 *     int i;
 *
 *     for(i = 0; i < DATA_LENGTH; i++) {
 *         recv_data[i] = slave.receivedData[i];
 *     }
 * }
 * \endcode
 *
 * Set up the interrupt handler:
 * \code
 * ISR(TWIF_TWIS_vect) {
 *     TWI_SlaveInterruptHandler(&slave);
 * }
 * \endcode
 *
 * We create a packet for the data that we will send to the slave TWI:
 * \code
 *     twi_package_t packet = {
 *         .addr_length = 0,
 *         .chip        = TWI_SLAVE_ADDR,
 *         .buffer      = (void *)data,
 *         .length      = DATA_LENGTH,
 *         .no_wait     = false
 *     };
 * \endcode
 *
 * We need to set SDA/SCL pins for the master TWI to be wired and
 * enable pull-up:
 * \code
 *     TWI_MASTER_PORT.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;
 *     TWI_MASTER_PORT.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;
 * \endcode
 *
 * We enable all interrupt levels:
 * \code
 *     irq_initialize_vectors();
 * \endcode
 *
 * We enable the clock to the master module, and initialize it with the
 * options we described before:
 * \code
 *     sysclk_enable_peripheral_clock(&TWI_MASTER);
 *     twi_master_init(&TWI_MASTER, &m_options);
 *     twi_master_enable(&TWI_MASTER);
 * \endcode
 *
 * We do the same for the slave, using the slave portion of the driver,
 * passing through the slave_process function, its address, and set medium
 * interrupt level:
 * \code
 *     sysclk_enable_peripheral_clock(&TWI_SLAVE);
 *     TWI_SlaveInitializeDriver(&slave, &TWI_SLAVE, *slave_process);
 *     TWI_SlaveInitializeModule(&slave, TWI_SLAVE_ADDR,
 *             TWI_SLAVE_INTLVL_MED_gc);
 * \endcode
 *
 * We zero out the receive buffer in the slave handle:
 * \code
 *     for (i = 0; i < TWIS_SEND_BUFFER_SIZE; i++) {
 *         slave.receivedData[i] = 0;
 *     }
 * \endcode
 *
 * And enable interrupts:
 * \code
 *     cpu_irq_enable();
 * \endcode
 *
 * Finally, we write our packet through the master TWI module:
 * \code
 *     twi_master_write(&TWI_MASTER, &packet);
 * \endcode
 *
 * We wait for the slave to finish receiving:
 * \code
 *     do {
 *         // Waiting
 *     } while(slave.result != TWIS_RESULT_OK);
 * \endcode
 * \note When the slave has finished receiving, the slave_process()
 *       function will copy the received data into our recv_data buffer,
 *       which now contains what was sent through the master.
 *
 */


 /** 
 * \page xmega_twi_xmegae XMEGA E TWI additions with Bridge and Fast Mode Plus
 *
 * XMEGA E TWI module provides two additionnnal features compare to regular 
 * XMEGA TWI module:
 * - Fast Mode Plus communication speed
 * - Bridge Mode
 *
 * The following use case will set up the TWI module to be used in in Fast Mode 
 * Plus together with bridge mode. 
 * This use case is similar to the regular XMEGA TWI initialization, it only
 * differs by the activation of both Bridge and Fast Mode Plus mode.
 *
 * \subsection xmegae_twi_quickstart_use_case_example_code Example code
 *
 * \code
 * #define TWI_MASTER       TWIC
 * #define TWI_MASTER_PORT  PORTC
 * #define TWI_SLAVE        TWIC
 * #define TWI_SPEED        1000000
 * #define TWI_MASTER_ADDR  0x50
 * #define TWI_SLAVE_ADDR   0x50
 *
 * #define DATA_LENGTH     8
 *
 * TWI_Slave_t slave;
 *
 * uint8_t data[DATA_LENGTH] = {
 *     0x0f, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f
 * };
 *
 * uint8_t recv_data[DATA_LENGTH] = {
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
 * };
 *
 * twi_options_t m_options = {
 *     .speed     = TWI_SPEED,
 *     .chip      = TWI_MASTER_ADDR,
 *     .speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI_SPEED)
 * };
 *
 * static void slave_process(void) {
 *     int i;
 *
 *     for(i = 0; i < DATA_LENGTH; i++) {
 *         recv_data[i] = slave.receivedData[i];
 *     }
 * }
 *
 * ISR(TWIC_TWIS_vect) {
 *     TWI_SlaveInterruptHandler(&slave);
 * }
 *
 * void send_and_recv_twi()
 * {
 *     twi_package_t packet = {
 *         .addr_length = 0,
 *         .chip        = TWI_SLAVE_ADDR,
 *         .buffer      = (void *)data,
 *         .length      = DATA_LENGTH,
 *         .no_wait     = false
 *     };
 *
 *     uint8_t i;
 *
 *     TWI_MASTER_PORT.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;
 *     TWI_MASTER_PORT.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;
 *
 *     irq_initialize_vectors();
 *
 *     sysclk_enable_peripheral_clock(&TWI_MASTER);
 *
 *     twi_bridge_enable(&TWI_MASTER);
 *     twi_fast_mode_enable(&TWI_MASTER);
 *     twi_slave_fast_mode_enable(&TWI_SLAVE);
 *
 *     twi_master_init(&TWI_MASTER, &m_options);
 *     twi_master_enable(&TWI_MASTER);
 *
 *     sysclk_enable_peripheral_clock(&TWI_SLAVE);
 *     TWI_SlaveInitializeDriver(&slave, &TWI_SLAVE, *slave_process);
 *     TWI_SlaveInitializeModule(&slave, TWI_SLAVE_ADDR,
 *             TWI_SLAVE_INTLVL_MED_gc);
 *
 *     for (i = 0; i < TWIS_SEND_BUFFER_SIZE; i++) {
 *         slave.receivedData[i] = 0;
 *     }
 *
 *     cpu_irq_enable();
 *
 *     twi_master_write(&TWI_MASTER, &packet);
 *
 *     do {
 *         // Nothing
 *     } while(slave.result != TWIS_RESULT_OK);
 * }
 * \endcode
 *
 * \subsection xmegae_twi_quickstart_use_case_workflow Workflow
 * We first create some definitions. TWI master and slave, speed, and
 * addresses:
 * \code
 * #define TWI_MASTER       TWIC
 * #define TWI_MASTER_PORT  PORTC
 * #define TWI_SLAVE        TWIC
 * #define TWI_SPEED        1000000
 * #define TWI_MASTER_ADDR  0x50
 * #define TWI_SLAVE_ADDR   0x50
 *
 * #define DATA_LENGTH     8
 * \endcode
 *
 * We create a handle to contain information about the slave module:
 * \code
 * TWI_Slave_t slave;
 * \endcode
 *
 * We create two variables, one which contains data that will be transmitted,
 * and one which will contain the received data:
 * \code
 * uint8_t data[DATA_LENGTH] = {
 *     0x0f, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f
 * };
 *
 * uint8_t recv_data[DATA_LENGTH] = {
 *     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
 * };
 * \endcode
 *
 * Options for the TWI module initialization procedure are given below:
 * \code
 * twi_options_t m_options = {
 *     .speed     = TWI_SPEED,
 *     .chip      = TWI_MASTER_ADDR,
 *     .speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), TWI_SPEED)
 * };
 * \endcode
 *
 * The TWI slave will fire an interrupt when it has received data, and the
 * function below will be called, which will copy the data from the driver
 * to our recv_data buffer:
 * \code
 * static void slave_process(void) {
 *     int i;
 *
 *     for(i = 0; i < DATA_LENGTH; i++) {
 *         recv_data[i] = slave.receivedData[i];
 *     }
 * }
 * \endcode
 *
 * Set up the interrupt handler:
 * \code
 * ISR(TWIC_TWIS_vect) {
 *     TWI_SlaveInterruptHandler(&slave);
 * }
 * \endcode
 *
 * We create a packet for the data that we will send to the slave TWI:
 * \code
 *     twi_package_t packet = {
 *         .addr_length = 0,
 *         .chip        = TWI_SLAVE_ADDR,
 *         .buffer      = (void *)data,
 *         .length      = DATA_LENGTH,
 *         .no_wait     = false
 *     };
 * \endcode
 *
 * We need to set SDA/SCL pins for the master TWI to be wired and
 * enable pull-up:
 * \code
 *     TWI_MASTER_PORT.PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;
 *     TWI_MASTER_PORT.PIN1CTRL = PORT_OPC_WIREDANDPULL_gc;
 * \endcode
 *
 * We enable all interrupt levels:
 * \code
 *     irq_initialize_vectors();
 * \endcode
 *
 * We enable the clock to the master module:
 * \code
 *     sysclk_enable_peripheral_clock(&TWI_MASTER);
 * \endcode
 * 
 * We enable the global TWI bridge mode as well as the Fast Mode Plus
 * communication speed for both master and slave:
 * \code
 *     twi_bridge_enable(&TWI_MASTER);
 *     twi_fast_mode_enable(&TWI_MASTER);
 *     twi_slave_fast_mode_enable(&TWI_SLAVE);
 * \endcode
 *
 * Initialize the master module with the options we described before:
 * \code
 *     twi_master_init(&TWI_MASTER, &m_options);
 *     twi_master_enable(&TWI_MASTER);
 * \endcode
 *
 * We do the same for the slave, using the slave portion of the driver,
 * passing through the slave_process function, its address, and set medium
 * interrupt level:
 * \code
 *     sysclk_enable_peripheral_clock(&TWI_SLAVE);
 *     TWI_SlaveInitializeDriver(&slave, &TWI_SLAVE, *slave_process);
 *     TWI_SlaveInitializeModule(&slave, TWI_SLAVE_ADDR,
 *             TWI_SLAVE_INTLVL_MED_gc);
 * \endcode
 *
 * We zero out the receive buffer in the slave handle:
 * \code
 *     for (i = 0; i < TWIS_SEND_BUFFER_SIZE; i++) {
 *         slave.receivedData[i] = 0;
 *     }
 * \endcode
 *
 * And enable interrupts:
 * \code
 *     cpu_irq_enable();
 * \endcode
 *
 * Finally, we write our packet through the master TWI module:
 * \code
 *     twi_master_write(&TWI_MASTER, &packet);
 * \endcode
 *
 * We wait for the slave to finish receiving:
 * \code
 *     do {
 *         // Waiting
 *     } while(slave.result != TWIS_RESULT_OK);
 * \endcode
 * \note When the slave has finished receiving, the slave_process()
 *       function will copy the received data into our recv_data buffer,
 *       which now contains what was sent through the master.
 * 
 */


#if XMEGA_E

/*! \brief Enable bridge mode on TWIC.
 * SDA and SCL are on PORTC for Master and on PORTD for slave
 *
 * \param twi       Base address of the TWI instance.
 */
static inline void
twi_bridge_enable (TWI_t * twi)
{
  twi->CTRL |= TWI_BRIDGEEN_bm;
}

/*! \brief Disable bridge mode on TWIC.
 *
 * \param twi       Base address of the TWI instance.
 */
static inline void
twi_bridge_disable (TWI_t * twi)
{
  twi->CTRL &= (~TWI_BRIDGEEN_bm);
}


/*! \brief Enable Fast mode plus on TWIC (1MHz).
 * FMPEN bit enables 1MHz on master and slave.
 * In bridge mode, it enables only 1MHz on master.
 *
 * \param twi       Base address of the TWI instance.
 */
static inline void
twi_fast_mode_enable (TWI_t * twi)
{
  twi->CTRL |= TWI_FMPEN_bm;
}

/*! \brief Disable Fast mode plus on TWIC (1MHz).
 *
 * \param twi       Base address of the TWI instance.
 */
static inline void
twi_fast_mode_disable (TWI_t * twi)
{
  twi->CTRL &= (~TWI_FMPEN_bm);
}

/*! \brief Enable Fast mode plus for slave.
 * If set in bridge mode, it enables 1MHz on slave.
 *
 * \param twi       Base address of the TWI instance.
 */
static inline void
twi_slave_fast_mode_enable (TWI_t * twi)
{
  twi->CTRL |= TWI_SFMPEN_bm;
}

/*! \brief Disable Fast mode plus for slave.
 * If reset in bridge mode, it disables 1MHz on slave.
 *
 * \param twi       Base address of the TWI instance.
 */
static inline void
twi_slave_fast_mode_disable (TWI_t * twi)
{
  twi->CTRL &= (~TWI_SFMPEN_bm);
}
#endif

#endif // TWI_COMMON_H
