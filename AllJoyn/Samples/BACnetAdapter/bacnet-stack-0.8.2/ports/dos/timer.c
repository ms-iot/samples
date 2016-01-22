/**************************************************************************
*
* Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
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
#include <stdio.h>
#include <dos.h>

/* global variable counts milliseconds */
volatile unsigned long Timer_Milliseconds;
/* MS/TP Silence Timer */
static volatile int SilenceTime;
/* counts ticks */
volatile unsigned long Timer_Milliseconds;

#define RTC_CMD_ADDR	0x70    /* RTC internal register offset goes here */
#define RTC_DAT_ADDR	0x71    /* RTC internal register R/W access here */

static uint8_t RTC_RS_Convert(
    uint16_t hertz)
{
    uint8_t RS = 0;
    /* from DS12887A datasheet
       SELECT BITS     tPI PERIODIC  
       REGISTER A      INTERRUPT    SQW OUTPUT
       RS3 RS2 RS1 RS0 RATE         FREQUENCY
       --- --- --- --- ------------ ----------
       0   0   0   0  None         0Hz
       0   0   0   1  3.90625ms    256Hz
       0   0   1   0  7.8125ms     128Hz
       0   0   1   1  122.070탎    8192Hz
       0   1   0   0  244.141탎    4096Hz
       0   1   0   1  488.281탎    2048Hz
       0   1   1   0  976.5625탎   1024Hz
       0   1   1   1  1.953125ms   512Hz
       1   0   0   0  3.90625ms    256Hz
       1   0   0   1  7.8125ms     128Hz
       1   0   1   0  15.625ms     64Hz
       1   0   1   1  31.25ms      32Hz
       1   1   0   0  62.5ms       16Hz
       1   1   0   1  125ms        8Hz
       1   1   1   0  250ms        4Hz
       1   1   1   1  500ms        2Hz    
     */
    /* FIXME: create a clever formula to replace switch */
    switch (hertz) {
        case 8192:
            RS = 3;
            break;
        case 4096:
            RS = 4;
            break;
        case 2048:
            RS = 5;
            break;
        case 1024:
            RS = 6;
            break;
        case 512:
            RS = 7;
            break;
        case 256:
            RS = 8;
            break;
        case 128:
            RS = 9;
            break;
        case 64:
            RS = 10;
            break;
        case 32:
            RS = 11;
            break;
        case 16:
            RS = 12;
            break;
        case 8:
            RS = 13;
            break;
        case 4:
            RS = 14;
            break;
        case 2:
            RS = 15;
            break;
        default:
            break;
    }

    return RS;
}

/* setting for 8192 interrupts per second 
   which is an interrupt every 122uS. */
#define INT_FREQ 8192

static void interrupt Timer_Interrupt_Handler(
    void)
{
    static uint16_t Timer_Ticks = 0;
    static uint16_t Elapsed_Milliseconds = 0;
    uint16_t milliseconds = 0;
    uint16_t diff = 0;
    uint8_t temp_reg;

    Timer_Ticks++;
    milliseconds = (Timer_Ticks * 1000) / INT_FREQ;
    diff = milliseconds - Elapsed_Milliseconds;
    if (diff >= 1) {
        Elapsed_Milliseconds = milliseconds;
        Timer_Milliseconds++;
        if (SilenceTime < 60000)
            SilenceTime++;
    }
    /* max resolution */
    if (Timer_Ticks >= INT_FREQ) {
        Timer_Ticks = 0;
        Elapsed_Milliseconds = 0;
    }

    /* clear interrupt */
    outportb(RTC_CMD_ADDR, 0x0C);       /* select RTC register C */
    temp_reg = inportb(RTC_DAT_ADDR);   /* read   RTC register C */
    /* signal end of interrupt to slave PIC */
    outportb(0xA0, 0x20);
    /* signal end of interrupt to master PIC */
    outportb(0x20, 0x20);
}

/* previous interrrupt vector */
static void interrupt(
    *OldVector) (
    );

void Timer_Cleanup(
    void)
{
    setvect(0x70, OldVector);
}

void Timer_Init(
    void)
{
    uint8_t RC = RTC_RS_Convert(INT_FREQ);

    /*  get old interrupt vector to re-install on exit */
    OldVector = getvect(0x70);
    /* disable interrupts */
    disable();
    /* set RTC int. vector for our routine */
    setvect(0x70, Timer_Interrupt_Handler);
    /* set register B with PIE enabled */
    outportb(RTC_CMD_ADDR, 0x0B);
    outportb(RTC_DAT_ADDR, 0x42);
    /* set register A to our frequency */
    outportb(RTC_CMD_ADDR, 0x0A);
    outportb(RTC_DAT_ADDR, (0x20 | (RC & 0x0F)));
    /* re-enable system interrupts */
    enable();
    atexit(Timer_Cleanup);
}

int Timer_Silence(
    void)
{
    uint16_t time_value;

    disable();
    time_value = SilenceTime;
    enable();

    return time_value;
}

void Timer_Silence_Reset(
    void)
{
    disable();
    SilenceTime = 0;
    enable();
}
