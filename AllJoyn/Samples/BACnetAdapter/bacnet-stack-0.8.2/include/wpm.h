/**************************************************************************
*
* Copyright (C) 2011 Krzysztof Malorny <malornykrzysztof@gmail.com>
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
#ifndef WRITEPROPERTYMULTIPLE_H
#define WRITEPROPERTYMULTIPLE_H

#include <stdint.h>
#include <stdbool.h>
#include "bacdcode.h"
#include "bacapp.h"
#include "wp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    struct BACnet_Write_Access_Data;
    typedef struct BACnet_Write_Access_Data {
        BACNET_OBJECT_TYPE object_type;
        uint32_t object_instance;
        /* simple linked list of values */
        BACNET_PROPERTY_VALUE *listOfProperties;
        struct BACnet_Write_Access_Data *next;
    } BACNET_WRITE_ACCESS_DATA;

    /* decode the service request only */
    int wpm_decode_object_id(
        uint8_t * apdu,
        uint16_t apdu_len,
        BACNET_WRITE_PROPERTY_DATA * data);

    int wpm_decode_object_property(
        uint8_t * apdu,
        uint16_t apdu_len,
        BACNET_WRITE_PROPERTY_DATA * wpm_data);


    /* encode objects */
    int wpm_encode_apdu_init(
        uint8_t * apdu,
        uint8_t invoke_id);
    int wpm_encode_apdu_object_begin(
        uint8_t * apdu,
        BACNET_OBJECT_TYPE object_type,
        uint32_t object_instance);
    int wpm_encode_apdu_object_end(
        uint8_t * apdu);
    int wpm_encode_apdu_object_property(
        uint8_t * apdu,
        BACNET_WRITE_PROPERTY_DATA * wpdata);
    int wpm_encode_apdu(
        uint8_t * apdu,
        size_t max_apdu,
        uint8_t invoke_id,
        BACNET_WRITE_ACCESS_DATA * write_access_data);

    /* encode service */
    int wpm_ack_encode_apdu_init(
        uint8_t * apdu,
        uint8_t invoke_id);

    int wpm_error_ack_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_WRITE_PROPERTY_DATA * wp_data);


#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @defgroup DSWP Data Sharing - Write Property Multiple Service (DS-WPM)
 * @ingroup DataShare
 * 15.10 WriteProperty Multiple Service <br>
 * The WritePropertyMultiple service is used by a client BACnet-user
 * to modify the value of one or more specified properties of a BACnet object.
 * This service potentially allows write access to any property of any object,
 * whether a BACnet-defined object or not.
 * Properties shall be modified by the WritePropertyMultiple service
 * in the order specified in the 'List of Write Access Specifications' parameter,
 * and execution of the service shall continue until all of the specified
 * properties have been written to or a property is encountered that
 * for some reason cannot be modified as requested.
 * Some implementors may wish to restrict write access to certain properties
 * of certain objects. In such cases, an attempt to modify a restricted property
 * shall result in the return of an error of 'Error Class' PROPERTY and 'Error Code'
 * WRITE_ACCESS_DENIED. Note that these restricted properties may be accessible
 * through the use of Virtual Terminal services or other means at the discretion
 * of the implementor.
*/
#endif
