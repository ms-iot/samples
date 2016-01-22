/**
 * \file timeout.h
 *
 * \brief Timeout service for XMEGA
 *
 * Copyright (C) 2011-2012 Atmel Corporation. All rights reserved.
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
#ifndef TIMEOUT_H
#define TIMEOUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <compiler.h>
#include <asf.h>
#include "conf_timeout.h"

/**
 * \defgroup timeout_group Timeout service XMEGA
 *
 * See \ref timeout_quickstart.
 *
 * The timeout service uses the asynchronous RTC/RTC32 in order to have a
 * system tick. Typical tick rate is 1-1000Hz. Clock sources available:
 * - Internal 32kHz ULP oscillator
 * - Internal 32kHz calibrated RC oscillator
 * - External 32kHz crystal oscillator
 * - External clock (Not available on all devices)
 *
 * The timeout service is configurable to a number of independent timeout
 * channels, each with different delay setup in a number of ticks. Both
 * singleshot and periodic timeouts are supported.
 *
 * As this service provides a software layer on top of the RTC/RTC32 module it
 * will have some performance penalty, so for high performance it would be
 * recommended to implement a more specific use by implementing your own
 * interrupt handler based on this as a reference.
 *
 * \section timeout_configuration Configuration
 * Configuration is done in the config file : conf_timeout.h
 *
 * Configuration defines:
 * - \ref TIMEOUT_CLOCK_SOURCE_HZ : Frequency of clock source, used in
 *                                  calculation of tick rate
 *
 * - \ref TIMEOUT_COUNT :           Number of independent timeout channels
 *                                  (Max 8 channels)
 *
 * - \ref TIMEOUT_TICK_HZ :         Desired tick rate in Hz
 *
 * - \ref CLOCK_SOURCE_RTC32        Used to disable the RTC module (default)
 *                                  and use the RTC32 module found in ATxmegaA3B
 *                                  and ATxmegaA3BU.
 *
 * \section tc_timeout_interface Interface
 * The timeout internal setup needs to be configured and this is done by the
 * function tc_timeout_init().
 *
 * There are different functions for starting a timer:
 * - \ref timeout_start_singleshot() : Start a singleshot timeout.
 * - \ref timeout_start_periodic() :   Start a periodic timeout.
 * - \ref timeout_start_offset() :     Start a periodic timeout with a specific
 *                                     start offset.
 *
 * Polling for timer status can be done with
 * \ref timeout_test_and_clear_expired(), and this will also clear the
 * expired flag in case of periodic timer.
 *
 * A running timer can be stopped with \ref timeout_stop().
 *
 * Common to all the function arguments are a timeout identifier, this is a
 * number starting from 0 to identify the timeout channel. Maximum of this
 * parameter is controlled by the configuration \ref TIMEOUT_COUNT.
 *
 * The start timeout functions uses timeout values represented in number of
 * ticks.
 *
 * \subsection tc_timeout_usage Usage
 * First of all, the include file is needed:
 * \code #include "timeout.h" \endcode
 *
 * Then the timeout internals need to be set up by calling:
 * \code timeout_init(); \endcode
 *
 * For simple usage starting a singleshot timeout for timeout id 0 and a timeout
 * value of 100 ticks:
 * \code
 * tc_timeout_start_singleshot(0, 100);
 * while (!timeout_test_and_clear_expired(0));
 * // do whats needed after timeout has expired
 * \endcode
 *
 * \section tc_timeout_accuracy Accuracy
 * Since this is a timeout layer on top of a system tick; the trigger time of a
 * timeout is fully depending on this system tick. This means that you might
 * not know when the next tick will count down your timeout, and this inaccuracy
 * can be from 0 to 1 system tick.
 *
 * E.g.: If you want a timeout of 1 system tick and use 1 as your timeout
 * value, this might trigger immediately. So, if you have a requirement to wait
 * at least 1 system tick, it would be recommended to use the requested value
 * + 1.
 *
 * However, if you know the system tick has passed or are using periodic timeout
 * you can be confident in the timing.
 */


// Test for missing configurations
#if !defined(TIMEOUT_CLOCK_SOURCE_HZ)
#  error "configuration define missing: TIMEOUT_CLOCK_SOURCE_HZ"
#endif

#if !defined(TIMEOUT_TICK_HZ)
#  error "configuration define missing: TIMEOUT_TICK_HZ"
#endif

#if !defined(TIMEOUT_COUNT)
#  error "configuration define missing: TIMEOUT_COUNT"
#endif

