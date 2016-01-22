/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2004 Steve Karg

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

/** @file win32/rs485.c  Provides Windows-specific functions for RS-485 */

/* Suggested USB to RS485 devices:
   B&B Electronics USOPTL4
   SerialGear USB-COMi-SI-M
   USB-RS485-WE-1800-BT
*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mstp.h"
#include "dlmstp.h"
#define WIN32_LEAN_AND_MEAN
#define STRICT 1
#include <windows.h>
#include "rs485.h"
#include "fifo.h"

/* details from Serial Communications in Win32 at MSDN */

/* Win32 handle for the port */
HANDLE RS485_Handle;
/* Original COM Timeouts */
static COMMTIMEOUTS RS485_Timeouts;
/* COM port name COM1, COM2, etc  */
static char RS485_Port_Name[256] = "COM4";
/* baud rate - MS enumerated
    CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400,
    CBR_4800, CBR_9600, CBR_14400, CBR_19200, CBR_38400,
    CBR_56000, CBR_57600, CBR_115200, CBR_128000, CBR_256000 */
static DWORD RS485_Baud = CBR_38400;
/* ByteSize in bits: 5, 6, 7, 8 are valid */
static DWORD RS485_ByteSize = 8;
/* Parity - MS enumerated:
    NOPARITY, EVENPARITY, ODDPARITY, MARKPARITY, SPACEPARITY */
static DWORD RS485_Parity = NOPARITY;
/* StopBits - MS enumerated:
    ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS */
static DWORD RS485_StopBits = ONESTOPBIT;
/* DTRControl - MS enumerated:
    DTR_CONTROL_ENABLE, DTR_CONTROL_DISABLE, DTR_CONTROL_HANDSHAKE */
static DWORD RS485_DTRControl = DTR_CONTROL_DISABLE;
/* RTSControl - MS enumerated:
    RTS_CONTROL_ENABLE, RTS_CONTROL_DISABLE,
    RTS_CONTROL_HANDSHAKE, RTS_CONTROL_TOGGLE */
static DWORD RS485_RTSControl = RTS_CONTROL_DISABLE;

/****************************************************************************
* DESCRIPTION: Change the characters in a string to uppercase
* RETURN:      nothing
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
static void strupper(
    char *str)
{
    char *p;
    for (p = str; *p != '\0'; ++p) {
        *p = (char) toupper(*p);
    }
}

/****************************************************************************
* DESCRIPTION: Initializes the RS485 hardware and variables, and starts in
*              receive mode.
* RETURN:      none
* ALGORITHM:   none
* NOTES:       expects a constant char ifname, or char from the heap
*****************************************************************************/
void RS485_Set_Interface(
    char *ifname)
{
    /* For COM ports greater than 9 you have to use a special syntax
       for CreateFile. The syntax also works for COM ports 1-9. */
    /* http://support.microsoft.com/kb/115831 */
    if (ifname) {
        strupper(ifname);
        if (strncmp("COM", ifname, 3) == 0) {
            if (strlen(ifname) > 3) {
                sprintf(RS485_Port_Name, "\\\\.\\COM%i", atoi(ifname + 3));
                fprintf(stderr, "Adjusted interface name to %s\r\n",
                    RS485_Port_Name);
            }
        }
    }
}

