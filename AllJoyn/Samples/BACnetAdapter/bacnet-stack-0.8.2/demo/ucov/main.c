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

/* command line tool that sends a BACnet service */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>       /* for time */
#include <errno.h>
#include "bactext.h"
#include "iam.h"
#include "cov.h"
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
}

int main(
    int argc,
    char *argv[])
{
    char *value_string = NULL;
    bool status = false;
    BACNET_COV_DATA cov_data;
    BACNET_PROPERTY_VALUE value_list;
    uint8_t tag;

    if (argc < 7) {
        /* note: priority 16 and 0 should produce the same end results... */
        printf("Usage: %s pid device-id object-type object-instance "
            "time property tag value [priority] [index]\r\n" "\r\n" "pid:\r\n"
            "Process Identifier for this broadcast.\r\n" "\r\n"
            "device-id:\r\n"
            "The Initiating BACnet Device Object Instance number.\r\n" "\r\n"
            "object-type:\r\n"
            "The monitored object type is the integer value of the\r\n"
            "enumeration BACNET_OBJECT_TYPE in bacenum.h.  For example,\r\n"
            "if you were monitoring Analog Output 2, the object-type\r\n"
            "would be 1.\r\n" "\r\n" "object-instance:\r\n"
            "The monitored object instance number.\r\n" "\r\n" "time:\r\n"
            "The subscription time remaining is conveyed in seconds.\r\n"
            "\r\n" "property:\r\n"
            "The property is an integer value of the enumeration \r\n"
            "BACNET_PROPERTY_ID in bacenum.h. For example, if you were\r\n"
            "monitoring the Present Value property, you would use 85\r\n"
            "as the property.\r\n" "\r\n" "tag:\r\n"
            "Tag is the integer value of the enumeration BACNET_APPLICATION_TAG \r\n"
            "in bacenum.h.  It is the data type of the value that you are\r\n"
            "monitoring.  For example, if you were monitoring a REAL value, you would \r\n"
            "use a tag of 4." "\r\n" "value:\r\n"
            "The value is an ASCII representation of some type of data that you\r\n"
            "are monitoring.  It is encoded using the tag information provided.  For\r\n"
            "example, if you were writing a REAL value of 100.0, you would use \r\n"
            "100.0 as the value.\r\n" "\r\n" "[priority]:\r\n"
            "This optional parameter is used for reporting the priority of the\r\n"
            "value. If no priority is given, none is sent, and the BACnet \r\n"
            "standard requires that the value is reported at the lowest \r\n"
            "priority (16) if the object property supports priorities.\r\n"
            "\r\n" "[index]\r\n"
            "This optional integer parameter is the index number of an array.\r\n"
            "If the property is an array, individual elements can be reported.\r\n"
            "\r\n" "Here is a brief overview of BACnet property and tags:\r\n"
            "Certain properties are expected to be written with certain \r\n"
            "application tags, so you probably need to know which ones to use\r\n"
            "with each property of each object.  It is almost safe to say that\r\n"
            "given a property and an object and a table, the tag could be looked\r\n"
            "up automatically.  There may be a few exceptions to this, such as\r\n"
            "the Any property type in the schedule object and the Present Value\r\n"
            "accepting REAL, BOOLEAN, NULL, etc.  Perhaps it would be simpler for\r\n"
            "the demo to use this kind of table - but I also wanted to be able\r\n"
            "to do negative testing by passing the wrong tag and have the server\r\n"
            "return a reject message.\r\n" "\r\n" "Example:\r\n"
            "If you want generate an unconfirmed COV,\r\n"
            "you could send the following command:\r\n"
            "%s 1 2 3 4 5 85 4 100.0\r\n"
            "where 1=pid, 2=device-id, 3=AV, 4=object-id, 5=time,\r\n"
            "85=Present-Value, 4=REAL, 100.0=value\r\n",
            filename_remove_path(argv[0]), filename_remove_path(argv[0]));
        return 0;
    }
    /* decode the command line parameters */
    cov_data.subscriberProcessIdentifier = strtol(argv[1], NULL, 0);
    cov_data.initiatingDeviceIdentifier = strtol(argv[2], NULL, 0);
    cov_data.monitoredObjectIdentifier.type = strtol(argv[3], NULL, 0);
    cov_data.monitoredObjectIdentifier.instance = strtol(argv[4], NULL, 0);
    cov_data.timeRemaining = strtol(argv[5], NULL, 0);
    cov_data.listOfValues = &value_list;
    value_list.next = NULL;
    value_list.propertyIdentifier = strtol(argv[6], NULL, 0);
    tag = strtol(argv[7], NULL, 0);
    value_string = argv[8];
    /* optional priority */
    if (argc > 9)
        value_list.priority = strtol(argv[9], NULL, 0);
    else
        value_list.priority = BACNET_NO_PRIORITY;
    /* optional index */
    if (argc > 10)
        value_list.propertyArrayIndex = strtol(argv[10], NULL, 0);
    else
        value_list.propertyArrayIndex = BACNET_ARRAY_ALL;

    if (cov_data.initiatingDeviceIdentifier >= BACNET_MAX_INSTANCE) {
        fprintf(stderr, "device-instance=%u - it must be less than %u\r\n",
            cov_data.initiatingDeviceIdentifier, BACNET_MAX_INSTANCE);
        return 1;
    }
    if (cov_data.monitoredObjectIdentifier.type >= MAX_BACNET_OBJECT_TYPE) {
        fprintf(stderr, "object-type=%u - it must be less than %u\r\n",
            cov_data.monitoredObjectIdentifier.type, MAX_BACNET_OBJECT_TYPE);
        return 1;
    }
    if (cov_data.monitoredObjectIdentifier.instance > BACNET_MAX_INSTANCE) {
        fprintf(stderr, "object-instance=%u - it must be less than %u\r\n",
            cov_data.monitoredObjectIdentifier.instance,
            BACNET_MAX_INSTANCE + 1);
        return 1;
    }
    if (cov_data.listOfValues->propertyIdentifier > MAX_BACNET_PROPERTY_ID) {
        fprintf(stderr, "object-type=%u - it must be less than %u\r\n",
            cov_data.listOfValues->propertyIdentifier,
            MAX_BACNET_PROPERTY_ID + 1);
        return 1;
    }
    if (tag >= MAX_BACNET_APPLICATION_TAG) {
        fprintf(stderr, "tag=%u - it must be less than %u\r\n", tag,
            MAX_BACNET_APPLICATION_TAG);
        return 1;
    }
    status =
        bacapp_parse_application_data(tag, value_string,
        &cov_data.listOfValues->value);
    if (!status) {
        /* FIXME: show the expected entry format for the tag */
        fprintf(stderr, "unable to parse the tag value\r\n");
        return 1;
    }
    /* setup my info */
    Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
    Init_Service_Handlers();
    dlenv_init();
    atexit(datalink_cleanup);
    Send_UCOV_Notify(&Handler_Transmit_Buffer[0], &cov_data);

    return 0;
}
