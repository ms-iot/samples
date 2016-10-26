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

#include "central.h"
#include "utils.h"
#include "bluez.h"

#include "cagattservice.h"  // For CA_GATT_SERVICE_UUID.
#include "logger.h"

#include <stdbool.h>
#include <assert.h>


// Logging tag.
static char const TAG[] = "BLE_CENTRAL";

static bool CACentralGetBooleanProperty(GDBusProxy * device,
                                        char const * property)
{
    GVariant * const cached_property =
        g_dbus_proxy_get_cached_property(device, property);

    if (cached_property == NULL)
    {
        return false;
    }

    bool const value = g_variant_get_boolean(cached_property);

    g_variant_unref(cached_property);

    return value;
}

static void CACentralStartDiscoveryImpl(gpointer proxy, gpointer user_data)
{
    assert(proxy != NULL);
    assert(user_data != NULL);

    GDBusProxy * const adapter = G_DBUS_PROXY(proxy);
    CAResult_t * const result  = user_data;

    *result = CA_STATUS_FAILED;

    bool const is_discovering =
        CACentralGetBooleanProperty(adapter, "Discovering");

    if (is_discovering)
    {
        // Nothing to do.  Avoid invoking a method over D-Bus.
        *result = CA_STATUS_OK;
        return;
    }


    // Make sure the adapter is powered on before starting discovery.
    if (!CASetBlueZObjectProperty(adapter,
                                  BLUEZ_ADAPTER_INTERFACE,
                                  "Powered",
                                  g_variant_new_boolean(TRUE)))
    {
        OIC_LOG(ERROR,
                TAG,
                "Unable to power on LE central adapter.");

        return;
    }

    /*
      Only scan for LE peripherals that advertise the OIC Transport
      Profile GATT service UUID by setting a discovery filter on the BlueZ
      org.bluez.Adapter1 object with two parameters:

          (1) "UUIDs": set to an array containing that OIC Transport
                       Profile GATT service UUID
          (2) "Transport": set to "le"

      See the documentation for the SetDiscoveryFilter() method in the
      BlueZ `adapter-api.txt' document for more details.
    */
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

    static char const * const UUIDs[] =
        {
            CA_GATT_SERVICE_UUID
        };

    g_variant_builder_add(&builder,
                          "{sv}",
                          "UUIDs",
                          g_variant_new_strv(
                              UUIDs,
                              sizeof(UUIDs) / sizeof(UUIDs[0])));

    g_variant_builder_add(&builder,
                          "{sv}",
                          "Transport",
                          g_variant_new_string("le"));

    GVariant * const filter = g_variant_builder_end(&builder);

    /*
      SetDiscoveryFilter() expects a dictionary but it must be packed
      into a tuple for the actual call through the proxy.
    */
    GVariant * const filter_parameters =
        g_variant_new("(@a{sv})", filter);

    GError * error = NULL;

    /*
      This is a synchronous call, but the actually discovery is
      performed asynchronously.  org.bluez.Device1 objects will
      reported through the
      org.freedesktop.DBus.ObjectManager.InterfacesAdded signal as
      peripherals that match our discovery filter criteria are found.
     */
    GVariant * ret =
        g_dbus_proxy_call_sync(adapter,
                               "SetDiscoveryFilter",
                               filter_parameters,
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,    // timeout (default == -1),
                               NULL,  // cancellable
                               &error);

    if (ret == NULL)
    {
        OIC_LOG_V(ERROR,
                  TAG,
                  "SetDiscoveryFilter() call failed: %s",
                  error->message);

        g_error_free(error);

        return;
    }

    g_variant_unref(ret);

    // Start device discovery.
    ret = g_dbus_proxy_call_sync(adapter,
                                 "StartDiscovery",
                                 NULL,  // parameters
                                 G_DBUS_CALL_FLAGS_NONE,
                                 -1,    // timeout (default == -1),
                                 NULL,  // cancellable
                                 &error);

    if (ret == NULL)
    {
        OIC_LOG_V(ERROR,
                  TAG,
                  "StartDiscovery() call failed: %s",
                  error->message);

        g_error_free(error);

        return;
    }

    g_variant_unref(ret);

    *result = CA_STATUS_OK;
}

static void CACentralStopDiscoveryImpl(gpointer proxy, gpointer user_data)
{
    assert(proxy != NULL);
    assert(user_data != NULL);

    GDBusProxy * const adapter = G_DBUS_PROXY(proxy);
    CAResult_t * const result  = user_data;

    bool const is_discovering =
        CACentralGetBooleanProperty(adapter, "Discovering");

    if (!is_discovering)
    {
        // Nothing to do.  Avoid invoking a method over D-Bus.
        *result = CA_STATUS_OK;
        return;
    }

    *result = CA_STATUS_FAILED;


    GError * error = NULL;

    // Stop discovery sessions.
    GVariant * const ret =
        g_dbus_proxy_call_sync(adapter,
                               "StopDiscovery",
                               NULL,  // parameters
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,    // timeout (default == -1)
                               NULL,  // cancellable
                               &error);

    if (ret == NULL)
    {
        OIC_LOG_V(ERROR,
                  TAG,
                  "StopDiscovery() call failed: %s",
                  error->message);

        g_error_free(error);

        return;
    }

    g_variant_unref(ret);

    *result = CA_STATUS_OK;
}