/****************************************************************************
* DESCRIPTION: Check the serial port to see if port exists
* RETURN:      true if port exists
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
bool RS485_Interface_Valid(
    unsigned port_number)
{
    HANDLE h = 0;
    DWORD err = 0;
    bool status = false;
    char ifname[255] = "";

    sprintf(ifname, "\\\\.\\COM%u", port_number);
    h = CreateFile(ifname, GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        err = GetLastError();
        if ((err == ERROR_ACCESS_DENIED) || (err == ERROR_GEN_FAILURE) ||
            (err == ERROR_SHARING_VIOLATION) || (err == ERROR_SEM_TIMEOUT)) {
            status = true;
        }
    } else {
        status = true;
        CloseHandle(h);
    }

    return status;
}

const char *RS485_Interface(
    void)
{
    return RS485_Port_Name;
}

void RS485_Print_Error(
    void)
{
    LPVOID lpMsgBuf;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) & lpMsgBuf, 0, NULL);
    MessageBox(NULL, lpMsgBuf, "GetLastError", MB_OK | MB_ICONINFORMATION);
    LocalFree(lpMsgBuf);

    return;
}

static void RS485_Configure_Status(
    void)
{
    DCB dcb = { 0 };
    COMMTIMEOUTS ctNew;


    dcb.DCBlength = sizeof(dcb);
    /* get current DCB settings */
    if (!GetCommState(RS485_Handle, &dcb)) {
        fprintf(stderr, "Unable to get status from %s\n", RS485_Port_Name);
        RS485_Print_Error();
        exit(1);
    }

    /* update DCB rate, byte size, parity, and stop bits size */
    dcb.BaudRate = RS485_Baud;
    dcb.ByteSize = (unsigned char) RS485_ByteSize;
    dcb.Parity = (unsigned char) RS485_Parity;
    dcb.StopBits = (unsigned char) RS485_StopBits;

    /* update flow control settings */
    dcb.fDtrControl = RS485_DTRControl;
    dcb.fRtsControl = RS485_RTSControl;
    /*
       dcb.fOutxCtsFlow    = CTSOUTFLOW(TTYInfo);
       dcb.fOutxDsrFlow    = DSROUTFLOW(TTYInfo);
       dcb.fDsrSensitivity = DSRINFLOW(TTYInfo);
       dcb.fOutX           = XONXOFFOUTFLOW(TTYInfo);
       dcb.fInX            = XONXOFFINFLOW(TTYInfo);
       dcb.fTXContinueOnXoff = TXAFTERXOFFSENT(TTYInfo);
       dcb.XonChar         = XONCHAR(TTYInfo);
       dcb.XoffChar        = XOFFCHAR(TTYInfo);
       dcb.XonLim          = XONLIMIT(TTYInfo);
       dcb.XoffLim         = XOFFLIMIT(TTYInfo);
       // DCB settings not in the user's control
       dcb.fParity = TRUE;
     */
    if (!SetCommState(RS485_Handle, &dcb)) {
        fprintf(stderr, "Unable to set status on %s\n", RS485_Port_Name);
        RS485_Print_Error();
    }
    /* configure the COM port timeout values */
    ctNew.ReadIntervalTimeout = MAXDWORD;
    ctNew.ReadTotalTimeoutMultiplier = MAXDWORD;
    ctNew.ReadTotalTimeoutConstant = 1000;
    ctNew.WriteTotalTimeoutMultiplier = 0;
    ctNew.WriteTotalTimeoutConstant = 0;
    if (!SetCommTimeouts(RS485_Handle, &ctNew)) {
        RS485_Print_Error();
    }
    /* Get rid of any stray characters */
    if (!PurgeComm(RS485_Handle, PURGE_TXABORT | PURGE_RXABORT)) {
        fprintf(stderr, "Unable to purge %s\n", RS485_Port_Name);
        RS485_Print_Error();
    }
    /* Set the Comm buffer size */
    SetupComm(RS485_Handle, MAX_MPDU, MAX_MPDU);
    /* raise DTR */
    if (!EscapeCommFunction(RS485_Handle, SETDTR)) {
        fprintf(stderr, "Unable to set DTR on %s\n", RS485_Port_Name);
        RS485_Print_Error();
    }
}

