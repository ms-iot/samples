/**************************************************************************
*
* Copyright (C) 2008 Steve Karg <skarg@users.sourceforge.net>
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
#include <string.h>
#include <errno.h>
#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "apdu.h"
#include "npdu.h"
#include "abort.h"
/* special for this module */
#include "cov.h"
#include "bactext.h"
#include "handlers.h"

#ifndef MAX_COV_PROPERTIES
#define MAX_COV_PROPERTIES 2
#endif

/** @file h_ucov.c  Handles Unconfirmed COV Notifications. */

/*  */
/** Handler for an Unconfirmed COV Notification.
 * @ingroup DSCOV
 * Decodes the received list of Properties to update,
 * and print them out with the subscription information.
 * @note Nothing is specified in BACnet about what to do with the
 *       information received from Unconfirmed COV Notifications.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message (unused)
 */
void handler_ucov_notification(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src)
{
    BACNET_COV_DATA cov_data;
    BACNET_PROPERTY_VALUE property_value[MAX_COV_PROPERTIES];
    BACNET_PROPERTY_VALUE *pProperty_value = NULL;
    int len = 0;
    unsigned index = 0;

    /* src not needed for this application */
    src = src;
    /* create linked list to store data if more
       than one property value is expected */
    pProperty_value = &property_value[0];
    while (pProperty_value) {
        index++;
        if (index < MAX_COV_PROPERTIES) {
            pProperty_value->next = &property_value[index];
        } else {
            pProperty_value->next = NULL;
        }
        pProperty_value = pProperty_value->next;
    }
    cov_data.listOfValues = &property_value[0];
#if PRINT_ENABLED
    fprintf(stderr, "UCOV: Received Notification!\n");
#endif
    /* decode the service request only */
    len =
        cov_notify_decode_service_request(service_request, service_len,
        &cov_data);
#if PRINT_ENABLED
    if (len > 0) {
        fprintf(stderr, "UCOV: PID=%u ", cov_data.subscriberProcessIdentifier);
        fprintf(stderr, "instance=%u ", cov_data.initiatingDeviceIdentifier);
        fprintf(stderr, "%s %u ",
            bactext_object_type_name(cov_data.monitoredObjectIdentifier.type),
            cov_data.monitoredObjectIdentifier.instance);
        fprintf(stderr, "time remaining=%u seconds ", cov_data.timeRemaining);
        fprintf(stderr, "\n");
        pProperty_value = &property_value[0];
        while (pProperty_value) {
            fprintf(stderr, "UCOV: ");
            if (pProperty_value->propertyIdentifier < 512) {
                fprintf(stderr, "%s ",
                    bactext_property_name
                    (pProperty_value->propertyIdentifier));
            } else {
                fprintf(stderr, "proprietary %u ",
                    pProperty_value->propertyIdentifier);
            }
            if (pProperty_value->propertyArrayIndex != BACNET_ARRAY_ALL) {
                fprintf(stderr, "%u ", pProperty_value->propertyArrayIndex);
            }
            fprintf(stderr, "\n");
            pProperty_value = pProperty_value->next;
        }
    } else {
        fprintf(stderr, "UCOV: Unable to decode service request!\n");
    }
#endif
}
