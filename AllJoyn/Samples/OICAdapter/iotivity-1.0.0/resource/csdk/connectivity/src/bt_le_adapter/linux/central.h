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

#ifndef CA_BLE_LINUX_CENTRAL_H
#define CA_BLE_LINUX_CENTRAL_H

#include "context.h"

#include "cacommon.h"


/**
 * Initialize and start a Linux BLE "central".
 *
 * Initialize all Linux BLE "central" state (i.e. a global
 * @c CACentralContext instance), as well as start
 * discovery of OIC GATT transport service capable peripherals.
 *
 * @param[in] context Context containing BlueZ adapter information.
 *
 * @return @c CA_STATUS_OK on success.
 */
CAResult_t CACentralStart(CALEContext * context);

/**
 * Stop the Linux BLE "central".
 *
 * @param[in] context Context containing BlueZ adapter information.
 *
 * @return @c CA_STATUS_OK on success.
 */
CAResult_t CACentralStop(CALEContext * context);

/**
 * Start discovery of OIC Transport Profile capable LE peripherals.
 *
 * @param[in] context Context containing BlueZ adapter information.
 *
 * @return @c CA_STATUS_OK on success.
 */
CAResult_t CACentralStartDiscovery(CALEContext * context);

/**
 * Stop discovery of OIC Transport Profile capable LE peripherals.
 *
 * @param[in] context Context containing BlueZ adapter information.
 *
 * @return @c CA_STATUS_OK on success.
 */
CAResult_t CACentralStopDiscovery(CALEContext * context);

/**
 * Connect to the LE peripheral pointed by @a device.
 *
 * @param[in] device Proxy to the BlueZ @c org.bluez.Device1 object
 *                   through which the connection to the LE peripheral
 *                   will be established.
 *
 * @return @c true on success, @c false otherwise.
 */
bool CACentralConnect(GDBusProxy * device);

/**
 * Connect to all discovered LE peripherals.
 *
 * @param[in] context Context containing BlueZ adapter information.
 *
 * @return @c true on success, @c false otherwise.
 */
bool CACentralConnectToAll(CALEContext * context);


#endif  /* CA_BLE_LINUX_CENTRAL_H */
