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
/* hardware specific */
#include "timer.h"
/* standard libraries */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
/* BACnet */
#include "rs485.h"
#include "datalink.h"
#include "npdu.h"
#include "apdu.h"
#include "dcc.h"
#include "iam.h"
#include "handlers.h"
#include "device.h"
#include "dcc.h"
#include "iam.h"
#include "txbuf.h"

static unsigned long DCC_Timer = 1000;

static void millisecond_timer(
    void)
{
    while (Timer_Milliseconds) {
        Timer_Milliseconds--;
        if (DCC_Timer) {
            DCC_Timer--;
        }
    }
    /* note: MS/TP silence timer is updated in ISR */
}

static void bacnet_init(
    void)
{
#if defined(BACDL_MSTP)
    RS485_Set_Baud_Rate(38400);
    dlmstp_set_mac_address(57);
    dlmstp_set_max_master(127);
    dlmstp_set_max_info_frames(1);
    dlmstp_init(NULL);
#endif
    Device_Set_Object_Instance_Number(11111);
#ifndef DLMSTP_TEST
    /* we need to handle who-is to support dynamic device binding */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    /* Set the handlers for any confirmed services that we support. */
    /* We must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
        handler_reinitialize_device);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        handler_write_property);
    /* handle communication so we can shutup when asked */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
        handler_device_communication_control);
#endif
}

int main(
    void)
{
    uint16_t pdu_len = 0;
    BACNET_ADDRESS src; /* source address */
    uint8_t pdu[MAX_MPDU];      /* PDU data */

    Timer_Init();
    bacnet_init();
    /* broadcast an I-Am on startup */
    Send_I_Am(&Handler_Transmit_Buffer[0]);
    for (;;) {
        millisecond_timer();
        if (!DCC_Timer) {
            dcc_timer_seconds(1);
            DCC_Timer = 1000;
        }
        /* BACnet handling */
        pdu_len = datalink_receive(&src, &pdu[0], MAX_MPDU, 0);
        if (pdu_len) {
#ifndef DLMSTP_TEST
            npdu_handler(&src, &pdu[0], pdu_len);
#endif
        }
    }
}
