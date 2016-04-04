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

#include "descriptor.h"
#include "service.h"
#include "gatt_dbus.h"
#include "utils.h"
#include "bluez.h"

#include "logger.h"
#include "cagattservice.h"

#include <assert.h>


// Logging tag.
static char const TAG[] = "BLE_DESCRIPTOR";

/**
 * Implementation of the @c org.bluez.GattDescriptor1.ReadValue()
 * method.
 *
 * This function is implemented as a GDBus signal handler that returns
 * a byte array containing the @c Value property of the GATT
 * descriptor.
 *
 * @param[in] object     @c org.bluez.GattDescriptoror1 skeleton
 *                       object associated with this method call.
 * @param[in] invocation D-Bus method invocation related object used
 *                       when sending results/errors asynchronously.
 * @param[in] user_data  Unused.
 *
 * @return @c TRUE to indicate that
 *         @c org.bluez.Descriptor.ReadValue()
 *         method is implemented.
 */
static gboolean CAGattDescriptorReadValue(
    GattDescriptor1 * object,
    GDBusMethodInvocation * invocation,
    gpointer user_data)
{
    (void)user_data;
    /**
     * @todo The @c GattDescriptor1 object still owns the returned
     *       variant when using the below call.  Should be we use
     *       @c gatt_descriptor1_dup_value() instead?
     */
    GVariant * const value = gatt_descriptor1_get_value(object);

    gatt_descriptor1_complete_read_value(object, invocation, value);

    return TRUE;
}

/**
 * Initialize GATT descriptor fields.
 *
 * This function initializes the @c CAGattDescriptor object fields.
 *
 * @param[out] c               Information about GATT characteristic
 *                             to which the descriptor belongs.
 * @param[in]  connection      D-Bus connection to the bus on which
 *                             the descriptor will be exported.
 * @param[in]  s               Information about GATT service
 *                             information to which the descriptor
 *                             belongs.
 * @param[in]  descriptor_path @c GattDescriptor1 object path for the
 *                             user description descriptor.
 * @param[in]  value           User description descriptor value,
 *                             e.g. @c "OIC Node Request" or
 *                             @c "OIC Node Response".
 *
 * @note This function does not allocate the @a descriptor object
 *       itself.  The caller is responsible for allocating that
 *       memory.
 *
 * @todo Too many parameters.  Perhaps pass in a pointer to a struct
 *       instead.
 */
static bool CAGattDescriptorInitialize(CAGattCharacteristic * c,
                                       GDBusConnection * connection,
                                       CAGattService * s,
                                       char const * descriptor_path,
                                       char const * value)
{
    (void)s;
    CAGattDescriptor * const d = &c->descriptor;

    // Path of the form /org/iotivity/gatt/hci0/service0/char0/desc0.
    d->object_path =
        g_strdup_printf("%s/%s", c->object_path, descriptor_path);
    assert(g_variant_is_object_path(d->object_path));

    d->descriptor = gatt_descriptor1_skeleton_new();

    gatt_descriptor1_set_uuid(
        d->descriptor,
        CA_GATT_CHRC_USER_DESCRIPTION_DESC_UUID);

    gatt_descriptor1_set_characteristic(d->descriptor,
                                        c->object_path);

    gatt_descriptor1_set_value (d->descriptor,
                                g_variant_new_bytestring(value));

    // Readable, no encryption, no authorization.
    static char const * flags[] = { "read", NULL };
    gatt_descriptor1_set_flags(d->descriptor, flags);

    /*
      Connect the signal handler that implements the
      orb.bluez.GattDescriptor1.ReadValue() method.
    */
    g_signal_connect(d->descriptor,
                     "handle-read-value",
                     G_CALLBACK(CAGattDescriptorReadValue),
                     NULL);

    // Export the descriptor interface on the bus.
    GError * error = NULL;
    if (!g_dbus_interface_skeleton_export(
            G_DBUS_INTERFACE_SKELETON(d->descriptor),
            connection,
            d->object_path,
            &error))
    {
        CAGattDescriptorDestroy(d);

        OIC_LOG_V(ERROR,
                  TAG,
                  "Unable to export D-Bus GATT descriptor "
                  "interface: %s",
                  error->message);

        g_error_free(error);

        return false;
    }

    return true;
}

bool CAGattRequestDescriptorInitialize(struct CAGattService * s,
                                       GDBusConnection * connection)
{
    CAGattCharacteristic * const c = &s->request_characteristic;

    return CAGattDescriptorInitialize(c,
                                      connection,
                                      s,
                                      CA_GATT_REQUEST_USER_DESC_PATH,
                                      CA_GATT_REQUEST_USER_DESCRIPTION);
}

bool CAGattResponseDescriptorInitialize(struct CAGattService * s,
                                        GDBusConnection * connection)
{
    CAGattCharacteristic * const c = &s->response_characteristic;

    return CAGattDescriptorInitialize(c,
                                      connection,
                                      s,
                                      CA_GATT_RESPONSE_USER_DESC_PATH,
                                      CA_GATT_RESPONSE_USER_DESCRIPTION);
}

void CAGattDescriptorDestroy(CAGattDescriptor * d)
{
    assert(d != NULL);  // As designed, d is always non-NULL.

    g_clear_object(&d->descriptor);

    g_free(d->object_path);
    d->object_path = NULL;
}

GVariant * CAGattDescriptorGetProperties(GattDescriptor1 * descriptor)
{
    /**
     * Create a variant containing the @c GattDescriptor1 properties,
     * of the form @c a{sa{sv}}.
     *
     * @note We don't care about the "Value" property here since it is
     *       automatically made available by BlueZ on the client
     *       side.
     */

    /*
      Populate the property table, and create the variant to be
      embedded in the results of the
      org.freedesktop.Dbus.ObjectManager.GetManagedObjects() method
      call.
    */
    CADBusSkeletonProperty const properties[] = {
        { "UUID",
          g_variant_new_string(
              gatt_descriptor1_get_uuid(descriptor)) },
        { "Characteristic",
          g_variant_new_object_path(
              gatt_descriptor1_get_characteristic(descriptor)) },
        { "Flags",
          g_variant_new_strv(
              gatt_descriptor1_get_flags(descriptor),
              -1) }
    };

    return
        CAMakePropertyDictionary(
            BLUEZ_GATT_DESCRIPTOR_INTERFACE,
            properties,
            sizeof(properties) / sizeof(properties[0]));
}
