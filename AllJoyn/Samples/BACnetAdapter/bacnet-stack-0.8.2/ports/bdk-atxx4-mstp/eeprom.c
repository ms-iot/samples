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
#include <stdlib.h>
#include "hardware.h"
#include "eeprom.h"

/* Internal EEPROM of the AVR - http://supp.iar.com/Support/?note=45745 */


int eeprom_bytes_read(
    uint16_t eeaddr,    /* EEPROM starting memory address (offset of zero) */
    uint8_t * buf,      /* data to store */
    int len)
{       /* number of bytes of data to read */
    int count = 0;      /* return value */

    while (len) {
        __EEGET(buf[count], eeaddr);
        count++;
        eeaddr++;
        len--;
    }

    return count;
}

int eeprom_bytes_write(
    uint16_t eeaddr,    /* EEPROM starting memory address */
    uint8_t * buf,      /* data to send */
    int len)
{       /* number of bytes of data */
    int count = 0;

    while (len) {
        __EEPUT(eeaddr, buf[count]);
        count++;
        eeaddr++;
        len--;
    }

    return count;
}
