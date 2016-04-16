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

#ifndef CA_BLE_LINUX_CLIENT_H
#define CA_BLE_LINUX_CLIENT_H

#include "context.h"


/**
 * Information needed to when sending a request through a GATT
 * client.
 */
typedef struct _CAGattRequestInfo
{
    /**
     * Proxy or list of proxies to @c org.bluez.GattCharacteristic1
     * object(s) through which request data will be sent to the GATT
     * server.
     *
     * In the case of a unicast-style send, @c info will be a
     * @c GDBusProxy* to an @c org.bluez.GattCharacteristic1 object.
     * For a multicast-style send, @c info will be a * @c GList* of
     * @c GDBusProxy* to @c GattCharacterstic1 objects on all GATT
     * servers to which a connection exists.
     */
    void * const characteristic_info;

    /**
     * Context containing additional information that may be needed
     * when sending a request.
     */
    CALEContext * const context;

} CAGattRequestInfo;

/**
 * Send request data through a single user-specified BLE connection.
 *
 * @param[in] method_info Information necessary to send request.
 * @param[in] data        Octet array of request data to be sent.
 * @param[in] length      Length of the @a data octet array.
 *
 * @see @c CAGattSendMethod() for further details.
 */
bool CAGattSendRequest(void const * method_info,
                       uint8_t const * data,
                       size_t length);

// ---------------------------------------------------------------
//                   Multicast-style Request Send
// ---------------------------------------------------------------
/**
 * Send request data through all BLE connections.
 *
 * Send the @a data to the GATT server found in all discovered LE
 * peripherals.
 *
 * @param[in] method_info Information necessary to send request.
 * @param[in] data        Octet array of request data to be sent.
 * @param[in] length      Length of the @a data octet array.

 * @note Since a multicast-like operation is being performed, an
 *       assumption is made that a GATT client is sending data to a
 *       server.  It makes no sense to multicast a response from a
 *       single GATT server to multiple GATT clients in IoTivity's
 *       case.
 *
 * @return @c CA_STATUS_OK on success, @c CA_STATUS_FAILED otherwise.
 */
CAResult_t CAGattClientSendDataToAll(void const * method_info,
                                     uint8_t const * data,
                                     size_t length);


#endif  /* CA_BLE_LINUX_CLIENT_H */
