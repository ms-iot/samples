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

#ifndef CA_BLE_LINUX_UTILS_H
#define CA_BLE_LINUX_UTILS_H

#include "context.h"


/**
 * Proxy retrieval filter function type.
 *
 * A function that implements this interface may be passed to
 * @c CAGetBlueZManagedObjectProxies() to filter proxies to be added
 * to the returned list.
 *
 * @return @c true if the proxy should be added to the proxy list,
 *         @c false otherwise.
 */
typedef bool(*CALEProxyFilter)(GDBusProxy * proxy);

/**
 * Get information for all BlueZ managed objects with a given
 * interface.
 *
 * @param[out] proxies   List containing @a proxies (@c GDBusProxy)
 *                       for all found BlueZ objects that match the
 *                       given D-Bus @a interface.
 * @param[in]  interface D-Bus interface of BlueZ object for which a
 *                       proxy will be created.
 * @param[in]  context   BLE Linux adapter context.
 * @param[in]  filter    Filter function used to determine whether or
 *                       not a proxy retrieved from the BlueZ
 *                       @c ObjectManager should be added to the
 *                       returned @a proxies list.
 *
 * @return @c true if objects were found.
 */
bool CAGetBlueZManagedObjectProxies(GList ** proxies,
                                    char const * interface,
                                    CALEContext * context,
                                    CALEProxyFilter filter);


/**
 * Get the proxy to a object that implements the given D-Bus
 * @a interface.
 *
 * @param[in] tuple      A D-Bus tuple that contains the D-Bus object
 *                       path and corresponding dictionary of
 *                       properties, i.e. with the format specifier
 *                       @c "(oa{sv})".
 * @param[in] interface  The D-Bus interface in which we're
 *                       interested.  A @c GDBusProxy object will be
 *                       create using the properties found in the
 *                       dictionary item whose key matches this
 *                       interface name.
 * @param[in] manager    The D-Bus object manager that contains the
 *                       information about the object in question.
 */
GDBusProxy * CAGetBlueZInterfaceProxy(GVariant * tuple,
                                      char const * interface,
                                      GDBusObjectManager * object_manager);

/**
 * Set @a property on BlueZ object to given @a value.
 *
 * Using the @c org.freedesktop.DBus.Properties.Set() method
 * implemented by BlueZ objects, set the desired @a property on the
 * object pointed to by @a proxy to the given @a value.
 *
 * @param[in] proxy     D-Bus proxy to the BlueZ object.
 * @param[in] interface Interface name of the object on which the
 *                      property is being set.
 * @param[in] property  Property name.
 * @param[in] value     Property value.  Ownership is transferred from
 *                      the caller.
 */
bool CASetBlueZObjectProperty(GDBusProxy * proxy,
                              char const * interface,
                              char const * property,
                              GVariant * value);

/**
 * D-Bus skeleton propery name/value pair.
 *
 * This name value pair will be used when constructing the property
 * dictionary embedded in
 * @c org.freedesktop.DBus.ObjectManager.GetManagedObjects() results.
 */
typedef struct _CADBusSkeletonProperty
{
    /// Property name.
    char const * const name;

    /**
     * Property value.
     *
     * @c Ownership is transferred to the function this variant passed
     * to.
     */
    GVariant * const value;
} CADBusSkeletonProperty;

/**
 * Construct property dictionary suitable for place in ObjectManager
 * results.
 *
 * @param[in] interface_name The name of the interface to which the
 *                           properties correspond.
 * @param[in] properties     Array of property name/value pairs.
 *                           Ownership of the value variant will be
 *                           transferred to this function.
 * @param[in] count          Number of elements in the @a properties
 *                           array.
 *
 * @return A variant of the form a{sa{sv}}, suitable for use in the
 *         results of
 *         @c org.freedesktop.DBus.ObjectManager.GetManagedObjects()
 *         or @c org.freedesktop.DBus.Properties.GetAll() method
 *         implementations.
 *
 * @note Ownership of the returned @c GVariant is transferred to the
 *       caller.
 */
GVariant * CAMakePropertyDictionary(
    char const * interface_name,
    CADBusSkeletonProperty const * properties,
    size_t count);


#endif  // CA_BLE_LINUX_UTILS_H
