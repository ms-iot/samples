/**
 * \file
 *
 * \brief PWM service for XMEGA.
 *
 * Copyright (c) 2012 Atmel Corporation. All rights reserved.
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

#ifndef PWM_H
#define PWM_H

#include "tc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup pwm_group XMEGA Pulse Width Modulation (PWM) service
 *
 * See \ref pwm_quickstart.
 *
 * This is a service for single slope wave form generation on the XMEGA.
 * It provides functions for enabling, disabling and configuring the TC modules
 * in single slope PWM mode.
 *
 * The API uses a \ref pwm_config "structure" which contain the configuration.
 * This structure must be set up before the PWM can be started.
 *
 * \section dependencies Dependencies
 * This driver depends on the following modules:
 * - \ref tc_group to set up TC in PWM mode.
 * @{
 */

 /**
 * \brief PWM compare channel index
 */
enum pwm_channel_t {
	/** Channel A. PWM output on pin 0 */
	PWM_CH_A = 1,
	/** Channel B. PWM output on pin 1 */
	PWM_CH_B = 2,
	/** Channel C. PWM output on pin 2 */
	PWM_CH_C = 3,
	/** Channel D. PWM output on pin 3 */
	PWM_CH_D = 4,
};

 /**
 * \brief Valid timer/counters to use
 * \note Not all timer/counters are available on all devices.
 * Please refer to the datasheet for more information on what
 * timer/counters are available for the device you are using.
 */
enum pwm_tc_t {
	/** PWM on port C, pin 0, 1, 2 or 3 (depending on
	    \ref pwm_channel_t "channel") */
	PWM_TCC0,
	/** PWM on port C, pin 4 or 5 (depending on
	    \ref pwm_channel_t "channel") */
	PWM_TCC1,
	/** PWM on port D, pin 0, 1, 2 or 3 (depending on
	    \ref pwm_channel_t "channel") */
	PWM_TCD0,
	/** PWM on port D, pin 4 or 5 (depending on
	    \ref pwm_channel_t "channel") */
	PWM_TCD1,
	/** PWM on port E, pin 0, 1, 2 or 3 (depending on
	    \ref pwm_channel_t "channel") */
	PWM_TCE0,
	/** PWM on port E, pin 4 or 5 (depending on
	    \ref pwm_channel_t "channel") */
	PWM_TCE1,
	/** PWM on port F, pin 0, 1, 2 or 3 (depending on
	    \ref pwm_channel_t "channel") */
	PWM_TCF0,
	/** PWM on port F, pin 4 or 5 (depending on
	    \ref pwm_channel_t "channel") */
	PWM_TCF1,
};

 /**
 * \brief Valid clock source indexes
 */
enum pwm_clk_sel {
	PWM_CLK_OFF     = TC_CLKSEL_OFF_gc,
	PWM_CLK_DIV1    = TC_CLKSEL_DIV1_gc,
	PWM_CLK_DIV2    = TC_CLKSEL_DIV2_gc,
	PWM_CLK_DIV4    = TC_CLKSEL_DIV4_gc,
	PWM_CLK_DIV8    = TC_CLKSEL_DIV8_gc,
	PWM_CLK_DIV64   = TC_CLKSEL_DIV64_gc,
	PWM_CLK_DIV256  = TC_CLKSEL_DIV256_gc,
	PWM_CLK_DIV1024 = TC_CLKSEL_DIV1024_gc,
};

 /**
 * \brief PWM configuration
 */
struct pwm_config {
	void *tc;
	enum pwm_channel_t channel;
	enum tc_cc_channel_mask_enable_t cc_mask;
	enum pwm_clk_sel clk_sel;
	uint16_t period;
};

/** \brief Interrupt callback type */
typedef void (*pwm_callback_t) (void);

void pwm_init(struct pwm_config *config, enum pwm_tc_t tc,
		enum pwm_channel_t channel, uint16_t freq_hz);
void pwm_set_frequency(struct pwm_config *config, uint16_t freq_hz);
void pwm_start(struct pwm_config *config, uint8_t duty_cycle_scale);

/**
 * \brief Function to set PWM duty cycle
 *
 * The duty cycle can be set on a scale between 0-100%. This value
 * will be used to update the CCx register for the selected PWM channel.
 *
 * \param *config           Pointer to the PWM configuration struct
 * \param duty_cycle_scale  Duty cycle as a value between 0 and 100.
 */
static inline void pwm_set_duty_cycle_percent(struct pwm_config *config,
		uint8_t duty_cycle_scale)
{
	Assert( duty_cycle_scale <= 100 );
	tc_write_cc_buffer(config->tc, config->channel,
			(uint16_t)(((uint32_t)config->period *
			(uint32_t)duty_cycle_scale) / 100));
}

/**
 * \brief Function that stops the PWM timer
 *
 * The PWM timer is stopped by writing the prescaler register to "clock off"
 *
 * \param *config           Pointer to the PWM configuration struct
 */
static inline void pwm_stop(struct pwm_config *config)
{
	tc_write_clock_source(config->tc, TC_CLKSEL_OFF_gc);
}

/**
 * \brief Disable the PWM timer
 *
 * This function disables the peripheral clock for the timer and shut down
 * module when unused in order to save power.
 *
 * \param *config           Pointer to the PWM configuration struct
 */
static inline void pwm_disable(struct pwm_config *config)
{
	pwm_stop(config);
	tc_disable(config->tc);
}

/**
 * \brief Function that resets the PWM timer
 *
 * This function reset the CNT register for the selected timer used for PWM
 *
 * \param *config Pointer to the PWM configuration struct
 */
static inline void pwm_timer_reset(struct pwm_config *config)
{
	tc_write_count(config->tc, 0);
}

