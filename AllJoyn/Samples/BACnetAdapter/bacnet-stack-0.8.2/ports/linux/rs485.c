/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
 Updated by Nikola Jelic 2011 <nikola.jelic@euroicc.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307
 USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/

/** @file linux/rs485.c  Provides Linux-specific functions for RS-485 serial. */

/* The module handles sending data out the RS-485 port */
/* and handles receiving data from the RS-485 port. */
/* Customize this file for your specific hardware */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Linux includes */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sched.h>
#include <linux/serial.h>       /* for struct serial_struct */
#include <math.h>       /* for calculation of custom divisor */
#include <sys/ioctl.h>

/* Local includes */
#include "mstp.h"
#include "rs485.h"
#include "fifo.h"

#include <sys/select.h>
#include <sys/time.h>

#include "dlmstp_linux.h"

/* Posix serial programming reference:
http://www.easysw.com/~mike/serial/serial.html */

/* Use ionice wrapper to improve serial performance:
   $ sudo ionice -c 1 -n 0 ./bin/bacserv 12345
*/

/* handle returned from open() */
static int RS485_Handle = -1;
/* baudrate settings are defined in <asm/termbits.h>, which is
   included by <termios.h> */
static unsigned int RS485_Baud = B38400;
/* serial port name, /dev/ttyS0,
  /dev/ttyUSB0 for USB->RS485 from B&B Electronics USOPTL4 */
static char *RS485_Port_Name = "/dev/ttyUSB0";
/* some terminal I/O have RS-485 specific functionality */
#ifndef RS485MOD
#define RS485MOD 0
#endif
/* serial I/O settings */
static struct termios RS485_oldtio;
/* for setting custom divisor */
static struct serial_struct RS485_oldserial;
/* indicator of special baud rate */
static bool RS485_SpecBaud = false;

/* Ring buffer for incoming bytes, in order to speed up the receiving. */
static FIFO_BUFFER Rx_FIFO;
/* buffer size needs to be a power of 2 */
static uint8_t Rx_Buffer[4096];

#define _POSIX_SOURCE 1 /* POSIX compliant source */

/*********************************************************************
* DESCRIPTION: Configures the interface name
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*********************************************************************/
void RS485_Set_Interface(
    char *ifname)
{
    /* note: expects a constant char, or char from the heap */
    if (ifname) {
        RS485_Port_Name = ifname;
    }
}

/*********************************************************************
* DESCRIPTION: Returns the interface name
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*********************************************************************/
const char *RS485_Interface(
    void)
{
    return RS485_Port_Name;
}

/****************************************************************************
* DESCRIPTION: Returns the baud rate that we are currently running at
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
uint32_t RS485_Get_Baud_Rate(
    void)
{
    uint32_t baud = 0;

    switch (RS485_Baud) {
        case B0:
            baud = 0;
            break;
        case B50:
            baud = 50;
            break;
        case B75:
            baud = 75;
            break;
        case B110:
            baud = 110;
            break;
        case B134:
            baud = 134;
            break;
        case B150:
            baud = 150;
            break;
        case B200:
            baud = 200;
            break;
        case B300:
            baud = 300;
            break;
        case B600:
            baud = 600;
            break;
        case B1200:
            baud = 1200;
            break;
        case B1800:
            baud = 1800;
            break;
        case B2400:
            baud = 2400;
            break;
        case B4800:
            baud = 4800;
            break;
        case B9600:
            baud = 9600;
            break;
        case B19200:
            baud = 19200;
            break;
        case B38400:
            if (!RS485_SpecBaud) {
                /* Linux asks for custom divisor
                   only when baud is set on 38400 */
                baud = 38400;
            } else {
                baud = 76800;
            }
            break;
        case B57600:
            baud = 57600;
            break;
        case B115200:
            baud = 115200;
            break;
        case B230400:
            baud = 230400;
            break;
        default:
            baud = 9600;
    }

    return baud;
}

