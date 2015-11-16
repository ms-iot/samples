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

/* command line tool that sends a BACnet service, and displays the reply */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>       /* for time */
#include <errno.h>
#include "bactext.h"
#include "iam.h"
#include "awf.h"
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
/* some demo stuff needed */
#include "filename.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#include "dlenv.h"

/* buffer used for receive */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

/* global variables used in this file */
static uint32_t Target_File_Object_Instance = 4194303;
static uint32_t Target_Device_Object_Instance = 4194303;
static uint32_t Target_File_Requested_Octet_Count;
static uint8_t Target_File_Requested_Octet_Pad_Byte;
static BACNET_ADDRESS Target_Address;
static char *Local_File_Name = NULL;
static bool End_Of_File_Detected = false;
static bool Error_Detected = false;
static uint8_t Current_Invoke_ID = 0;

static void Atomic_Write_File_Error_Handler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    BACNET_ERROR_CLASS error_class,
    BACNET_ERROR_CODE error_code)
{
    if (address_match(&Target_Address, src) &&
        (invoke_id == Current_Invoke_ID)) {
        printf("\r\nBACnet Error!\r\n");
        printf("Error Class: %s\r\n", bactext_error_class_name(error_class));
        printf("Error Code: %s\r\n", bactext_error_code_name(error_code));
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
        (invoke_id == Current_Invoke_ID)) {
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
        (invoke_id == Current_Invoke_ID)) {
        printf("BACnet Reject: %s\r\n",
            bactext_reject_reason_name((int) reject_reason));
        Error_Detected = true;
    }
}

static void LocalIAmHandler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
    int len = 0;
    uint32_t device_id = 0;
    unsigned max_apdu = 0;
    int segmentation = 0;
    uint16_t vendor_id = 0;

    (void) src;
    (void) service_len;
    len =
        iam_decode_service_request(service_request, &device_id, &max_apdu,
        &segmentation, &vendor_id);
    if (len != -1) {
        address_add(device_id, max_apdu, src);
    } else
        fprintf(stderr, "!\n");

    return;
}

