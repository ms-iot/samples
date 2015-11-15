/*************************************************************************
* Copyright (C) 2015 B Weitsch
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

/* command line tool that sends a BACnet service, and displays the reply */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

/* core stuff needed */
#include "bacdef.h"
#include "config.h"
#include "bactext.h"
#include "bacerror.h"
#include "iam.h"
#include "arf.h"
#include "tsm.h"
#include "address.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "net.h"
#include "datalink.h"
#include "whois.h"
#include "getevent.h"

/* some demo stuff needed */
#include "filename.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#include "dlenv.h"

/* Depending on on the max-APDU-length-accepted (varies per device),
   the amount of event entries in the GetEventInformation ACK can sum
   up to 24 max (max APDU length is 1476), especially if the flag
   MORE_EVENTS is true. */
#define MAX_OBJ_IDS_IN_GE_ACK 24

/* buffer used for receive */
static uint8_t Rx_Buf[MAX_MPDU] = {0};

/* converted command line arguments */
static uint32_t Target_Device_Object_Instance = BACNET_MAX_INSTANCE;
/* the invoke id is needed to filter incoming messages */
static uint8_t Request_Invoke_ID = 0;
static BACNET_ADDRESS Target_Address;
static bool Error_Detected = false;
static bool Recieved_Ack = false;
static bool More_Events = false;
static BACNET_OBJECT_ID LastReceivedObjectIdentifier;

static void MyErrorHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    BACNET_ERROR_CLASS error_class,
    BACNET_ERROR_CODE error_code)
{
    if (address_match(&Target_Address, src) &&
        (invoke_id == Request_Invoke_ID)) {
        printf("BACnet Error: %s: %s\r\n",
            bactext_error_class_name((int) error_class),
            bactext_error_code_name((int) error_code));
        Error_Detected = true;
    }
}

void MyAbortHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    uint8_t abort_reason,
    bool server)
{
    (void) server;
    if (address_match(&Target_Address, src) &&
        (invoke_id == Request_Invoke_ID)) {
        printf("BACnet Abort: %s\r\n",
            bactext_abort_reason_name((int) abort_reason));
        Error_Detected = true;
    }
}

void MyRejectHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    uint8_t reject_reason)
{
    if (address_match(&Target_Address, src) &&
        (invoke_id == Request_Invoke_ID)) {
        printf("BACnet Reject: %s\r\n",
            bactext_reject_reason_name((int) reject_reason));
        Error_Detected = true;
    }
}

/** Handler for a GetEvent ACK.
 * Print out the ACK data of a matching request and check if we need
 * to repeat or GetEventInformation request.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void My_Get_Event_Ack_Handler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
    int len = 0;
    int i;
    BACNET_GET_EVENT_INFORMATION_DATA data[MAX_OBJ_IDS_IN_GE_ACK];
    for (i = 0; i < MAX_OBJ_IDS_IN_GE_ACK - 1; i++)
        data[i].next = &data[i+1];

    printf("Recieved Ack. Saved invoke ID was %i, service returned %i\n", Request_Invoke_ID, service_data->invoke_id);

    if (service_data->invoke_id == Request_Invoke_ID) {
        len = getevent_ack_decode_service_request(service_request,
                                                  service_len,
                                                  &data[0],
                                                  &More_Events);
        printf("Decode of Ack returned length %i. MoreEvents flag was %i \n", len, More_Events);
        if (len > 0) {
            ge_ack_print_data(&(data[0]), Target_Device_Object_Instance);
            if (More_Events) {
                BACNET_GET_EVENT_INFORMATION_DATA * lastData = &(data[0]);
                    while (lastData->next) lastData = lastData->next;
                    LastReceivedObjectIdentifier = lastData->objectIdentifier;
            }
        }
    }
    Recieved_Ack = true;
}

static void Init_Service_Handlers(void)
{
    Device_Init(NULL);
    /* we need to handle who-is
       to support dynamic device binding to us */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
    /* set the handler for all the services we don't implement
       It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler(handler_unrecognized_service);
    /* we must implement getevent - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_GET_EVENT_INFORMATION,
                               handler_get_event_information);
    /* handle the data coming back from confirmed requests */
    apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_GET_EVENT_INFORMATION,
                                   My_Get_Event_Ack_Handler);
    /* handle any errors coming back */
    apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY, MyErrorHandler);
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);
}