/****************************************************************************
* DESCRIPTION: Cleans up any handles that were created at startup.
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
static void RS485_Cleanup(
    void)
{
    if (!EscapeCommFunction(RS485_Handle, CLRDTR)) {
        RS485_Print_Error();
    }

    if (!SetCommTimeouts(RS485_Handle, &RS485_Timeouts)) {
        RS485_Print_Error();
    }

    CloseHandle(RS485_Handle);
}

/****************************************************************************
* DESCRIPTION: Initializes the RS485 hardware and variables, and starts in
*              receive mode.
* RETURN:      none
* ALGORITHM:   none
* NOTES:       none
*****************************************************************************/
void RS485_Initialize(
    void)
{
    RS485_Handle =
        CreateFile(RS485_Port_Name, GENERIC_READ | GENERIC_WRITE, 0, 0,
        OPEN_EXISTING,
        /*FILE_FLAG_OVERLAPPED */ 0,
        0);
    if (RS485_Handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Unable to open %s\n", RS485_Port_Name);
        RS485_Print_Error();
        exit(1);
    }
    if (!GetCommTimeouts(RS485_Handle, &RS485_Timeouts)) {
        RS485_Print_Error();
    }
    RS485_Configure_Status();
#if PRINT_ENABLED
    fprintf(stderr, "RS485 Interface: %s\n", RS485_Port_Name);
#endif

    atexit(RS485_Cleanup);

    return;
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
    switch (RS485_Baud) {
        case CBR_19200:
            return 19200;
        case CBR_38400:
            return 38400;
        case CBR_57600:
            return 57600;
        case CBR_115200:
            return 115200;
        case CBR_110:
            return 110;
        case CBR_300:
            return 300;
        case CBR_600:
            return 600;
        case CBR_1200:
            return 1200;
        case CBR_2400:
            return 2400;
        case CBR_4800:
            return 4800;
        case CBR_14400:
            return 14400;
        case CBR_56000:
            return 56000;
        case CBR_128000:
            return 128000;
        case CBR_256000:
            return 256000;
        case 76800:
            /* See comments in RS485_Set_Baud_Rate() below
             * also look at definition of CBR_xx in winbase.h
             * some serial drivers will only support the defined
             * baud rates but others will try and configure the
             * requested baud rate (or as close as they can get)
             */
            return 76800;
        case CBR_9600:
        default:
            return 9600;
    }
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

    switch (baud) {
        case 9600:
            RS485_Baud = CBR_9600;
            break;
        case 19200:
            RS485_Baud = CBR_19200;
            break;
        case 38400:
            RS485_Baud = CBR_38400;
            break;
        case 57600:
            RS485_Baud = CBR_57600;
            break;
        case 115200:
            RS485_Baud = CBR_115200;
            break;
        case 110:
            RS485_Baud = CBR_110;
            break;
        case 300:
            RS485_Baud = CBR_300;
            break;
        case 600:
            RS485_Baud = CBR_600;
            break;
        case 1200:
            RS485_Baud = CBR_1200;
            break;
        case 2400:
            RS485_Baud = CBR_2400;
            break;
        case 4800:
            RS485_Baud = CBR_4800;
            break;
        case 14400:
            RS485_Baud = CBR_14400;
            break;
        case 56000:
            RS485_Baud = CBR_56000;
            break;
        case 128000:
            RS485_Baud = CBR_128000;
            break;
        case 256000:
            RS485_Baud = CBR_256000;
            break;
        case 76800:
            /* I'm using the B&B Electronics USOPTL4 USB RS485 adapter
             * on Win 7 and building with VS2008 Express Edition and it
             * seems to work for the most part if I use the following.
             * I get the occasional data errors especially if the devices
             * are transmitting with 1 stop bit (some devices receive with
             * 1 stop bit but effectivly end up transmitting with 2 stop
             * bits, usually because of synchroisation issues in some UARTs
             * which mean that if you wait until the serialiser has finished
             * with the current character and then load the TX buffer it has
             * to wait until the next bit boundary to start transmitting.
             * PMcS
             */
            RS485_Baud = 76800;
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

/* Transmits a Frame on the wire */
void RS485_Send_Frame(
    volatile struct mstp_port_struct_t *mstp_port,      /* port specific data */
    uint8_t * buffer,   /* frame to send (up to 501 bytes of data) */
    uint16_t nbytes)
{       /* number of bytes of data (up to 501) */
    DWORD dwWritten = 0;

    if (mstp_port) {
        uint32_t baud;
        uint8_t turnaround_time;
        baud = RS485_Get_Baud_Rate();
        /* wait about 40 bit times since reception */
        if (baud == 9600)
            turnaround_time = 4;
        else if (baud == 19200)
            turnaround_time = 2;
        else
            turnaround_time = 2;
        while (mstp_port->SilenceTimer(NULL) < turnaround_time) {
            /* do nothing - wait for timer to increment */
        };
    }
    WriteFile(RS485_Handle, buffer, nbytes, &dwWritten, NULL);

    /* per MSTP spec, reset SilenceTimer after each byte is sent */
    if (mstp_port) {
        mstp_port->SilenceTimerReset(NULL);
    }

    return;
}

/* called by timer, interrupt(?) or other thread */
void RS485_Check_UART_Data(
    volatile struct mstp_port_struct_t *mstp_port)
{
    char lpBuf[1];
    DWORD dwRead = 0;

    if (mstp_port->ReceiveError == true) {
        /* wait for state machine to clear this */
    }
    /* wait for state machine to read from the DataRegister */
    else if (mstp_port->DataAvailable == false) {
        /* check for data */
        if (!ReadFile(RS485_Handle, lpBuf, sizeof(lpBuf), &dwRead, NULL)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                mstp_port->ReceiveError = TRUE;
            }
        } else {
            if (dwRead) {
                mstp_port->DataRegister = lpBuf[0];
                mstp_port->DataAvailable = TRUE;
            }
        }
    }
}

/*************************************************************************
* Description: print available COM ports
* Returns: none
* Notes: none
**************************************************************************/
void RS485_Print_Ports(
    void)
{
    unsigned i = 0;

    /* try to open all 255 COM ports */
    for (i = 1; i < 256; i++) {
        if (RS485_Interface_Valid(i)) {
            /* note: format for Wireshark ExtCap */
            printf("interface {value=COM%u}"
                "{display=BACnet MS/TP on COM%u}\n", i, i);
        }
    }
}

#ifdef TEST_RS485

#include "mstpdef.h"


static void test_transmit_task(
    void *pArg)
{
    char *TxBuf = "BACnet MS/TP";
    size_t len = strlen(TxBuf) + 1;

    while (TRUE) {
        Sleep(1000);
        RS485_Send_Frame(NULL, &TxBuf[0], len);
    }
}

#if defined(_WIN32)
static BOOL WINAPI CtrlCHandler(
    DWORD dwCtrlType)
{
    dwCtrlType = dwCtrlType;
    exit(0);
    return TRUE;
}
#endif

static int ascii_hex_to_int(
    char ch)
{
    int rv = -1;

    if ((ch >= '0') && (ch <= '9')) {
        rv = ch - '0';
    } else if ((ch >= 'a') && (ch <= 'f')) {
        rv = 10 + ch - 'a';
    } else if ((ch >= 'A') && (ch <= 'F')) {
        rv = 10 + ch - 'a';
    }

    return rv;
}

int main(
    int argc,
    char *argv[])
{
    unsigned long hThread = 0;
    uint32_t arg_value = 0;
    char lpBuf[1];
    DWORD dwRead = 0;
    unsigned i = 0, len = 0, count = 0;
    char hex_pair[5] = "0xff";
    char ch = ' ';
    int lsb = 0, msb = 0;
    long my_baud = 38400;
    uint8_t buffer[501] = { 0 };

    if (argc > 1) {
        RS485_Set_Interface(argv[1]);
    }
    if (argc > 2) {
        my_baud = strtol(argv[2], NULL, 0);
    }
    RS485_Set_Baud_Rate(my_baud);
    RS485_Initialize();
#if defined(_WIN32)
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_PROCESSED_INPUT);
    SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlCHandler, TRUE);