static void Init_Service_Handlers(
    void)
{
    Device_Init(NULL);
    /* we need to handle who-is
       to support dynamic device binding to us */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, LocalIAmHandler);
    /* set the handler for all the services we don't implement
       It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    /* we must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
    /* handle any errors coming back */
    apdu_set_error_handler(SERVICE_CONFIRMED_ATOMIC_WRITE_FILE,
        Atomic_Write_File_Error_Handler);
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
    unsigned timeout = 100;     /* milliseconds */
    unsigned max_apdu = 0;
    time_t elapsed_seconds = 0;
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    time_t timeout_seconds = 0;
    int fileStartPosition = 0;
    unsigned requestedOctetCount = 0;
    uint8_t invoke_id = 0;
    bool found = false;
    uint16_t my_max_apdu = 0;
    FILE *pFile = NULL;
    static BACNET_OCTET_STRING fileData;
    size_t len = 0;
    bool pad_byte = false;

    if (argc < 4) {
        /* FIXME: what about access method - record or stream? */
        printf
            ("%s device-instance file-instance local-name [octet count] [pad value]\r\n",
            filename_remove_path(argv[0]));
        return 0;
    }
    /* decode the command line parameters */
    Target_Device_Object_Instance = strtol(argv[1], NULL, 0);
    Target_File_Object_Instance = strtol(argv[2], NULL, 0);
    Local_File_Name = argv[3];
    if (Target_Device_Object_Instance >= BACNET_MAX_INSTANCE) {
        fprintf(stderr, "device-instance=%u - it must be less than %u\r\n",
            Target_Device_Object_Instance, BACNET_MAX_INSTANCE);
        return 1;
    }
    if (Target_File_Object_Instance >= BACNET_MAX_INSTANCE) {
        fprintf(stderr, "file-instance=%u - it must be less than %u\r\n",
            Target_File_Object_Instance, BACNET_MAX_INSTANCE + 1);
        return 1;
    }
    if (argc > 4) {
        Target_File_Requested_Octet_Count = strtol(argv[4], NULL, 0);
    }
    if (argc > 5) {
        Target_File_Requested_Octet_Pad_Byte = strtol(argv[5], NULL, 0);
        pad_byte = true;
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

        /* returns 0 bytes on timeout */
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);

        /* process */
        if (pdu_len) {
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }
        /* at least one second has passed */
        if (current_seconds != last_seconds) {
            tsm_timer_milliseconds(((current_seconds - last_seconds) * 1000));
        }
        /* wait until the device is bound, or timeout and quit */
        if (!found) {
            found =
                address_bind_request(Target_Device_Object_Instance, &max_apdu,
                &Target_Address);
        }
        if (found) {
            if (Target_File_Requested_Octet_Count) {
                requestedOctetCount = Target_File_Requested_Octet_Count;
            } else {
                /* calculate the smaller of our APDU size or theirs
                   and remove the overhead of the APDU (varies depending on size).
                   note: we could fail if there is a bottle neck (router)
                   and smaller MPDU in betweeen. */
                if (max_apdu < MAX_APDU) {
                    my_max_apdu = max_apdu;
                } else {
                    my_max_apdu = MAX_APDU;
                }
                /* Typical sizes are 50, 128, 206, 480, 1024, and 1476 octets */
                if (my_max_apdu <= 50) {
                    requestedOctetCount = my_max_apdu - 19;
                } else if (my_max_apdu <= 480) {
                    requestedOctetCount = my_max_apdu - 32;
                } else if (my_max_apdu <= 1476) {
                    requestedOctetCount = my_max_apdu - 64;
                } else {
                    requestedOctetCount = my_max_apdu / 2;
                }
            }
            /* has the previous invoke id expired or returned?
               note: invoke ID = 0 is invalid, so it will be idle */
            if ((invoke_id == 0) || tsm_invoke_id_free(invoke_id)) {
                if (End_Of_File_Detected || Error_Detected) {
                    printf("\r\n");
                    break;
                }
                if (invoke_id != 0) {
                    fileStartPosition += requestedOctetCount;
                }
                /* we'll read the file in chunks
                   less than max_apdu to keep unsegmented */
                pFile = fopen(Local_File_Name, "rb");
                if (pFile) {
                    (void) fseek(pFile, fileStartPosition, SEEK_SET);
                    len =
                        fread(octetstring_value(&fileData), 1,
                        requestedOctetCount, pFile);
                    if (len < requestedOctetCount) {
                        End_Of_File_Detected = true;
                        if (pad_byte) {
                            memset(octetstring_value(&fileData) + len + 1,
                                (int) Target_File_Requested_Octet_Pad_Byte,
                                requestedOctetCount - len);
                            len = requestedOctetCount;
                        }
                    }
                    octetstring_truncate(&fileData, len);
                    fclose(pFile);
                } else {
                    End_Of_File_Detected = true;
                }
                printf("\rSending %d bytes", (int)(fileStartPosition + len));
                invoke_id =
                    Send_Atomic_Write_File_Stream
                    (Target_Device_Object_Instance,
                    Target_File_Object_Instance, fileStartPosition, &fileData);
                Current_Invoke_ID = invoke_id;
            } else if (tsm_invoke_id_failed(invoke_id)) {
                fprintf(stderr, "\rError: TSM Timeout!\r\n");
                tsm_free_invoke_id(invoke_id);
                Error_Detected = true;
                /* try again or abort? */
                break;
            }
        } else {
            /* increment timer - exit if timed out */
            elapsed_seconds += (current_seconds - last_seconds);
            if (elapsed_seconds > timeout_seconds) {
                fprintf(stderr, "\rError: APDU Timeout!\r\n");
                Error_Detected = true;
                break;
            }
        }
        /* keep track of time for next check */
        last_seconds = current_seconds;
    }

    if (Error_Detected) {
        return 1;
    }
    return 0;
}
