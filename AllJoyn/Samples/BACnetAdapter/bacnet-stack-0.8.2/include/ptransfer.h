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
#ifndef PRIVATE_TRANSFER_H
#define PRIVATE_TRANSFER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct BACnet_Private_Transfer_Data {
    uint16_t vendorID;
    uint32_t serviceNumber;
    uint8_t *serviceParameters;
    int serviceParametersLen;
} BACNET_PRIVATE_TRANSFER_DATA;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    int ptransfer_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_PRIVATE_TRANSFER_DATA * private_data);
    int uptransfer_encode_apdu(
        uint8_t * apdu,
        BACNET_PRIVATE_TRANSFER_DATA * private_data);
    int ptransfer_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_PRIVATE_TRANSFER_DATA * private_data);

    int ptransfer_error_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_ERROR_CLASS error_class,
        BACNET_ERROR_CODE error_code,
        BACNET_PRIVATE_TRANSFER_DATA * private_data);
    int ptransfer_error_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_ERROR_CLASS * error_class,
        BACNET_ERROR_CODE * error_code,
        BACNET_PRIVATE_TRANSFER_DATA * private_data);

    int ptransfer_ack_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_PRIVATE_TRANSFER_DATA * private_data);
/* ptransfer_ack_decode_service_request() is the same as
       ptransfer_decode_service_request */

#ifdef TEST
#include "ctest.h"
    void test_Private_Transfer_Request(
        Test * pTest);
    void test_Private_Transfer_Ack(
        Test * pTest);
    void test_Private_Transfer_Error(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
