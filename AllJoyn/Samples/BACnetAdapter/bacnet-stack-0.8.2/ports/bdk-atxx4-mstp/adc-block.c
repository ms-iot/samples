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

/* ADMUX: channel bits 0..4
      ADLAR = Left Adjust Result
      REFSx = hardware setup: cap on AREF
*/
/* ADCSRA:
      ADEN = Enable
      ADSC = Start conversion
      ADIF = Interrupt Flag
      ADIE = Interrupt Enable
      ADATE = Auto Trigger Enable
*/

void adc_enable(
    uint8_t index)
{
    index = index;
    /* do nothing */
}

/**************************************************
* Description: Run a Analog to Digital conversion
* Returns: none
* Notes: none
**************************************************/
uint8_t adc_result_8bit(
    uint8_t channel)
{       /* 0..7 = ADC0..ADC7, respectively */
    uint8_t value = 0;  /* return value */

    while (ADCSRA & (1 << ADSC));
    ADMUX = channel | (1 << ADLAR) | (0 << REFS1) | (1 << REFS0);
    /* Delay needed for the stabilization of the ADC input voltage */
    _delay_us(10);
    /* Start the analog to digital conversion */
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADIF) | ADPS_8BIT;
    /* Wait for the analog to digital conversion to complete */
    while ((ADCSRA & (1 << ADIF)) == 0);
    value = ADCH;

    return value;
}

/**************************************************
* Description: Run a Analog to Digital conversion
* Returns: none
* Notes: none
**************************************************/
uint16_t adc_result_10bit(
    uint8_t channel)
{       /* 0..7 = ADC0..ADC7, respectively */
    uint16_t value = 0; /* return value */

    while (ADCSRA & (1 << ADSC));
    ADMUX = channel | (0 << ADLAR) | (0 << REFS1) | (1 << REFS0);
    /* Delay needed for the stabilization of the ADC input voltage */
    _delay_us(10);
    /* Start the analog to digital conversion */
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADIF) | ADPS_10BIT;
    /* Wait for the analog to digital conversion to complete */
    while ((ADCSRA & (1 << ADIF)) == 0);
    value = ADCL;
    value |= (ADCH << 8);

    return value;
}

/**************************************************
* Description: Initializes the Analog to Digital Converter
* Returns: none
* Notes: none
**************************************************/
void adc_init(
    void)
{
    /* configure ADC for Free Running Mode - ADTS = 000 */
    /* AIN1 is applied to the negative input of the Analog Comparator - ACME */
    BITMASK_CLEAR(ADCSRB, _BV(ACME) | _BV(ADTS2) | _BV(ADTS1) | _BV(ADTS0));
    /* Digital input not needed on ADC0, so disable it to save power */
    BIT_SET(DIDR0, ADC0D);
    /* Clear the Power Reduction bit to enable ADC */
    BIT_CLEAR(PRR, PRADC);
}
