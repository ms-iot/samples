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

#include "service.h"
#include "gatt_dbus.h"
#include "utils.h"
#include "bluez.h"

#include "cagattservice.h"
#include "logger.h"

#include <assert.h>


// Logging tag.
static char const TAG[] = "BLE_SERVICE";

static GVariant * CAGattServiceGetProperties(GattService1 * service)
{
    /*
      Create a variant containing the @c GattService1 properties, of
      the form @c a{sa{sv}}.
    */

    /**
     * Populate the property table, and create the variant to be
     * embedded in the results of the
     * @c org.freedesktop.Dbus.ObjectManager.GetManagedObjects()
     * method call.
     *
     * The @c "Device" property is only available on the client side
     * so we don't bother returning it here.
     *
     * @todo Do we care about the @c "Includes" property?
     *       @c "Includes" isn't implemented by BlueZ as of version
     *       5.30, so we can leave it out.
    */
    CADBusSkeletonProperty const properties[] = {
        { "UUID",
          g_variant_new_string(gatt_service1_get_uuid(service)) },
        { "Primary",
          g_variant_new_boolean(gatt_service1_get_primary(service)) },
        { "Characteristics",
          g_variant_new_objv(
              gatt_service1_get_characteristics(service),
              -1) }
    };

    return
        CAMakePropertyDictionary(
            BLUEZ_GATT_SERVICE_INTERFACE,
            properties,
            sizeof(properties) / sizeof(properties[0]));
}

/**
 * Implementation of the
 * @c org.freedesktop.DBus.ObjectManager.GetManagedObjects() method
 * for the @c org.bluez.GattService1 interface.
 */
static gboolean CAGattServiceHandleGetManagedObjects(
    ObjectManager * object,
    GDBusMethodInvocation * invocation,
    gpointer user_data)
{
    /**
     * @note Ideally we shouldn't need this implementation, and should
     *       be able to simply rely GDBusObjectManagerServer instead.
     *       Unfortunately, BlueZ expects the @c
     *       org.bluez.GattService1 object to implement the @c
     *       ObjectManager interface, and both interfaces must rooted
     *       at the same object path, as well.  That requirement
     *       prevents us from using @c GDBusObjectManagerServer since
     *       it won't allow us to export more than interface on a
     *       given object path.
     */

    /*
      Build the object array containing the IoTivity
      org.bluez.GattService1 hierarchy.

      a{oa{sa{sv}}}
    */
    CAGattService * const service = user_data;

    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{oa{sa{sv}}}"));

    // Start out with the service itself.
    g_variant_builder_add(&builder,
                          "{o@a{sa{sv}}}",
                          service->object_path,
                          CAGattServiceGetProperties(service->service));

    /*
      Add the request characteristic and user description
      descriptor.
    */
    CAGattCharacteristic * const request_chrc =
        &service->request_characteristic;

    g_variant_builder_add(&builder,
                          "{o@a{sa{sv}}}",
                          request_chrc->object_path,
                          CAGattCharacteristicGetProperties(
                              request_chrc->characteristic));

    CAGattDescriptor * const request_desc = &request_chrc->descriptor;

    g_variant_builder_add(&builder,
                          "{o@a{sa{sv}}}",
                          request_desc->object_path,
                          CAGattDescriptorGetProperties(
                              request_desc->descriptor));

    /*
      Add the response characteristic and user description
      descriptor.
    */
    CAGattCharacteristic * const response_chrc =
        &service->response_characteristic;

    g_variant_builder_add(&builder,
                          "{o@a{sa{sv}}}",
                          response_chrc->object_path,
                          CAGattCharacteristicGetProperties(
                              response_chrc->characteristic));

    CAGattDescriptor * const response_desc = &response_chrc->descriptor;

    g_variant_builder_add(&builder,
                          "{o@a{sa{sv}}}",
                          response_desc->object_path,
                          CAGattDescriptorGetProperties(
                              response_desc->descriptor));

    GVariant * const objects = g_variant_builder_end(&builder);

    object_manager_complete_get_managed_objects(object,
                                                invocation,
                                                objects);

    return TRUE;
}

bool CAGattServiceInitialize(CAGattService * s,
                             CALEContext * context,
                             char const * hci_name)
{
    assert(s != NULL);
    assert(context != NULL);
    assert(hci_name != NULL);

    // Path of the form /org/iotivity/gatt/hci0/service0.
    s->object_path =
        g_strdup_printf("%s/%s/%s",
                        CA_GATT_SERVICE_ROOT_PATH,
                        hci_name,
                        CA_GATT_SERVICE_PATH);

    assert(g_variant_is_object_path(s->object_path));

    s->object_manager = object_manager_skeleton_new();
    s->service = gatt_service1_skeleton_new();

    gatt_service1_set_uuid(s->service, CA_GATT_SERVICE_UUID);
    gatt_service1_set_primary(s->service, TRUE);

    if (!CAGattRequestCharacteristicInitialize(s, context)
        || !CAGattResponseCharacteristicInitialize(s, context))
    {
        CAGattServiceDestroy(s);
        return false;
    }

    /*
      The characteristic object paths are not fixed at compile-time.
      Retrieve the object paths that were set at run-time.
    */
    char const * characteristic_paths[] = {
        s->request_characteristic.object_path,
        s->response_characteristic.object_path,
        NULL
    };

    gatt_service1_set_characteristics(s->service, characteristic_paths);

    /*
      Set the org.freedesktop.DBus.ObjectManager.GetManagedObjects()
      handler for our BlueZ GATT service.
    */
    g_signal_connect(
        s->object_manager,
        "handle-get-managed-objects",
        G_CALLBACK(CAGattServiceHandleGetManagedObjects),
        s);

    /*
      BlueZ expects both the org.freedesktop.DBus.ObjectManager and
      org.bluez.GattService1 interfaces to be rooted at the same
      object path.  Export the service and object manager interface
      skeletons with the same object path.
     */
    GError * error = NULL;
    if (!g_dbus_interface_skeleton_export(
            G_DBUS_INTERFACE_SKELETON(s->object_manager),
            context->connection,
            s->object_path,
            &error)
        || !g_dbus_interface_skeleton_export(
            G_DBUS_INTERFACE_SKELETON(s->service),
            context->connection,
            s->object_path,
            &error))
    {
        OIC_LOG_V(ERROR,
                  TAG,
                  "Unable to export GATT service interfaces: %s",
                  error->message);

        return false;
    }

    return true;
}

void CAGattServiceDestroy(CAGattService * s)
{
    /**
     * @todo If necessary, emit the
     *       @c org.freedesktop.DBus.ObjectManager.InterfacesRemoved
     *       signal via @c object_manager_emit_interfaces_removed() if
     *       the CA GATT service objects were removed from the
     *       @c ObjectManager.
     */

    assert(s != NULL);  // As designed, s is always non-NULL.

    g_clear_object(&s->gatt_manager);

    CAGattCharacteristicDestroy(&s->response_characteristic);
    CAGattCharacteristicDestroy(&s->request_characteristic);

    g_clear_object(&s->service);
    g_clear_object(&s->object_manager);

    g_free(s->object_path);
    s->object_path = NULL;
}