static int print_help(char *exe_name){
    printf("Usage:\n" "\n" "%s device-instance [--help]\n" "\n"
        "  Send BACnet GetEventInformation service retruequest to given device, and wait\n"
        "  for responses.\n\n", exe_name);
    return 1;
}

int main(int argc, char *argv[])
{
    BACNET_ADDRESS src = {0};  /* address where message came from */
    uint16_t pdu_len = 0;
    unsigned timeout = 100;     /* milliseconds */
    unsigned max_apdu = 0;
    time_t elapsed_seconds = 0;
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    time_t timeout_seconds = 0;
    LastReceivedObjectIdentifier.instance = 0;
    LastReceivedObjectIdentifier.type = 0;

    bool found = false;
    if (argc <= 1) {
        printf("Usage: %s device-instance\r\n", filename_remove_path(argv[0]));
        return 0;
    } else if(strcmp(argv[1], "--help") == 0) {
        print_help(filename_remove_path(argv[0]));
        return 0;
    }
    /* decode the command line parameter */
    Target_Device_Object_Instance = strtol(argv[1], NULL, 0);
    /* setup my info */
    Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
    address_init();
    Init_Service_Handlers();
    dlenv_init();
    atexit(datalink_cleanup);
    /* configure the timeout values */
    last_seconds = time(NULL);
    timeout_seconds = (apdu_timeout() / 1000) * apdu_retries();
    /* try to bind with the device */
    found = address_bind_request(Target_Device_Object_Instance,
								&max_apdu,
								&Target_Address);
    if (!found) {
        Send_WhoIs(Target_Device_Object_Instance, Target_Device_Object_Instance);
    }
    /* loop forever */
    for (;;) {
        /* increment timer - exit if timed out */
        current_seconds = time(NULL);

        /* at least one second has passed */
        if (current_seconds != last_seconds)
            tsm_timer_milliseconds((uint16_t) ((current_seconds - last_seconds) * 1000));

        if (Error_Detected){break;}
        /* wait until the device is bound, or timeout and quit */
        if (!found) {
            found = address_bind_request(Target_Device_Object_Instance,
                                         &max_apdu,
                                         &Target_Address);
        }
        if (found) {
            if (Request_Invoke_ID == 0) {
                printf("\nSending first GetEventInformation request ...\n");
                Request_Invoke_ID = Send_GetEvent(&Target_Address, NULL);
            } else if (More_Events) {
                printf("\nSending another GetEventInformation request ...\n");
                Request_Invoke_ID = Send_GetEvent(&Target_Address,
                                                  &LastReceivedObjectIdentifier);
                More_Events = false;
            } else if (tsm_invoke_id_free(Request_Invoke_ID)) {
                if (Recieved_Ack) {
                    break;
                }
            } else if (tsm_invoke_id_failed(Request_Invoke_ID)) {
                fprintf(stderr, "\rError: TSM Timeout!\r\n");
                tsm_free_invoke_id(Request_Invoke_ID);
                Error_Detected = true;
                /* try again or abort? */
                break;
            }
        } else {
            /* increment timer - exit if timed out */
            elapsed_seconds += (current_seconds - last_seconds);
            if (elapsed_seconds > timeout_seconds) {
                printf("\rError: APDU Timeout!\r\n");
                Error_Detected = true;
                break;
            }
        }
        /* returns 0 bytes on timeout */
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);

        /* process */
        if (pdu_len) {
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }

        /* keep track of time for next check */
        last_seconds = current_seconds;
    }
    if (Error_Detected)
        return 1;
    return 0;
}
