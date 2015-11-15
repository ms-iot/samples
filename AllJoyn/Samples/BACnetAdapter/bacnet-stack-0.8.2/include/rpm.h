/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
*********************************************************************/
#ifndef RPM_H
#define RPM_H

#include <stdint.h>
#include <stdbool.h>
#include "bacenum.h"
#include "bacdef.h"
#include "bacapp.h"
#include "proplist.h"
/*
 * Bundle together commonly used data items for convenience when calling
 * rpm helper functions.
 */

typedef struct BACnet_RPM_Data {
    BACNET_OBJECT_TYPE object_type;
    uint32_t object_instance;
    BACNET_PROPERTY_ID object_property;
    uint32_t array_index;
    BACNET_ERROR_CLASS error_class;
    BACNET_ERROR_CODE error_code;
} BACNET_RPM_DATA;

struct BACnet_Read_Access_Data;
typedef struct BACnet_Read_Access_Data {
    BACNET_OBJECT_TYPE object_type;
    uint32_t object_instance;
    /* simple linked list of values */
    BACNET_PROPERTY_REFERENCE *listOfProperties;
    struct BACnet_Read_Access_Data *next;
} BACNET_READ_ACCESS_DATA;

/** Fetches the lists of properties (array of BACNET_PROPERTY_ID's) for this
 *  object type, grouped by Required, Optional, and Proprietary.
 * A function template; @see device.c for assignment to object types.
 * @ingroup ObjHelpers
 *
 * @param pRequired [out] Pointer reference for the list of Required properties.
 * @param pOptional [out] Pointer reference for the list of Optional properties.
 * @param pProprietary [out] Pointer reference for the list of Proprietary
 *                           properties for this BACNET_OBJECT_TYPE.
 */
typedef void (
    *rpm_property_lists_function) (
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary);

typedef void (
    *rpm_object_property_lists_function) (
    BACNET_OBJECT_TYPE object_type,
    struct special_property_list_t * pPropertyList);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* encode functions */
/* Start with the Init function, and then add an object,
 then add its properties, and then end the object.
 Continue to add objects and properties as needed
 until the APDU is full.*/

/* RPM */
    int rpm_encode_apdu_init(
        uint8_t * apdu,
        uint8_t invoke_id);

    int rpm_encode_apdu_object_begin(
        uint8_t * apdu,
        BACNET_OBJECT_TYPE object_type,
        uint32_t object_instance);

    int rpm_encode_apdu_object_property(
        uint8_t * apdu,
        BACNET_PROPERTY_ID object_property,
        uint32_t array_index);

    int rpm_encode_apdu_object_end(
        uint8_t * apdu);

    int rpm_encode_apdu(
        uint8_t * apdu,
        size_t max_apdu,
        uint8_t invoke_id,
        BACNET_READ_ACCESS_DATA * read_access_data);

/* decode the object portion of the service request only */
    int rpm_decode_object_id(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_RPM_DATA * rpmdata);

/* is this the end of this object property list? */
    int rpm_decode_object_end(
        uint8_t * apdu,
        unsigned apdu_len);

/* decode the object property portion of the service request only */
    int rpm_decode_object_property(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_RPM_DATA * rpmdata);

/* RPM Ack - reply from server */
    int rpm_ack_encode_apdu_init(
        uint8_t * apdu,
        uint8_t invoke_id);

    int rpm_ack_encode_apdu_object_begin(
        uint8_t * apdu,
        BACNET_RPM_DATA * rpmdata);

    int rpm_ack_encode_apdu_object_property(
        uint8_t * apdu,
        BACNET_PROPERTY_ID object_property,
        uint32_t array_index);

    int rpm_ack_encode_apdu_object_property_value(
        uint8_t * apdu,
        uint8_t * application_data,
        unsigned application_data_len);

    int rpm_ack_encode_apdu_object_property_error(
        uint8_t * apdu,
        BACNET_ERROR_CLASS error_class,
        BACNET_ERROR_CODE error_code);

    int rpm_ack_encode_apdu_object_end(
        uint8_t * apdu);

    int rpm_ack_decode_object_id(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_OBJECT_TYPE * object_type,
        uint32_t * object_instance);
/* is this the end of the list of this objects properties values? */
    int rpm_ack_decode_object_end(
        uint8_t * apdu,
        unsigned apdu_len);
    int rpm_ack_decode_object_property(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_PROPERTY_ID * object_property,
        uint32_t * array_index);
#ifdef TEST
#include "ctest.h"
    int rpm_decode_apdu(
        uint8_t * apdu,
        unsigned apdu_len,
        uint8_t * invoke_id,
        uint8_t ** service_request,
        unsigned *service_request_len);

    int rpm_ack_decode_apdu(
        uint8_t * apdu,
        int apdu_len,   /* total length of the apdu */
        uint8_t * invoke_id,
        uint8_t ** service_request,
        unsigned *service_request_len);

    void testReadPropertyMultiple(
        Test * pTest);
    void testReadPropertyMultipleAck(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @defgroup DSRPM Data Sharing -Read Property Multiple Service (DS-RPM)
 * @ingroup DataShare
 * 15.7 ReadPropertyMultiple Service <br>
 * The ReadPropertyMultiple service is used by a client BACnet-user to request
 * the values of one or more specified properties of one or more BACnet Objects.
 * This service allows read access to any property of any object, whether a
 * BACnet-defined object or not. The user may read a single property of a single
 * object, a list of properties of a single object, or any number of properties
 * of any number of objects.
 * A 'Read Access Specification' with the property identifier ALL can be used to
 * learn the implemented properties of an object along with their values.
 */
#endif
