/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "config.h"
#include "address.h"
#include "bacdef.h"
#include "handlers.h"
#include "client.h"
#include "dlenv.h"
#include "bacdcode.h"
#include "npdu.h"
#include "apdu.h"
#include "iam.h"
#include "tsm.h"
#include "device.h"
#include "bacfile.h"
#include "datalink.h"
#include "dcc.h"
#include "getevent.h"
#include "net.h"
#include "txbuf.h"
#include "tsm.h"
#include "version.h"
/* include the device object */
#include "device.h"
#include "bi.h"
#include "bo.h"
#include "pifacedigital.h"

/** @file server/main.c  Example server application using the BACnet Stack. */

/* (Doxygen note: The next two lines pull all the following Javadoc
 *  into the ServerDemo module.) */
/** @addtogroup ServerDemo */
/*@{*/

/** Buffer used for receiving */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

/** Initialize the handlers we will utilize.
 * @see Device_Init, apdu_set_unconfirmed_handler, apdu_set_confirmed_handler
 */
static void Init_Service_Handlers(
    void)
{
    Device_Init(NULL);
    /* we need to handle who-is to support dynamic device binding */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, handler_who_has);
    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
    /* set the handler for all the services we don't implement */
    /* It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    /* Set the handlers for any confirmed services that we support. */
    /* We must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
        handler_read_property_multiple);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        handler_write_property);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROP_MULTIPLE,
        handler_write_property_multiple);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_RANGE,
        handler_read_range);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
        handler_reinitialize_device);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_UTC_TIME_SYNCHRONIZATION,
        handler_timesync_utc);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_TIME_SYNCHRONIZATION,
        handler_timesync);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_SUBSCRIBE_COV,
        handler_cov_subscribe);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_COV_NOTIFICATION,
        handler_ucov_notification);
    /* handle communication so we can shutup when asked */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
        handler_device_communication_control);
    /* handle the data coming back from private requests */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_PRIVATE_TRANSFER,
        handler_unconfirmed_private_transfer);
}

static void piface_init(void)
{
    int hw_addr = 0;   /**< PiFaceDigital hardware address  */
#ifdef PIFACE_INTERRUPT_ENABLE
    int intenable = 1; /**< Whether or not interrupts are enabled  */
#endif

    /**
     * Open piface digital SPI connection(s)
     */
    printf("Opening piface digital connection at location %d\n", hw_addr);
    pifacedigital_open(hw_addr);

#ifdef PIFACE_INTERRUPT_ENABLE
    /**
     * Enable interrupt processing (only required for all
     * blocking/interrupt methods)
     */
    intenable = pifacedigital_enable_interrupts();
    if ( intenable == 0) {
        printf("Interrupts enabled.\n");
    } else {
        printf("Could not enable interrupts.  "
            "Try running using sudo to enable PiFaceDigital interrupts.\n");
    }
#endif
}

/* track the Piface pin state to react on changes only */
static bool PiFace_Pin_Status[MAX_BINARY_INPUTS];

/** 
 * Clean up the PiFace interface 
 */
static void piface_cleanup(void)
{
    pifacedigital_close(0);
}

/**
 * Perform a periodic task for the PiFace card
 */
static void piface_task(void) 
{
    unsigned i = 0;
    BACNET_BINARY_PV present_value = BINARY_INACTIVE;
    bool pin_status = false;

    for (i = 0; i < MAX_BINARY_INPUTS; i++) {
        if (!Binary_Input_Out_Of_Service(i)) {
            present_value = Binary_Input_Present_Value(i);
            pin_status = false;
            if (pifacedigital_digital_read(i)) {
                pin_status = true;
            }
            if (pin_status != PiFace_Pin_Status[i]) {
                PiFace_Pin_Status[i] = pin_status;
                if (pin_status) {
                    /* toggle the input only when button is pressed */
                    if (present_value == BINARY_INACTIVE) {
                        present_value = BINARY_ACTIVE;
                    } else {
                        present_value = BINARY_INACTIVE;
                    }
                    Binary_Input_Present_Value_Set(i, present_value);
                }
            }
        }
    }
    for (i = 0; i < MAX_BINARY_OUTPUTS; i++) {
        if (!Binary_Output_Out_Of_Service(i)) {
            present_value = Binary_Output_Present_Value(i);
            if (present_value == BINARY_INACTIVE) {
                pifacedigital_digital_write(i, 0);
            } else {
                pifacedigital_digital_write(i, 1);
            }
        }
    }   
}

/** Main function of server demo.
 *
 * @see Device_Set_Object_Instance_Number, dlenv_init, Send_I_Am,
 *      datalink_receive, npdu_handler,
 *      dcc_timer_seconds, bvlc_maintenance_timer,
 *      handler_cov_task,
 *      tsm_timer_milliseconds
 *
 * @param argc [in] Arg count.
 * @param argv [in] Takes one argument: the Device Instance #.
 * @return 0 on success.
 */
int main(
    int argc,
    char *argv[])
{
    BACNET_ADDRESS src = {
        0
    };  /* address where message came from */
    uint16_t pdu_len = 0;
    unsigned timeout = 1;       /* milliseconds */
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    uint32_t elapsed_seconds = 0;
    uint32_t elapsed_milliseconds = 0;
    uint32_t address_binding_tmr = 0;
        
    /* allow the device ID to be set */
    if (argc > 1) {
        Device_Set_Object_Instance_Number(strtol(argv[1], NULL, 0));
    }
    printf("BACnet Raspberry Pi PiFace Digital Demo\n" 
        "BACnet Stack Version %s\n"
        "BACnet Device ID: %u\n" 
        "Max APDU: %d\n", BACnet_Version,
        Device_Object_Instance_Number(), MAX_APDU);
    /* load any static address bindings to show up
       in our device bindings list */
    address_init();
    Init_Service_Handlers();
    dlenv_init();
    atexit(datalink_cleanup);
    piface_init();
    atexit(piface_cleanup);
    /* configure the timeout values */
    last_seconds = time(NULL);
    /* broadcast an I-Am on startup */
    Send_I_Am(&Handler_Transmit_Buffer[0]);
    /* loop forever */
    for (;;) {
        /* input */
        current_seconds = time(NULL);

        /* returns 0 bytes on timeout */
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);

        /* process */
        if (pdu_len) {
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }
        /* at least one second has passed */
        elapsed_seconds = (uint32_t) (current_seconds - last_seconds);
        if (elapsed_seconds) {
            last_seconds = current_seconds;
            dcc_timer_seconds(elapsed_seconds);
#if defined(BACDL_BIP) && BBMD_ENABLED
            bvlc_maintenance_timer(elapsed_seconds);
#endif
            dlenv_maintenance_timer(elapsed_seconds);
            elapsed_milliseconds = elapsed_seconds * 1000;
            handler_cov_timer_seconds(elapsed_seconds);
            tsm_timer_milliseconds(elapsed_milliseconds);
        }
        handler_cov_task();
        /* scan cache address */
        address_binding_tmr += elapsed_seconds;
        if (address_binding_tmr >= 60) {
            address_cache_timer(address_binding_tmr);
            address_binding_tmr = 0;
        }
        /* output/input */
        piface_task();
    }

    return 0;
}

/* @} */

/* End group ServerDemo */
