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

#ifndef CA_BLE_LINUX_BLUEZ_H
#define CA_BLE_LINUX_BLUEZ_H


/// BlueZ D-Bus service name.
#define BLUEZ_NAME "org.bluez"

/// BlueZ D-Bus adapter interface name.
static char const BLUEZ_ADAPTER_INTERFACE[] = BLUEZ_NAME ".Adapter1";

/// BlueZ D-Bus device interface name.
static char const BLUEZ_DEVICE_INTERFACE[] = BLUEZ_NAME ".Device1";

/// BlueZ D-Bus LE advertising manager interface.
static char const BLUEZ_ADVERTISING_MANAGER_INTERFACE[] =
    BLUEZ_NAME ".LEAdvertisingManager1";

/// BlueZ D-Bus GATT manager interface.
static char const BLUEZ_GATT_MANAGER_INTERFACE[] =
    BLUEZ_NAME ".GattManager1";

/// BlueZ D-Bus adapter GATT service interface name.
static char const BLUEZ_GATT_SERVICE_INTERFACE[] =
    BLUEZ_NAME ".GattService1";

/// BlueZ D-Bus adapter GATT characteristic interface name.
static char const BLUEZ_GATT_CHARACTERISTIC_INTERFACE[] =
    BLUEZ_NAME ".GattCharacteristic1";

/// BlueZ D-Bus adapter GATT service interface name.
static char const BLUEZ_GATT_DESCRIPTOR_INTERFACE[] =
    BLUEZ_NAME ".GattDescriptor1";


#endif  // CA_BLE_LINUX_BLUEZ_H
