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

#ifndef CA_BLE_LINUX_GATT_DBUS_H
#define CA_BLE_LINUX_GATT_DBUS_H


/**
 * @name BlueZ GATT Service D-Bus Object Paths
 *
 * The IoTivity BlueZ GATT Service hierarchy is the following:
 *
 *  -> /org/iotivity/gatt/advertisement0
 *
 *  -> /org/iotivity/gatt/<hciX>/service0
 *    |   - OIC GATT Service
 *    |
 *    -> /org/iotivity/gatt/<hciX>/service0/char0
 *    | |   - OIC GATT Request Characteristic Value
 *    | |
 *    | -> /org/iotivity/gatt/<hciX>/service0/char0/desc0
 *    |       - OIC GATT Request User Description Descriptor
 *    |
 *    -> /org/iotivity/gatt/<hciX>/service0/char1
 *      |   - OIC GATT Response Characteristic Value
 *      |
 *      -> /org/iotivity/gatt/<hciX>/service0/char1/desc0
 *            - OIC GATT Response User Description Descriptor
 *
 * where <hciX> corresponds to the bluetooth hardware adapter with
 * which the GATT service is being registered, e.g. "hci0".
 *
 * @note The OIC GATT Client Characterstic Configuration Descriptor is
 *       implicitly added to the response characteristic hierarchy by
 *       BlueZ since its "notify" property is set.
 */
//@{

/**
 * Root object path of the GATT service hierarchy.
 *
 * The GATT service object manager (i.e. implementation of
 * @c org.freedesktop.DBus.ObjectManager) is found at this object
 * path.
 */
#define CA_GATT_SERVICE_ROOT_PATH "/org/iotivity/gatt"

// ------------------------

#define CA_LE_ADVERTISEMENT_PATH "advertisement0"

// ------------------------

/// GATT service object path basename.
#define CA_GATT_SERVICE_PATH  "service0"

// ------------------------

/// Request GATT characteristic object path basename.
#define CA_GATT_REQUEST_CHRC_PATH "char0"

/// Request GATT user description descriptor object path basename.
#define CA_GATT_REQUEST_USER_DESC_PATH "desc0"

// ------------------------

/// Response GATT characteristic object path basename.
#define CA_GATT_RESPONSE_CHRC_PATH "char1"

/// Response GATT user description descriptor object path basename.
#define CA_GATT_RESPONSE_USER_DESC_PATH "desc0"
//@}

#endif // CA_BLE_LINUX_GATT_DBUS_H
