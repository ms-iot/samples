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

#include "advertisement.h"
#include "gatt_dbus.h"

#include "cagattservice.h"
#include "logger.h"

#include <assert.h>


// Logging tag.
static char const TAG[] = "BLE_ADVERTISEMENT";

bool CALEAdvertisementInitialize(CALEAdvertisement * a,
                                 GDBusConnection * connection,
                                 GList * managers)
{
    assert(a != NULL);
    assert(connection != NULL);

    /*
      D-Bus object path for the LEAdvertisement1 object of the form
      /org/iotivity/gatt/advertisement0.

      The same object path is registered with all
      org.bluez.LEAdvertisingManager1 objects since the advertisement
      data is same for all Bluetooth hardware adapters.
    */
    static char const object_path[] =
        CA_GATT_SERVICE_ROOT_PATH "/" CA_LE_ADVERTISEMENT_PATH;

    assert(g_variant_is_object_path(object_path));

    a->advertisement  = leadvertisement1_skeleton_new();

    /*
      Setting the BlueZ advertisement type to "peripheral" causes the
      Bluetooth adapter to go into LE connectable and general
      discoverable modes upon successful registration of the
      advertisement with BlueZ.
     */
    leadvertisement1_set_type_(a->advertisement, "peripheral");

    static char const * service_uuids[] = {
        CA_GATT_SERVICE_UUID,  // Advertise OIC Transport Profile
        NULL
    };

    leadvertisement1_set_service_uuids(a->advertisement, service_uuids);
    leadvertisement1_set_manufacturer_data(a->advertisement, NULL);
    leadvertisement1_set_solicit_uuids(a->advertisement, NULL);
    leadvertisement1_set_service_data(a->advertisement, NULL);
    leadvertisement1_set_include_tx_power(a->advertisement, FALSE);

    a->managers = managers;

    // Export the LEAdvertisement1 interface skeleton.
    GError * error = NULL;
    if (!g_dbus_interface_skeleton_export(
            G_DBUS_INTERFACE_SKELETON(a->advertisement),
            connection,
            object_path,
            &error))
    {
        OIC_LOG_V(ERROR,
                  TAG,
                  "Unable to export LE advertisement interface: %s\n",
                  error->message);

        return false;
    }

    return true;
}

void CALEAdvertisementDestroy(CALEAdvertisement * a)
{
    if (a->advertisement != NULL)
    {
        char const * const advertisement_path =
            g_dbus_interface_skeleton_get_object_path(
                G_DBUS_INTERFACE_SKELETON(
                    a->advertisement));

        if (advertisement_path != NULL)
        {
            for (GList * l = a->managers; l != NULL; l = l->next)
            {
                GDBusProxy * const manager = G_DBUS_PROXY(l->data);

                GVariant * const parameters =
                    g_variant_new("(o)", advertisement_path, NULL);

                /*
                  Unregister our LE advertisement from the BlueZ LE
                  advertising manager.
                */
                g_dbus_proxy_call(
                    manager,
                    "UnregisterAdvertisement",
                    parameters,
                    G_DBUS_CALL_FLAGS_NONE,
                    -1,    // timeout (default == -1),
                    NULL,  // cancellable
                    NULL,  // callback
                    NULL);
            }
        }

        g_clear_object(&a->advertisement);
    }

    g_list_free_full(a->managers, g_object_unref);
    a->managers = NULL;
}
