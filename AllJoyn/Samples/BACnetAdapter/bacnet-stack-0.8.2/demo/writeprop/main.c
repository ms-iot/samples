/**************************************************************************
*
* Copyright (C) 2006-2007 Steve Karg <skarg@users.sourceforge.net>
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

/* command line tool that sends a BACnet service, and displays the response */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>       /* for time */
#include <string.h>
#include <errno.h>
#include <ctype.h>      /* toupper */
#include "bactext.h"
#include "iam.h"
#include "arf.h"
#include "tsm.h"
#include "address.h"
#include "config.h"
#include "bacdef.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "net.h"
#include "datalink.h"
#include "whois.h"
#include "version.h"
/* some demo stuff needed */
#include "filename.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#include "dlenv.h"

#ifndef MAX_PROPERTY_VALUES
#define MAX_PROPERTY_VALUES 64
#endif

/* buffer used for receive */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

/* global variables used in this file */
static uint32_t Target_Device_Object_Instance = BACNET_MAX_INSTANCE;
static uint32_t Target_Object_Instance = BACNET_MAX_INSTANCE;
static BACNET_OBJECT_TYPE Target_Object_Type = OBJECT_ANALOG_INPUT;
static BACNET_PROPERTY_ID Target_Object_Property = PROP_ACKED_TRANSITIONS;
/* array index value or BACNET_ARRAY_ALL */
static int32_t Target_Object_Property_Index = BACNET_ARRAY_ALL;
static BACNET_APPLICATION_DATA_VALUE
    Target_Object_Property_Value[MAX_PROPERTY_VALUES];

/* 0 if not set, 1..16 if set */
static uint8_t Target_Object_Property_Priority = 0;

/* needed to filter incoming messages */
static uint8_t Request_Invoke_ID = 0;
static BACNET_ADDRESS Target_Address;
/* needed for return value of main application */
static bool Error_Detected = false;

static void MyErrorHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    BACNET_ERROR_CLASS error_class,
    BACNET_ERROR_CODE error_code)
{
    if (address_match(&Target_Address, src) &&
        (invoke_id == Request_Invoke_ID)) {
        printf("BACnet Error: %s: %s\n",
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
        printf("BACnet Abort: %s\n",
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
        printf("BACnet Reject: %s\n",
            bactext_reject_reason_name((int) reject_reason));
        Error_Detected = true;
    }
}

void MyWritePropertySimpleAckHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id)
{
    if (address_match(&Target_Address, src) &&
        (invoke_id == Request_Invoke_ID)) {
        printf("\nWriteProperty Acknowledged!\n");
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
    /* handle the ack coming back */
    apdu_set_confirmed_simple_ack_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        MyWritePropertySimpleAckHandler);
    /* handle any errors coming back */
    apdu_set_error_handler(SERVICE_CONFIRMED_WRITE_PROPERTY, MyErrorHandler);
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);
}

static void print_usage(char *filename)
{
    printf("Usage: %s device-instance object-type object-instance "
        "property priority index tag value [tag value...]\n",
        filename);
}