/****************************************************************************
* DESCRIPTION: Returns the baud rate that we are currently running at
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
uint32_t RS485_Get_Port_Baud_Rate(
    volatile struct mstp_port_struct_t * mstp_port)
{
    uint32_t baud = 0;
    SHARED_MSTP_DATA *poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
    if (!poSharedData) {
        return 0;
    }
    switch (poSharedData->RS485_Baud) {
        case B0:
            baud = 0;
            break;
        case B50:
            baud = 50;
            break;
        case B75:
            baud = 75;
            break;
        case B110:
            baud = 110;
            break;
        case B134:
            baud = 134;
            break;
        case B150:
            baud = 150;
            break;
        case B200:
            baud = 200;
            break;
        case B300:
            baud = 300;
            break;
        case B600:
            baud = 600;
            break;
        case B1200:
            baud = 1200;
            break;
        case B1800:
            baud = 1800;
            break;
        case B2400:
            baud = 2400;
            break;
        case B4800:
            baud = 4800;
            break;
        case B9600:
            baud = 9600;
            break;
        case B19200:
            baud = 19200;
            break;
        case B38400:
            baud = 38400;
            break;
        case B57600:
            baud = 57600;
            break;
        case B115200:
            baud = 115200;
            break;
        case B230400:
            baud = 230400;
            break;
        default:
            baud = 9600;
            break;
    }

    return baud;
}

/****************************************************************************
* DESCRIPTION: Sets the baud rate for the chip USART
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
bool RS485_Set_Baud_Rate(
    uint32_t baud)
{
    bool valid = true;

    RS485_SpecBaud = false;
    switch (baud) {
        case 0:
            RS485_Baud = B0;
            break;
        case 50:
            RS485_Baud = B50;
            break;
        case 75:
            RS485_Baud = B75;
            break;
        case 110:
            RS485_Baud = B110;
            break;
        case 134:
            RS485_Baud = B134;
            break;
        case 150:
            RS485_Baud = B150;
            break;
        case 200:
            RS485_Baud = B200;
            break;
        case 300:
            RS485_Baud = B300;
            break;
        case 600:
            RS485_Baud = B600;
            break;
        case 1200:
            RS485_Baud = B1200;
            break;
        case 1800:
            RS485_Baud = B1800;
            break;
        case 2400:
            RS485_Baud = B2400;
            break;
        case 4800:
            RS485_Baud = B4800;
            break;
        case 9600:
            RS485_Baud = B9600;
            break;
        case 19200:
            RS485_Baud = B19200;
            break;
        case 38400:
            RS485_Baud = B38400;
            break;
        case 57600:
            RS485_Baud = B57600;
            break;
        case 76800:
            RS485_Baud = B38400;
            RS485_SpecBaud = true;
            break;
        case 115200:
            RS485_Baud = B115200;
            break;
        case 230400:
            RS485_Baud = B230400;
            break;
        default:
            valid = false;
            break;
    }

    if (valid) {
        /* FIXME: store the baud rate */
    }

    return valid;
}

/****************************************************************************
* DESCRIPTION: Transmit a frame on the wire
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Send_Frame(
    volatile struct mstp_port_struct_t *mstp_port,      /* port specific data */
    uint8_t * buffer,   /* frame to send (up to 501 bytes of data) */
    uint16_t nbytes)
{       /* number of bytes of data (up to 501) */
    uint32_t turnaround_time = Tturnaround * 1000;
    uint32_t baud;
    ssize_t written = 0;
    int greska;
    SHARED_MSTP_DATA *poSharedData = NULL;

    if (mstp_port) {
        poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
    }
    if (!poSharedData) {
        baud = RS485_Get_Baud_Rate();
        /* sleeping for turnaround time is necessary to give other devices
           time to change from sending to receiving state. */
        usleep(turnaround_time / baud);
        /*
           On  success,  the  number of bytes written are returned (zero indicates
           nothing was written).  On error, -1  is  returned,  and  errno  is  set
           appropriately.   If  count  is zero and the file descriptor refers to a
           regular file, 0 will be returned without causing any other effect.  For
           a special file, the results are not portable.
         */
        written = write(RS485_Handle, buffer, nbytes);
        greska = errno;
        if (written <= 0) {
            printf("write error: %s\n", strerror(greska));
        } else {
            /* wait until all output has been transmitted. */
            tcdrain(RS485_Handle);
        }
        /*  tcdrain(RS485_Handle); */
        /* per MSTP spec, sort of */
        if (mstp_port) {
            mstp_port->SilenceTimerReset((void *) mstp_port);
        }
    } else {
        baud = RS485_Get_Port_Baud_Rate(mstp_port);
        /* sleeping for turnaround time is necessary to give other devices
           time to change from sending to receiving state. */
        usleep(turnaround_time / baud);
        /*
           On  success,  the  number of bytes written are returned (zero indicates
           nothing was written).  On error, -1  is  returned,  and  errno  is  set
           appropriately.   If  count  is zero and the file descriptor refers to a
           regular file, 0 will be returned without causing any other effect.  For
           a special file, the results are not portable.
         */
        written = write(poSharedData->RS485_Handle, buffer, nbytes);
        greska = errno;
        if (written <= 0) {
            printf("write error: %s\n", strerror(greska));
        } else {
            /* wait until all output has been transmitted. */
            tcdrain(poSharedData->RS485_Handle);
        }
        /*  tcdrain(RS485_Handle); */
        /* per MSTP spec, sort of */
        if (mstp_port) {
            mstp_port->SilenceTimerReset((void *) mstp_port);
        }
    }

    return;
}

