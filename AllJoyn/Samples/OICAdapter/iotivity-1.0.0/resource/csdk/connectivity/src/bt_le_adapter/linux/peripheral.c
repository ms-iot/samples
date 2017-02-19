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

#include "peripheral.h"
#include "utils.h"
#include "bluez.h"
#include "service.h"
#include "characteristic.h"
#include "descriptor.h"

#include "oic_malloc.h"
#include "logger.h"

#include <string.h>
#include <assert.h>


#define MICROSECS_PER_SEC 1000000

// Logging tag.
static char const TAG[] = "BLE_PERIPHERAL";

static CAPeripheralContext g_context = {
    .lock = NULL
};

static bool CAPeripheralCheckStarted()
{
    ca_mutex_lock(g_context.lock);

    bool const started = (g_context.base != NULL);

    ca_mutex_unlock(g_context.lock);

    /**
     * @todo Fix potential TOCTOU race condition.  A peripheral could
     *       have been started or stopped between the mutex unlock and
     *       boolean check.
     */
    return started;
}


static bool CAPeripheralAdaptersFound(CALEContext * context)
{
    // Check if BlueZ detected bluetooth hardware adapters.
    ca_mutex_lock(context->lock);

    bool const found = (context->adapters != NULL);

    ca_mutex_unlock(context->lock);

    if (!found)
    {
        OIC_LOG(WARNING, TAG, "No bluetooth hardware found.");
    }

    return found;
}

static void CAPeripheralDestroyGattServices(gpointer data)
{
    CAGattService * const service = data;

    CAGattServiceDestroy(service);

    OICFree(service);
}

static GList * CAPeripheralInitializeGattServices(CALEContext * context)
{
    GList * result = NULL;

    /*
      Create a proxies to the org.bluez.GattManager1 D-Bus objects that
      will later be used to register the OIC GATT service.
    */
    GList * gatt_managers = NULL;
    if (!CAGetBlueZManagedObjectProxies(&gatt_managers,
                                        BLUEZ_GATT_MANAGER_INTERFACE,
                                        context,
                                        NULL))
        return result;

    GList * gatt_services = NULL;

    for (GList * l = gatt_managers; l != NULL; )
    {
        CAGattService * const service = OICCalloc(1, sizeof(*service));

        if (service == NULL)
        {
            g_list_free_full(gatt_services,
                             CAPeripheralDestroyGattServices);
            g_list_free_full(gatt_managers, g_object_unref);
            return result;
        }

        /*
          Use the HCI device name (e.g. hci0) in GattManager1 object
          path (e.g. /org/bluez/hci0) as a means to differentiate the
          GATT service hierarchy for each GattManager1 object.
        */
        GDBusProxy * const manager = G_DBUS_PROXY(l->data);
        char const * const path = g_dbus_proxy_get_object_path(manager);

        /*
          The return value will actually be a pointer to a substring
          starting with the '/' before the HCI name.  We add 1 later
          on to skip that character.
        */
        char const * const hci_name = strrchr(path, '/');

        if (hci_name == NULL
            || !CAGattServiceInitialize(service, context, hci_name + 1))
        {
            g_list_free_full(gatt_services,
                             CAPeripheralDestroyGattServices);
            g_list_free_full(gatt_managers, g_object_unref);
            return result;
        }

        service->gatt_manager = manager;

        /*
          The GattManager1 proxies are now owned by the CAGattService
          objects.
        */
        GList * const tmp = l;
        l = l->next;
        gatt_managers = g_list_delete_link(gatt_managers, tmp);

        // Prepend for efficiency.
        gatt_services = g_list_prepend(gatt_services, service);
    }

    result = gatt_services;

    return result;
}

static bool CAPeripheralRegisterGattServices(
    CAPeripheralContext * context)
{
    assert(context != NULL);

    bool success = false;

    ca_mutex_lock(context->lock);

    for (GList * l = context->gatt_services; l != NULL; l = l->next)
    {
        CAGattService * const service = l->data;

        // Register the OIC service with the corresponding BlueZ Gatt
        // Manager.

        /*
          org.bluez.GattManager1.RegisterService() accepts two
          parameters: the service object path, and an options
          dictionary.  No options are used so pass a NULL pointer to
          reflect an empty dictionary.
        */
        GVariant * const parameters =
            g_variant_new("(oa{sv})", service->object_path, NULL);

        GError * error = NULL;

        GVariant * const ret =
            g_dbus_proxy_call_sync(
                service->gatt_manager,
                "RegisterService",
                parameters,
                G_DBUS_CALL_FLAGS_NONE,
                -1,    // timeout (default == -1),
                NULL,  // cancellable
                &error);

        if (ret == NULL)
        {
            OIC_LOG_V(ERROR,
                      TAG,
                      "GATT service registration failed: %s",
                      error->message);

            g_error_free(error);

            success = false;

            break;
        }

        g_variant_unref(ret);
    }

    ca_mutex_unlock(context->lock);

    success = true;

    return success;
}

