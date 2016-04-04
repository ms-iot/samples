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
#include "bactext.h"
#include "iam.h"
#include "address.h"
#include "config.h"
#include "bacdef.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "datalink.h"
#include "bactext.h"
#include "version.h"
/* some demo stuff needed */
#include "filename.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#if defined(BACDL_MSTP)
#include "rs485.h"
#endif
#include "dlenv.h"
#include "net.h"

/* buffer used for receive */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

/* global variables used in this file */
static int32_t Target_Object_Instance_Min = -1;
static int32_t Target_Object_Instance_Max = -1;
static bool Error_Detected = false;

#define BAC_ADDRESS_MULT 1

struct address_entry {
    struct address_entry *next;
    uint8_t Flags;
    uint32_t device_id;
    unsigned max_apdu;
    BACNET_ADDRESS address;
};

static struct address_table {
    struct address_entry *first;
    struct address_entry *last;
} Address_Table = {
0};


struct address_entry *alloc_address_entry(
    void)
{
    struct address_entry *rval;
    rval = (struct address_entry *) calloc(1, sizeof(struct address_entry));
    if (Address_Table.first == 0) {
        Address_Table.first = Address_Table.last = rval;
    } else {
        Address_Table.last->next = rval;
        Address_Table.last = rval;
    }
    return rval;
}



bool bacnet_address_matches(
    BACNET_ADDRESS * a1,
    BACNET_ADDRESS * a2)
{
    int i = 0;
    if (a1->net != a2->net)
        return false;
    if (a1->len != a2->len)
        return false;
    for (; i < a1->len; i++)
        if (a1->adr[i] != a2->adr[i])
            return false;
    return true;
}

void address_table_add(
    uint32_t device_id,
    unsigned max_apdu,
    BACNET_ADDRESS * src)
{
    struct address_entry *pMatch;
    uint8_t flags = 0;

    pMatch = Address_Table.first;
    while (pMatch) {
        if (pMatch->device_id == device_id) {
            if (bacnet_address_matches(&pMatch->address, src))
                return;
            flags |= BAC_ADDRESS_MULT;
            pMatch->Flags |= BAC_ADDRESS_MULT;
        }
        pMatch = pMatch->next;
    }

    pMatch = alloc_address_entry();

    pMatch->Flags = flags;
    pMatch->device_id = device_id;
    pMatch->max_apdu = max_apdu;
    pMatch->address = *src;

    return;
}



void my_i_am_handler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
    int len = 0;
    uint32_t device_id = 0;
    unsigned max_apdu = 0;
    int segmentation = 0;
    uint16_t vendor_id = 0;

    (void) service_len;
    len =
        iam_decode_service_request(service_request, &device_id, &max_apdu,
        &segmentation, &vendor_id);
#if PRINT_ENABLED
    fprintf(stderr, "Received I-Am Request");
#endif
    if (len != -1) {
#if PRINT_ENABLED
        fprintf(stderr, " from %lu, MAC = %d.%d.%d.%d.%d.%d\n",
            (unsigned long) device_id, src->mac[0], src->mac[1], src->mac[2],
            src->mac[3], src->mac[4], src->mac[5]);
#endif
        address_table_add(device_id, max_apdu, src);
    } else {
#if PRINT_ENABLED
        fprintf(stderr, ", but unable to decode it.\n");
#endif
    }

    return;
}

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
    fprintf(stderr, "BACnet Abort: %s\n",
        bactext_abort_reason_name(abort_reason));
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
    fprintf(stderr, "BACnet Reject: %s\n",
        bactext_reject_reason_name(reject_reason));
    Error_Detected = true;
}

static void init_service_handlers(
    void)
{
    Device_Init(NULL);
    /* Note: this applications doesn't need to handle who-is
       it is confusing for the user! */
    /* set the handler for all the services we don't implement
       It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    /* we must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
    /* handle the reply (request) coming back */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, my_i_am_handler);
    /* handle any errors coming back */
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);
}

void print_macaddr(
    uint8_t * addr,
    int len)
{
    int j = 0;

    while (j < len) {
        if (j != 0) {
            printf(":");
        }
        printf("%02X", addr[j]);
        j++;
    }
    while (j < MAX_MAC_LEN) {
        printf("   ");
        j++;
    }
}

static void print_address_cache(
    void)
{
    BACNET_ADDRESS address;
    unsigned total_addresses = 0;
    unsigned dup_addresses = 0;
    struct address_entry *addr;
    uint8_t local_sadr = 0;

    /*  NOTE: this string format is parsed by src/address.c,
       so these must be compatible. */

    printf(";%-7s  %-20s %-5s %-20s %-4s\n", "Device", "MAC (hex)", "SNET",
        "SADR (hex)", "APDU");
    printf(";-------- -------------------- ----- -------------------- ----\n");


    addr = Address_Table.first;
    while (addr) {
        bacnet_address_copy(&address, &addr->address);
        total_addresses++;
        if (addr->Flags & BAC_ADDRESS_MULT) {
            dup_addresses++;
            printf(";");
        } else {
            printf(" ");
        }
        printf(" %-7u ", addr->device_id);
        print_macaddr(address.mac, address.mac_len);
        printf(" %-5hu ", address.net);
        if (address.net) {
            print_macaddr(address.adr, address.len);
        } else {
            print_macaddr(&local_sadr, 1);
        }
        printf(" %-4hu ", addr->max_apdu);
        printf("\n");

        addr = addr->next;
    }
    printf(";\n; Total Devices: %u\n", total_addresses);
    if (dup_addresses) {
        printf("; * Duplicate Devices: %u\n", dup_addresses);
    }
}

