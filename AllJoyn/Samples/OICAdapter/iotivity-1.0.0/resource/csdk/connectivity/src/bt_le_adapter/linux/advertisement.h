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

#ifndef CA_BLE_LINUX_ADVERTISEMENT_H
#define CA_BLE_LINUX_ADVERTISEMENT_H

#include "bluez-glue.h"

#include <stdbool.h>


/**
 * Information needed for registering an LE advertisement with BlueZ.
 */
typedef struct CALEAdvertisement
{
    /// OIC LE advertisement D-Bus interface skeleton object.
    LEAdvertisement1 * advertisement;

    /**
     * Proxies to the BlueZ D-Bus objects that implement the
     * "org.bluez.LEAdvertisingManager1" interface with which the @c
     * advertisement is registered.
     */
    GList * managers;

} CALEAdvertisement;

/**
 * Initialize LE advertisement fields.
 *
 * This function initializes the @c CALEAdvertisement object fields.
 *
 * @param[out] a          LE advertisement information to be
 *                        initialized.
 * @param[in]  connection D-Bus connection to the bus on which the
 *                        advertisement will be exported.
 * @param[in]  managers   List of @c org.bluez.LEAdvertisingManager1
 *                        proxies.
 *
 * @return @c true on success, @c false otherwise.
 *
 * @note This function does not allocate the @a adv object itself.
 *       The caller is responsible for allocating that memory.
 */
bool CALEAdvertisementInitialize(CALEAdvertisement * a,
                                 GDBusConnection * connection,
                                 GList * managers);

/**
 * Destroy LE advertisement fields.
 *
 * This function finalizes the @c CALEAdvertisement object fields.
 *
 * @param[in] adv LE advertisement information to be finalized.
 *
 * @note This function does not deallocate the @a adv object itself.
 *       The caller is responsible for deallocating that memory.
 */
void CALEAdvertisementDestroy(CALEAdvertisement * adv);


#endif  // CA_BLE_LINUX_ADVERTISEMENT_H

