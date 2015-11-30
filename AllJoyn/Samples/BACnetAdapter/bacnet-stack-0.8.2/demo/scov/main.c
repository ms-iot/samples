/*************************************************************************
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

/* command line tool that sends a BACnet service, and displays the reply */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>       /* for time */
#include <ctype.h>      /* for toupper */

#define PRINT_ENABLED 1

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
/* some demo stuff needed */
#include "filename.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#include "dlenv.h"

/* buffer used for receive */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

/* converted command line arguments */
static uint32_t Target_Device_Object_Instance = BACNET_MAX_INSTANCE;
/* Process identifier for matching replies */
static uint32_t Target_Device_Process_Identifier = 0;
/* the invoke id is needed to filter incoming messages */
static uint8_t Request_Invoke_ID = 0;
/* MAC and SNET address of target */
static BACNET_ADDRESS Target_Address;
/* indication of error, reject, or abort */
static bool Error_Detected = false;
/* data used in COV subscription request */
BACNET_SUBSCRIBE_COV_DATA *COV_Subscribe_Data = NULL;
/* flags to signal early termination */
static bool Notification_Detected = false;
static bool Simple_Ack_Detected = false;
static bool Cancel_Requested = false;

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

void My_Unconfirmed_COV_Notification_Handler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
    handler_ucov_notification(service_request, service_len, src);
    Notification_Detected = true;
}

void My_Confirmed_COV_Notification_Handler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_DATA * service_data)
{
    handler_ccov_notification(service_request, service_len, src, service_data);
    Notification_Detected = true;
}

void MyWritePropertySimpleAckHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id)
{
    if (address_match(&Target_Address, src) &&
        (invoke_id == Request_Invoke_ID)) {
        printf("SubscribeCOV Acknowledged!\r\n");
        Simple_Ack_Detected = true;
    }
}

static void Init_Service_Handlers(
    void)
{
    Device_Init(NULL);
    /* we need to handle who-is
       to support dynamic device binding to us */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
    /* set the handler for all the services we don't implement
       It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    /* we must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
    /* handle the data coming back from COV subscriptions */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_COV_NOTIFICATION,
        My_Confirmed_COV_Notification_Handler);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_COV_NOTIFICATION,
        My_Unconfirmed_COV_Notification_Handler);
    /* handle the Simple ack coming back from SubscribeCOV */
    apdu_set_confirmed_simple_ack_handler(SERVICE_CONFIRMED_SUBSCRIBE_COV,
        MyWritePropertySimpleAckHandler);
    /* handle any errors coming back */
    apdu_set_error_handler(SERVICE_CONFIRMED_SUBSCRIBE_COV, MyErrorHandler);
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);
}

void cleanup(
    void)
{
    BACNET_SUBSCRIBE_COV_DATA *cov_data = NULL;
    BACNET_SUBSCRIBE_COV_DATA *cov_data_old = NULL;

    cov_data = COV_Subscribe_Data;
    while (cov_data) {
        cov_data_old = cov_data;
        cov_data = cov_data->next;
        free(cov_data_old);
    }
}

