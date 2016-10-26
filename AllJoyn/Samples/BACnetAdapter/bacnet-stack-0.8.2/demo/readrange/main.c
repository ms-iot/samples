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

#define PRINT_ENABLED 1

/* command line tool that sends a BACnet service, and displays the reply */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>       /* for time */
#include "bacdef.h"
#include "config.h"
#include "bactext.h"
#include "bacerror.h"
#include "iam.h"
#include "tsm.h"
#include "address.h"
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
#include "readrange.h"

/* buffer used for receive */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

/* converted command line arguments */
static uint32_t Target_Device_Object_Instance = BACNET_MAX_INSTANCE;
static uint32_t Target_Object_Instance = BACNET_MAX_INSTANCE;
static BACNET_OBJECT_TYPE Target_Object_Type = OBJECT_ANALOG_INPUT;
static BACNET_PROPERTY_ID Target_Object_Property = PROP_ACKED_TRANSITIONS;
static long Target_Object_Range_Type = 0;
static long Target_Object_Index = 0;
static long Target_Object_Count = 0;
/* the invoke id is needed to filter incoming messages */
static uint8_t Request_Invoke_ID = 0;
static BACNET_ADDRESS Target_Address;
static bool Error_Detected = false;
/* specific request data */
static BACNET_READ_RANGE_DATA RR_Request;

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
    /* handle the data coming back from confirmed requests */
    apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_RANGE,
        handler_read_range_ack);

    /* handle any errors coming back */
    apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY, MyErrorHandler);
    apdu_set_error_handler(SERVICE_CONFIRMED_PRIVATE_TRANSFER, MyErrorHandler);
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);
}

static void print_usage(char *filename)
{
    printf("Usage: %s device-instance object-type object-instance property\n",
        filename);
    printf("       range-type <index|<date time>> count\n");
    printf("       [--version][--help]\n");
}

