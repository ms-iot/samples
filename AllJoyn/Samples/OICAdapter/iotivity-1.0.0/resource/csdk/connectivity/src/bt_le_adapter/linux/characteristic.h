/* ****************************************************************
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

#ifndef CA_BLE_LINUX_CHARACTERISTIC_H
#define CA_BLE_LINUX_CHARACTERISTIC_H

#include "bluez-glue.h"
#include "descriptor.h"
#include "recv.h"
#include "context.h"


/**
 * OIC GATT Characteristic Information
 *
 * OIC GATT characteristics contain one user description descriptor.
 *
 * @note The response characteristic should also have a client
 *       characteristic configuration descriptor.  However, BlueZ
 *       implicitly adds that descriptor to the characteristic when
 *       the characteristic "notify" property is set.  There is no
 *       need to explicitly add that descriptor.
 */
typedef struct CAGattCharacteristic
{

    /**
     * Object containing the D-Bus connection list of connected
     * devices.
     */
    CALEContext * context;

    /**
     * IoTivity OIC GATT service information.
     *
     * @note This is currently only used by the response
     *       characteristic to gain access to the request
     *       characteristic @c client endpoint field.
     *
     * @todo It seems somewhat wasteful have a field available to both
     *       response and request characteristics, but used by only
     *       one of them.
     */
    struct CAGattService * service;

    /// D-Bus object path for the GattCharacteristic1 object.
    char * object_path;

    /// OIC GATT service D-Bus interface skeleton object.
    GattCharacteristic1 * characteristic;

    /// OIC GATT user description descriptor information.
    CAGattDescriptor descriptor;

    /**
     * Information used to keep track of received data fragments.
     *
     * @note This is only used by the OIC GATT request characteristic
     *       skeleton implementation.  It is not needed by the
     *       response characteristic skeleton.
     *
     * @todo It seems somewhat wasteful have a field available to both
     *       response and request characteristics, but used by only
     *       one of them.
     */
    CAGattRecvInfo recv_info;

} CAGattCharacteristic;

/**
 * Initialize GATT request characteristic fields.
 *
 * This function initializes the request @c CAGattCharacteristic
 * object fields.
 *
 * @param[in,out] s       Information about GATT service to which the
 *                        characteristic belongs.
 * @param[in]     context Object containing the D-Bus connection to
 *                        the bus on which the characteristic will be
 *                        exported, as well as the list of connected
 *                        devices.
 *
 * @return @c true on success, @c false otherwise.
 */
bool CAGattRequestCharacteristicInitialize(struct CAGattService * s,
                                           CALEContext * context);

/**
 * Initialize GATT response characteristic fields.
 *
 * This function initializes the response @c CAGattCharacteristic
 * object fields.
 *
 * @param[in,out] s        Information about GATT service to which the
 *                         characteristic belongs.
 * @param[in]     context  Object containing the D-Bus connection to
 *                         the bus on which the characteristic will be
 *                         exported, as well as the list of connected
 *                         devices.
 *
 * @return @c true on success, @c false otherwise.
 */
bool CAGattResponseCharacteristicInitialize(struct CAGattService * s,
                                            CALEContext * context);

/**
 * Destroy GATT characteristic fields.
 *
 * This function finalizes the @c CAGattCharacteristic object fields.
 *
 * @param[in] characteristic GATT characteristic information to be
 *                           finalized.
 */
void CAGattCharacteristicDestroy(CAGattCharacteristic * characteristic);

/**
 * Get all characteristic properties.
 *
 * @param[in] characteristic The D-Bus skeleton object from which the
 *                           characteristic properties will be extracted.
 * @return A variant of the form a{sa{sv}}, suitable for use in the
 *         result of the
 *         @c org.freedesktop.DBus.ObjectManager.GetManagedObjects()
 *         method provided by the IoTivity the
 *         @c org.bluez.GattService1 implementation.
 */
GVariant * CAGattCharacteristicGetProperties(
    GattCharacteristic1 * characteristic);


#endif  // CA_BLE_LINUX_CHARACTERISTIC_H
