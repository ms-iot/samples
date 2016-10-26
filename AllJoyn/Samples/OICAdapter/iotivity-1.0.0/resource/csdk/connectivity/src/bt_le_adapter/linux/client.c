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

#include "client.h"
#include "recv.h"
#include "context.h"
#include "bluez.h"
#include "utils.h"

#include "cafragmentation.h"
#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"

#include <gio/gio.h>

#include <string.h>
#include <assert.h>


// Logging tag.
static char const TAG[] = "BLE_CLIENT";

// ---------------------------------------------------------------------
//                        GATT Client Set-up
// ---------------------------------------------------------------------
static bool CAGattClientServiceFilter(GDBusProxy * service)
{
    /*
      On the client side, we only care about the GATT services on
      remote devices.  Ignore the locally created ones by checking for
      the existence of the org.bluez.GattService1.Device property in
      the service proxy.  GATT services on remote devices will have
      that property.
    */
    GVariant * const remote_device =
        g_dbus_proxy_get_cached_property(service, "Device");

    if (remote_device == NULL)
    {
        return false;
    }

    /*
      org.bluez.GattService1.Device property exists, meaning the
      GATT service was advertised from a remote object.
    */
    g_object_unref(remote_device);
    return true;
}

bool CAGattClientsInitialize(CALEContext * context)
{
    /*
      Create a proxies to the org.bluez.GattService1 D-Bus objects that
      will later be used to send requests and receive responses on the
      client side.
    */
    GList * services = NULL;
    bool success =
        CAGetBlueZManagedObjectProxies(&services,
                                       BLUEZ_GATT_SERVICE_INTERFACE,
                                       context,
                                       CAGattClientServiceFilter);

    /**
     * @todo Is this really an error?
     */
    if (!success)
    {
        return success;
    }

    /*
      Map Bluetooth MAC address to OIC Transport Profile
      characteristics.
    */
    GHashTable * const characteristic_map =
        g_hash_table_new_full(g_str_hash,
                              g_str_equal,
                              OICFree,
                              g_object_unref);

    char const * const address = NULL;  // OICMalloc(...);
    GDBusProxy * const client = NULL;

#if GLIB_CHECK_VERSION(2,40,0)
    /*
      GLib hash table functions started returning a boolean result in
       version 2.40.x.
    */
    success =
#endif
        g_hash_table_insert(characteristic_map,
                            OICStrdup(address),
                            client);

    // An empty services list is NULL.
    if (success && services != NULL)
    {
        ca_mutex_lock(context->lock);
        context->characteristic_map = characteristic_map;
        ca_mutex_unlock(context->lock);
    }

    return success;
}

bool CAGattClientsDestroy(CALEContext * context)
{
    (void)context;
    /* g_hash_table_destroy(...); */   // FIXME
    return false;
}

// ---------------------------------------------------------------------
//                        GATT Request Send
// ---------------------------------------------------------------------
/**
 * Send data to the GATT server through the given request
 * @a characteristic proxy.
 *
 * @param[in] characteristic The D-Bus proxy of the request
 *                           characteristic through which the
 *                           @c WriteValue() method will be invoked.
 * @param[in] data           The byte array to be sent.
 * @param[in] length         The number of elements in the byte
 *                           array.
 */
static bool CAGattClientSendRequestData(GDBusProxy * characteristic,
                                        CALEContext * context,
                                        uint8_t const * data,
                                        size_t length)
{
    assert(context != NULL);

    GVariant * const value =
        g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
                                  data,
                                  length,
                                  1);  // sizeof(data[0]) == 1

    GError * error = NULL;

    GVariant * const ret =
        g_dbus_proxy_call_sync(characteristic,
                               "WriteValue",
                               value,  // parameters
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,    // timeout (default == -1),
                               NULL,  // cancellable
                               &error);

    if (ret == NULL)
    {
        OIC_LOG_V(ERROR,
                  TAG,
                  "[%p] WriteValue() call failed: %s",
                  characteristic,
                  error->message);

        g_error_free(error);

        ca_mutex_lock(context->lock);

        if (context->on_client_error != NULL)
        {
            /*
              At this point endpoint and send data information is
              available.
            */
            context->on_client_error(NULL,   // endpoint
                                     data,
                                     length,
                                     CA_STATUS_FAILED);
        }

        ca_mutex_unlock(context->lock);

        return false;
    }

    g_variant_unref(ret);

    return true;
}

CAResult_t CAGattClientSendData(void const * method_info,
                                uint8_t const * data,
                                size_t length)
{
    assert(method_info != NULL);

    CAGattRequestInfo const * const info = method_info;

    GDBusProxy * const characteristic =
        G_DBUS_PROXY(info->characteristic_info);

    return CAGattClientSendRequestData(characteristic,
                                       info->context,
                                       data,
                                       length);
}

CAResult_t CAGattClientSendDataToAll(void const * method_info,
                                     uint8_t const * data,
                                     size_t length)
{
    assert(method_info != NULL);

    CAResult_t result = CA_STATUS_OK;

    CAGattRequestInfo const * const info = method_info;

    for (GList const * l = info->characteristic_info;
         l != NULL && result == CA_STATUS_OK;
         l = l->next)
    {
        GDBusProxy * const characteristic = G_DBUS_PROXY(l->data);

        result = CAGattClientSendRequestData(characteristic,
                                             info->context,
                                             data,
                                             length);
    }

    return result;
}

// ---------------------------------------------------------------------
//                      GATT Response Receive
// ---------------------------------------------------------------------
void CAGattReceiveResponse(GDBusConnection * connection,
                           char const * sender_name,
                           char const * object_path,
                           char const * interface_name,
                           char const * signal_name,
                           GVariant   * parameters,
                           gpointer     user_data)
{
    (void)connection;
    (void)sender_name;
    (void)object_path;
    (void)interface_name;
    (void)signal_name;
    /*
      This handler is only trigged in a GATT client when receiving
      data sent by a GATT server through a notification, e.g. such as
      when a GATT server sent a response.
    */
    gsize fragment_len = 0;
    gconstpointer const fragment =
        g_variant_get_fixed_array(parameters,
                                  &fragment_len,
                                  1);  // sizeof(guchar) == 1

    CAGattRecvInfo * const info = user_data;

    if (CAGattRecv(info, fragment, fragment_len))
    {
    }
    else
    {
    }
}