static void print_usage(
    char *filename)
{
    printf("Usage: %s", filename);
    printf(" [device-instance-min [device-instance-max]]\n");
    printf("       [--dnet][--dadr][--mac]\n");
    printf("       [--version][--help]\n");
}

static void print_help(
    char *filename)
{
    printf("Send BACnet WhoIs service request to a device or multiple\n"
        "devices, and wait for responses. Displays any devices found\n"
        "and their network information.\n"
        "\n"
        "device-instance:\n"
        "BACnet Device Object Instance number that you are trying\n"
        "to send a Who-Is service request. The value should be in\n"
        "the range of 0 to 4194303. A range of values can also be\n"
        "specified by using a minimum value and a maximum value.\n"
        "\n");
    printf("--mac A\n"
        "BACnet mac address."
        "Valid ranges are from 00 to FF (hex) for MS/TP or ARCNET,\n"
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
        "Valid ranges are from 00 to FF (hex) for MS/TP or ARCNET,\n"
        "or an IP string with optional port number like 10.1.2.3:47808\n"
        "or an Ethernet MAC in hex like 00:21:70:7e:32:bb\n"
        "\n");
    printf("Send a WhoIs request to DNET 123:\n"
        "%s --dnet 123\n", filename);
    printf("Send a WhoIs request to MAC 10.0.0.1 DNET 123 DADR 05h:\n"
        "%s --mac 10.0.0.1 --dnet 123 --dadr 05\n", filename);
    printf("Send a WhoIs request to MAC 10.1.2.3:47808:\n"
        "%s --mac 10.1.2.3:47808\n", filename);
    printf("Send a WhoIs request to Device 123:\n"
        "%s 123\n", filename);
    printf("Send a WhoIs request to Devices from 1000 to 9000:\n"
        "%s 1000 9000\n", filename);
    printf("Send a WhoIs request to Devices from 1000 to 9000 on DNET 123:\n"
        "%s 1000 9000 --dnet 123\n", filename);
    printf("Send a WhoIs request to all devices:\n"
        "%s\n", filename);
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
    time_t total_seconds = 0;
    time_t elapsed_seconds = 0;
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    time_t timeout_seconds = 0;
    long dnet = -1;
    BACNET_MAC_ADDRESS mac = { 0 };
    BACNET_MAC_ADDRESS adr = { 0 };
    BACNET_ADDRESS dest = { 0 };
    bool global_broadcast = true;
    int argi = 0;
    unsigned int target_args = 0;
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
        } else if (strcmp(argv[argi], "--dnet") == 0) {
            if (++argi < argc) {
                dnet = strtol(argv[argi], NULL, 0);
                if ((dnet >= 0) && (dnet <= BACNET_BROADCAST_NETWORK)) {
                    global_broadcast = false;
                }
            }
        } else if (strcmp(argv[argi], "--dadr") == 0) {
            if (++argi < argc) {
                if (address_mac_from_ascii(&adr, argv[argi])) {
                    global_broadcast = false;
                }
            }
        } else {
            if (target_args == 0) {
                Target_Object_Instance_Min = Target_Object_Instance_Max =
                    strtol(argv[argi], NULL, 0);
                target_args++;
            } else if (target_args == 1) {
                Target_Object_Instance_Max = strtol(argv[argi], NULL, 0);
                target_args++;
            } else {
                print_usage(filename);
                return 1;
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
    if (Target_Object_Instance_Min > BACNET_MAX_INSTANCE) {
        fprintf(stderr, "device-instance-min=%u - it must be less than %u\n",
            Target_Object_Instance_Min, BACNET_MAX_INSTANCE + 1);
        return 1;
    }
    if (Target_Object_Instance_Max > BACNET_MAX_INSTANCE) {
        fprintf(stderr, "device-instance-max=%u - it must be less than %u\n",
            Target_Object_Instance_Max, BACNET_MAX_INSTANCE + 1);
        return 1;
    }
    /* setup my info */
    Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
    init_service_handlers();
    address_init();
    dlenv_init();
    atexit(datalink_cleanup);
    /* configure the timeout values */
    last_seconds = time(NULL);
    timeout_seconds = apdu_timeout() / 1000;
    /* send the request */
    Send_WhoIs_To_Network(&dest, Target_Object_Instance_Min,
        Target_Object_Instance_Max);
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
        if (Error_Detected)
            break;
        /* increment timer - exit if timed out */
        elapsed_seconds = current_seconds - last_seconds;
        if (elapsed_seconds) {
#if defined(BACDL_BIP) && BBMD_ENABLED
            bvlc_maintenance_timer(elapsed_seconds);
#endif
        }
        total_seconds += elapsed_seconds;
        if (total_seconds > timeout_seconds)
            break;
        /* keep track of time for next check */
        last_seconds = current_seconds;
    }
    print_address_cache();

    return 0;
}
