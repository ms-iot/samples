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

#ifndef CA_GATT_SERVICE_H
#define CA_GATT_SERVICE_H


/**
 * @name OIC GATT Transport Constants
 *
 * Group of constants, such as UUIDs, specific to the OIC GATT
 * Transport Profile.
 */
//@{
/// OIC Transport Profile GATT service UUID.
#define CA_GATT_SERVICE_UUID "ADE3D529-C784-4F63-A987-EB69F70EE816"

/// OIC Transport Profile GATT request characteristic UUID.
#define CA_GATT_REQUEST_CHRC_UUID "AD7B334F-4637-4B86-90B6-9D787F03D218"

/**
 * Standard Bluetooth GATT characteristic user description descriptor
 * UUID.
 *
 * @note Used by both the OIC GATT request and response
 *       characteristics.
 */
#define CA_GATT_CHRC_USER_DESCRIPTION_DESC_UUID "2901"

/**
 * OIC Transport Profile GATT request characteristic user description
 * descriptor value.
 */
#define CA_GATT_REQUEST_USER_DESCRIPTION "OIC Node Request"

/// OIC Transport Profile GATT response characteristic UUID.
#define CA_GATT_RESPONSE_CHRC_UUID "E9241982-4580-42C4-8831-95048216B256"

/**
 * OIC Transport Profile GATT response characteristic user description
 * descriptor value.
 */
#define CA_GATT_RESPONSE_USER_DESCRIPTION "OIC Node Response"

/**
 * Standard Bluetooth GATT client characteristic configuration
 * descriptor UUID.
 *
 * @note Only used by the OIC GATT response characteristic.
 */
#define CA_GATT_CONFIGURATION_DESC_UUID "2902"

/**
 * OIC Transport Profile GATT response client characteristic
 * configuration descriptor value.
 */
#define CA_GATT_RESPONSE_CONFIG_DESC "0001"
//@}


#endif  // CA_GATT_SERVICE_H
