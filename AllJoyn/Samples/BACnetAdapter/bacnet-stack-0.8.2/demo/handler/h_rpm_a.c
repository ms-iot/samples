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
#include <stdlib.h>
#include <assert.h>
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
#include "rpm.h"
/* some demo stuff needed */
#include "handlers.h"
#include "txbuf.h"

/** @file h_rpm_a.c  Handles Read Property Multiple Acknowledgments. */

/** Decode the received RPM data and make a linked list of the results.
 * @ingroup DSRPM
 *
 * @param apdu [in] The received apdu data.
 * @param apdu_len [in] Total length of the apdu.
 * @param read_access_data [out] Pointer to the head of the linked list
 * 			where the RPM data is to be stored.
 * @return The number of bytes decoded, or -1 on error
 */
int rpm_ack_decode_service_request(
    uint8_t * apdu,
    int apdu_len,
    BACNET_READ_ACCESS_DATA * read_access_data)
{
    int decoded_len = 0;        /* return value */
    uint32_t error_value = 0;   /* decoded error value */
    int len = 0;        /* number of bytes returned from decoding */
    uint8_t tag_number = 0;     /* decoded tag number */
    uint32_t len_value = 0;     /* decoded length value */
    BACNET_READ_ACCESS_DATA *rpm_object;
    BACNET_READ_ACCESS_DATA *old_rpm_object;
    BACNET_PROPERTY_REFERENCE *rpm_property;
    BACNET_PROPERTY_REFERENCE *old_rpm_property;
    BACNET_APPLICATION_DATA_VALUE *value;
    BACNET_APPLICATION_DATA_VALUE *old_value;

    assert(read_access_data != NULL);
    rpm_object = read_access_data;
    old_rpm_object = rpm_object;
    while (rpm_object && apdu_len) {
        len =
            rpm_ack_decode_object_id(apdu, apdu_len, &rpm_object->object_type,
            &rpm_object->object_instance);
        if (len <= 0) {
            old_rpm_object->next = NULL;
            free(rpm_object);
            break;
        }
        decoded_len += len;
        apdu_len -= len;
        apdu += len;
        rpm_property = calloc(1, sizeof(BACNET_PROPERTY_REFERENCE));
        rpm_object->listOfProperties = rpm_property;
        old_rpm_property = rpm_property;
        while (rpm_property && apdu_len) {
            len =
                rpm_ack_decode_object_property(apdu, apdu_len,
                &rpm_property->propertyIdentifier,
                &rpm_property->propertyArrayIndex);
            if (len <= 0) {
                old_rpm_property->next = NULL;
                if (rpm_object->listOfProperties == rpm_property) {
                    /* was this the only property in the list? */
                    rpm_object->listOfProperties = NULL;
                }
                free(rpm_property);
                break;
            }
            decoded_len += len;
            apdu_len -= len;
            apdu += len;
            if (apdu_len && decode_is_opening_tag_number(apdu, 4)) {
                /* propertyValue */
                decoded_len++;
                apdu_len--;
                apdu++;
                /* note: if this is an array, there will be
                   more than one element to decode */
                value = calloc(1, sizeof(BACNET_APPLICATION_DATA_VALUE));
                rpm_property->value = value;
                old_value = value;
                while (value && (apdu_len > 0)) {
                    if (IS_CONTEXT_SPECIFIC(*apdu)) {
                        len =
                            bacapp_decode_context_data(apdu, apdu_len, value,
                            rpm_property->propertyIdentifier);
                    } else {
                        len =
                            bacapp_decode_application_data(apdu, apdu_len,
                            value);
                    }
                    /* If len == 0 then it's an empty structure, which is OK. */
                    if (len < 0) {
                        /* problem decoding */
                        /* calling function will free the memory */
                        return BACNET_STATUS_ERROR;
                    }
                    decoded_len += len;
                    apdu_len -= len;
                    apdu += len;
                    if (apdu_len && decode_is_closing_tag_number(apdu, 4)) {
                        decoded_len++;
                        apdu_len--;
                        apdu++;
                        break;
                    } else {
                        old_value = value;
                        value =
                            calloc(1, sizeof(BACNET_APPLICATION_DATA_VALUE));
                        old_value->next = value;
                    }
                }
            } else if (apdu_len && decode_is_opening_tag_number(apdu, 5)) {
                /* propertyAccessError */
                decoded_len++;
                apdu_len--;
                apdu++;
                /* decode the class and code sequence */
                len =
                    decode_tag_number_and_value(apdu, &tag_number, &len_value);
                decoded_len += len;
                apdu_len -= len;
                apdu += len;
                /* FIXME: we could validate that the tag is enumerated... */
                len = decode_enumerated(apdu, len_value, &error_value);
                rpm_property->error.error_class = error_value;
                decoded_len += len;
                apdu_len -= len;
                apdu += len;
                len =
                    decode_tag_number_and_value(apdu, &tag_number, &len_value);
                decoded_len += len;
                apdu_len -= len;
                apdu += len;
                /* FIXME: we could validate that the tag is enumerated... */
                len = decode_enumerated(apdu, len_value, &error_value);
                rpm_property->error.error_code = error_value;
                decoded_len += len;
                apdu_len -= len;
                apdu += len;
                if (apdu_len && decode_is_closing_tag_number(apdu, 5)) {
                    decoded_len++;
                    apdu_len--;
                    apdu++;
                }
            }
            old_rpm_property = rpm_property;
            rpm_property = calloc(1, sizeof(BACNET_PROPERTY_REFERENCE));
            old_rpm_property->next = rpm_property;
        }
        len = rpm_decode_object_end(apdu, apdu_len);
        if (len) {
            decoded_len += len;
            apdu_len -= len;
            apdu += len;
        }
        if (apdu_len) {
            old_rpm_object = rpm_object;
            rpm_object = calloc(1, sizeof(BACNET_READ_ACCESS_DATA));
            old_rpm_object->next = rpm_object;
        }
    }

    return decoded_len;
}

