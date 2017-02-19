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

#ifndef CA_BLE_LINUX_DESCRIPTOR_H
#define CA_BLE_LINUX_DESCRIPTOR_H

#include "bluez-glue.h"

#include <stdbool.h>


/**
 * OIC GATT Descriptor Information
 */
typedef struct CAGattDescriptor
{
    /// D-Bus object path for the GattCharacteristic1 object.
    char * object_path;

    /// OIC GATT service D-Bus interface skeleton object.
    GattDescriptor1 * descriptor;

} CAGattDescriptor;

// Forward declarations.
struct CAGattService;

/**
 * Initialize GATT request descriptor fields.
 *
 * This function initializes the request @c CAGattDescriptor object
 * fields.
 *
 * @param[in,out] s          Information about GATT service to which the
 *                           characteristic and descriptor belong.
 * @param[in]     connection D-Bus connection to the bus on which the
 *                           descriptor will be exported.
 *
 * @return @c true on success, @c false otherwise.
 */
bool CAGattRequestDescriptorInitialize(struct CAGattService * s,
                                       GDBusConnection * connection);

/**
 * Initialize GATT response descriptor fields.
 *
 * This function initializes the response @c CAGattDescriptor object
 * fields.
 *
 * @param[in,out] s          Information about GATT service to which the
 *                           characteristic and descriptor belong.
 * @param[in]     connection D-Bus connection to the bus on which the
 *                           descriptor will be exported.
 *
 * @return @c true on success, @c false otherwise.
 */
bool CAGattResponseDescriptorInitialize(struct CAGattService *s,
                                        GDBusConnection * connection);

/**
 * Destroy GATT descriptor fields.
 *
 * This function finalizes the @c CAGattDescriptor object fields.
 *
 * @param[in] descriptor GATT characteristic information to be
 *                       finalized.
 */
void CAGattDescriptorDestroy(CAGattDescriptor * descriptor);

/**
 * Get all descriptor properties.
 *
 * @param[in] descriptor The D-Bus skeleton object from which the
 *                       descriptor properties will be extracted.
 *
 * @return A variant of the form a{sa{sv}}, suitable for use in the
 *         result of the
 *         @c org.freedesktop.DBus.ObjectManager.GetManagedObjects()
 *         method provided by the IoTivity the
 *         @c org.bluez.GattService1 implementation.
 */
GVariant * CAGattDescriptorGetProperties(GattDescriptor1 * descriptor);


#endif  // CA_BLE_LINUX_DESCRIPTOR_H
