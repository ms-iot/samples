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
static bool Target_Broadcast;
static uint16_t Target_DNET;
static uint32_t Target_Device_Object_Instance = BACNET_MAX_INSTANCE;
static uint16_t Target_Vendor_Identifier = 260;
static uint32_t Target_Service_Number = 0;
/* property value encodings */
#ifndef MAX_PROPERTY_VALUES
#define MAX_PROPERTY_VALUES 64
#endif
static BACNET_APPLICATION_DATA_VALUE
    Target_Object_Property_Value[MAX_PROPERTY_VALUES];
/* buffer for service parameters */
static uint8_t Service_Parameters[MAX_APDU];
/* the invoke id is needed to filter incoming messages */
static uint8_t Request_Invoke_ID = 0;
static BACNET_ADDRESS Target_Address;
static bool Error_Detected = false;

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
    /* handle the data coming back from requests */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_PRIVATE_TRANSFER,
        handler_unconfirmed_private_transfer);
    /* handle any errors coming back */
    apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY, MyErrorHandler);
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);
}

int main(
    int argc,
    char *argv[])
{
    BACNET_ADDRESS src = {
        0
    };  /* address where message came from */
    uint16_t pdu_len = 0;
    unsigned timeout = 10;      /* milliseconds */
    unsigned max_apdu = 0;
    time_t elapsed_seconds = 0;
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    time_t timeout_seconds = 0;
    time_t delta_seconds = 0;
    bool found = false;
    char *filename = NULL;
    char *value_string = NULL;
    bool status = false;
    int args_remaining = 0, tag_value_arg = 0, i = 0;
    BACNET_APPLICATION_TAG property_tag;
    uint8_t context_tag = 0;
    BACNET_PRIVATE_TRANSFER_DATA private_data = { 0 };
    int len = 0;
    bool sent_message = false;

    if (argc < 6) {
        filename = filename_remove_path(argv[0]);
        printf("Usage: %s <device-instance|broadcast|dnet=> vendor-id"
            " service-number tag value [tag value...]\r\n", filename);
        if ((argc > 1) && (strcmp(argv[1], "--help") == 0)) {
            printf("device-instance:\r\n"
                "BACnet Device Object Instance number that you are\r\n"
                "trying to communicate to.  This number will be used\r\n"
                "to try and bind with the device using Who-Is and\r\n"
                "I-Am services.  For example, if you were transferring to\r\n"
                "Device Object 123, the device-instance would be 123.\r\n"
                "For Global Broadcast, use the word 'broadcast'.\r\n"
                "For Local Broadcast to a particular DNET n, use 'dnet=n'.\r\n"
                "\r\n" "vendor_id:\r\n"
                "the unique vendor identification code for the type of\r\n"
                "vendor proprietary service to be performed.\r\n" "\r\n"
                "service-number (Unsigned32):\r\n"
                "the desired proprietary service to be performed.\r\n" "\r\n"
                "tag:\r\n" "Tag is the integer value of the enumeration \r\n"
                "BACNET_APPLICATION_TAG in bacenum.h.\r\n"
                "It is the data type of the value that you are sending.\r\n"
                "For example, if you were transfering a REAL value, you would \r\n"
                "use a tag of 4.\r\n"
                "Context tags are created using two tags in a row.\r\n"
                "The context tag is preceded by a C.  Ctag tag.\r\n"
                "C2 4 creates a context 2 tagged REAL.\r\n" "\r\n" "value:\r\n"
                "The value is an ASCII representation of some type of data\r\n"
                "that you are transfering.\r\n"
                "It is encoded using the tag information provided.\r\n"
                "For example, if you were transferring a REAL value of 100.0,\r\n"
                "you would use 100.0 as the value.\r\n"
                "If you were transferring an object identifier for Device 123,\r\n"
                "you would use 8:123 as the value.\r\n" "\r\n" "Example:\r\n"
                "If you want to transfer a REAL value of 1.1 to service 23 of \r\n"
                "vendor 260 in Device 99, you could send the following command:\r\n"
                "%s 99 260 23 4 1.1\r\n", filename);
        }
        return 0;
    }
    /* decode the command line parameters */
    if (strcmp(argv[1], "broadcast") == 0) {
        Target_Broadcast = true;
        Target_DNET = BACNET_BROADCAST_NETWORK;
    } else if (strncmp(argv[1], "dnet=", 5) == 0) {
        Target_Broadcast = true;
        Target_DNET = strtol(&argv[1][5], NULL, 0);
    } else {
        Target_Device_Object_Instance = strtol(argv[1], NULL, 0);
    }
    Target_Vendor_Identifier = strtol(argv[2], NULL, 0);
    Target_Service_Number = strtol(argv[3], NULL, 0);
    if ((!Target_Broadcast) &&
        (Target_Device_Object_Instance > BACNET_MAX_INSTANCE)) {
        fprintf(stderr, "device-instance=%u - it must be less than %u\r\n",
            Target_Device_Object_Instance, BACNET_MAX_INSTANCE);
        return 1;
    }
    args_remaining = (argc - (6 - 2));
    for (i = 0; i < MAX_PROPERTY_VALUES; i++) {
        tag_value_arg = (6 - 2) + (i * 2);
        /* special case for context tagged values */
        if (toupper(argv[tag_value_arg][0]) == 'C') {
            context_tag = strtol(&argv[tag_value_arg][1], NULL, 0);
            tag_value_arg++;
            args_remaining--;
            Target_Object_Property_Value[i].context_tag = context_tag;
            Target_Object_Property_Value[i].context_specific = true;
        } else {
            Target_Object_Property_Value[i].context_specific = false;
        }
        property_tag = strtol(argv[tag_value_arg], NULL, 0);
        args_remaining--;
        if (args_remaining <= 0) {
            fprintf(stderr, "Error: not enough tag-value pairs\r\n");
            return 1;
        }
        value_string = argv[tag_value_arg + 1];
        args_remaining--;
        /* printf("tag[%d]=%u value[%d]=%s\r\n",
           i, property_tag, i, value_string); */
        if (property_tag >= MAX_BACNET_APPLICATION_TAG) {
            fprintf(stderr, "Error: tag=%u - it must be less than %u\r\n",
                property_tag, MAX_BACNET_APPLICATION_TAG);
            return 1;
        }
        status =
            bacapp_parse_application_data(property_tag, value_string,
            &Target_Object_Property_Value[i]);
        if (!status) {
            /* FIXME: show the expected entry format for the tag */
            fprintf(stderr, "Error: unable to parse the tag value\r\n");
            return 1;
        }
        Target_Object_Property_Value[i].next = NULL;
        if (i > 0) {
            Target_Object_Property_Value[i - 1].next =
                &Target_Object_Property_Value[i];
        }
        if (args_remaining <= 0) {
            break;
        }
    }
    if (args_remaining > 0) {
        fprintf(stderr, "Error: Exceeded %d tag-value pairs.\r\n",
            MAX_PROPERTY_VALUES);
        return 1;
    }
    /* setup my info */
    Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
    address_init();
    Init_Service_Handlers();
    dlenv_init();
    atexit(datalink_cleanup);
    /* configure the timeout values */
    last_seconds = time(NULL);
    if (Target_Broadcast) {
        datalink_get_broadcast_address(&Target_Address);
        Target_Address.net = Target_DNET;
        found = true;
        timeout_seconds = 0;
    } else {
        timeout_seconds = (apdu_timeout() / 1000) * apdu_retries();
        /* try to bind with the device */
        found =
            address_bind_request(Target_Device_Object_Instance, &max_apdu,
            &Target_Address);
        if (!found) {
            Send_WhoIs(Target_Device_Object_Instance,
                Target_Device_Object_Instance);
        }
    }
    /* loop forever */
    for (;;) {
        /* increment timer - exit if timed out */
        current_seconds = time(NULL);
        /* at least one second has passed */
        if (current_seconds != last_seconds) {
            /* increment timer - exit if timed out */
            delta_seconds = current_seconds - last_seconds;
            elapsed_seconds += delta_seconds;
            tsm_timer_milliseconds(((current_seconds - last_seconds) * 1000));
        }
        if (Error_Detected)
            break;
        /* wait until the device is bound, or timeout and quit */
        if (!found) {
            found =
                address_bind_request(Target_Device_Object_Instance, &max_apdu,
                &Target_Address);
        }
        if (!sent_message) {
            if (found) {
                len =
                    bacapp_encode_data(&Service_Parameters[0],
                    &Target_Object_Property_Value[0]);
                private_data.serviceParameters = &Service_Parameters[0];
                private_data.serviceParametersLen = len;
                private_data.vendorID = Target_Vendor_Identifier;
                private_data.serviceNumber = Target_Service_Number;
                Send_UnconfirmedPrivateTransfer(&Target_Address,
                    &private_data);
                printf("Sent PrivateTransfer.");
                if (timeout_seconds) {
                    printf(" Waiting %u seconds.\r\n",
                        (unsigned) (timeout_seconds - elapsed_seconds));
                } else {
                    printf("\r\n");
                }
                sent_message = true;
            } else {
                if (elapsed_seconds > timeout_seconds) {
                    printf("\rError: APDU Timeout!\r\n");
                    Error_Detected = true;
                    break;
                }
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
        /* unconfirmed - so just wait until our timeout value */
        if (elapsed_seconds > timeout_seconds) {
            break;
        }
        /* keep track of time for next check */
        last_seconds = current_seconds;
    }

    if (Error_Detected)
        return 1;
    return 0;
}