static bool CAPeripheralRegisterAdvertisements(
    CAPeripheralContext * context)
{
    bool success = false;

    /*
      Register the OIC LE advertisement service with each BlueZ
      LE Advertisement Manager.
    */

    ca_mutex_lock(context->lock);

    char const * const advertisement_path =
        g_dbus_interface_skeleton_get_object_path(
            G_DBUS_INTERFACE_SKELETON(
                context->advertisement.advertisement));

    GList * managers = context->advertisement.managers;

    for (GList * l = managers; l != NULL; )
    {
        GDBusProxy * const manager = G_DBUS_PROXY(l->data);

        /**
         * @c org.bluez.LEAdvertisingManager1.RegisterService()
         * accepts two parameters: the advertisement object path, and
         * an options dictionary.  No options are used so pass a NULL
         * pointer to reflect an empty dictionary.
         *
         *  @todo The parameters don't change between loop iterations.
         *        Ideally we should initialize this variable before
         *        the loop is executed, but g_dbus_proxy_call_sync()
         *        takes ownership of the variant.  Is there anyway to
         *        create a non-floating GVariant and pass it to that
         *        function?
         */
        GVariant * const parameters =
            g_variant_new("(oa{sv})", advertisement_path, NULL);

        GError * error = NULL;

        GVariant * const ret =
            g_dbus_proxy_call_sync(
                manager,
                "RegisterAdvertisement",
                parameters,
                G_DBUS_CALL_FLAGS_NONE,
                -1,    // timeout (default == -1),
                NULL,  // cancellable
                &error);

        if (ret == NULL)
        {
            OIC_LOG_V(WARNING,
                      TAG,
                      "LE advertisement registration on %s failed: %s",
                      g_dbus_proxy_get_object_path(manager),
                      error->message);

            g_error_free(error);

            // We can't use the LE advertising manager.  Drop it.
            g_object_unref(manager);

            GList * const tmp = l;
            l = l->next;
            managers = g_list_delete_link(managers, tmp);

            continue;
        }

        g_variant_unref(ret);

        /*
          Note that we can do this in the for-statement because of
          the similar code in the error case above.
        */
        l = l->next;
    }

    // Use the updated list of managers.
    context->advertisement.managers = managers;

    if (managers == NULL)   // Empty
    {
        OIC_LOG(ERROR,
                TAG,
                "LE advertisment registration failed for all "
                "Bluetooth adapters.");
    }
    else
    {

        success = true;
    }

    ca_mutex_unlock(context->lock);

    return success;
}

static void CAPeripheralSetDiscoverable(gpointer data,
                                        gpointer user_data,
                                        gboolean discoverable)
{
    assert(data != NULL);
    assert(user_data != NULL);

    GDBusProxy * const adapter = G_DBUS_PROXY(data);
    CAResult_t * const result = (CAResult_t *) user_data;
    *result = CA_STATUS_FAILED;

    /*
      Make sure the adapter is powered on before making it
      discoverable.
    */
    if (!CASetBlueZObjectProperty(adapter,
                                  BLUEZ_ADAPTER_INTERFACE,
                                  "Powered",
                                  g_variant_new_boolean(discoverable)))
    {
        OIC_LOG_V(ERROR,
                  TAG,
                  "Unable to power %s LE peripheral adapter.",
                  discoverable ? "on" : "off");

        return;
    }

    /**
     * @note Enabling LE advertising used to be done here using the
     *       kernel bluetooth management API.  However, we now
     *       leverage the BlueZ LE Advertisment D-Bus API instead
     *       since it handles all of the desired advertising
     *       operations without need of the calling process to have
     *       @c CAP_NET_ADMIN capabilities.  Advertisment registration
     *       is performed in this source file.
     */

    *result = CA_STATUS_OK;
}

static void CAPeripheralMakeDiscoverable(gpointer adapter,
                                         gpointer result)
{
    CAPeripheralSetDiscoverable(adapter, result, TRUE);
}

static void CAPeripheralMakeUndiscoverable(gpointer adapter,
                                           gpointer result)
{
    CAPeripheralSetDiscoverable(adapter, result, FALSE);
}

