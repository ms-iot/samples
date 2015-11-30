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
#ifndef WHOHAS_H
#define WHOHAS_H

#include <stdint.h>
#include <stdbool.h>
#include "bacstr.h"

typedef struct BACnet_Who_Has_Data {
    int32_t low_limit;  /* deviceInstanceRange */
    int32_t high_limit;
    bool is_object_name;        /* true if a string */
    union {
        BACNET_OBJECT_ID identifier;
        BACNET_CHARACTER_STRING name;
    } object;
} BACNET_WHO_HAS_DATA;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* encode service  - use -1 for limit if you want unlimited */
    int whohas_encode_apdu(
        uint8_t * apdu,
        BACNET_WHO_HAS_DATA * data);

    int whohas_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_WHO_HAS_DATA * data);

    int whohas_decode_apdu(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_WHO_HAS_DATA * data);

#ifdef TEST
#include "ctest.h"
    void testWhoHas(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @defgroup DMDOB Device Management-Dynamic Object Binding (DM-DOB)
 * @ingroup RDMS
 * 16.9 Who-Has and I-Have Services <br>
 * The Who-Has service is used by a sending BACnet-user to identify the device
 * object identifiers and network addresses of other BACnet devices whose local
 * databases contain an object with a given Object_Name or a given Object_Identifier.
 * The I-Have service is used to respond to Who-Has service requests or to
 * advertise the existence of an object with a given Object_Name or
 * Object_Identifier. The I-Have service request may be issued at any time and
 * does not need to be preceded by the receipt of a Who-Has service request.
 * The Who-Has and I-Have services are unconfirmed services.
 *
 */
#endif
