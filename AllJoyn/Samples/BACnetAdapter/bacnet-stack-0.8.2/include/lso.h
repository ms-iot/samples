/**************************************************************************
*
* Copyright (C) 2006 John Minack
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
#ifndef LSO_H
#define LSO_H

#include <stdint.h>
#include <stdbool.h>
#include "bacenum.h"
#include "bacdef.h"
#include "bacstr.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Life Safety Operation Service */

    typedef struct {
        uint32_t processId;
        BACNET_CHARACTER_STRING requestingSrc;
        BACNET_LIFE_SAFETY_OPERATION operation;
        BACNET_OBJECT_ID targetObject;
    } BACNET_LSO_DATA;


    int lso_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_LSO_DATA * data);
/* decode the service request only */
    int lso_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_LSO_DATA * data);


#ifdef TEST
#include "ctest.h"
    void testLSO(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
