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
#ifndef IAM_H
#define IAM_H

#include <stdint.h>
#include <stdbool.h>
#include "bacdef.h"
#include "bacaddr.h"
#include "npdu.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    int iam_encode_apdu(
        uint8_t * apdu,
        uint32_t device_id,
        unsigned max_apdu,
        int segmentation,
        uint16_t vendor_id);

    int iam_decode_service_request(
        uint8_t * apdu,
        uint32_t * pDevice_id,
        unsigned *pMax_apdu,
        int *pSegmentation,
        uint16_t * pVendor_id);

#ifdef TEST
#include "ctest.h"
    int iam_decode_apdu(
        uint8_t * apdu,
        uint32_t * pDevice_id,
        unsigned *pMax_apdu,
        int *pSegmentation,
        uint16_t * pVendor_id);

    void testIAm(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
