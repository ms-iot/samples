/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

/* This is one way to use the embedded BACnet stack under RTOS-32  */
/* compiled with Borland C++ 5.02 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>      /* for kbhit */
#include "config.h"
#include "bacdef.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "handlers.h"
#include "datalink.h"
#include "iam.h"
#include "txbuf.h"

/* RTOS-32 */
#include "rtkernel.h"
#if defined(RTK32_VER)
#define _USER32_
#define _KERNEL32_
#include <windows.h>
#include <rttarget.h>   /* for RTCMOSSetSystemTime */
#include <rtfiles.h>    /* file system */
#include <rtfsys.h>     /* file system */
#include <Rttbios.h>
#endif
#include <rtcom.h>      /* serial port driver */
#include <itimer.h>     /* time measurement & timer interrupt rate control */
#include <rtkeybrd.h>   /* interrupt handler for the keyboard */

/* buffers used for transmit and receive */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

static void Init_Service_Handlers(
    void)
{
    /* we need to handle who-is to support dynamic device binding */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    /* set the handler for all the services we don't implement */
    /* It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    /* we must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        handler_write_property);
}

void millisecond_task(
    void)
{
    Time ticks = 0;     /* task cycle */
    int i = 0;  /* loop counter */

    ticks = RTKGetTime() + MilliSecsToTicks(1);
    while (TRUE) {
        RTKDelayUntil(ticks);
        dlmstp_millisecond_timer();
        ticks += MilliSecsToTicks(1);
    }
}

void RTOS_Initialize(
    void)
{
    /* allow OS to setup IRQ 1 by using a dummy call */
    (void) kbhit();
    RTKernelInit(5);    /* get the kernel going */
    RTKeybrdInit();
    /*(void)CPUMoniInit(); /* not needed - just monitor idle task */ */
        RTComInit();
    ITimerInit();

    if (RTCallDebugger(RT_DBG_MONITOR, 0, 0) != -1) {
        /* Win32 structured exception - if no handler is
           installed, TerminateProcess() will be called,
           which will reboot - a good thing in our case. */
        RTRaiseCPUException(0); /* Divide Error DIV and IDIV instructions. */
        RTRaiseCPUException(1); /* Debug Any code or data reference. */
        RTRaiseCPUException(2); /* NMI */
        RTRaiseCPUException(3); /* Breakpoint INT 3 instruction. */
        RTRaiseCPUException(4); /* Overflow INTO instruction. */
        RTRaiseCPUException(5); /* BOUND Range Exceeded BOUND instruction. */
        RTRaiseCPUException(6); /* Invalid Opcode (Undefined Opcode) */
        /*  RTRaiseCPUException(7); // Device Not Available (No Math Coprocessor) */
        RTRaiseCPUException(8); /* Double Fault any exception instruction,NMI,INTR. */
        RTRaiseCPUException(9); /* Co-Processor overrun */
        RTRaiseCPUException(10);        /* Invalid TSS Task switch or TSS access. */
        RTRaiseCPUException(11);        /* Segment Not Present Loading segment registers */
        RTRaiseCPUException(12);        /* Stack Seg Fault Stack ops /SS reg loads. */
        RTRaiseCPUException(13);        /* General Protection Any memory reference */
        RTRaiseCPUException(14);        /* Page Fault Any memory reference. */
        RTRaiseCPUException(15);        /* reserved */
        RTRaiseCPUException(16);        /* Floating-Point Error (Math Fault) */
    }
    /* setup 1ms timer tick */
    SetTimerIntVal(1000);
    /* per recommendation in manual */
    RTKDelay(1);
    RTCMOSSetSystemTime();      /* get the right time-of-day */

    /* create timer tick task */
    RTKCreateTask(millisecond_task, 16, 1024 * 8, "millisec task");
}

int main(
    int argc,
    char *argv[])
{
    BACNET_ADDRESS src = { 0 }; /* address where message came from */
    uint16_t pdu_len = 0;
    unsigned timeout = 100;     /* milliseconds */

    (void) argc;
    (void) argv;
    Device_Set_Object_Instance_Number(126);
    Init_Service_Handlers();
    RTOS_Initialize();
    /* init the physical layer */
#ifdef BACDL_MSTP
    dlmstp_set_my_address(0x05);
#endif
    datalink_init(NULL);
    Send_I_Am(&Handler_Transmit_Buffer[0]);
    /* loop forever */
    for (;;) {
        /* input */

        /* returns 0 bytes on timeout */
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);
        /* process */
        if (pdu_len) {
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }
        /* output */



        /* blink LEDs, Turn on or off outputs, etc */
    }
}