static void print_help(char *filename)
{
    printf("Read a range of properties from an array or list property\n"
        "in an object in a BACnet device and print the values.\n"
        "device-instance:\n"
        "BACnet Device Object Instance number that you are\n"
        "trying to communicate to.  This number will be used\n"
        "to try and bind with the device using Who-Is and\n"
        "I-Am services.  For example, if you were reading\n"
        "Device Object 123, the device-instance would be 123.\n"
        "\nobject-type:\n"
        "The object type is the integer value of the enumeration\n"
        "BACNET_OBJECT_TYPE in bacenum.h.  It is the object\n"
        "that you are reading.  For example if you were\n"
        "reading Trend Log 2, the object-type would be 20.\n"
        "\nobject-instance:\n"
        "This is the object instance number of the object that\n"
        "you are reading.  For example, if you were reading\n"
        "Trend Log 2, the object-instance would be 2.\n"
        "\nproperty:\n"
        "The property is an integer value of the enumeration\n"
        "BACNET_PROPERTY_ID in bacenum.h.  It is the property\n"
        "you are reading.  For example, if you were reading the\n"
        "Log_Buffer property, use 131 as the property.\n"
        "\nrange-type:\n"
        "1=By Position\n"
        "2=By Sequence\n"
        "3=By Time\n"
        "4=All\n"
        "\nindex or date/time:\n"
        "This integer parameter is the starting index, or date & time.\n"
        "\ncount:\n"
        "This integer parameter is the number of elements to read.\n"
        "\nExample:\n"
        "If you want read the Log_Buffer of Trend Log 2\n"
        "in Device 123, from starting position 1 and read 10 entries,\n"
        "you could send the following command:\n"
        "%s 123 20 2 131 1 1 10\n", filename);
    printf("%s 123 20 2 131 2 1 10\n", filename);
    printf("%s 123 20 2 131 3 1/1/2014 00:00:01 10\n", filename);
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
    int argi = 0;
    int count = 0;
    int hour, min, sec, hundredths;
    int year, month, day, wday;
    char *filename = NULL;

    filename = filename_remove_path(argv[0]);
    for (argi = 1; argi < argc; argi++) {
        if (strcmp(argv[argi], "--help") == 0) {
            print_usage(filename);
            print_help(filename);
            return 0;
        }
        if (strcmp(argv[argi], "--version") == 0) {
            printf("%s %s\n", filename, BACNET_VERSION_TEXT);
            printf("Copyright (C) 2014 by Steve Karg and others.\n"
                "This is free software; see the source for copying conditions.\n"
                "There is NO warranty; not even for MERCHANTABILITY or\n"
                "FITNESS FOR A PARTICULAR PURPOSE.\n");
            return 0;
        }
    }
    if (argc < 5) {
        print_usage(filename);
        return 0;
    }
    /* decode the command line parameters */
    Target_Device_Object_Instance = strtol(argv[1], NULL, 0);
    Target_Object_Type = strtol(argv[2], NULL, 0);
    Target_Object_Instance = strtol(argv[3], NULL, 0);
    Target_Object_Property = strtol(argv[4], NULL, 0);
    Target_Object_Range_Type = strtol(argv[5], NULL, 0);
    /* some bounds checking */
    if (Target_Device_Object_Instance > BACNET_MAX_INSTANCE) {
        fprintf(stderr, "device-instance=%u - it must be less than %u\r\n",
            Target_Device_Object_Instance, BACNET_MAX_INSTANCE);
        return 1;
    }
    if (Target_Object_Range_Type == 1) {
        if (argc < 7) {
            print_usage(filename);
            return 0;
        }
        RR_Request.RequestType = RR_BY_POSITION;
        Target_Object_Index = strtol(argv[6], NULL, 0);
        Target_Object_Count = strtol(argv[7], NULL, 0);
        RR_Request.Range.RefIndex = Target_Object_Index;
        RR_Request.Count = Target_Object_Count;
    } else if (Target_Object_Range_Type == 2) {
        if (argc < 7) {
            print_usage(filename);
            return 0;
        }
        RR_Request.RequestType = RR_BY_SEQUENCE;
        Target_Object_Index = strtol(argv[6], NULL, 0);
        Target_Object_Count = strtol(argv[7], NULL, 0);
        RR_Request.Range.RefSeqNum = Target_Object_Index;
        RR_Request.Count = Target_Object_Count;
    } else if (Target_Object_Range_Type == 3) {
        if (argc < 8) {
            print_usage(filename);
            return 0;
        }
        RR_Request.RequestType = RR_BY_TIME;
        count =
            sscanf(argv[6], "%4d/%3d/%3d:%3d", &year, &month, &day,
            &wday);
        if (count == 3) {
            datetime_set_date(&RR_Request.Range.RefTime.date,
                (uint16_t) year,
                (uint8_t) month,
                (uint8_t) day);
        } else if (count == 4) {
            RR_Request.Range.RefTime.date.year = (uint16_t) year;
            RR_Request.Range.RefTime.date.month = (uint8_t) month;
            RR_Request.Range.RefTime.date.day = (uint8_t) day;
            RR_Request.Range.RefTime.date.wday = (uint8_t) wday;
        } else {
            fprintf(stderr, "Invalid date format!\r\n");
            return 1;
        }
        count =
            sscanf(argv[7], "%3d:%3d:%3d.%3d", &hour, &min, &sec,
            &hundredths);
        if (count == 4) {
            RR_Request.Range.RefTime.time.hour = (uint8_t) hour;
            RR_Request.Range.RefTime.time.min = (uint8_t) min;
            RR_Request.Range.RefTime.time.sec = (uint8_t) sec;
            RR_Request.Range.RefTime.time.hundredths = (uint8_t) hundredths;
        } else if (count == 3) {
            RR_Request.Range.RefTime.time.hour = (uint8_t) hour;
            RR_Request.Range.RefTime.time.min = (uint8_t) min;
            RR_Request.Range.RefTime.time.sec = (uint8_t) sec;
            RR_Request.Range.RefTime.time.hundredths = 0;
        } else if (count == 2) {
            RR_Request.Range.RefTime.time.hour = (uint8_t) hour;
            RR_Request.Range.RefTime.time.min = (uint8_t) min;
            RR_Request.Range.RefTime.time.sec = 0;
            RR_Request.Range.RefTime.time.hundredths = 0;
        } else {
            fprintf(stderr, "Invalid time format!\r\n");
            return 1;
        }
        Target_Object_Count = strtol(argv[8], NULL, 0);
        RR_Request.Count = Target_Object_Count;
    } else if (Target_Object_Range_Type == 4) {
        RR_Request.RequestType = RR_READ_ALL;
        RR_Request.Count = Target_Object_Count;
    } else {
        fprintf(stderr, "Invalid Range Type.  Use 1, 2, 3, or 4.\r\n");
        return 1;
    }
    RR_Request.object_type = Target_Object_Type;
    RR_Request.object_instance = Target_Object_Instance;
    RR_Request.object_property = Target_Object_Property;
    RR_Request.array_index = BACNET_ARRAY_ALL;
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
                Request_Invoke_ID = Send_ReadRange_Request(
                    Target_Device_Object_Instance,
                    &RR_Request);
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
                printf("\rError: APDU Timeout!\n");
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
