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

#ifndef CA_BLE_LINUX_CONTEXT_H
#define CA_BLE_LINUX_CONTEXT_H

#include "caadapterinterface.h"
#include "camutex.h"
#include "cathreadpool.h"
#include "caleinterface.h"

#include <gio/gio.h>


/**
 * @internal
 *
 * BLE Linux adapter base context.
 */
typedef struct _CALEContext
{
    /// Connection to the D-Bus system bus.
    GDBusConnection * connection;

    /**
     * Proxy to the BlueZ D-Bus object that implements the
     * "org.freedesktop.DBus.ObjectManager" interface.
     *
     * @todo There's probably no need to keep this around after we've
     *       retrieved the managed objects the first time around since
     *       we can rely on signals to alert us to any subsequent
     *       changes.
     */
    GDBusObjectManager * object_manager;

    /**
     * List of @c GDBusObject objects obtained from the BlueZ
     * @c ObjectManager.
     *
     * @note This list will be updated later on as needed if changes
     *       in the BlueZ ObjectManager are detected.
     */
    GList * objects;

    /**
     * BlueZ adapter list.
     *
     * List of @c GDBusProxy objects for all BlueZ adapters (i.e.
     * @c org.bluez.Adapter1).  More than one adapter can exist if
     * multiple Bluetooth hardware interfaces are detected by BlueZ.
     */
    GList * adapters;

    /**
     * BlueZ device list.
     *
     * List of @c GDBusProxy objects for all BlueZ devices (i.e.
     * @c org.bluez.Device1), such as those that matched the discovery
     * criteria.
     */
    GList * devices;

    /**
     * Bluetooth MAC address to GATT characteristic map.
     *
     * Hash table that maps Bluetooth MAC address to a OIC Transport
     * Profile GATT characteristic.  The key is a string containing
     * the peer Bluetooth adapter MAC address.   The value is an
     * interface proxy (@c GDBusProxy) to an
     * @c org.bluez.GattCharacteristic1 object.
     *
     * On the client side, this maps a Bluetooth peripheral MAC
     * address to the corresponding request characteristic proxy.  On
     * the server side, this maps Bluetooth central MAC address to the
     * corresponding response characteristic proxy.
     *
     * @note On the server side a map is overkill since only one
     *       client is ever connected to the server.  No?
     *
     * @todo We may want to have a seperate server-side map to reduce
     *       contention on this map.
     */
    GHashTable * characteristic_map;

    /**
     * GATT characteristics to Bluetooth MAC address map.
     *
     * Hash table that maps OIC Transport Profile GATT characteristic
     * to a Bluetooth MAC address.  The key is an interface proxy
     * (@c GDBusProxy) to an @c org.bluez.GattCharacteristic1 object.
     * The value is a pointer to the peer @c CAEndpoint_t object.
     *
     * On the client side, this maps a response characteristic to the
     * corresponding MAC address.  On the server side, this maps
     * request characteristic to the corresponding MAC address.
     *
     * @note On the server side a map is overkill since only one
     *       client is ever connected to the server.  No?
     *
     * @todo We may want to have a seperate server-side map to reduce
     *       contention on this map.
     */
    GHashTable * address_map;

    /**
     * D-Bus signal subscription identifiers.
     *
     * The Linux BLE transport implementation subscribes to three
     * D-Bus signals:
     *
     * @li @c org.freedesktop.DBus.ObjectManager.InterfacesAdded
     * @li @c org.freedesktop.DBus.ObjectManager.InterfacesRemoved
     * @li @c org.freedesktop.DBus.Properties.PropertiesChanged
     * @li @c org.bluez.Adapter1.PropertyChanged
     *
     * These subscription identifiers are only used when unsubscribing
     * from the signals when stopping the LE transport.
     *
     * @todo Verify if we need the two property related signals at
     *       this level.
     */
    //@{
    guint interfaces_added_sub_id;
    guint interfaces_removed_sub_id;
    guint properties_changed_sub_id;
    guint property_changed_sub_id;
    //@}

    /// Glib event loop that drives D-Bus signal handling.
    GMainLoop * event_loop;

    /**
     * Callback invoked upon change in local Bluetooth adapter state.
     */
    CALEDeviceStateChangedCallback on_device_state_changed;

    /// Callback invoked upon server receiving request data.
    CABLEDataReceivedCallback on_server_received_data;

    /// Callback invoked upon client receiving response data.
    CABLEDataReceivedCallback on_client_received_data;

    /**
     * Handle to thread pool to which client side tasks will be
     * added.
     */
    ca_thread_pool_t client_thread_pool;

    /**
     * Handle to thread pool to which server side tasks will be
     * added.
     */
    ca_thread_pool_t server_thread_pool;

    /// Callback invoked when reporting a client side error.
    CABLEErrorHandleCallback on_client_error;

    /// Callback invoked when reporting a server side error.
    CABLEErrorHandleCallback on_server_error;

    /// Mutex used to synchronize access to context fields.
    ca_mutex lock;

    /**
     * BlueZ adapter list initialization condition variable.
     *
     * This condition variable is used to prevent the BLE adapter
     * "start" from completing until the thread performing BlueZ
     * adapter query completes.  Initialization is performed in the
     * same thread that will run the event loop.  The condition
     * variable is also used to wait for peripheral devices to be
     * discovered.
     *
     * @see @c GMainLoop documentation for further details.
     */
    ca_cond condition;

} CALEContext;


#endif  /* CA_BLE_LINUX_CONTEXT_H */