/****************************************************************************
* DESCRIPTION: Get a byte of receive data
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Check_UART_Data(
    volatile struct mstp_port_struct_t *mstp_port)
{
    fd_set input;
    struct timeval waiter;
    uint8_t buf[2048];
    int n;

    SHARED_MSTP_DATA *poSharedData = (SHARED_MSTP_DATA *) mstp_port->UserData;
    if (!poSharedData) {
        if (mstp_port->ReceiveError == true) {
            /* do nothing but wait for state machine to clear the error */
            /* burning time, so wait a longer time */
            waiter.tv_sec = 0;
            waiter.tv_usec = 5000;
        } else if (mstp_port->DataAvailable == false) {
            /* wait for state machine to read from the DataRegister */
            if (FIFO_Count(&Rx_FIFO) > 0) {
                /* data is available */
                mstp_port->DataRegister = FIFO_Get(&Rx_FIFO);
                mstp_port->DataAvailable = true;
                /* FIFO is giving data - don't wait very long */
                waiter.tv_sec = 0;
                waiter.tv_usec = 10;
            } else {
                /* FIFO is empty - wait a longer time */
                waiter.tv_sec = 0;
                waiter.tv_usec = 5000;
            }
        }
        /* grab bytes and stuff them into the FIFO every time */
        FD_ZERO(&input);
        FD_SET(RS485_Handle, &input);
        n = select(RS485_Handle + 1, &input, NULL, NULL, &waiter);
        if (n < 0) {
            return;
        }
        if (FD_ISSET(RS485_Handle, &input)) {
            n = read(RS485_Handle, buf, sizeof(buf));
            FIFO_Add(&Rx_FIFO, &buf[0], n);
        }
    } else {
        if (mstp_port->ReceiveError == true) {
            /* do nothing but wait for state machine to clear the error */
            /* burning time, so wait a longer time */
            waiter.tv_sec = 0;
            waiter.tv_usec = 5000;
        } else if (mstp_port->DataAvailable == false) {
            /* wait for state machine to read from the DataRegister */
            if (FIFO_Count(&poSharedData->Rx_FIFO) > 0) {
                /* data is available */
                mstp_port->DataRegister = FIFO_Get(&poSharedData->Rx_FIFO);
                mstp_port->DataAvailable = true;
                /* FIFO is giving data - don't wait very long */
                waiter.tv_sec = 0;
                waiter.tv_usec = 10;
            } else {
                /* FIFO is empty - wait a longer time */
                waiter.tv_sec = 0;
                waiter.tv_usec = 5000;
            }
        }
        /* grab bytes and stuff them into the FIFO every time */
        FD_ZERO(&input);
        FD_SET(poSharedData->RS485_Handle, &input);
        n = select(poSharedData->RS485_Handle + 1, &input, NULL, NULL,
            &waiter);
        if (n < 0) {
            return;
        }
        if (FD_ISSET(poSharedData->RS485_Handle, &input)) {
            n = read(poSharedData->RS485_Handle, buf, sizeof(buf));
            FIFO_Add(&poSharedData->Rx_FIFO, &buf[0], n);
        }
    }
}

void RS485_Cleanup(
    void)
{
    /* restore the old port settings */
    tcsetattr(RS485_Handle, TCSANOW, &RS485_oldtio);
    ioctl(RS485_Handle, TIOCSSERIAL, &RS485_oldserial);
    close(RS485_Handle);
}


