/**
 * \file
 *
 * \brief AVR XMEGA WatchDog Timer driver.
 *
 * Copyright (c) 2011-2012 Atmel Corporation. All rights reserved.
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
#ifndef _WDT_H_
#define _WDT_H_

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

#include "compiler.h"

/**
 * \defgroup wdt_group Watchdog Timer (WDT)
 *
 * See \ref wdt_quickstart.
 *
 * This is a driver for configuring, enabling, disabling and use of the on-chip
 * WDT.
 *
 * \section dependencies Dependencies
 *
 * The WDT module depends on the following modules:
 *  - \ref ccp_group for writing in a CCP-protected 8-bit I/O register.
 *
 * @{
 */


//! Watchdog timeout period setting
enum wdt_timeout_period_t {
	//! Timeout period = 8 cycles or 8 ms @ 3.3V
	WDT_TIMEOUT_PERIOD_8CLK = (0x00),
	//! Timeout period = 16 cycles or 16 ms @ 3.3V
	WDT_TIMEOUT_PERIOD_16CLK = (0x01),
	//! Timeout period = 32 cycles or 32m s @ 3.3V
	WDT_TIMEOUT_PERIOD_32CLK = (0x02),
	//! Timeout period = 64 cycles or 64ms @ 3.3V
	WDT_TIMEOUT_PERIOD_64CLK = (0x03),
	//! Timeout period = 125 cycles or 125ms @ 3.3V
	WDT_TIMEOUT_PERIOD_125CLK = (0x04),
	//! 250 cycles or 250ms @ 3.3V)
	WDT_TIMEOUT_PERIOD_250CLK = (0x05),
	//! Timeout period = 500 cycles or 500ms @ 3.3V
	WDT_TIMEOUT_PERIOD_500CLK = (0x06),
	//! Timeout period =1K cycles or 1s @ 3.3V
	WDT_TIMEOUT_PERIOD_1KCLK = (0x07),
	//! Timeout period = 2K cycles or 2s @ 3.3V
	WDT_TIMEOUT_PERIOD_2KCLK = (0x08),
	//! Timeout period = 4K cycles or 4s @ 3.3V
	WDT_TIMEOUT_PERIOD_4KCLK = (0x09),
	//! Timeout period = 8K cycles or 8s @ 3.3V
	WDT_TIMEOUT_PERIOD_8KCLK = (0x0A),
};

//! Watchdog window period setting
enum wdt_window_period_t {
	//! Window period = 8 cycles or 8 ms @ 3.3V
	WDT_WINDOW_PERIOD_8CLK = (0x00),
	//! Window period = 16 cycles or 16 ms @ 3.3V
	WDT_WINDOW_PERIOD_16CLK = (0x01),
	//! Window period = 32 cycles or 32m s @ 3.3V
	WDT_WINDOW_PERIOD_32CLK = (0x02),
	//! Window period = 64 cycles or 64ms @ 3.3V
	WDT_WINDOW_PERIOD_64CLK = (0x03),
	//! Window period = 125 cycles or 125ms @ 3.3V
	WDT_WINDOW_PERIOD_125CLK = (0x04),
	//! 250 cycles or 250ms @ 3.3V)
	WDT_WINDOW_PERIOD_250CLK = (0x05),
	//! Window period = 500 cycles or 500ms @ 3.3V
	WDT_WINDOW_PERIOD_500CLK = (0x06),
	//! Window period =1K cycles or 1s @ 3.3V
	WDT_WINDOW_PERIOD_1KCLK = (0x07),
	//! Window period = 2K cycles or 2s @ 3.3V
	WDT_WINDOW_PERIOD_2KCLK = (0x08),
	//! Window period = 4K cycles or 4s @ 3.3V
	WDT_WINDOW_PERIOD_4KCLK = (0x09),
	//! Window period = 8K cycles or 8s @ 3.3V
	WDT_WINDOW_PERIOD_8KCLK = (0x0A),
};


/*! \brief This macro resets (clears/refreshes) the Watchdog Timer.
 */
#if defined(__GNUC__)
#define wdt_reset() __asm__ __volatile__("wdr");
#elif defined(__ICCAVR__)
#define wdt_reset() __watchdog_reset();
#else
#error Unsupported compiler.
#endif


/*! \brief Wait until WD settings are synchronized to the WD clock domain.
 *
 */
static inline void wdt_wait_while_busy(void)
{
	while ((WDT.STATUS & WDT_SYNCBUSY_bm) == WDT_SYNCBUSY_bm) {
		// Wait until synchronization
	}
}


