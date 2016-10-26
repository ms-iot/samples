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

#ifndef CA_BLE_LINUX_PERIPHERAL_H
#define CA_BLE_LINUX_PERIPHERAL_H

#include "context.h"
#include "advertisement.h"

#include "cacommon.h"


/**
 * @internal
 *
 * Linux BLE "peripheral" context.
 */
typedef struct _CAPeripheralContext
{
    /**
     * Base context.
     *
     * The base context contains core state used throughout the Linux
     * BLE adapter implementation.
     */
    CALEContext * base;

    /// D-Bus bus name owner ID.
    guint owner_id;

    /**
     * LE advertising information.
     *
     * The LE advertising information will be registered with BlueZ,
     * which BlueZ will then use to add advertising data to each of
     * the detected Bluetooth hardware adapters.  Only one set of
     * advertising data is needed since the data is the same for all
     * adapters.
     */
    CALEAdvertisement advertisement;

    /**
     * List of @c CAGattService objects containing information for all
     * exported and registered IoTivity BlueZ GATT related D-Bus
     * objects.
     */
    GList * gatt_services;

    /**
     * Glib event loop that drives peripheral D-Bus signal
     * handling.
     *
     * @note A seperate thread drives this event loop.  This is
     *       necessitated by the need for signal subscriptions to be
     *       done in what the GLib documentation refers to as the
     *       "thread-default main context".  By the time the
     *       peripheral is started it's too late the use the main loop
     *       that was run when this Linux BLE transport adapter itself
     *       was started through @c CASelectNetwork() and the @c
     *       CAAdapterStart() callback the peripheral must have its
     *       own main loop.
     */
    GMainLoop * event_loop;

    /// Mutex used to synchronize access to context fields.
    ca_mutex lock;

    /**
     * Service registration condition variable.
     *
     * This condition variable is used to delay service registration
     * until the thread performing service initialization completes.
     * Initialization is performed in the same thread that will run
     * the peripheral's event loop.
     *
     * @see @c GMainLoop documentation for further details.
     */
    ca_cond condition;

} CAPeripheralContext;

/**
 * Initialize global state.
 */
void CAPeripheralInitialize();

/**
 * Finalize global state.
 */
void CAPeripheralFinalize();

/**
 * Initialize and start a Linux BLE "peripheral".
 *
 * Initialize all Linux BLE "peripheral" state (i.e. a global
 * @c CAPeripheralContext instance), as well as register the OIC
 * GATT transport service with BlueZ to begin advertising.
 *
 * @param[in] context Base context.
 *
 * @return @c CA_STATUS_OK on success.
 */
CAResult_t CAPeripheralStart(CALEContext * context);

/**
 * Stop the Linux BLE "peripheral".
 *
 * @return @c CA_STATUS_OK on success.
 */
CAResult_t CAPeripheralStop();


#endif  /* CA_BLE_LINUX_PERIPHERAL_H */