void RS485_Initialize(
    void)
{
    struct termios newtio;
    struct serial_struct newserial;
    float baud_error = 0.0;

    printf("RS485: Initializing %s", RS485_Port_Name);
    /*
       Open device for reading and writing.
       Blocking mode - more CPU effecient
     */
    RS485_Handle = open(RS485_Port_Name, O_RDWR | O_NOCTTY /*| O_NDELAY */ );
    if (RS485_Handle < 0) {
        perror(RS485_Port_Name);
        exit(-1);
    }
#if 0
    /* non blocking for the read */
    fcntl(RS485_Handle, F_SETFL, FNDELAY);
#else
    /* efficient blocking for the read */
    fcntl(RS485_Handle, F_SETFL, 0);
#endif
    /* save current serial port settings */
    tcgetattr(RS485_Handle, &RS485_oldtio);
    /* we read the old serial setup */
    ioctl(RS485_Handle, TIOCGSERIAL, &RS485_oldserial);
    /* we need a copy of existing settings */
    memcpy(&newserial, &RS485_oldserial, sizeof(struct serial_struct));
    /* clear struct for new port settings */
    bzero(&newtio, sizeof(newtio));
    /*
       BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
       CRTSCTS : output hardware flow control (only used if the cable has
       all necessary lines. See sect. 7 of Serial-HOWTO)
       CS8     : 8n1 (8bit,no parity,1 stopbit)
       CLOCAL  : local connection, no modem contol
       CREAD   : enable receiving characters
     */
    newtio.c_cflag = RS485_Baud | CS8 | CLOCAL | CREAD | RS485MOD;
    /* Raw input */
    newtio.c_iflag = 0;
    /* Raw output */
    newtio.c_oflag = 0;
    /* no processing */
    newtio.c_lflag = 0;
    /* activate the settings for the port after flushing I/O */
    tcsetattr(RS485_Handle, TCSAFLUSH, &newtio);
    if (RS485_SpecBaud) {
        /* 76800, custom divisor must be set */
        newserial.flags |= ASYNC_SPD_CUST;
        newserial.custom_divisor =
            round(((float) newserial.baud_base) / 76800);
        /* we must check that we calculated some sane value;
           small baud bases yield bad custom divisor values */
        baud_error =
            fabs(1 -
            ((float) newserial.baud_base) /
            ((float) newserial.custom_divisor) / 76800);
        if ((newserial.custom_divisor == 0) || (baud_error > 0.02)) {
            /* bad divisor */
            fprintf(stderr, "bad custom divisor %d, base baud %d\n",
                newserial.custom_divisor, newserial.baud_base);
            exit(EXIT_FAILURE);
        }
        /* if all goes well, set new divisor */
        ioctl(RS485_Handle, TIOCSSERIAL, &newserial);
    }
    printf(" at Baud Rate %u", RS485_Get_Baud_Rate());
    /* destructor */
    atexit(RS485_Cleanup);
    /* flush any data waiting */
    usleep(200000);
    tcflush(RS485_Handle, TCIOFLUSH);
    /* ringbuffer */
    FIFO_Init(&Rx_FIFO, Rx_Buffer, sizeof(Rx_Buffer));
    printf("=success!\n");
}

void RS485_Print_Ports(void)
{
    /* note: format for Wireshark ExtCap */
    //printf("interface {value=/dev/ttyUSB%u}"
    //    "{display=MS/TP Capture on /dev/ttyUSB%u}\n", i, i);
    /* FIXME: add code to print ports */
}

#ifdef TEST_RS485
#include <string.h>
int main(
    int argc,
    char *argv[])
{
    volatile struct mstp_port_struct_t mstp_port = {0};
    uint8_t token_buf[8] = {0x55, 0xFF, 0x00, 0x7E, 0x07, 0x00, 0x00, 0xFD};
    uint8_t pfm_buf[8] = {0x55, 0xFF, 0x01, 0x67, 0x07, 0x00, 0x00, 0x3E};
    long baud = 38400;
    bool write_token = false;
    bool write_pfm = false;

    /* argv has the "/dev/ttyS0" or some other device */
    if (argc > 1) {
        RS485_Set_Interface(argv[1]);
    }
    if (argc > 2) {
        baud = strtol(argv[2], NULL, 0);
    }
    if (argc > 3) {
        if (strcmp("token", argv[3]) == 0) {
            write_token = true;
        }
        if (strcmp("pfm", argv[3]) == 0) {
            write_pfm = true;
        }
    }
    RS485_Set_Baud_Rate(baud);
    RS485_Initialize();
    for (;;) {
        if (write_token) {
            RS485_Send_Frame(NULL,token_buf, sizeof(token_buf));
            usleep(25000);
        } else if (write_pfm) {
            RS485_Send_Frame(NULL,pfm_buf, sizeof(pfm_buf));
            usleep(100000);
        } else {
            RS485_Check_UART_Data(&mstp_port);
            if (mstp_port.DataAvailable) {
                fprintf(stderr, "%02X ", mstp_port.DataRegister);
                mstp_port.DataAvailable = false;
            }
        }
    }

    return 0;
}
#endif
