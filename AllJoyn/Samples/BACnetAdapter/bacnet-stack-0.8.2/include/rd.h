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
#ifndef REINITIALIZE_DEVICE_H
#define REINITIALIZE_DEVICE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct BACnet_Reinitialize_Device_Data {
    BACNET_REINITIALIZED_STATE state;
    BACNET_CHARACTER_STRING password;
    BACNET_ERROR_CLASS error_class;
    BACNET_ERROR_CODE error_code;
} BACNET_REINITIALIZE_DEVICE_DATA;

typedef bool(
    *reinitialize_device_function) (
    BACNET_REINITIALIZE_DEVICE_DATA * rd_data);


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* encode service */
    int rd_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_REINITIALIZED_STATE state,
        BACNET_CHARACTER_STRING * password);

/* decode the service request only */
    int rd_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_REINITIALIZED_STATE * state,
        BACNET_CHARACTER_STRING * password);

#ifdef TEST
#include "ctest.h"
    int rd_decode_apdu(
        uint8_t * apdu,
        unsigned apdu_len,
        uint8_t * invoke_id,
        BACNET_REINITIALIZED_STATE * state,
        BACNET_CHARACTER_STRING * password);

    void test_ReinitializeDevice(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @defgroup DMRD Device Management-ReinitializeDevice (DM-RD)
 * @ingroup RDMS
 * 16.4 ReinitializeDevice Service <br>
 * The ReinitializeDevice service is used by a client BACnet-user to instruct
 * a remote device to reboot itself (cold start), reset itself to some
 * predefined initial state (warm start), or to control the backup or restore
 * procedure. Resetting or rebooting a device is primarily initiated by a human
 * operator for diagnostic purposes. Use of this service during the backup or
 * restore procedure is usually initiated on behalf of the user by the device
 * controlling the backup or restore. Due to the sensitive nature of this
 * service, a password may be required from the responding BACnet-user prior
 * to executing the service.
 *
 */
#endif
