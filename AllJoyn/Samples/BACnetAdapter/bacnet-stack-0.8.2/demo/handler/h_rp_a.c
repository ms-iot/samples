/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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
#include <stdlib.h>
#include "config.h"
#include "txbuf.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "address.h"
#include "tsm.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "datalink.h"
#include "bactext.h"
#include "rp.h"
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"

/** @file h_rp_a.c  Handles Read Property Acknowledgments. */

/** For debugging...
 * @param [in] data portion of the ACK
 */
void rp_ack_print_data(
    BACNET_READ_PROPERTY_DATA * data)
{
    BACNET_OBJECT_PROPERTY_VALUE object_value;  /* for bacapp printing */
    BACNET_APPLICATION_DATA_VALUE value;        /* for decode value data */
    int len = 0;
    uint8_t *application_data;
    int application_data_len;
    bool first_value = true;
    bool print_brace = false;

    if (data) {
        application_data = data->application_data;
        application_data_len = data->application_data_len;
        /* FIXME: what if application_data_len is bigger than 255? */
        /* value? need to loop until all of the len is gone... */
        for (;;) {
            len =
                bacapp_decode_application_data(application_data,
                (uint8_t) application_data_len, &value);
            if (first_value && (len < application_data_len)) {
                first_value = false;
#if PRINT_ENABLED
                fprintf(stdout, "{");
#endif
                print_brace = true;
            }
            object_value.object_type = data->object_type;
            object_value.object_instance = data->object_instance;
            object_value.object_property = data->object_property;
            object_value.array_index = data->array_index;
            object_value.value = &value;
            bacapp_print_value(stdout, &object_value);
            if (len > 0) {
                if (len < application_data_len) {
                    application_data += len;
                    application_data_len -= len;
                    /* there's more! */
#if PRINT_ENABLED
                    fprintf(stdout, ",");
#endif
                } else {
                    break;
                }
            } else {
                break;
            }
        }
#if PRINT_ENABLED
        if (print_brace)
            fprintf(stdout, "}");
        fprintf(stdout, "\r\n");
#endif
    }
}


/** Handler for a ReadProperty ACK.
 * @ingroup DSRP
 * Doesn't actually do anything, except, for debugging, to
 * print out the ACK message.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_read_property_ack(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
    int len = 0;
    BACNET_READ_PROPERTY_DATA data;

    (void) src;
    (void) service_data;        /* we could use these... */
    len = rp_ack_decode_service_request(service_request, service_len, &data);
#if 0
    fprintf(stderr, "Received Read-Property Ack!\n");
#endif
    if (len > 0)
        rp_ack_print_data(&data);
}

/** Decode the received RP data into a linked list of the results, with the
 *  same data structure used by RPM ACK replies.
 *  This function is provided to provide common handling for RP and RPM data,
 *  and fully decodes the value(s) portion of the data for one property.
 * @ingroup DSRP
 * @see rp_ack_decode_service_request(), rpm_ack_decode_service_request()
 *
 * @param apdu [in] The received apdu data.
 * @param apdu_len [in] Total length of the apdu.
 * @param read_access_data [out] Pointer to the head of the linked list
 * 			where the RP data is to be stored.
 * @return Number of decoded bytes (could be less than apdu_len),
 * 			or -1 on decoding error.
 */
int rp_ack_fully_decode_service_request(
    uint8_t * apdu,
    int apdu_len,
    BACNET_READ_ACCESS_DATA * read_access_data)
{
    int decoded_len = 0;        /* return value */
    BACNET_READ_PROPERTY_DATA rp1data;
    BACNET_PROPERTY_REFERENCE *rp1_property;    /* single property */
    BACNET_APPLICATION_DATA_VALUE *value, *old_value;
    uint8_t *vdata;
    int vlen, len;

    decoded_len = rp_ack_decode_service_request(apdu, apdu_len, &rp1data);
    if (decoded_len > 0) {
        /* Then we have to transfer to the BACNET_READ_ACCESS_DATA structure
         * and decode the value(s) portion
         */
        read_access_data->object_type = rp1data.object_type;
        read_access_data->object_instance = rp1data.object_instance;
        rp1_property = calloc(1, sizeof(BACNET_PROPERTY_REFERENCE));
        read_access_data->listOfProperties = rp1_property;
        if (rp1_property == NULL) {
            /* can't proceed if calloc failed. */
            return BACNET_STATUS_ERROR;
        }
        rp1_property->propertyIdentifier = rp1data.object_property;
        rp1_property->propertyArrayIndex = rp1data.array_index;
        /* Is there no Error case possible here, as there is when decoding RPM? */
        /* rp1_property->error.error_class = ?? */
        /* rp_ack_decode_service_request() processing already removed the
         * Opening and Closing '3' Tags.
         * note: if this is an array, there will be
         more than one element to decode */
        vdata = rp1data.application_data;
        vlen = rp1data.application_data_len;
        value = calloc(1, sizeof(BACNET_APPLICATION_DATA_VALUE));
        rp1_property->value = value;
        old_value = value;
        while (value && vdata && (vlen > 0)) {
            if (IS_CONTEXT_SPECIFIC(*vdata)) {
                len =
                    bacapp_decode_context_data(vdata, vlen, value,
                    rp1_property->propertyIdentifier);
            } else {
                len = bacapp_decode_application_data(vdata, vlen, value);
            }
            if (len < 0) {
                /* unable to decode the data */
                while (value) {
                    /* free the linked list of values */
                    old_value = value;
                    value = value->next;
                    free(old_value);
                }
                free(rp1_property);
                read_access_data->listOfProperties = NULL;
                return len;
            }
            decoded_len += len;
            vlen -= len;
            vdata += len;
            /* If unexpected closing tag here: */
            if (vlen && decode_is_closing_tag_number(vdata, 3)) {
                decoded_len++;
                vlen--;
                vdata++;
                break;
            } else {
                if (len == 0) {
                    /* nothing decoded and no closing tag, so malformed */
                    while (value) {
                        /* free the linked list of values */
                        old_value = value;
                        value = value->next;
                        free(old_value);
                    }
                    free(rp1_property);
                    read_access_data->listOfProperties = NULL;
                    return BACNET_STATUS_ERROR;
                }
                if (vlen > 0) {
                    /* If more values */
                    old_value = value;
                    value = calloc(1, sizeof(BACNET_APPLICATION_DATA_VALUE));
                    old_value->next = value;
                }
            }
        }
    }

    return decoded_len;
}