int main(
    int argc,
    char *argv[])
{
    BACNET_ADDRESS src = {
        0
    };  /* address where message came from */
    uint16_t pdu_len = 0;
    unsigned timeout = 100;     /* milliseconds */
    unsigned max_apdu = 0;
    time_t elapsed_seconds = 0;
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    time_t timeout_seconds = 0;
    time_t delta_seconds = 0;
    bool found = false;
    char *filename = NULL;
    bool print_usage_terse = false;
    bool print_usage_verbose = false;
    BACNET_SUBSCRIBE_COV_DATA *cov_data = NULL;
    int argi = 0;
    int arg_remaining = 0;

    if (argc < 6) {
        print_usage_terse = true;
    }
    if ((argc > 1) && (strcmp(argv[1], "--help") == 0)) {
        print_usage_terse = true;
        print_usage_verbose = true;
    }
    if (print_usage_terse) {
        filename = filename_remove_path(argv[0]);
        printf("Usage: %s device-id object-type object-instance "
            "process-id <[un]confirmed lifetime|cancel>\r\n", filename);
        if (!print_usage_verbose) {
            return 0;
        }
    }
    if (print_usage_verbose) {
        printf("\r\n" "device-id:\r\n"
            "The subscriber BACnet Device Object Instance number.\r\n" "\r\n"
            "object-type:\r\n"
            "The monitored object type is the integer value of the\r\n"
            "enumeration BACNET_OBJECT_TYPE in bacenum.h.  For example,\r\n"
            "if you were monitoring Analog Output 2, the object-type\r\n"
            "would be 1.\r\n" "\r\n" "object-instance:\r\n"
            "The monitored object instance number.\r\n" "\r\n"
            "process-id:\r\n"
            "Process Identifier for this COV subscription.\r\n" "\r\n"
            "confirmed:\r\n"
            "Optional flag to subscribe using Confirmed notifications.\r\n"
            "Use the word \'confirmed\' or \'unconfirmed\'.\r\n" "\r\n"
            "lifetime:\r\n"
            "Optional subscription lifetime is conveyed in seconds.\r\n" "\r\n"
            "cancel:\r\n"
            "Use the word \'cancel\' instead of confirm and lifetime.\r\n"
            "This shall indicate a cancellation request.\r\n" "\r\n"
            "Example:\r\n"
            "If you want subscribe to Device 123 Analog Input 9 object\r\n"
            "using confirmed COV notifications for 5 minutes,\r\n"
            "you could send the following command:\r\n"
            "%s 123 0 9 1 confirmed 600\r\n"
            "To send the same COV subscription request for unconfirmed\r\n"
            "notifications, send the following command:\r\n"
            "%s 123 0 9 1 unconfirmed 600\r\n"
            "To cancel the same COV subscription request,\r\n"
            "send the following command:\r\n" "%s 123 0 9 1 cancel\r\n",
            filename, filename, filename);
        return 0;
    }
    /* decode the command line parameters */
    Target_Device_Object_Instance = strtol(argv[1], NULL, 0);
    if (Target_Device_Object_Instance >= BACNET_MAX_INSTANCE) {
        fprintf(stderr, "device-instance=%u - it must be less than %u\r\n",
            Target_Device_Object_Instance, BACNET_MAX_INSTANCE);
        return 1;
    }
    atexit(cleanup);
    COV_Subscribe_Data = calloc(1, sizeof(BACNET_SUBSCRIBE_COV_DATA));
    cov_data = COV_Subscribe_Data;
    argi = 2;
    while (cov_data) {
        cov_data->monitoredObjectIdentifier.type = strtol(argv[argi], NULL, 0);
        if (cov_data->monitoredObjectIdentifier.type >= MAX_BACNET_OBJECT_TYPE) {
            fprintf(stderr, "object-type=%u - it must be less than %u\r\n",
                cov_data->monitoredObjectIdentifier.type,
                MAX_BACNET_OBJECT_TYPE);
            return 1;
        }
        argi++;
        cov_data->monitoredObjectIdentifier.instance =
            strtol(argv[argi], NULL, 0);
        if (cov_data->monitoredObjectIdentifier.instance > BACNET_MAX_INSTANCE) {
            fprintf(stderr, "object-instance=%u - it must be less than %u\r\n",
                cov_data->monitoredObjectIdentifier.instance,
                BACNET_MAX_INSTANCE + 1);
            return 1;
        }
        argi++;
        cov_data->subscriberProcessIdentifier = strtol(argv[argi], NULL, 0);
        argi++;
        if (strcmp(argv[argi], "cancel") == 0) {
            cov_data->cancellationRequest = true;
            argi++;
        } else {
            cov_data->cancellationRequest = false;
            if (strcmp(argv[argi], "confirmed") == 0) {
                cov_data->issueConfirmedNotifications = true;
            } else if (strcmp(argv[argi], "unconfirmed") == 0) {
                cov_data->issueConfirmedNotifications = false;
            } else {
                fprintf(stderr, "unknown option: %s\r\n", argv[argi]);
                return 1;
            }
            argi++;
            arg_remaining = argc - argi;
            if (arg_remaining > 0) {
                cov_data->lifetime = strtol(argv[argi], NULL, 0);
                argi++;
            } else {
                cov_data->lifetime = 0;
            }
        }
        arg_remaining = argc - argi;
        if (arg_remaining < 5) {
            break;
        } else {
            cov_data->next = calloc(1, sizeof(BACNET_SUBSCRIBE_COV_DATA));
            cov_data = cov_data->next;
        }
    }
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
    found =
        address_bind_request(Target_Device_Object_Instance, &max_apdu,
        &Target_Address);
    if (!found) {
        Send_WhoIs(Target_Device_Object_Instance,
            Target_Device_Object_Instance);
    }
    /* start at the beginning of the subscribe list */
    cov_data = COV_Subscribe_Data;
    /* loop forever */
    for (;;) {
        /* increment timer - exit if timed out */
        current_seconds = time(NULL);
        /* at least one second has passed */
        if (current_seconds != last_seconds) {
            /* increment timer - exit if timed out */
            delta_seconds = current_seconds - last_seconds;
            elapsed_seconds += delta_seconds;
            tsm_timer_milliseconds((delta_seconds * 1000));
            /* keep track of time for next check */
            last_seconds = current_seconds;
        }
        if (Error_Detected) {
            break;
        }
        /* wait until the device is bound, or timeout and quit */
        if (!found) {
            found =
                address_bind_request(Target_Device_Object_Instance, &max_apdu,
                &Target_Address);
        }
        if (found) {
            if (Request_Invoke_ID == 0) {
                Simple_Ack_Detected = false;
                Notification_Detected = false;
                if (cov_data->cancellationRequest) {
                    Cancel_Requested = true;
                } else {
                    Cancel_Requested = false;
                }
                Target_Device_Process_Identifier =
                    cov_data->subscriberProcessIdentifier;
                Request_Invoke_ID =
                    Send_COV_Subscribe(Target_Device_Object_Instance,
                    cov_data);
                if (!cov_data->cancellationRequest &&
                    (timeout_seconds < cov_data->lifetime)) {
                    /* increase the timeout to the longest lifetime */
                    timeout_seconds = cov_data->lifetime;
                }
                printf("Sent SubscribeCOV request. "
                    " Waiting up to %u seconds....\r\n",
                    (unsigned) (timeout_seconds - elapsed_seconds));
            } else if (tsm_invoke_id_free(Request_Invoke_ID)) {
                if (cov_data->next) {
                    cov_data = cov_data->next;
                    Request_Invoke_ID = 0;
                } else {
                    if (Notification_Detected) {
                        break;
                    }
                    if (Cancel_Requested && Simple_Ack_Detected) {
                        break;
                    }
                }
            } else if (tsm_invoke_id_failed(Request_Invoke_ID)) {
                fprintf(stderr, "\rError: TSM Timeout!\r\n");
                tsm_free_invoke_id(Request_Invoke_ID);
                Error_Detected = true;
                /* try again or abort? */
                break;
            }
        } else {
            /* exit if timed out */
            if (elapsed_seconds > timeout_seconds) {
                Error_Detected = true;
                printf("\rError: APDU Timeout!\r\n");
                break;
            }
        }
        /* returns 0 bytes on timeout */
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);
        /* process */
        if (pdu_len) {
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }
        if (Error_Detected) {
            break;
        }
        /* COV - so just wait until lifetime value expires */
        if (elapsed_seconds > timeout_seconds) {
            break;
        }
    }
    if (Error_Detected)
        return 1;
    return 0;
}