#endif
#ifdef TEST_RS485_TRANSMIT
    /* read a stream of characters from stdin or argument */
    if (argc > 3) {
        len = strlen(argv[3]);
        for (i = 0; i < len; i++) {
            /* grab pairs of hex characters, skip spaces */
            ch = argv[3][i];
            if (ch == ' ') {
                continue;
            }
            msb = ascii_hex_to_int(ch);
            if (msb >= 0) {
                i++;
                ch = argv[3][i];
                lsb = ascii_hex_to_int(ch);
                if (lsb >= 0) {
                    buffer[count] = msb << 4 | lsb;
                } else {
                    buffer[count] = msb;
                }
                count++;
                if (count >= sizeof(buffer)) {
                    break;
                }
            }
        }
        RS485_Send_Frame(NULL, buffer, count);
    }
#endif
#ifdef TEST_RS485_RECEIVE
    /* receive task */
    for (;;) {
        if (!ReadFile(RS485_Handle, lpBuf, sizeof(lpBuf), &dwRead, NULL)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                RS485_Print_Error();
            }
        } else {
            /* print any characters received */
            if (dwRead) {
                for (i = 0; i < dwRead; i++) {
                    fprintf(stderr, "%02X ", lpBuf[i]);
                }
            }
            dwRead = 0;
        }
    }
#endif
}
#endif