/*! \brief Check if the Watchdog Enable flag is set.
 *
 *  \retval  false  WDT disabled
 *           true   WDT enabled
 */
static inline bool wdt_is_enabled(void)
{
	return ((WDT.CTRL & WDT_ENABLE_bm) == WDT_ENABLE_bm);
}


/*! \brief Check if the Watchdog Window mode flag is set.
 *
 *  \retval  false  WDT Window disabled
 *           true   WDT Window enabled
 */
static inline bool wdt_window_mode_is_enabled(void)
{
	return ((WDT.WINCTRL & WDT_WEN_bm) == WDT_WEN_bm);
}


/*! \brief Gets the Watchdog timeout period.
 *
 *  This function reads the value of the WDT timeout period.
 *
 *  \retval  The WDT timeout period.
 */
static inline enum wdt_timeout_period_t wdt_get_timeout_period(void)
{
	return ((enum wdt_timeout_period_t)
		((WDT.CTRL & WDT_PER_gm) >> WDT_PER_gp));
}


/*! \brief Gets the Watchdog window period.
 *
 *  This function reads the value of the WDT closed window coded period.
 *
 *  \retval  The WDT window period.
 */
static inline enum wdt_window_period_t wdt_get_window_period(void)
{
	return ((enum wdt_window_period_t)
		((WDT.WINCTRL & WDT_WPER_gm) >> WDT_WPER_gp));
}


/*! \brief Set Watchdog timeout period.
 *
 *  This function sets the coded field of the WDT timeout period.
 *
 *  The function writes the correct signature to the Configuration
 *  Change Protection register before writing the CTRL register. Interrupts are
 *  automatically ignored during the change enable period. The function will
 *  wait for the WDT to be synchronized to the WDT clock domain before
 *  proceeding
 *
 *  \param  to_period  WDT timeout coded period
 */
void wdt_set_timeout_period(enum wdt_timeout_period_t to_period);


/*! \brief Set Watchdog window period.
 *
 *  This function sets the coded field of the WDT closed window period.
 *  Note that this setting is available only if the WDT is enabled (hardware
 *  behaviour relayed by software).
 *
 *  The function writes the correct signature to the Configuration
 *  Change Protection register before writing the WINCTRL register. Interrupts
 *  are automatically ignored during the change enable period. The function will
 *  wait for the WDT to be synchronized to the WDT clock domain before
 *  proceeding
 *
 *  \param  win_period  Window coded period
 *
 *  \retval  true   The WDT was enabled and the setting is done.
 *           false  The WDT is disabled and the setting is discarded.
 */
bool wdt_set_window_period(enum wdt_window_period_t win_period);


/*! \brief Disable Watchdog.
 *
 *  This function disables the WDT without changing period settings.
 *
 *  The function writes the correct signature to the Configuration
 *  Change Protection register before writing the CTRL register. Interrupts are
 *  automatically ignored during the change enable period. Disable functions
 *  operate asynchronously with immediate effect.
 */
void wdt_disable(void);


/*! \brief Enable Watchdog.
 *
 *  This function enables the WDT without changing period settings.
 *
 *  The function writes the correct signature to the Configuration
 *  Change Protection register before writing the CTRL register. Interrupts are
 *  automatically ignored during the change enable period. The function will
 *  wait for the WDT to be synchronized to the WDT clock domain before
 *  proceeding
 */
void wdt_enable(void);


/*! \brief Disable Watchdog window mode without changing period settings.
 *
 *  This function disables the WDT window mode without changing period settings.
 *
 *  The function writes the correct signature to the Configuration
 *  Change Protection register before writing the WINCTRL register. Interrupts
 *  are automatically ignored during the change enable period. Disable functions
 *  operate asynchronously with immediate effect.
 *
 *  \retval  true   The WDT was enabled and the window mode is disabled.
 *           false  The WDT (& the window mode) is already disabled.
 */
bool wdt_disable_window_mode(void);


/*! \brief Enable Watchdog window mode.
 *
 *  This function enables the WDT window mode without changing period settings.
 *
 *  The function writes the correct signature to the Configuration
 *  Change Protection register before writing the WINCTRL register. Interrupts
 *  are automatically ignored during the change enable period. The function will
 *  wait for the WDT to be synchronized to the WDT clock domain before
 *  proceeding
 *
 *  \retval  true   The WDT was enabled and the setting is done.
 *           false  The WDT is disabled and the setting is discarded.
 */
bool wdt_enable_window_mode(void);