/**
 * \brief Callback function for timer overflow interrupts
 *
 * This function enables T/C overflow interrupts (low level interrupts)
 * and defines the callback function for the overflow ISR interrupt routine.
 *
 * \param *config           Pointer to the PWM configuration struct
 * \param callback          Callback function
 */
static inline void pwm_overflow_int_callback(struct pwm_config *config,
		pwm_callback_t callback)
{
	tc_set_overflow_interrupt_level(config->tc, TC_INT_LVL_LO);
	tc_set_overflow_interrupt_callback(config->tc, callback);
}

/** @} */

#ifdef __cplusplus
}
#endif

/**
 * \page pwm_quickstart Quickstart guide for AVR XMEGA PWM service
 *
 * This is the quickstart guide for the \ref pwm_group,
 * with step-by-step instructions on how to configure and use the service in a
 * selection of use cases.
 *
 * The use cases contain several code fragments. The code fragments in the
 * steps for setup can be copied into a custom initialization function, while
 * the steps for usage can be copied into, e.g., the main application function.
 *
 * \section basic_use_case Basic use case
 * In the most basic use case, we configure one PWM channel in non-interrupt
 * mode.
 *
 * \section pwm_basic_use_case_setup Setup steps
 * \subsection pwm_basic_use_case_setup_code Example code
 * Add to application C-file:
 * \code
 *    struct pwm_config pwm_cfg;
 *
 *    sysclk_init();
 *    pwm_init(&pwm_cfg, PWM_TCE0, PWM_CH_A, 500);
 * \endcode
 *
 * \subsection pwm_basic_use_case_setup_flow Workflow
 * -# Ensure that \ref conf_clock.h is present for the driver.
 *   \note This file is only for the driver and should not be included by the
 * user.
 * -# Define config struct for PWM module:
 *   \code struct pwm_config pwm_cfg; \endcode
 * -# Initialize sysclock module:
 *   \code sysclk_init();\endcode
 * -# Initialize config struct and set up PWM with frequency of 500 Hz.\n
 *   \code pwm_init(&pwm_cfg, PWM_TCE0, PWM_CH_A, 500); \endcode
 *   \note Since the timer/counter \ref PWM_TCE0 and channel \ref PWM_CH_A
 *    is used, the PWM will be output on port E, pin 0.
 *    See \ref pwm_tc_t and \ref pwm_channel_t for more information
 *    on what port/pin is used for different timer/counters.

 * \attention This step must not be skipped or the initial content of the
 * structs will be unpredictable, possibly causing misconfiguration.
 *
 * \section pwm_basic_use_case_usage Usage steps
 * \subsection pwm_basic_use_case_usage_code Example code
 * Add to, e.g., main loop in application C-file:
 * \code pwm_start(&pwm_config, 50); \endcode
 *
 * \subsection pwm_basic_use_case_usage_flow Workflow
 * -# Start PWM with 50% duty cycle:
 *   \code pwm_start(&pwm_config, 50); \endcode
 *
 * \section pwm_use_cases Advanced use cases
 * For more advanced use of the PWM service, see the following use cases:
 * - \subpage pwm_use_case_1 : PWM with interrupt
 */
/**
 * \page pwm_use_case_1 Use case #1
 * In this use case the PWM module is configured with overflow interrupt.
 *
 * \section pwm_use_case_1_setup Setup steps
 * \subsection pwm_use_case_1_setup_code Example code
 *
 * Add to application C-file:
 * \code
 * struct pwm_config pwm_cfg;
 *
 * void my_callback(void)
 * {
 *      do_something();
 * }

 * void pwm_init(void)
 * {
 *     pmic_init();
 *     sysclk_init();
 *
 *     cpu_irq_enable();
 *
 *     pwm_init(&pwm_cfg, PWM_TCE0, PWM_CH_A, 75);
 *     pwm_overflow_int_callback(&pwm_cfg, my_callback);
 * }
 * \endcode
 *
 * \subsection pwm_use_case_1_setup_flow Workflow
 * -# Define config struct for PWM module:
 *   \code struct pwm_config pwm_cfg; \endcode
 * -# Define a callback function in the application which does whatever task
 * you want it to do:
 *   \code
 * void my_callback(void)
 * {
 *      do_something();
 * }
 *   \endcode
 * -# Initialize interrupt controller module:
 *   \code pmic_init();\endcode
 * -# Initialize sysclock module:
 *   \code sysclk_init();\endcode
 * -# Enable global interrupts:
 *   \code cpu_irq_enable();\endcode
 * -# Initialize config struct and set up PWM with frequency of 75 Hz:
 *   \code pwm_init(&pwm_cfg, PWM_TCE0, PWM_CH_A, 75); \endcode
 *   \note Since the timer/counter \ref PWM_TCE0 and channel \ref PWM_CH_A
 *    is used, the PWM will be output on port E, pin 0.
 *    See \ref pwm_tc_t and \ref pwm_channel_t for more information
 *    on what port/pin is used for different timer/counters.
 * \attention This step must not be skipped or the initial content of the
 * structs will be unpredictable, possibly causing misconfiguration.
 * -# Set callback function on PWM TC channel overflow:
 *   \code pwm_overflow_int_callback(&pwm_cfg, my_callback); \endcode
 *
 * \section pwm_use_case_1_usage Usage steps
 * \subsection pwm_use_case_1_usage_code Example code
 * Add to, e.g., main loop in application C-file:
 * \code pwm_start(&pwm_cfg, 50); \endcode
 *
 * \subsection pwm_basic_use_case_usage_flow Workflow
 * -# Start PWM with 50% duty cycle:
 *   \code pwm_start(&pwm_cfg, 50); \endcode
 *
 */

#endif /* PWM_H */