/* for debugging... */
void rpm_ack_print_data(
    BACNET_READ_ACCESS_DATA * rpm_data)
{
    BACNET_OBJECT_PROPERTY_VALUE object_value;  /* for bacapp printing */
    BACNET_PROPERTY_REFERENCE *listOfProperties;
    BACNET_APPLICATION_DATA_VALUE *value;
    bool array_value = false;

    if (rpm_data) {
#if PRINT_ENABLED
        fprintf(stdout, "%s #%lu\r\n",
            bactext_object_type_name(rpm_data->object_type),
            (unsigned long) rpm_data->object_instance);
        fprintf(stdout, "{\r\n");
#endif
        listOfProperties = rpm_data->listOfProperties;
        while (listOfProperties) {
#if PRINT_ENABLED
            if (listOfProperties->propertyIdentifier < 512) {
                fprintf(stdout, "    %s: ",
                    bactext_property_name(listOfProperties->
                        propertyIdentifier));
            } else {
                fprintf(stdout, "    proprietary %u: ",
                    (unsigned) listOfProperties->propertyIdentifier);
            }
#endif
            if (listOfProperties->propertyArrayIndex != BACNET_ARRAY_ALL) {
#if PRINT_ENABLED
                fprintf(stdout, "[%d]", listOfProperties->propertyArrayIndex);
#endif
            }
            value = listOfProperties->value;
            if (value) {
#if PRINT_ENABLED
                if (value->next) {
                    fprintf(stdout, "{");
                    array_value = true;
                } else {
                    array_value = false;
                }
#endif
                object_value.object_type = rpm_data->object_type;
                object_value.object_instance = rpm_data->object_instance;
                while (value) {
                    object_value.object_property =
                        listOfProperties->propertyIdentifier;
                    object_value.array_index =
                        listOfProperties->propertyArrayIndex;
                    object_value.value = value;
                    bacapp_print_value(stdout, &object_value);
#if PRINT_ENABLED
                    if (value->next) {
                        fprintf(stdout, ",\r\n        ");
                    } else {
                        if (array_value) {
                            fprintf(stdout, "}\r\n");
                        } else {
                            fprintf(stdout, "\r\n");
                        }
                    }
#endif
                    value = value->next;
                }
            } else {
#if PRINT_ENABLED
                /* AccessError */
                fprintf(stdout, "BACnet Error: %s: %s\r\n",
                    bactext_error_class_name((int) listOfProperties->
                        error.error_class),
                    bactext_error_code_name((int) listOfProperties->
                        error.error_code));
#endif
            }
            listOfProperties = listOfProperties->next;
        }
#if PRINT_ENABLED
        fprintf(stdout, "}\r\n");
#endif
    }
}

/** Handler for a ReadPropertyMultiple ACK.
 * @ingroup DSRPM
 * For each read property, print out the ACK'd data for debugging,
 * and free the request data items from linked property list.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
void handler_read_property_multiple_ack(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
    int len = 0;
    BACNET_READ_ACCESS_DATA *rpm_data;
    BACNET_READ_ACCESS_DATA *old_rpm_data;
    BACNET_PROPERTY_REFERENCE *rpm_property;
    BACNET_PROPERTY_REFERENCE *old_rpm_property;
    BACNET_APPLICATION_DATA_VALUE *value;
    BACNET_APPLICATION_DATA_VALUE *old_value;

    (void) src;
    (void) service_data;        /* we could use these... */

    rpm_data = calloc(1, sizeof(BACNET_READ_ACCESS_DATA));
    if (rpm_data) {
        len =
            rpm_ack_decode_service_request(service_request, service_len,
            rpm_data);
    }
#if 1
    fprintf(stderr, "Received Read-Property-Multiple Ack!\n");
#endif
    if (len > 0) {
        while (rpm_data) {
            rpm_ack_print_data(rpm_data);
            rpm_property = rpm_data->listOfProperties;
            while (rpm_property) {
                value = rpm_property->value;
                while (value) {
                    old_value = value;
                    value = value->next;
                    free(old_value);
                }
                old_rpm_property = rpm_property;
                rpm_property = rpm_property->next;
                free(old_rpm_property);
            }
            old_rpm_data = rpm_data;
            rpm_data = rpm_data->next;
            free(old_rpm_data);
        }
    } else {
#if 1
        fprintf(stderr, "RPM Ack Malformed! Freeing memory...\n");
#endif
        while (rpm_data) {
            rpm_property = rpm_data->listOfProperties;
            while (rpm_property) {
                value = rpm_property->value;
                while (value) {
                    old_value = value;
                    value = value->next;
                    free(old_value);
                }
                old_rpm_property = rpm_property;
                rpm_property = rpm_property->next;
                free(old_rpm_property);
            }
            old_rpm_data = rpm_data;
            rpm_data = rpm_data->next;
            free(old_rpm_data);
        }
    }
}