// Check if timeout count is within allowed range
#if (TIMEOUT_COUNT > 8)
#  error "TIMEOUT_COUNT outside allowed range"
#endif


// Calculate tick rate
#define TIMEOUT_COMP TIMEOUT_CLOCK_SOURCE_HZ / TIMEOUT_TICK_HZ

/**
 * \brief Timeout identifier
 *
 * Index for timeout channel to use. Limited by max value configured with \ref
 * TIMEOUT_COUNT.
 */
typedef uint8_t timeout_id_t;

// API functions
void timeout_init(void);
void timeout_start_singleshot(timeout_id_t id, uint16_t timeout);
void timeout_start_periodic(timeout_id_t id, uint16_t period);
void timeout_start_offset(timeout_id_t id, uint16_t period,
		uint16_t start_offset);
bool timeout_test_and_clear_expired(timeout_id_t id);
void timeout_stop(timeout_id_t id);

#ifdef __cplusplus
}
#endif

 /**
 * \page timeout_quickstart Quick start guide for Timeout service
 *
 * This is the quick start guide for the \ref timeout_group, with
 * step-by-step instructions on how to configure and use the driver in a
 * selection of use cases.
 *
 * The use cases contain several code fragments. The code fragments in the
 * steps for setup can be copied into a custom initialization function, while
 * the steps for usage can be copied into, e.g., the main application function.
 *
 * \section timeout_use_cases Timeout use cases
 * - \ref timeout_basic_use_case
 * - \subpage timeout_use_case_1
 *
 * \section timeout_basic_use_case Basic use case - Toggle LEDs with periodic timeout
 * In this use case, two periodic timeouts are used to toggle two leds.
 *
 * \section timeout_basic_use_case_setup Setup steps
 *
 * \subsection timeout_basic_use_case_setup_prereq Prerequisites
 * For the setup code of this use case to work, the following must
 * be added to the project:
 * -# \ref sysclk_group
 * -# \ref pmic_group
 * -# \ref gpio_group
 * -# \ref rtc_group
 * -# Configuration info for the timeout service must be added to the
 *    conf_timeout.h file (located in the config folder):
 * \code
 * #define TIMEOUT_CLOCK_SOURCE_HZ     1024
 * #define TIMEOUT_COUNT               8
 * #define TIMEOUT_TICK_HZ             4
 * \endcode
 * -# Configuration info for the RTC driver must be added to the
 *    conf_rtc.h file (located in the config folder):
 * \code
 * #define CONFIG_RTC_PRESCALER          RTC_PRESCALER_DIV1_gc
 * #define CONFIG_RTC_CLOCK_SOURCE       CLK_RTCSRC_ULP_gc
 * \endcode
 *
 * \subsection timeout_basic_use_case_setup_code Example code
 * The following must be added to the project:
 * \code
 * #define TIMEOUT_0   0
 * #define TIMEOUT_1   1
 * \endcode
 *
 * Add to application initialization:
 * \code
 * sysclk_init();
 * pmic_init();
 * timeout_init();
 * timeout_start_periodic(TIMEOUT_0, 1);
 * timeout_start_periodic(TIMEOUT_1, 2);
 * \endcode
 *
 * \subsection timeout_basic_use_case_setup_flow Workflow
 * -# Initialize system clock:
 *   - \code sysclk_init(); \endcode
 * -# Initialize the PMIC driver:
 *   - \code pmic_init(); \endcode
 * -# Initialize timeout service:
 *   - \code timout_init(); \endcode
 * -# Start timeout channel 0 with a period of 1 tick:
 *   - \code timeout_start_periodic(TIMEOUT_0, 1); \endcode
 * -# Start timeout channel 1 with a period of 2 ticks:
 *   - \code timeout_start_periodic(TIMEOUT_1, 2); \endcode
 *
 * \section timeout_basic_use_case_usage Usage steps
 *
 * \subsection timeout_basic_use_case_usage_code Example code
 * Add to application C-file:
 * \code
 * while (1) {
 *    if (timeout_test_and_clear_expired(TIMEOUT_0)) {
 *       gpio_toggle_pin(LED0_GPIO);
 *    }
 *    if (timeout_test_and_clear_expired(TIMEOUT_1)) {
 *       gpio_toggle_pin(LED1_GPIO);
 *    }
 * }
 * \endcode
 *
 * \subsection timeout_basic_use_case_usage_flow Workflow
 * -# Check if timeout on channel 0 has expired, and toggle led if it has:
 *   - \code
 * if (timeout_test_and_clear_expired(TIMEOUT_0)) {
 *    gpio_toggle_pin(LED0_GPIO);
 * }
 *   \endcode
 * -# Check if timeout on channel 1 has expired, and toggle led if it has:
 *   - \code
 * if (timeout_test_and_clear_expired(TIMEOUT_1)) {
 *    gpio_toggle_pin(LED1_GPIO);
 * }
 *   \endcode
 */