static void CACentralConnectToDevice(gpointer data, gpointer user_data)
{
    assert(data != NULL);
    assert(user_data != NULL);

    GDBusProxy * const device = G_DBUS_PROXY(data);
    bool * const connected = user_data;

    if (!CACentralConnect(device))
    {
        *connected = false;
    }
}

/**
 * Disconnect from all LE peripherals.
 *
 * @param[in] context Context containing BlueZ device information.
 */
static void CACentralDisconnect(CALEContext * context)
{
    assert(context != NULL);

    ca_mutex_lock(context->lock);

    for (GList * l = context->devices; l != NULL; l = l->next)
    {
        GDBusProxy * const device = G_DBUS_PROXY(l->data);

        bool const is_connected =
            CACentralGetBooleanProperty(device, "Connected");

        if (is_connected)
        {
            /*
              Asynchronously disconnect.  We don't care about the
              result so don't bother passing in a callback.
            */
            g_dbus_proxy_call(device,
                              "Disconnect",
                              NULL,  // parameters
                              G_DBUS_CALL_FLAGS_NONE,
                              -1,    // timeout (default == -1),
                              NULL,  // cancellable
                              NULL,  // callback
                              NULL); // user data
        }
    }

    ca_mutex_unlock(context->lock);
}

// -----------------------------------------------------------------------

CAResult_t CACentralStart(CALEContext * context)
{
    assert(context != NULL);

    CAResult_t result = CA_STATUS_FAILED;

    /*
      Synchronize access to the adapter information using the base
      context lock since we don't own the adapters.
     */
    ca_mutex_lock(context->lock);

    /**
     * Start discovery on all detected adapters.
     *
     * @todo The current start start_discovery() implementation makes
     *       two synchronous D-Bus calls to each BlueZ @c Adapter1
     *       object in the @a adapters list.  We may want to make them
     *       asynchronous to minimize blocking.
     */
    g_list_foreach(context->adapters,
                   CACentralStartDiscoveryImpl,
                   &result);

    ca_mutex_unlock(context->lock);

    return result;
}

CAResult_t CACentralStop(CALEContext * context)
{
    assert(context != NULL);

    CAResult_t result = CA_STATUS_FAILED;

    // Stop discovery on all detected adapters.
    result = CACentralStopDiscovery(context);

    // Disconnect from all adapters.
    CACentralDisconnect(context);

    /**
     * @todo Stop notifications on all response characteristics.
     */

    return result;
}

CAResult_t CACentralStartDiscovery(CALEContext * context)
{
    assert(context != NULL);

    CAResult_t result = CA_STATUS_FAILED;

    /*
      Synchronize access to the adapter information using the base
      context lock since we don't own the adapters.
     */
    ca_mutex_lock(context->lock);

    // Start discovery on all detected adapters.
    g_list_foreach(context->adapters,
                   CACentralStartDiscoveryImpl,
                   &result);

    ca_mutex_unlock(context->lock);

    return result;
}

CAResult_t CACentralStopDiscovery(CALEContext * context)
{
    assert(context != NULL);

    CAResult_t result = CA_STATUS_FAILED;

    /*
      Synchronize access to the adapter information using the base
      context lock since we don't own the adapters.
     */
    ca_mutex_lock(context->lock);

    // Stop discovery on all detected adapters.
    g_list_foreach(context->adapters,
                   CACentralStopDiscoveryImpl,
                   &result);

    /**
     * @todo Stop notifications on all response characteristics.
     */

    ca_mutex_unlock(context->lock);

    return result;
}

bool CACentralConnect(GDBusProxy * device)
{
    assert(device != NULL);

    /*
      Check if a connection to the LE peripheral was already
      established.  If not, establish a connection.
     */
    bool const is_connected =
        CACentralGetBooleanProperty(device, "Connected");

    if (is_connected)
    {
        return true;
    }

    GError * error = NULL;

    // Connect to the discovered LE peripheral asynchronously.
    GVariant * const ret =
        g_dbus_proxy_call_sync(device,
                               "Connect",
                               NULL,  // parameters
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,    // timeout (default == -1),
                               NULL,  // cancellable
                               &error);

    if (ret == NULL)
    {
        OIC_LOG_V(ERROR,
                  TAG,
                  "%s.Connect() call failed: %s",
                  BLUEZ_DEVICE_INTERFACE,
                  error->message);

        g_error_free(error);

        return false;
    }

    g_variant_unref(ret);

    return true;
}

bool CACentralConnectToAll(CALEContext * context)
{
    bool connected = true;

    ca_mutex_lock(context->lock);

    // Connect to the LE peripherals, if we're not already connected.
    g_list_foreach(context->devices,
                   CACentralConnectToDevice,
                   &connected);

    ca_mutex_unlock(context->lock);

    return connected;
}
