/**************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
* Used algorithm and code from Joerg Wunsch and Ruwan Jayanetti.
* http://www.nongnu.org/avr-libc/user-manual/group__twi__demo.html
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
/* me */
#include "seeprom.h"

/* the SEEPROM chip select bits A2, A1, and A0 are grounded */
/* control byte is 0xAx */
#ifndef SEEPROM_I2C_ADDRESS
#define SEEPROM_I2C_ADDRESS 0xA0
#endif

/* SEEPROM Clock Frequency */
#ifndef SEEPROM_I2C_CLOCK
#define SEEPROM_I2C_CLOCK 400000UL
#endif

/* max number of bytes that can be written in a single write */
#ifndef SEEPROM_PAGE_SIZE
#define SEEPROM_PAGE_SIZE 128
#endif

/* word addressing - is it 8-bit or 16-bit */
#ifndef SEEPROM_WORD_ADDRESS_16BIT
#define SEEPROM_WORD_ADDRESS_16BIT 1
#endif

/* maximum write cycle time in milliseconds - see datasheet */
#ifndef EEPROM_WRITE_CYCLE
#define EEPROM_WRITE_CYCLE 5
#endif

/* The lower 3 bits of TWSR are reserved on the ATmega163 */
#define TW_STATUS_MASK (_BV(TWS7)|_BV(TWS6)|_BV(TWS5)|_BV(TWS4)|_BV(TWS3))
/* start condition transmitted */
#define TW_START 0x08
/* repeated start condition transmitted */
#define TW_REP_START 0x10
/* ***Master Transmitter*** */
/* SLA+W transmitted, ACK received */
#define TW_MT_SLA_ACK 0x18
/* SLA+W transmitted, NACK received */
#define TW_MT_SLA_NACK 0x20
/* data transmitted, ACK received */
#define TW_MT_DATA_ACK 0x28
/* data transmitted, NACK received */
#define TW_MT_DATA_NACK 0x30
/* arbitration lost in SLA+W or data */
#define TW_MT_ARB_LOST 0x38
/* ***Master Receiver*** */
/* arbitration lost in SLA+R or NACK */
#define TW_MR_ARB_LOST 0x38
/* SLA+R transmitted, ACK received */
#define TW_MR_SLA_ACK 0x40
/* SLA+R transmitted, NACK received */
#define TW_MR_SLA_NACK 0x48
/* data received, ACK returned */
#define TW_MR_DATA_ACK 0x50
/* data received, NACK returned */
#define TW_MR_DATA_NACK 0x58

/* SLA+R address */
#define TW_READ 1
/* SLA+W address */
#define TW_WRITE 0

/* Number of iterations is the max amount to wait for write cycle
   to complete a full page write */
/* .005s/.000025=200 */
#define MAX_ITER (((SEEPROM_I2C_CLOCK/1000)/10)*SEEPROM_WRITE_CYCLE)