/**
 * \page timeout_use_case_1 Debounce filter on a button
 *
 * In this use case, a simple debounce filter on a button will be set up.
 *
 * \section timeout_use_case_1_setup Setup steps
 *
 * \subsection timeout_use_case_1_setup_prereq Prerequisites
 * For the setup code of this use case to work, the following must
 * be added to the project:
 * -# \ref sysclk_group
 * -# \ref pmic_group
 * -# \ref gpio_group
 * -# \ref rtc_group
 * -# Configuration info for the timeout service must be added to the
 *    conf_timeout.h file (located in the config folder):
 * \code
 * #define TIMEOUT_CLOCK_SOURCE_HZ     1024
 * #define TIMEOUT_COUNT               1
 * #define TIMEOUT_TICK_HZ             100
 * \endcode
 * -# Configuration info for the RTC driver must be added to the
 *    conf_rtc.h file (located in the config folder):
 * \code
 * #define CONFIG_RTC_PRESCALER          RTC_PRESCALER_DIV1_gc
 * #define CONFIG_RTC_CLOCK_SOURCE       CLK_RTCSRC_ULP_gc
 * \endcode
 *
 * \subsection timeout_use_case_1_setup_code Example code
 * The following must be added to the project:
 * \code
 * #define DEBOUNCE_TIMEOUT    0
 * #define DEBOUNCE_TICKS      (50 * TIMEOUT_TICK_HZ / 1000)
 * \endcode
 *
 * Add to application initialization:
 * \code
 * sysclk_init();
 * pmic_init();
 * timeout_init();
 * \endcode
 *
 * \subsection timeout_use_case_1_setup_flow Workflow
 * -# Initialize system clock:
 *   - \code sysclk_init(); \endcode
 * -# Initialize the PMIC driver:
 *   - \code pmic_init(); \endcode
 * -# Initialize timeout service:
 *   - \code timout_init(); \endcode
 *
 * \subsection timeout_use_case_1_usage_code Example code
 * Add to application C-file:
 * \code
 * bool button_pressed;
 * bool button_previous_state_pressed = false;
 * while (1) {
 *    button_pressed = gpio_pin_is_low(GPIO_PUSH_BUTTON_0);
 *    if (button_previous_state_pressed != button_pressed) {
 *       timeout_start_singleshot(DEBOUNCE_TIMEOUT, DEBOUNCE_TICKS);
 *       button_previous_state_pressed = button_pressed;
 *    }
 *
 *    if (timeout_test_and_clear_expired(DEBOUNCE_TIMEOUT)) {
 *       if (button_pressed) {
 *          gpio_toggle_pin(LED0_GPIO);
 *       }
 *    }
 * }
 * \endcode
 *
 * \subsection timeout_use_case_1_usage_flow Workflow
 * -# Create a variable to hold state of push button:
 *   - \code bool button_pressed; \endcode
 * -# Create a variable to hold previous state of push button:
 *   - \code bool button_previous_state_pressed; \endcode
 * -# Get button state:
 *   - \code button_pressed = gpio_pin_is_low(GPIO_PUSH_BUTTON_0); \endcode
 * -# Check if button state has changed since last iteration:
 *   - \code if (button_previous_state_pressed != button_pressed) \endcode
 * -# Start debounce timeout:
 *   - \code
 * timeout_start_singleshot(DEBOUNCE_TIMEOUT, DEBOUNCE_TICKS);
 *   \endcode
 * -# Set previous state of button:
 *   - \code button_previous_state_pressed = button_pressed; \endcode
 * -# Check if debounce timeout has expired:
 *   - \code if (timeout_test_and_clear_expired(DEBOUNCE_TIMEOUT)) \endcode
 * -# Check if button is pressed down:
 *   - \code if (button_pressed) \endcode
 * -# Toggle led:
 *   - \code gpio_toggle_pin(LED0_GPIO); \endcode
 */

#endif /* TIMEOUT_H */
