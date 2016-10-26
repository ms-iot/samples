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
#include <ctype.h>
#include <time.h>       /* for time */
#include <errno.h>
#include "address.h"
#include "bactext.h"
#include "config.h"
#include "bacdef.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "net.h"
#include "datalink.h"
#include "timesync.h"
#include "version.h"
/* some demo stuff needed */
#include "filename.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#include "dlenv.h"

/* buffer used for receive */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };
/* error flag */
static bool Error_Detected = false;

void MyAbortHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    uint8_t abort_reason,
    bool server)
{
    /* FIXME: verify src and invoke id */
    (void) src;
    (void) invoke_id;
    (void) server;
    printf("BACnet Abort: %s\n", bactext_abort_reason_name(abort_reason));
    Error_Detected = true;
}

void MyRejectHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    uint8_t reject_reason)
{
    /* FIXME: verify src and invoke id */
    (void) src;
    (void) invoke_id;
    printf("BACnet Reject: %s\n", bactext_reject_reason_name(reject_reason));
    Error_Detected = true;
}

static void Init_Service_Handlers(
    void)
{
    Device_Init(NULL);
    /* we need to handle who-is
       to support dynamic device binding to us */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    /* set the handler for all the services we don't implement
       It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    /* we must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
    /* handle the reply (request) coming in */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_UTC_TIME_SYNCHRONIZATION,
        handler_timesync_utc);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_TIME_SYNCHRONIZATION,
        handler_timesync);
    /* handle any errors coming back */
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);
}

static void print_usage(char *filename)
{
    printf("Usage: %s [--dnet][--dadr][--mac]\n", filename);
    printf("       [--version][--help]\n");
}

static void print_help(char *filename)
{
    printf("Send BACnet TimeSynchronization request.\n"
        "\n"
        "--mac A\n"
        "BACnet mac address."
        "Valid ranges are from 0 to 255\n"
        "or an IP string with optional port number like 10.1.2.3:47808\n"
        "or an Ethernet MAC in hex like 00:21:70:7e:32:bb\n"
        "\n"
        "--dnet N\n"
        "BACnet network number N for directed requests.\n"
        "Valid range is from 0 to 65535 where 0 is the local connection\n"
        "and 65535 is network broadcast.\n"
        "\n"
        "--dadr A\n"
        "BACnet mac address on the destination BACnet network number.\n"
        "Valid ranges are from 0 to 255\n"
        "or an IP string with optional port number like 10.1.2.3:47808\n"
        "or an Ethernet MAC in hex like 00:21:70:7e:32:bb\n"
        "\n");
    printf("Examples:\n"
        "Send a TimeSynchronization request to DNET 123:\n"
        "%s --dnet 123\n", filename);
    printf("Send a TimeSynchronization request to MAC 10.0.0.1 DNET 123 DADR 5:\n"
        "%s --mac 10.0.0.1 --dnet 123 --dadr 5\n", filename);
    printf("Send a TimeSynchronization request to MAC 10.1.2.3:47808:\n"
        "%s --mac 10.1.2.3:47808\n", filename);
#if 0
    /* FIXME: it would be nice to be able to send arbitrary time values */
        "date format: year/month/day:dayofweek (e.g. 2006/4/1:6)\n"
        "year: AD, such as 2006\n" "month: 1=January, 12=December\n"
        "day: 1-31\n" "dayofweek: 1=Monday, 7=Sunday\n" "\n"
        "time format: hour:minute:second.hundredths (e.g. 23:59:59.12)\n"
        "hour: 0-23\n" "minute: 0-59\n" "second: 0-59\n"
        "hundredths: 0-99\n" "\n"
        "Optional device-instance sends a unicast time sync.\n",
        filename);
#endif
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
    time_t elapsed_seconds = 0;
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    time_t timeout_seconds = 0;
    time_t rawtime;
    struct tm *my_time;
    BACNET_DATE bdate;
    BACNET_TIME btime;
    long dnet = -1;
    BACNET_MAC_ADDRESS mac = { 0 };
    BACNET_MAC_ADDRESS adr = { 0 };
    BACNET_ADDRESS dest = { 0 };
    bool global_broadcast = true;
    int argi = 0;
    char *filename = NULL;

    /* decode any command line parameters */
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
        if (strcmp(argv[argi], "--mac") == 0) {
            if (++argi < argc) {
                if (address_mac_from_ascii(&mac, argv[argi])) {
                    global_broadcast = false;
                }
            }
        }
        if (strcmp(argv[argi], "--dnet") == 0) {
            if (++argi < argc) {
                dnet = strtol(argv[argi], NULL, 0);
                if ((dnet >= 0) && (dnet <= BACNET_BROADCAST_NETWORK)) {
                    global_broadcast = false;
                }
            }
        }
        if (strcmp(argv[argi], "--dadr") == 0) {
            if (++argi < argc) {
                if (address_mac_from_ascii(&adr, argv[argi])) {
                    global_broadcast = false;
                }
            }
        }
    }
    if (global_broadcast) {
        datalink_get_broadcast_address(&dest);
    } else {
        if (adr.len && mac.len) {
            memcpy(&dest.mac[0], &mac.adr[0], mac.len);
            dest.mac_len = mac.len;
            memcpy(&dest.adr[0], &adr.adr[0], adr.len);
            dest.len = adr.len;
            if ((dnet >= 0) && (dnet <= BACNET_BROADCAST_NETWORK)) {
                dest.net = dnet;
            } else {
                dest.net = BACNET_BROADCAST_NETWORK;
            }
        } else if (mac.len) {
            memcpy(&dest.mac[0], &mac.adr[0], mac.len);
            dest.mac_len = mac.len;
            dest.len = 0;
            if ((dnet >= 0) && (dnet <= BACNET_BROADCAST_NETWORK)) {
                dest.net = dnet;
            } else {
                dest.net = 0;
            }
        } else {
            if ((dnet >= 0) && (dnet <= BACNET_BROADCAST_NETWORK)) {
                dest.net = dnet;
            } else {
                dest.net = BACNET_BROADCAST_NETWORK;
            }
            dest.mac_len = 0;
            dest.len = 0;
        }
    }
    /* setup my info */
    Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
    Init_Service_Handlers();
    dlenv_init();
    atexit(datalink_cleanup);
    /* configure the timeout values */
    last_seconds = time(NULL);
    timeout_seconds = apdu_timeout() / 1000;
    /* send the request */
    time(&rawtime);
    my_time = localtime(&rawtime);
    bdate.year = my_time->tm_year + 1900;
    bdate.month = my_time->tm_mon + 1;
    bdate.day = my_time->tm_mday;
    bdate.wday = my_time->tm_wday ? my_time->tm_wday : 7;
    btime.hour = my_time->tm_hour;
    btime.min = my_time->tm_min;
    btime.sec = my_time->tm_sec;
    btime.hundredths = 0;
    Send_TimeSync_Remote(&dest, &bdate, &btime);
    /* loop forever - not necessary for time sync, but we can watch */
    for (;;) {
        /* increment timer - exit if timed out */
        current_seconds = time(NULL);
        /* returns 0 bytes on timeout */
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);
        /* process */
        if (pdu_len) {
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }
        if (Error_Detected)
            break;
        /* increment timer - exit if timed out */
        elapsed_seconds += (current_seconds - last_seconds);
        if (elapsed_seconds > timeout_seconds)
            break;
        /* keep track of time for next check */
        last_seconds = current_seconds;
    }

    return 0;
}