/*************************************************************************
* DESCRIPTION: Return bytes from SEEPROM memory at address
* RETURN: number of bytes read, or -1 on error
* NOTES: none
**************************************************************************/
int seeprom_bytes_read(
    uint16_t eeaddr,    /* SEEPROM starting memory address */
    uint8_t * buf,      /* data to store */
    int len)
{       /* number of bytes of data to read */
    uint8_t sla, twcr, n = 0;
    int rv = 0;
    uint8_t twst;       /* status - only valid while TWINT is set. */
    uint16_t timeout = 0xFFFF;

#if SEEPROM_WORD_ADDRESS_16BIT
    /* 16bit address devices need only TWI Device Address */
    sla = SEEPROM_I2C_ADDRESS;
#else
    /* patch high bits of EEPROM address into SLA */
    sla = SEEPROM_I2C_ADDRESS | (((eeaddr >> 8) & 0x07) << 1);
#endif
    /* First cycle: master transmitter mode */
  restart:
    if (n++ >= MAX_ITER) {
        return -1;
    }
  begin:
    /* send start condition */
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    /* wait for transmission */
    while ((TWCR & _BV(TWINT)) == 0) {
        timeout--;
        if (timeout == 0) {
            return -1;
        }
    }
    twst = TWSR & TW_STATUS_MASK;
    switch (twst) {
        case TW_REP_START:
            /* OK, but should not happen */
        case TW_START:
            break;
        case TW_MT_ARB_LOST:
            /* Since the TWI bus is multi-master capable,
               there is potential for a bus contention when
               one master starts to access the bus. */
            goto begin;
        default:
            /* error: not in start condition */
            /* NB: do /not/ send stop condition */
            return -1;
    }
    /* Next, the device slave is going to be reselected using a repeated
       start condition which is meant to guarantee that the bus arbitration
       will remain at the current master.  This uses the same slave address
       (SLA), but this time with read intent (R/~W bit set to 1) in order
       to request the device slave to start transfering data from the slave
       to the master in the next packet. */
    /* send SLA+W */
    TWDR = sla | TW_WRITE;
    /* clear interrupt to start transmission */
    TWCR = _BV(TWINT) | _BV(TWEN);
    /* wait for transmission */
    while ((TWCR & _BV(TWINT)) == 0);
    twst = TWSR & TW_STATUS_MASK;
    switch (twst) {
        case TW_MT_SLA_ACK:
            break;
        case TW_MT_SLA_NACK:
            /* nack during select: device busy writing */
            /* If the EEPROM device is still busy writing one or more cells
               after a previous write request, it will simply leave its bus
               interface drivers at high impedance, and does not respond to
               a selection in any way at all. */
            goto restart;
        case TW_MT_ARB_LOST:
            /* re-arbitrate */
            goto begin;
        default:
            /* must send stop condition */
            goto error;
    }
#if SEEPROM_WORD_ADDRESS_16BIT
    /* 16 bit word address device, send high 8 bits of addr */
    TWDR = (eeaddr >> 8);
    /* clear interrupt to start transmission */
    TWCR = _BV(TWINT) | _BV(TWEN);
    /* wait for transmission */
    while ((TWCR & _BV(TWINT)) == 0);
    twst = TWSR & TW_STATUS_MASK;
    switch (twst) {
        case TW_MT_DATA_ACK:
            break;
        case TW_MT_DATA_NACK:
            goto quit;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            /* must send stop condition */
            goto error;
    }
#endif
    /* low 8 bits of addr */
    TWDR = eeaddr;
    /* clear interrupt to start transmission */
    TWCR = _BV(TWINT) | _BV(TWEN);
    /* wait for transmission */
    while ((TWCR & _BV(TWINT)) == 0);
    twst = TWSR & TW_STATUS_MASK;
    switch (twst) {
        case TW_MT_DATA_ACK:
            break;
        case TW_MT_DATA_NACK:
            goto quit;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            /* must send stop condition */
            goto error;
    }

    /* This is called master receiver mode: the bus master still supplies
       the SCL clock, but the device slave drives the SDA line with the
       appropriate data. After 8 data bits, the master responds with an ACK
       bit (SDA driven low) in order to request another data transfer from
       the slave, or it can leave the SDA line high (NACK), indicating to
       the slave that it is going to stop the transfer now.
       Assertion of ACK is handled by setting the TWEA bit in TWCR when
       starting the current transfer. */
    /* Next cycle(s): master receiver mode */
    /* send repeated start condition */
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    /* wait for transmission */
    while ((TWCR & _BV(TWINT)) == 0);
    twst = TWSR & TW_STATUS_MASK;
    switch (twst) {
        case TW_START:
            /* OK, but should not happen */
        case TW_REP_START:
            break;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            goto error;
    }

    /* send SLA+R */
    TWDR = sla | TW_READ;
    /* clear interrupt to start transmission */
    TWCR = _BV(TWINT) | _BV(TWEN);
    /* wait for transmission */
    while ((TWCR & _BV(TWINT)) == 0);
    twst = TWSR & TW_STATUS_MASK;
    switch (twst) {
        case TW_MR_SLA_ACK:
            break;
        case TW_MR_SLA_NACK:
            goto quit;
        case TW_MR_ARB_LOST:
            goto begin;
        default:
            goto error;
    }
    /* The control word sent out in order to initiate the transfer of the
       next data packet is initially set up to assert the TWEA bit.
       During the last loop iteration, TWEA is de-asserted so the client
       will get informed that no further transfer is desired. */
    twcr = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
    for (; len > 0; len--) {
        if (len == 1) {
            /* send NAK this time */
            twcr = _BV(TWINT) | _BV(TWEN);
        }
        /* clear int to start transmission */
        TWCR = twcr;
        /* wait for transmission */
        while ((TWCR & _BV(TWINT)) == 0);
        twst = TWSR & TW_STATUS_MASK;
        switch (twst) {
            case TW_MR_DATA_NACK:
                /* force end of loop */
                len = 0;
                /* FALLTHROUGH */
            case TW_MR_DATA_ACK:
                *buf = TWDR;
                buf++;
                rv++;
                break;
            default:
                goto error;
        }
    }
  quit:
    /* Except in the case of lost arbitration, all bus transactions
       must properly be terminated by the master initiating a
       stop condition. */
    /* send stop condition */
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
    return rv;
  error:
    rv = -1;
    goto quit;
}

