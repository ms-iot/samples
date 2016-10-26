	/**
 * \file
 *
 * \brief AVR XMEGA 32-bit Real Time Counter driver definitions
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
#ifndef DRIVERS_RTC32_RTC32_H
#define DRIVERS_RTC32_RTC32_H

#include <compiler.h>
#include <conf_rtc32.h>

/**
 * \defgroup rtc32_group 32-bit Real Time Counter (RTC32)
 *
 * See \ref xmega_rtc32_quickstart.
 *
 * This is a driver implementation for the XMEGA RTC32.
 *
 * This driver can be used to keep track of time; setting alarms, with or
 * without function callbacks; initializing and checking the battery backup
 * system.
 *
 * \section rtc32_min_alarm_time Minimum allowed alarm time
 *
 * Due to the RTC32 clock synchronization, there is a minimum alarm time that
 * will generate a interrupt. This minimum time is 2 RTC32 clock cycles.
 *
 * Also, if a new RTC32 clock cycle is imminent at the time of setting the
 * alarm, there is a risk that it will be missed even with the value 2. If there
 * is a risk that this may occur, it is recommended to use a minimum alarm time
 * of 3.
 *
 * @{
 */

/**
 * \def CONFIG_RTC32_COMPARE_INT_LEVEL
 * \brief Configuration symbol for interrupt level to use on alarm
 *
 * Define this in \ref conf_rtc32.h as the desired interrupt level, or leave it
 * undefined to use the default.
 */
#ifdef __DOXYGEN__
# define CONFIG_RTC32_COMPARE_INT_LEVEL
#endif

/**
 * \def CONFIG_RTC32_CLOCK_1024HZ
 * \brief Configuration symbol for selecting 1024Hz clock instead of 1Hz
 *
 * Define this in \ref conf_rtc32.h if 1024Hz clock is desired. Otherwise, leave
 * it undefined.
 */
#ifdef __DOXYGEN__
# define CONFIG_RTC32_CLOCK_1024HZ
#endif

//! \brief Battery backup system status codes
enum vbat_status_code
{
/**
 * \brief Backup system is operating and no errors were detected.
 *
 * The backup system is configured and had no issues while main power was
 * lost. Hence, all data stored in the backup domain is valid.
 */
  VBAT_STATUS_OK,

/**
 *  \brief No power detected on VBAT.
 *
 * No power was detected on the VBAT pin and therefore all data within the
 * backup system is invalid.
 *
 * The voltage on the VBAT pin is only sampled after a POR of the device,
 * therefore it is not possible to detect any voltage loss on the VBAT pin
 * during normal operation of the device.
 */
  VBAT_STATUS_NO_POWER,

/**
 *  \brief The backup system must be initialized.
 *
 * A POR was detected on VBAT input, indicating that a supply was connected to
 * the VBAT pin. Since this is also the first start-up of the device, it is
 * necessary to initialize the RTC32.
 */
  VBAT_STATUS_INIT,

/**
 *  \brief A POR was detected on the VBAT input.
 *
 * POR detection also works while the VBAT system is powered from main power,
 * but the detection flag is only latched after a POR of the main system.
 * A POR can happen when the power is lost and restored again on the VBAT pin
 * while main power was also not present, or even when main power was present,
 * but in this case the flag will only be latched after the next POR of the main
 * system.
 * If a POR is detected on VBAT, it should always be treated as if the backup
 * system is in an unknown state, i.e., that all data is invalid.
 */
  VBAT_STATUS_BBPOR,

/**
 * \brief A brown-out was detected on the VBAT input.
 *
 * The backup system is in an unknown state and therefore the time in the RTC32
 * is invalid. This can happen when the voltage on VBAT drops below the
 * brown-out detection level while main power is absent.
 */
  VBAT_STATUS_BBBOD,

/**
 * \brief A failure was detected on the oscillator.
 *
 * The oscillator stopped for at least TBD period of time and because of that
 * we can not rely on the RTC time any more.
 *
 * \todo Determine minimum period for detection of oscillator outage.
 */
  VBAT_STATUS_XOSCFAIL,

};

enum vbat_status_code rtc_vbat_system_check (bool first_time_init);

/**
 * \brief Callback definition for alarm callback
 *
 * \param time The time of the alarm
 */
typedef void (*rtc_callback_t) (uint32_t time);

void rtc_set_callback (rtc_callback_t callback);
void rtc_set_time (uint32_t time);
uint32_t rtc_get_time (void);
void rtc_set_alarm (uint32_t time);
bool rtc_alarm_has_triggered (void);

/**
 * \brief Set alarm relative to current time
 *
 * \param offset Offset to current time. This is minimum value, so the alarm
 * might happen at up to one time unit later. See also \ref
 * rtc32_min_alarm_time
 */
static inline void
rtc_set_alarm_relative (uint32_t offset)
{
  Assert (offset >= 2);

  rtc_set_alarm (rtc_get_time () + offset);
}

