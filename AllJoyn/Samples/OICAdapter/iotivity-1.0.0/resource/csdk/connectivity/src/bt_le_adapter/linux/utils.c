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

#include "utils.h"
#include "bluez.h"

#include "logger.h"

#include <assert.h>


// Logging tag.
static char const TAG[] = "BLE_UTILS";

bool CAGetBlueZManagedObjectProxies(GList ** proxies,
                                    char const * interface,
                                    CALEContext * context,
                                    CALEProxyFilter filter)
{
    assert(interface != NULL);
    assert(proxies != NULL);
    assert(context != NULL);

    /*
      Failure is only reported if an error occurred.  An empty
      returned list, for example, is not an error since it could
      be updated at a later time when handling D-Bus signals related
      to the given interface.
    */
    bool success = true;

    ca_mutex_lock(context->lock);

    if (context->objects == NULL)
    {
        ca_mutex_unlock(context->lock);
        return success;
    }

    /*
      Iterate over the objects to find those that implement the given
      BlueZ interface and them to the given list.
    */
    for (GList * l = context->objects; l != NULL; l = l->next)
    {
        GDBusProxy * const proxy =
            G_DBUS_PROXY(g_dbus_object_get_interface(
                G_DBUS_OBJECT(l->data),
                interface));

        if (proxy != NULL)
        {
            if (filter == NULL || filter(proxy))
            {
                /*
                  Add the object information to the list.

                  Note that we prepend instead of append in this case
                  since it is more efficient to do so for linked lists
                  like the one used here.
                */
                *proxies = g_list_prepend(*proxies, proxy);
            }
            else
            {
                // Rejected by filter.
                g_object_unref(proxy);
            }
        }
    }

    ca_mutex_unlock(context->lock);

    return success;
}

GDBusProxy * CAGetBlueZInterfaceProxy(GVariant * tuple,
                                      char const * interface,
                                      GDBusObjectManager * object_manager)
{
    /*
      The tuple is of the form "(oa{sv})".

      The object path is first tuple element, and the properties the
      second.  Check if the given interface exists in the properties.
      Return the object path (the dictionary item key) of the found
      property (the dictionary item value).
    */
    GVariant * const props = g_variant_get_child_value(tuple, 1);

    GVariant * const value =
        g_variant_lookup_value(props, interface, NULL);

    GDBusProxy * proxy = NULL;

    if (value != NULL)
    {
        /*
          A set of properties corresponding to the given D-Bus
          interface was found.  Create a proxy to the object that
          implements that interface.  Store that proxy, the D-Bus
          object path and properties in the given list.
        */

        gchar const * path = NULL;

        // The object path is the first tuple element.
        g_variant_get_child(tuple, 0, "&o", &path);

        /*
          Create a proxy to the object that implements the given
          interface.
        */
        proxy =
            G_DBUS_PROXY(
                g_dbus_object_manager_get_interface(object_manager,
                                                    path,
                                                    interface));

        g_variant_unref(value);
    }

    g_variant_unref(props);

    return proxy;
}

bool CASetBlueZObjectProperty(GDBusProxy * proxy,
                              char const * interface,
                              char const * property,
                              GVariant * value)
{
    /*
      Only make the D-Bus call to set the property if we need to
      change it.
     */
    GVariant * const cached_value =
        g_dbus_proxy_get_cached_property(proxy, property);

    if (cached_value != NULL)
    {
        bool const already_set = g_variant_equal(cached_value, value);

        g_variant_unref(cached_value);

        if (already_set)
        {
            /*
              Ownership of the value GVariant was transferred when
              this function was called.  It won't be used since the
              property with the same value is already set, so release
              our reference to it.
            */
            g_variant_unref(value);

            return true;
        }
    }

    /*
      Either the property wasn't previously set or it is being
      changed.  Set property on the given interface.
    */
    GError * error = NULL;

    GVariant * const ret =
        g_dbus_proxy_call_sync(proxy,
                               "org.freedesktop.DBus.Properties.Set",
                               g_variant_new("(ssv)",
                                             interface,
                                             property,
                                             value),
                               G_DBUS_CALL_FLAGS_NONE,
                               -1,    // timeout (default == -1),
                               NULL,  // cancellable
                               &error);

    if (ret == NULL)
    {
        OIC_LOG_V(ERROR,
                  TAG,
                  "Attempt to set \"%s\" property for "
                  "\"%s\" interface failed.: %s",
                  error->message);

        g_error_free(error);

        return false;
    }

    g_variant_unref(ret);

    return true;
}

GVariant * CAMakePropertyDictionary(
    char const * interface_name,
    CADBusSkeletonProperty const * properties,
    size_t count)
{
    /*
      Create a variant containing the proxy properties, of the form
      a{sa{sv}}.
    */

    GVariantBuilder builder;

    // Create the inner (property) dictionary.
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));

    CADBusSkeletonProperty const * const end =
        properties + count;

    for (CADBusSkeletonProperty const * prop = properties;
         prop != end;
         ++prop)
    {
        g_variant_builder_add(&builder, "{sv}", prop->name, prop->value);
    }

    GVariant * const props = g_variant_builder_end(&builder);

    // Now create the dictionary with the above property dictionary
    // embedded.
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sa{sv}}"));

    g_variant_builder_add(&builder, "{s@a{sv}}", interface_name, props);

    return g_variant_builder_end(&builder);
}