/*************************************************************************
* DESCRIPTION: Write some data and wait until it is sent
* RETURN: number of bytes written, or -1 on error
* NOTES: only writes from offset to end of page.
**************************************************************************/
static int seeprom_bytes_write_page(
    uint16_t eeaddr,    /* SEEPROM starting memory address */
    uint8_t * buf,      /* data to send */
    int len)
{       /* number of bytes of data */
    uint8_t sla, n = 0;
    int rv = 0;
    uint16_t endaddr;
    uint8_t twst;       /* status - only valid while TWINT is set. */
    uint16_t page_end_addr;
    uint16_t timeout = 0xFFFF;

    /* limit the length to end of the EEPROM page */
    page_end_addr = eeaddr | (SEEPROM_PAGE_SIZE - 1);
    if ((eeaddr + len) > page_end_addr) {
        endaddr = page_end_addr + 1;
        len = endaddr - eeaddr;
    }
#if SEEPROM_WORD_ADDRESS_16BIT
    /* 16bit address devices need only TWI Device Address */
    sla = SEEPROM_I2C_ADDRESS;
#else
    /* patch high bits of EEPROM address into SLA */
    sla = SEEPROM_I2C_ADDRESS | (((eeaddr >> 8) & 0x07) << 1);
#endif
  restart:
    if (n++ >= MAX_ITER) {
        return -1;
    }
  begin:
    /* Writing to the EEPROM device is simpler than reading,
       since only a master transmitter mode transfer is needed.
       Note that the first packet after the SLA+W selection is
       always considered to be the EEPROM address for the next operation.
       This packet is exactly the same as the one above sent before
       starting to read the device.
       In case a master transmitter mode transfer is going to send
       more than one data packet, all following packets will be considered
       data bytes to write at the indicated address.
       The internal address pointer will be incremented after each
       write operation. */
    /* send start condition */
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    /* wait for transmission */
    while ((TWCR & _BV(TWINT)) == 0) {
        timeout--;
        if (timeout == 0) {
            return -1;
        }
    }
    twst = TWSR & TW_STATUS_MASK;
    switch (twst) {
        case TW_REP_START:
            /* OK, but should not happen */
        case TW_START:
            break;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            /* error: not in start condition */
            /* NB: do /not/ send stop condition */
            return -1;
    }
    /* send SLA+W */
    TWDR = sla | TW_WRITE;
    /* clear interrupt to start transmission */
    TWCR = _BV(TWINT) | _BV(TWEN);
    /* wait for transmission */
    while ((TWCR & _BV(TWINT)) == 0);
    twst = TWSR & TW_STATUS_MASK;
    switch (twst) {
        case TW_MT_SLA_ACK:
            break;
        case TW_MT_SLA_NACK:
            /* nack during select: device busy writing */
            goto restart;
        case TW_MT_ARB_LOST:
            /* re-arbitrate */
            goto begin;
        default:
            /* must send stop condition */
            goto error;
    }
#if SEEPROM_WORD_ADDRESS_16BIT
    /* 16 bit word address device, send high 8 bits of addr */
    TWDR = (eeaddr >> 8);
    /* clear interrupt to start transmission */
    TWCR = _BV(TWINT) | _BV(TWEN);
    /* wait for transmission */
    while ((TWCR & _BV(TWINT)) == 0);
    twst = TWSR & TW_STATUS_MASK;
    switch (twst) {
        case TW_MT_DATA_ACK:
            break;
        case TW_MT_DATA_NACK:
            goto quit;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            /* must send stop condition */
            goto error;
    }
#endif
    /* low 8 bits of addr */
    TWDR = eeaddr;
    /* clear interrupt to start transmission */
    TWCR = _BV(TWINT) | _BV(TWEN);
    /* wait for transmission */
    while ((TWCR & _BV(TWINT)) == 0) {
    };
    twst = TWSR & TW_STATUS_MASK;
    switch (twst) {
        case TW_MT_DATA_ACK:
            break;
        case TW_MT_DATA_NACK:
            goto quit;
        case TW_MT_ARB_LOST:
            goto begin;
        default:
            /* must send stop condition */
            goto error;
    }
    for (; len > 0; len--) {
        TWDR = *buf;
        /* start transmission */
        TWCR = _BV(TWINT) | _BV(TWEN);
        /* wait for transmission */
        while ((TWCR & _BV(TWINT)) == 0);
        twst = TWSR & TW_STATUS_MASK;
        switch (twst) {
            case TW_MT_DATA_NACK:
                /* device write protected -- Note [16] */
                goto error;
            case TW_MT_DATA_ACK:
                buf++;
                rv++;
                break;
            default:
                goto error;
        }
    }
  quit:
    /* send stop condition */
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);

    return rv;

  error:
    rv = -1;
    goto quit;
}