/*! \brief Reset MCU via Watchdog.
 *
 *  This function generates an hardware microcontroller reset using the WDT.
 *
 *  The function loads enables the WDT in window mode. Executing a "wdr" asm
 *  instruction when the windows is closed, provides a quick mcu reset.
 *
 */
void wdt_reset_mcu(void);


//! @}

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond

/**
 * \page wdt_quickstart Quick start guide for WDT driver
 *
 * This is the quick start guide for the \ref wdt_group, with
 * step-by-step instructions on how to configure and use the driver in a
 * selection of use cases.
 *
 * The use cases contain several code fragments. The code fragments in the
 * steps for setup can be copied into a custom initialization function, while
 * the steps for usage can be copied into, e.g., the main application function.
 *
 * \section wdt_basic_use_case Basic use case
 * \section wdt_use_cases WDT use cases
 * - \ref wdt_basic_use_case
 * - \subpage wdt_use_case_1
 *
 * \section wdt_basic_use_case Basic use case - Reset WDT in standard mode
 * In this use case, the WDT is configured for:
 * - Standard mode
 * - Timeout period of 8 ms
 *
 * The use case enables the WDT, and resets it after 5 ms to prevent system
 * reset after time out period of 8 ms.
 *
 * \section wdt_basic_use_case_setup Setup steps
 *
 * \subsection wdt_basic_use_case_setup_prereq Prerequisites
 * For the setup code of this use case to work, the following must
 * be added to the project:
 * -# \ref group_common_services_delay "Busy-Wait Delay Routines"
 *
 * \subsection wdt_basic_use_case_setup_code Example code
 * Add to application initialization:
 * \code
 *    wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_8CLK);
 *    wdt_enable();
 * \endcode
 *
 * \subsection wdt_basic_use_case_setup_flow Workflow
 * -# Set timeout period to 8 cycles or 8 ms:
 *   - \code wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_8CLK); \endcode
 * -# Enable WDT:
 *   - \code wdt_enable(); \endcode
 * \section wdt_basic_use_case_usage Usage steps
 *
 * \subsection wdt_basic_use_case_usage_code Example code
 * Add to, e.g., main loop in application C-file:
 * \code
 *    delay_ms(5);
 *    wdt_reset();
 * \endcode
 *
 * \subsection wdt_basic_use_case_usage_flow Workflow
 * -# Wait for 5 ms:
 *   - \code delay_ms(5); \endcode
 * -# Reset the WDT before the timeout period is over to prevent system reset:
 *   - \code wdt_reset(); \endcode
 */

/**
 * \page wdt_use_case_1 Reset WDT in window mode
 *
 * In this use case, the WDT is configured for:
 * - Window mode
 * - Timeout period of 16 ms
 *
 * The use case enables the WDT in window mode, and resets it after 10 ms to
 * prevent system reset before window timeout after 8 ms and after time out
 * period of 16 ms.
 *
 * \section wdt_use_case_1_setup Setup steps
 *
 * \subsection usart_use_case_1_setup_prereq Prerequisites
 * For the setup code of this use case to work, the following must
 * be added to the project:
 * -# \ref group_common_services_delay "Busy-Wait Delay Routines"
 *
 * \subsection wdt_use_case_1_setup_code Example code
 * Add to application initialization:
 * \code
 *    wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_16CLK);
 *    wdt_enable();
 *    wdt_set_window_period(WDT_TIMEOUT_PERIOD_8CLK);
 *    wdt_enable_window_mode();
 * \endcode
 *
 * \subsection wdt_use_case_1_setup_flow Workflow
 * -# Set timeout period to 16 cycles or 16 ms:
 *   - \code wdt_set_timeout_period(WDT_TIMEOUT_PERIOD_16CLK); \endcode
 * -# Enable WDT:
 *   - \code wdt_enable(); \endcode
 * -# Set window period to 8 cycles or 8 ms:
 *   - \code wdt_set_window_period(WDT_TIMEOUT_PERIOD_8CLK); \endcode
 * -# Enable window mode:
 *   - \code wdt_enable_window_mode(); \endcode
 *
 * \section wdt_use_case_1_usage Usage steps
 *
 * \subsection wdt_use_case_1_usage_code Example code
 * Add to, e.g., main loop in application C-file:
 * \code
 *    delay_ms(10);
 *    wdt_reset();
 * \endcode
 *
 * \subsection wdt_use_case_1_usage_flow Workflow
 * -# Wait for 10 ms to not reset the WDT before window timeout:
 *   - \code delay_ms(10); \endcode
 * -# Reset the WDT before the timeout period is over to prevent system reset:
 *   - \code wdt_reset(); \endcode
 */

#endif // _WDT_H_
