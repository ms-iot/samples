/**************************************************************************
*
* Copyright (C) 2014 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <asf.h>
#include <util/atomic.h>
#include "adc-hdw.h"

/* samples */
#define ADC_CHANNELS_MAX 10
static uint16_t ADC_Channel_Value[ADC_CHANNELS_MAX];
static uint8_t ADC_Current_Channel;

/*************************************************************************
* DESCRIPTION: set the active channel in the ADC
* RETURN: nothing
* NOTES: called from ISR, so handle as non-blocking
**************************************************************************/
static void adc_set_channel(unsigned channel)
{
    struct adc_channel_config adc_ch_conf;

    ADC_Current_Channel = channel;
    adcch_read_configuration(&ADCA, ADC_CH0, &adc_ch_conf);
    switch (channel) {
        case 0:
            adcch_set_input(&adc_ch_conf, ADCCH_POS_PIN7, ADCCH_NEG_PIN1, 1);
            break;
        case 1:
            adcch_set_input(&adc_ch_conf, ADCCH_POS_PIN8, ADCCH_NEG_PIN1, 1);
            break;
        case 2:
            adcch_set_input(&adc_ch_conf, ADCCH_POS_PIN9, ADCCH_NEG_PIN1, 1);
            break;
        case 3:
            adcch_set_input(&adc_ch_conf, ADCCH_POS_PIN10, ADCCH_NEG_PIN1, 1);
            break;
        case 4:
            adcch_set_input(&adc_ch_conf, ADCCH_POS_PIN11, ADCCH_NEG_PIN1, 1);
            break;
        case 5:
            adcch_set_input(&adc_ch_conf, ADCCH_POS_PIN2, ADCCH_NEG_PIN1, 1);
            break;
        case 6:
            adcch_set_input(&adc_ch_conf, ADCCH_POS_PIN3, ADCCH_NEG_PIN1, 1);
            break;
        case 7:
            adcch_set_input(&adc_ch_conf, ADCCH_POS_PIN4, ADCCH_NEG_PIN1, 1);
            break;
        case 8:
            adcch_set_input(&adc_ch_conf, ADCCH_POS_PIN5, ADCCH_NEG_PIN1, 1);
            break;
        case 9:
            adcch_set_input(&adc_ch_conf, ADCCH_POS_PIN6, ADCCH_NEG_PIN1, 1);
            break;
        default:
            break;
    }
    adcch_set_interrupt_mode(&adc_ch_conf, ADCCH_MODE_COMPLETE);
    adcch_enable_interrupt(&adc_ch_conf);
    adcch_write_configuration(&ADCA, ADC_CH0, &adc_ch_conf);
}

/*************************************************************************
* DESCRIPTION: run the active channels through the ADC
* RETURN: nothing
* NOTES: called from ISR, so handle as non-blocking
**************************************************************************/
static void adc_handler(ADC_t *adc, uint8_t ch_mask, adc_result_t raw_value)
{
    unsigned channel;

    channel = ADC_Current_Channel;
    if (channel < ADC_CHANNELS_MAX) {
        ADC_Channel_Value[channel] = raw_value;
    }
    channel++;
    if (channel >= ADC_CHANNELS_MAX) {
        channel = 0;
    }
    adc_set_channel(channel);
}

/*************************************************************************
* DESCRIPTION: initialize Analog to Digital Converter (ADC)
* RETURN: nothing
* NOTES: none
**************************************************************************/
void adc_init(void)
{
    struct adc_config adc_conf;

    ioport_configure_pin(IOPORT_CREATE_PIN(PORTA, 7),IOPORT_DIR_INPUT);
    ioport_configure_pin(IOPORT_CREATE_PIN(PORTB, 0),IOPORT_DIR_INPUT);
    ioport_configure_pin(IOPORT_CREATE_PIN(PORTB, 1),IOPORT_DIR_INPUT);
    ioport_configure_pin(IOPORT_CREATE_PIN(PORTB, 2),IOPORT_DIR_INPUT);
    ioport_configure_pin(IOPORT_CREATE_PIN(PORTB, 3),IOPORT_DIR_INPUT);
    ioport_configure_pin(IOPORT_CREATE_PIN(PORTA, 2),IOPORT_DIR_INPUT);
    ioport_configure_pin(IOPORT_CREATE_PIN(PORTA, 3),IOPORT_DIR_INPUT);
    ioport_configure_pin(IOPORT_CREATE_PIN(PORTA, 4),IOPORT_DIR_INPUT);
    ioport_configure_pin(IOPORT_CREATE_PIN(PORTA, 5),IOPORT_DIR_INPUT);
    ioport_configure_pin(IOPORT_CREATE_PIN(PORTA, 6),IOPORT_DIR_INPUT);
    /* Clear the ADC configuration structs */
    adc_read_configuration(&ADCA, &adc_conf);
    adc_set_conversion_parameters(&adc_conf, ADC_SIGN_ON, ADC_RES_12, ADC_REF_AREFA);
    adc_set_clock_rate(&adc_conf, 200000UL);
    adc_set_conversion_trigger(&adc_conf, ADC_TRIG_MANUAL, 1, 0);
    adc_write_configuration(&ADCA, &adc_conf);
    adc_set_callback(&ADCA, &adc_handler);
    adc_set_channel(0);
    /* Enable the ADC and start the first conversion. */
    adc_enable(&ADCA);
}

/*************************************************************************
* DESCRIPTION: Get a result from the ADC 10-bit value
* RETURN: 12-bit ADC value
* NOTES: channel 0..9 are supported
**************************************************************************/
uint16_t adc_result_12bit(uint8_t channel)
{
    uint16_t value = 0;

    if (channel < ADC_CHANNELS_MAX) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            value = ADC_Channel_Value[channel];
        }
    }

    return value;
}

/*************************************************************************
* DESCRIPTION: Get a result from the ADC 10-bit value
* RETURN: 10-bit ADC value
* NOTES: channel 0..9 are supported
**************************************************************************/
uint16_t adc_result_10bit(uint8_t channel)
{
    uint16_t result;

    result = adc_result_12bit(channel);
    result >>= 2;

    return result;
}

/*************************************************************************
* DESCRIPTION: Get a result from the ADC 8-bit value
* RETURN: 8-bit ADC value
* NOTES: channel 0..9 are supported
**************************************************************************/
uint8_t adc_result_8bit(uint8_t channel)
{
    uint16_t result;

    result = adc_result_12bit(channel);
    result >>= 4;

    return result;
}