/*************************************************************************
* DESCRIPTION: Write some data and wait until it is sent
* RETURN: number of bytes written, or -1 on error
* NOTES:
*   When the word address, internally generated,
*   reaches the page boundary, the following
*   byte is placed at the beginning of the same
*   page. If more than 64 data words are
*   transmitted to the EEPROM, the data word
*   address will "roll over" and previous data will be
*   overwritten. The address "roll over" during write
*   is from the last byte of the current page to the
*   first byte of the same page.
**************************************************************************/
int seeprom_bytes_write(
    uint16_t off,       /* SEEPROM starting memory address */
    uint8_t * buf,      /* data to send */
    int len)
{       /* number of bytes of data */
    int status = 0;
    int rv = 0;

    while (len) {
        status = seeprom_bytes_write_page(off, buf, len);
        if (status <= 0) {
            if (rv == 0) {
                rv = status;
            }
            break;
        }
        buf += status;
        off += status;
        len -= status;
        rv += status;
    }

    return rv;
}

/*************************************************************************
* Description: Initialize the SEEPROM TWI connection
* Returns: none
* Notes: none
**************************************************************************/
void seeprom_init(
    void)
{
    /* bit rate prescaler */
    TWSR = 0;
    TWCR = _BV(TWEN) | _BV(TWEA);
    /* bit rate */
    /* SCL freq = F_CPU/(16+2*TWBR*4^TWPS) */
    /* since TWPS in TWSR is set to zero, 4^TWPS resolves to 1 */
    TWBR = (F_CPU / SEEPROM_I2C_CLOCK - 16) / 2;
    /* my address */
    TWAR = 0;
}
