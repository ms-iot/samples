/******************************************************************
 *
 * Copyright 2015 Intel Corporation All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#ifndef CA_BLE_LINUX_SERVER_H
#define CA_BLE_LINUX_SERVER_H

#include "bluez-glue.h"

#include <stdbool.h>
#include <sys/types.h>

/**
 * Information needed to complete a GATT server response send.
 */
typedef struct _CAGattResponseInfo
{
    /**
     * The BlueZ @c org.bluez.GattCharacteristic1 skeleton object
     * through which data will be sent.
     */
    GattCharacteristic1 * const characteristic;

} CAGattResponseInfo;

/**
 * Send response notification to the GATT client.
 *
 * @param[in] method_info Pointer to @c GattResponseInfo object that
 *                        contains information necessary to complete
 *                        send of response.
 * @param[in] data        Octet array of response data to be sent.
 * @param[in] length      Length of the @a data octet array.
 *
 * @see @c CAGattSendMethod() for further details.
 */
bool CAGattServerSendResponse(void const * method_info,
                              void const * data,
                              size_t length);


#endif  /* CA_BLE_LINUX_SERVER_H */
