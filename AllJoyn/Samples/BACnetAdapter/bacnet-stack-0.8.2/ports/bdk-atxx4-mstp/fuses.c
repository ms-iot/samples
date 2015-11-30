/************************************************************************
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
*
*************************************************************************/
#include "hardware.h"

#if defined(__GNUC__) && (__GNUC__ >= 4)
/* AVR fuse settings for ATmega644P */
FUSES = {
    /* == LOW FUSE or LFUSE settings == */
    /* CKSEL3..0- Clock Select Configuration
       CKSEL3  CKSEL2  CKSEL1  CKSEL0 Description
       1       1       1       x    1111-1000=Low Power Crystal Oscillator
       0       1       1       x    0111-0110=Full Swing Crystal Oscillator
       0       1       0       x    0101-0100=Low Frequency Crystal Oscillator
       0       0       1       1    Internal 128kHz RC Oscillator
       0       0       1       0    Calibrated Internal RC Oscillator
       0       0       0       0    External Clock]

       SUT1..0 - Clock Start Up Time selection
       If CKSEL0=0, then SUT1..0 is 14CK+: 00=4.1ms,01=65ms,10=BOD,11=4.1ms
       If CKSEL0=1, then SUT1..0 is 14CK+: 00=65ms,01=BOD,10=4.1ms,11=65ms
       BOD means wait until internal Brown Out Detect Voltage is sufficient.
     */
    /* CKOUT: clock output on CKOUT pin */
    /* CKDIV8: divide clock by 8 */
    /* External Ceramic Resonator - configuration */
    /* Full Swing Crystal Oscillator Clock Selection */
    /* Ceramic resonator, slowly rising power 1K CK 14CK + 65 ms */
    /* .low = (FUSE_CKSEL3 & FUSE_SUT0 & FUSE_SUT1), */
    /* Crystal Oscillator, 16K CK + 14CK + BOD Enabled */
    /* note: fuses are enabled by clearing the bit, so
       any fuses listed below are cleared fuses,
       or are CKSEL or SUT bits that are zero. */
    .low = (FUSE_CKSEL3 & FUSE_SUT1),
        /* == HIGH FUSE or HFUSE settings == */
        /* BOOTRST: Enable Bootloader Reset Vector */
        /* EESAVE: Enable preserve EEPROM on Chip Erase */
        /* WDTON: Enable watchdog timer always on */
        /* SPIEN: Enable Serial Program and Data Downloading */
        /* JTAGEN: Enable JTAG */
        /* OCDEN: Enable OCD */
        /* BOOTSZ configuration:
           BOOTSZ1 BOOTSZ0 Boot Size
           ------- ------- ---------
           1       1      512
           1       0     1024
           0       1     2048
           0       0     4096
         */
        /* note: fuses are enabled by clearing the bit, so
           any fuses listed below are cleared fuses,
           or are BOOTSZ bits that are zero. */
        .high = (FUSE_BOOTSZ1 & FUSE_EESAVE & FUSE_SPIEN & FUSE_JTAGEN),
        /* == EXTENDED FUSE or EFUSE settings == */
        /* BODLEVEL configuration
           BODLEVEL2 BODLEVEL1 BODLEVEL0 Voltage
           --------- --------- --------- --------
           1         1         1     disabled
           1         1         0       1.8V
           1         0         1       2.7V
           1         0         0       4.3V
         */
        /* note: fuses are enabled by clearing the bit, so
           any fuses listed below are cleared fuses,
           or are BODLEVEL bits that are zero. */
        /* Brown-out detection VCC=4.3V */
        .extended = (FUSE_BODLEVEL1 & FUSE_BODLEVEL0)
};

/* AVR lock bits - unlocked */
LOCKBITS = LOCKBITS_DEFAULT;
#endif