static CAResult_t CAPeripheralSetDiscoverability(
    CALEContext * context,
    GFunc discoverability_func)
{
    CAResult_t result = CA_STATUS_FAILED;

    /*
      Synchronize access to the adapter information using the base
      context lock since we don't own the adapter_infos.
     */
    ca_mutex_lock(context->lock);

    // Make all detected adapters discoverable.
    g_list_foreach(context->adapters,
                   discoverability_func,
                   &result);

    ca_mutex_unlock(context->lock);

    return result;
}

/**
 * Callback function invoked when a D-Bus bus name is acquired.
 *
 * @param[in] connection The D-Bus connection on which the name was
 *                       acquired.
 * @param[in] name       The bus name that was acquired.
 * @param[in] user_data  User-provided data.
 */
static void CAPeripheralOnNameAcquired(GDBusConnection * connection,
                                       gchar const * name,
                                       gpointer user_data)
{
    (void)connection;
    (void)name; // needed when logging is a noop
    (void)user_data;
    OIC_LOG_V(DEBUG,
              TAG,
              "Name \"%s\" acquired on D-Bus.", name);
}

/**
 * Callback function invoked when a D-Bus bus name is no longer owned
 * or the D-Bus connection has been closed.
 *
 * @param[in] connection The D-Bus connection on which the bus name
 *                       should have been acquired.
 * @param[in] name       The bus name that was not acquired.
 * @param[in] user_data  User-provided data.
 */
static void CAPeripheralOnNameLost(GDBusConnection * connection,
                                   gchar const * name,
                                   gpointer user_data)
{
    (void)connection;
    (void)name; // needed when logging is a noop
    (void)user_data;
    /*
      This can happen if the appropriate D-Bus policy is not
      installed, for example.
    */
    OIC_LOG_V(WARNING,
              TAG,
              "Lost name \"%s\" on D-Bus!", name);
}

static void CAPeripheralStartEventLoop(void * data)
{
    CALEContext * const context = data;

    assert(context != NULL);

    // Create the event loop.
    GMainContext * const loop_context = g_main_context_new();
    GMainLoop * const event_loop = g_main_loop_new(loop_context, FALSE);

    g_main_context_push_thread_default(loop_context);

    // Acquire the bus name after exporting our D-Bus objects.
    guint const owner_id =
        g_bus_own_name_on_connection(context->connection,
                                     CA_DBUS_GATT_SERVICE_NAME,
                                     G_BUS_NAME_OWNER_FLAGS_NONE,
                                     CAPeripheralOnNameAcquired,
                                     CAPeripheralOnNameLost,
                                     NULL, // user_data,
                                     NULL);

    /**
     * Create proxies to the @c org.bluez.LEAdvertisingManager1 D-Bus
     * objects that will later be used to register the OIC LE
     * advertisement data.
     *
     * @todo Failure to retrieve the LE advertising managers is
     *       currently ignored.  We should propagate the failure to
     *       the thread that spawned this one.
     *
     * @note Retrieval of the @c org.bluez.LEAdvertisingManager1
     *       proxies must be done in a thread separate from the one
     *       that makes calls through those proxies since the
     *       underlying GDBusObjectManagerClient sets up signal
     *       subscriptions that are used when dispatching D-Bus method
     *       handling calls (e.g. property retrieval, etc).
     *       Otherwise, a distributed deadlock situation could occur
     *       if a synchronous D-Bus proxy call is made that causes the
     *       recipient (like BlueZ) to call back in to the thread that
     *       handles signals.  For example, registration of our LE
     *       advertisment with BlueZ causes BlueZ itself to make a
     *       call to our own @c org.bluez.LEAdvertisement1 object.
     *       However, the thread that initiated the advertisement
     *       registration is blocked waiting for BlueZ to respond, but
     *       BlueZ is blocked waiting for that same thread to respond
     *       to its own advertisement property retrieval call.
     */
    GList * advertising_managers = NULL;
    if (!CAGetBlueZManagedObjectProxies(
            &advertising_managers,
            BLUEZ_ADVERTISING_MANAGER_INTERFACE,
            context,
            NULL))
    {
        OIC_LOG(ERROR,
                TAG,
                "Failed to retrieve BlueZ LE advertising "
                "manager interface.");
    }

    /**
     * Initialize all GATT services.
     *
     * @todo Failure to initialize the OIC GATT services is currently
     *       ignored.  We should propagate the failure to the thread
     *       that spawned this one.
     *
     * @note See the @c org.bluez.LEAdvertisingManager1 note above to
     *       understand why the GATT services must be initialized in
     *       a thread seperate from the one that initiates GATT
     *       service registration.
     */
    GList * const gatt_services =
        CAPeripheralInitializeGattServices(context);

    ca_mutex_lock(g_context.lock);

    assert(g_context.event_loop == NULL);
    g_context.event_loop = event_loop;

    g_context.base = context;

    g_context.owner_id = owner_id;

    CALEAdvertisementInitialize(&g_context.advertisement,
                                context->connection,
                                advertising_managers);

    g_context.gatt_services = gatt_services;

    ca_mutex_unlock(g_context.lock);

    ca_cond_signal(g_context.condition);

    g_main_loop_run(event_loop);
}

