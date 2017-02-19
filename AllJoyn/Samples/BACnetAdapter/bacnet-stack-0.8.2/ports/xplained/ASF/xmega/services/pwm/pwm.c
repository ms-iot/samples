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

 #include <asf.h>

/**
 * \brief Calculate TC settings from PWM frequency
 *
 * This function will find the correct TC settings (clock prescaler and
 * period) which will give the wanted PWM frequency.
 *
 * \note Since we want to be able to run the PWM at all duty-cycles ranging
 * from 0-100%, we require a period of at least 100 to achieve this. Thus, the
 * highest possible PWM frequency is CPU frequency / 100.
 *
 * \param config Pointer to PWM configuration.
 * \param freq_hz Wanted PWM frequency in Hz.
 */
void pwm_set_frequency(struct pwm_config *config, uint16_t freq_hz)
{
	uint32_t cpu_hz = sysclk_get_cpu_hz();
	uint16_t smallest_div;
	uint16_t dividor;

	/* Avoid division by zero. */
	Assert(freq_hz != 0);

	/* Calculate the smallest divider for the requested frequency
	   related to the CPU frequency */
	smallest_div = cpu_hz / freq_hz / 0xFFFF;
	if (smallest_div < 1) {
		dividor = 1;
		config->clk_sel = PWM_CLK_DIV1;
	} else if (smallest_div < 2) {
		dividor = 2;
		config->clk_sel = PWM_CLK_DIV2;
	} else if (smallest_div < 4) {
		dividor = 4;
		config->clk_sel = PWM_CLK_DIV4;
	} else if (smallest_div < 8) {
		dividor = 8;
		config->clk_sel = PWM_CLK_DIV8;
	} else if (smallest_div < 64) {
		dividor = 64;
		config->clk_sel = PWM_CLK_DIV64;
	} else if (smallest_div < 256) {
		dividor = 256;
		config->clk_sel = PWM_CLK_DIV256;
	} else {
		dividor = 1024;
		config->clk_sel = PWM_CLK_DIV1024;
	}

	/* Calculate the period from the just found divider */
	config->period = cpu_hz / dividor / freq_hz;

	/* Make sure our period is at least 100 ticks so we are able to provide
	   a full range (0-100% duty cycle */
	if (config->period < 100) {
		/* The period is too short. */
		config->clk_sel = PWM_CLK_OFF;
		config->period = 0;
		Assert(false);
	}
}

/**
 * \brief Initialize PWM configuration struct and set correct I/O pin to output
 *
 * \param config Pointer to PWM configuration struct.
 * \param tc \ref pwm_tc_t "TC" to use for this PWM.
 * \param channel \ref pwm_channel_t "CC channel" to use for this PWM.
 * \param freq_hz Frequency to use for this PWM.
  */
void pwm_init(struct pwm_config *config, enum pwm_tc_t tc,
		enum pwm_channel_t channel, uint16_t freq_hz)
{
	/* Number of channels for this TC */
	uint8_t num_chan = 0;
	UNUSED(num_chan);

	/* Set TC and correct I/O pin to output */
	switch (tc) {
#if defined(TCC0)
	case PWM_TCC0:
		config->tc = &TCC0;
		PORTC.DIR |= (1 << (channel-1));
		num_chan = 4;
		break;
#endif
#if defined(TCC1)
	case PWM_TCC1:
		config->tc = &TCC1;
		PORTC.DIR |= (1 << (channel+3));
		num_chan = 2;
		break;
#endif
#if defined(TCD0)
	case PWM_TCD0:
		config->tc = &TCD0;
		PORTD.DIR |= (1 << (channel-1));
		num_chan = 4;
		break;
#endif
#if defined(TCD1)
	case PWM_TCD1:
		config->tc = &TCD1;
		PORTD.DIR |= (1 << (channel+3));
		num_chan = 2;
		break;
#endif

#if defined(TCE0)
	case PWM_TCE0:
		config->tc = &TCE0;
		PORTE.DIR |= (1 << (channel-1));
		num_chan = 4;
		break;
#endif
#if defined(TCE1)
	case PWM_TCE1:
		config->tc = &TCE1;
		PORTE.DIR |= (1 << (channel+3));
		num_chan = 2;
		break;
#endif

#if defined(TCF0)
	case PWM_TCF0:
		config->tc = &TCF0;
		PORTF.DIR |= (1 << (channel-1));
		num_chan = 4;
		break;
#endif
#if defined(TCF1)
	case PWM_TCF1:
		config->tc = &TCF1;
		PORTF.DIR |= (1 << (channel+3));
		num_chan = 2;
		break;
#endif
	default:
		Assert(false);
		break;
	}

	/* Make sure we are not given a channel number larger
	   than this TC can handle */
	Assert(channel <= num_chan);
	config->channel = channel;

	/* Set the correct cc_mask */
	switch (channel) {
	case PWM_CH_A:
		config->cc_mask = TC_CCAEN;
		break;
	case PWM_CH_B:
		config->cc_mask = TC_CCBEN;
		break;
	case PWM_CH_C:
		config->cc_mask = TC_CCCEN;
		break;
	case PWM_CH_D:
		config->cc_mask = TC_CCDEN;
		break;
	default:
		Assert(false);
		break;
	}

	/* Enable peripheral clock for this TC */
	tc_enable(config->tc);

	/* Set this TC's waveform generator in single slope mode */
	tc_set_wgm(config->tc, TC_WG_SS);

	/* Default values (disable TC and set minimum period)*/
	config->period = 0;
	config->clk_sel = PWM_CLK_OFF;
	tc_write_clock_source(config->tc, PWM_CLK_OFF);

	/* Set the PWM frequency */
	pwm_set_frequency(config, freq_hz);
}

/**
 * \brief Start a PWM channel
 *
 * This function enables a channel with a given duty cycle.
 *
 * \param *config           Pointer to the PWM configuration struct
 * \param duty_cycle_scale  Duty cycle as a value between 0 and 100.
 */
void pwm_start(struct pwm_config *config, uint8_t duty_cycle_scale)
{
	/* Set given duty cycle */
	pwm_set_duty_cycle_percent(config, duty_cycle_scale);
	/* Set correct TC period */
	tc_write_period(config->tc, config->period);
	/* Enable CC channel for this TC */
	tc_enable_cc_channels(config->tc, config->cc_mask);
	/* Enable TC by setting correct clock prescaler */
	tc_write_clock_source(config->tc, config->clk_sel);
}