void rtc_init (void);

//! @}

/**
 * \page xmega_rtc32_quickstart Quick start guide for RTC32 driver
 *
 * This is the quick start guide for the \ref rtc32_group "RTC32 driver", with
 * step-by-step instructions on how to configure and use the drivers in a
 * selection of use cases.
 *
 * The use cases contain several code fragments. The code fragments in the
 * steps for setup can be copied into a custom initialization function, while
 * the steps for usage can be copied into, e.g., the main application function.
 *
 * \section rtc32_basic_use_case Basic use case
 *
 * \section rtc32_basic_use_case_setup Setup steps
 *
 * \subsection rtc32_basic_use_case_setup_code Example code
 * Add to the initialization code:
 * \code
 *    sysclk_init();
 *    rtc_init();
 * \endcode
 *
 * \subsection rtc32_basic_use_case_setup_flow Workflow
 * -# Ensure that conf_rtc.h is present for the driver.
 *   - \note This configuration file is used by the driver and
 * should not be included by the user.
 * -# Initialize system clock:
 *   - \code sysclk_init(); \endcode
 * -# Call RTC32 driver's own init function to initialize the 32kHz oscillator
 * and RTC32:
 *   - \code rtc_init(); \endcode
 *
 * \section rtc32_basic_use_case_usage Usage steps
 *
 * \subsection rtc32_basic_use_case_usage_code Example code
 * Add to, e.g., main loop in application C-file:
 * \code
 *    rtc_get_time();
 * \endcode
 *
 * \subsection rtc32_basic_use_case_usage_flow Workflow
 * -# Get current time of the RTC32:
 *   - \code rtc_get_time(); \endcode
 *
 * \section rtc32_use_cases Advanced use cases
 * For more advanced use of the RTC32 driver, see the following use cases:
 * - \subpage rtc32_use_case_1 : 
 */

/**
 * \page rtc32_use_case_1 Use case #1
 *
 * This use case shows how to set an alarm for the RTC32.
 *
 * \section rtc32_use_case_1_setup Setup steps
 *
 * \subsection rtc32_basic_use_case_setup_prereq Prerequisites
 * For the setup code of this use case to work, the following must
 * be added to the project:
 * -# PMIC for interrupt handling.
 * -# Sleep Manager.
 * -# A \ref rtc_callback_t "callback" function, called alarm, that
 * reschedules the alarm must be provided
 * by the user.
 * \code
 *   static void alarm(uint32_t time)
 *   {
 *       rtc_set_alarm(2);
 *   }
 * \endcode
 * \note Since the next alarm will be rounded up to the next second pass, this
 * will actually happen in 3 seconds.
 *
 * \subsection rtc32_use_case_1_setup_code Example code
 * Add to application initialization:
 * \code
 *    pmic_init();
 *    sysclk_init();
 *    sleepmgr_init();
 *    rtc_init();
 *    rtc_set_callback(alarm);
 *    cpu_irq_enable();
 * \endcode
 *
 * \subsection rtc32_use_case_1_setup_flow Workflow
 * -# Ensure that conf_rtc32.h is present for the driver.
 *   - \note This configuration file is used by the driver and
 * should not be included by the user.
 * -# Call the init function of the PMIC driver to enable all interrupt levels:
 *   - \code pmic_init(); \endcode
 * -# Initialize system clock:
 *   - \code sysclk_init(); \endcode
 * -# Call the init function of the sleep manager driver to be able to sleep
 * waiting for alarm:
 *   - \code sleepmgr_init(); \endcode
 * -# Call RTC32 driver's own init function to initialize the 32kHz oscillator
 * and RTC32:
 *   - \code rtc_init(); \endcode
 * -# Set callback function to call on alarm:
 *   - \code rtc_set_callback(alarm); \endcode
 *   - \note The callback function alarm must be defined by the user.
 * -# Enable interrupts globally:
 *   - \code cpu_irq_enable(); \endcode
 *
 * \section rtc32_use_case_1_usage Usage steps
 *
 * \subsection rtc32_use_case_1_usage_code Example code
 * \code
 *    rtc_set_alarm_relative(3);
 *    while (true) {
 *        sleepmgr_enter_sleep();
 *    }
 * \endcode
 *
 * \subsection rtc32_use_case_1_usage_flow Workflow
 * -# Set the alarm to trigger on next time unit roll over:
 *   - \code rtc_set_alarm_relative(3); \endcode
 * \note The lowest value which is safe to use is 3. The use of 2 could
 * happen in a second change, and we would not get an interrupt. A
 * value of 3 causes the alarm to be set of in 3-4 seconds.
 * -# Sleep between each triggered alarm:
 *   - \code 
 *      while (true) {
 *          sleepmgr_enter_sleep();
 *      }
 * \endcode
 */

#endif /* DRIVERS_RTC32_RTC32_H */