static void CAPeripheralStopEventLoop(CAPeripheralContext * context)
{
    ca_mutex_lock(context->lock);

    GMainLoop * const event_loop = context->event_loop;
    context->event_loop = NULL;

    ca_mutex_unlock(context->lock);

    if (event_loop != NULL)
    {
        g_main_loop_quit(event_loop);

        GMainContext * const loop_context =
            g_main_loop_get_context(event_loop);

        if (loop_context != NULL)
        {
            g_main_context_wakeup(loop_context);
            g_main_context_unref(loop_context);
        }

        g_main_loop_unref(event_loop);
    }
}

// ------------------------------------------------------

void CAPeripheralInitialize()
{
    g_context.lock      = ca_mutex_new();
    g_context.condition = ca_cond_new();
}

void CAPeripheralFinalize()
{
    ca_cond_free(g_context.condition);
    ca_mutex_free(g_context.lock);
}

CAResult_t CAPeripheralStart(CALEContext * context)
{
    /**
     * @todo Bluetooth adapters that are hot-plugged after the
     *       peripheral has started will not be started!
     */

    CAResult_t result = CA_STATUS_FAILED;

    // Only start if we were previously stopped.
    if (CAPeripheralCheckStarted())
    {
        result = CA_SERVER_STARTED_ALREADY;
        return result;
    }

    if (!CAPeripheralAdaptersFound(context))
    {
        // No Bluetooth adapters.  Don't bother continuing.
        return result;
    }

    /*
      Spawn a thread to run the Glib event loop that will drive D-Bus
      signal handling.
     */
    result = ca_thread_pool_add_task(context->server_thread_pool,
                                     CAPeripheralStartEventLoop,
                                     context);

    if (result != CA_STATUS_OK)
    {
        return result;
    }

    /*
      Wait until initialization completes before proceeding to
      service and advertisement registration.
    */

    // Number of times to wait for initialization to complete.
    static int const max_retries = 2;

    static uint64_t const timeout =
        2 * MICROSECS_PER_SEC;  // Microseconds

    ca_mutex_lock(g_context.lock);

    for (int i = 0;
         g_context.gatt_services == NULL && i < max_retries;
         ++i)
    {
        if (ca_cond_wait_for(g_context.condition,
                             g_context.lock,
                             timeout) == 0)
        {
            result = CA_STATUS_OK;
        }
    }

    ca_mutex_unlock(g_context.lock);

    if (result == CA_STATUS_FAILED)
    {
        return result;
    }

    /**
     * First register the GATT services, then register the LE
     * advertisments with BlueZ to make sure the service we're
     * advertising actually exists.
     */
    if (result == CA_STATUS_OK
        && !(CAPeripheralRegisterGattServices(&g_context)
             && CAPeripheralRegisterAdvertisements(&g_context)))
    {
        result = CA_STATUS_FAILED;
    }

    /*
      Make the local bluetooth adapters discoverable over LE by
      enabling LE, enabling advertising, and making the LE device
      connectable.
    */
    result = CAPeripheralSetDiscoverability(context,
                                            CAPeripheralMakeDiscoverable);

    return result;
}

CAResult_t CAPeripheralStop()
{
    CAResult_t result = CA_STATUS_FAILED;

    // Only stop if we were previously started.
    if (!CAPeripheralCheckStarted())
    {
        result = CA_STATUS_OK;
        return result;
    }

    /*
      Make the local bluetooth adapters undiscoverable.

      This function also sets the base context to NULL.
    */
    result =
        CAPeripheralSetDiscoverability(g_context.base,
                                       CAPeripheralMakeUndiscoverable);

    CAPeripheralStopEventLoop(&g_context);

    ca_mutex_lock(g_context.lock);

    guint const owner_id = g_context.owner_id;
    g_context.owner_id = 0;

    GList * const gatt_services = g_context.gatt_services;
    g_context.gatt_services = NULL;

    g_context.base = NULL;

    ca_mutex_unlock(g_context.lock);

    CALEAdvertisementDestroy(&g_context.advertisement);

    g_list_free_full(gatt_services, CAPeripheralDestroyGattServices);

    g_bus_unown_name(owner_id);

    return result;
}