static void print_help(char *filename)
{
    printf("device-instance:\n"
        "BACnet Device Object Instance number that you are trying to\n"
        "communicate to.  This number will be used to try and bind with\n"
        "the device using Who-Is and I-Am services.  For example, if you were\n"
        "writing to Device Object 123, the device-instance would be 123.\n"
        "\n" "object-type:\n"
        "The object type is the integer value of the enumeration\n"
        "BACNET_OBJECT_TYPE in bacenum.h.  It is the object that you are\n"
        "writing to.  For example if you were writing to Analog Output 2, \n"
        "the object-type would be 1.\n" "\n" "object-instance:\n"
        "This is the object instance number of the object that you are \n"
        "writing to.  For example, if you were writing to Analog Output 2, \n"
        "the object-instance would be 2.\n" "\n" "property:\n"
        "The property is an integer value of the enumeration \n"
        "BACNET_PROPERTY_ID in bacenum.h.  It is the property you are \n"
        "writing to.  For example, if you were writing to the Present Value\n"
        "property, you would use 85 as the property.\n" "\n"
        "priority:\n"
        "This parameter is used for setting the priority of the\n"
        "write. If Priority 0 is given, no priority is sent.  The BACnet \n"
        "standard states that the value is written at the lowest \n"
        "priority (16) if the object property supports priorities\n"
        "when no priority is sent.\n" "\n" "index\n"
        "This integer parameter is the index number of an array.\n"
        "If the property is an array, individual elements can be written\n"
        "to if supported.  If this parameter is -1, the index is ignored.\n"
        "\n" "tag:\n"
        "Tag is the integer value of the enumeration BACNET_APPLICATION_TAG \n"
        "in bacenum.h.  It is the data type of the value that you are\n"
        "writing.  For example, if you were writing a REAL value, you would \n"
        "use a tag of 4.\n"
        "Context tags are created using two tags in a row.  The context tag\n"
        "is preceded by a C.  Ctag tag. C2 4 creates a context 2 tagged REAL.\n"
        "\n" "value:\n"
        "The value is an ASCII representation of some type of data that you\n"
        "are writing.  It is encoded using the tag information provided.  For\n"
        "example, if you were writing a REAL value of 100.0, you would use \n"
        "100.0 as the value.\n" "\n"
        "Here is a brief overview of BACnet property and tags:\n"
        "Certain properties are expected to be written with certain \n"
        "application tags, so you probably need to know which ones to use\n"
        "with each property of each object.  It is almost safe to say that\n"
        "given a property and an object and a table, the tag could be looked\n"
        "up automatically.  There may be a few exceptions to this, such as\n"
        "the Any property type in the schedule object and the Present Value\n"
        "accepting REAL, BOOLEAN, NULL, etc.  Perhaps it would be simpler for\n"
        "the demo to use this kind of table - but I also wanted to be able\n"
        "to do negative testing by passing the wrong tag and have the server\n"
        "return a reject message.\n" "\nExample:\n"
        "If you want send a value of 100 to the Present-Value in\n"
        "Analog Output 0 of Device 123 at priority 16,\n"
        "send the following command:\n"
        "%s 123 1 0 85 16 -1 4 100\n"
        "To send a relinquish command to the same object:\n"
        "%s 123 1 0 85 16 -1 0 0\n", filename, filename);
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
    bool found = false;
    char *value_string = NULL;
    bool status = false;
    int args_remaining = 0, tag_value_arg = 0, i = 0;
    BACNET_APPLICATION_TAG property_tag;
    uint8_t context_tag = 0;
    int argi = 0;

    /* print help if requested */
    for (argi = 1; argi < argc; argi++) {
        if (strcmp(argv[argi], "--help") == 0) {
            print_usage(filename_remove_path(argv[0]));
            print_help(filename_remove_path(argv[0]));
            return 0;
        }
        if (strcmp(argv[argi], "--version") == 0) {
            printf("%s %s\n",
                filename_remove_path(argv[0]),
                BACNET_VERSION_TEXT);
            printf("Copyright (C) 2014 by Steve Karg\n"
                "This is free software; see the source for copying conditions.\n"
                "There is NO warranty; not even for MERCHANTABILITY or\n"
                "FITNESS FOR A PARTICULAR PURPOSE.\n");
            return 0;
        }
    }
    if (argc < 9) {
        print_usage(filename_remove_path(argv[0]));
        return 0;
    }
    /* decode the command line parameters */
    Target_Device_Object_Instance = strtol(argv[1], NULL, 0);
    Target_Object_Type = strtol(argv[2], NULL, 0);
    Target_Object_Instance = strtol(argv[3], NULL, 0);
    Target_Object_Property = strtol(argv[4], NULL, 0);
    Target_Object_Property_Priority = (uint8_t) strtol(argv[5], NULL, 0);
    Target_Object_Property_Index = strtol(argv[6], NULL, 0);
    if (Target_Object_Property_Index == -1)
        Target_Object_Property_Index = BACNET_ARRAY_ALL;
    if (Target_Device_Object_Instance > BACNET_MAX_INSTANCE) {
        fprintf(stderr, "device-instance=%u - it must be less than %u\n",
            Target_Device_Object_Instance, BACNET_MAX_INSTANCE + 1);
        return 1;
    }
    if (Target_Object_Type > MAX_BACNET_OBJECT_TYPE) {
        fprintf(stderr, "object-type=%u - it must be less than %u\n",
            Target_Object_Type, MAX_BACNET_OBJECT_TYPE + 1);
        return 1;
    }
    if (Target_Object_Instance > BACNET_MAX_INSTANCE) {
        fprintf(stderr, "object-instance=%u - it must be less than %u\n",
            Target_Object_Instance, BACNET_MAX_INSTANCE + 1);
        return 1;
    }
    if (Target_Object_Property > MAX_BACNET_PROPERTY_ID) {
        fprintf(stderr, "property=%u - it must be less than %u\n",
            Target_Object_Property, MAX_BACNET_PROPERTY_ID + 1);
        return 1;
    }
    args_remaining = (argc - 7);
    for (i = 0; i < MAX_PROPERTY_VALUES; i++) {
        tag_value_arg = 7 + (i * 2);
        /* special case for context tagged values */
        if (toupper(argv[tag_value_arg][0]) == 'C') {
            context_tag = (uint8_t) strtol(&argv[tag_value_arg][1], NULL, 0);
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
            fprintf(stderr, "Error: not enough tag-value pairs\n");
            return 1;
        }
        value_string = argv[tag_value_arg + 1];
        args_remaining--;
        /* printf("tag[%d]=%u value[%d]=%s\n",
           i, property_tag, i, value_string); */
        if (property_tag >= MAX_BACNET_APPLICATION_TAG) {
            fprintf(stderr, "Error: tag=%u - it must be less than %u\n",
                property_tag, MAX_BACNET_APPLICATION_TAG);
            return 1;
        }
        status =
            bacapp_parse_application_data(property_tag, value_string,
            &Target_Object_Property_Value[i]);
        if (!status) {
            /* FIXME: show the expected entry format for the tag */
            fprintf(stderr, "Error: unable to parse the tag value\n");
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
        fprintf(stderr, "Error: Exceeded %d tag-value pairs.\n",
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
    timeout_seconds = (apdu_timeout() / 1000) * apdu_retries();
    /* try to bind with the device */
    found =
        address_bind_request(Target_Device_Object_Instance, &max_apdu,
        &Target_Address);
    if (!found) {
        Send_WhoIs(Target_Device_Object_Instance,
            Target_Device_Object_Instance);
    }
    /* loop forever */
    for (;;) {
        /* increment timer - exit if timed out */
        current_seconds = time(NULL);

        /* at least one second has passed */
        if (current_seconds != last_seconds)
            tsm_timer_milliseconds((uint16_t) ((current_seconds -
                        last_seconds) * 1000));
        if (Error_Detected)
            break;
        /* wait until the device is bound, or timeout and quit */
        if (!found) {
            found =
                address_bind_request(Target_Device_Object_Instance, &max_apdu,
                &Target_Address);
        }
        if (found) {
            if (Request_Invoke_ID == 0) {
                Request_Invoke_ID =
                    Send_Write_Property_Request(Target_Device_Object_Instance,
                    Target_Object_Type, Target_Object_Instance,
                    Target_Object_Property, &Target_Object_Property_Value[0],
                    Target_Object_Property_Priority,
                    Target_Object_Property_Index);
            } else if (tsm_invoke_id_free(Request_Invoke_ID))
                break;
            else if (tsm_invoke_id_failed(Request_Invoke_ID)) {
                fprintf(stderr, "\rError: TSM Timeout!\n");
                tsm_free_invoke_id(Request_Invoke_ID);
                Error_Detected = true;
                /* try again or abort? */
                break;
            }
        } else {
            /* increment timer - exit if timed out */
            elapsed_seconds += (current_seconds - last_seconds);
            if (elapsed_seconds > timeout_seconds) {
                Error_Detected = true;
                printf("\rError: APDU Timeout!\n");
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
