/**************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include "hardware.h"
/* me */
#include "adc.h"

/* prescale select bits */
#if   (F_CPU >> 1) < 1000000
#define ADPS_8BIT    (1)
#define ADPS_10BIT   (3)
#elif (F_CPU >> 2) < 1000000
#define ADPS_8BIT    (2)
#define ADPS_10BIT   (4)
#elif (F_CPU >> 3) < 1000000
#define ADPS_8BIT    (3)
#define ADPS_10BIT   (5)
#elif (F_CPU >> 4) < 1000000
#define ADPS_8BIT    (4)
#define ADPS_10BIT   (6)
#elif (F_CPU >> 5) < 1000000
#define ADPS_8BIT    (5)
#define ADPS_10BIT   (7)
#else
#error  "ADC: F_CPU too large for accuracy."
#endif

/* Array of ADC results */
#define ADC_CHANNELS_MAX 8
static volatile uint16_t Sample_Result[ADC_CHANNELS_MAX];
static volatile uint8_t Enabled_Channels;

ISR(ADC_vect)
{
    uint8_t index;
    uint8_t mask;
    uint8_t channels;
    uint16_t value = 0;

    /* determine which conversion finished */
    index = BITMASK_CHECK(ADMUX, ((1 << MUX2) | (1 << MUX1) | (1 << MUX0)));
    /* read the results */
    value = ADCL;
    value |= (ADCH << 8);
    Sample_Result[index] = value;
    channels = Enabled_Channels;
    __enable_interrupt();
    /* clear the mux */
    BITMASK_CLEAR(ADMUX, ((1 << MUX2) | (1 << MUX1) | (1 << MUX0)));
    /* find the next enabled channel */
    while (channels) {
        index++;
        if (index >= ADC_CHANNELS_MAX) {
            index = 0;
        }
        mask = 1 << index;
        if (channels & mask) {
            break;
        }
    }
    /* configure the next channel */
    BITMASK_SET(ADMUX, ((index) << MUX0));
    /* Start the next conversion */
    BIT_SET(ADCSRA, ADSC);
}

void adc_enable(
    uint8_t index)
{       /* 0..7 = ADC0..ADC7, respectively */
    if (Enabled_Channels) {
        /* ADC interupt is already started */
        BIT_SET(Enabled_Channels, index);
    } else {
        if (index < ADC_CHANNELS_MAX) {
            /* not running yet */
            BIT_SET(Enabled_Channels, index);
            /* clear the mux */
            BITMASK_CLEAR(ADMUX, ((1 << MUX2) | (1 << MUX1) | (1 << MUX0)));
            /* configure the channel */
            BITMASK_SET(ADMUX, ((index) << MUX0));
            /* Start the next conversion */
            BIT_SET(ADCSRA, ADSC);
        }
    }
}

uint8_t adc_result_8bit(
    uint8_t index)
{       /* 0..7 = ADC0..ADC7, respectively */
    uint8_t result = 0;
    uint8_t sreg;

    if (index < ADC_CHANNELS_MAX) {
        adc_enable(index);
        sreg = SREG;
        __disable_interrupt();
        result = (uint8_t) (Sample_Result[index] >> 2);
        SREG = sreg;
    }

    return result;
}

uint16_t adc_result_10bit(
    uint8_t index)
{       /* 0..7 = ADC0..ADC7, respectively */
    uint16_t result = 0;
    uint8_t sreg;

    if (index < ADC_CHANNELS_MAX) {
        adc_enable(index);
        sreg = SREG;
        __disable_interrupt();
        result = Sample_Result[index];
        SREG = sreg;
    }

    return result;
}

void adc_init(
    void)
{
    /* Initial channel selection */
    /* ADLAR = Left Adjust Result
       REFSx = hardware setup: cap on AREF
     */
    ADMUX = (0 << ADLAR) | (0 << REFS1) | (1 << REFS0);
    /*  ADEN = Enable
       ADSC = Start conversion
       ADIF = Interrupt Flag - write 1 to clear!
       ADIE = Interrupt Enable
       ADATE = Auto Trigger Enable
     */
    ADCSRA =
        (1 << ADEN) | (1 << ADIE) | (1 << ADIF) | (0 << ADATE) | ADPS_10BIT;
    /* trigger selection bits
       0 0 0 Free Running mode
       0 0 1 Analog Comparator
       0 1 0 External Interrupt Request 0
       0 1 1 Timer/Counter0 Compare Match
       1 0 0 Timer/Counter0 Overflow
       1 0 1 Timer/Counter1 Compare Match B
       1 1 0 Timer/Counter1 Overflow
       1 1 1 Timer/Counter1 Capture Event
     */
    ADCSRB = (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0);
    power_adc_enable();
}
