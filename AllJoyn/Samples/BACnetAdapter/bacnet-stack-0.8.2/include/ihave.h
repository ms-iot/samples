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
#ifndef IHAVE_H
#define IHAVE_H

#include <stdint.h>
#include <stdbool.h>
#include "bacstr.h"

typedef struct BACnet_I_Have_Data {
    BACNET_OBJECT_ID device_id;
    BACNET_OBJECT_ID object_id;
    BACNET_CHARACTER_STRING object_name;
} BACNET_I_HAVE_DATA;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    int ihave_encode_apdu(
        uint8_t * apdu,
        BACNET_I_HAVE_DATA * data);

    int ihave_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_I_HAVE_DATA * data);

    int ihave_decode_apdu(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_I_HAVE_DATA * data);

#ifdef TEST
#include "ctest.h"
    void testIHave(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
