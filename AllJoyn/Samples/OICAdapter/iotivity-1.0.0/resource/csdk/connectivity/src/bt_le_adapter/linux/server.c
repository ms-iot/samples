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

#include "server.h"

#include "cacommon.h"
#include "logger.h"

#include <assert.h>


// Logging tag.
static char const TAG[] = "BLE_SERVER";

// ---------------------------------------------------------------------
//                      GATT Request Handling
// ---------------------------------------------------------------------
void CAGattServerHandleRequestData()
{
}

// ---------------------------------------------------------------------
//                      GATT Response Handling
// ---------------------------------------------------------------------
/**
 * Send response data to the GATT client.
 *
 * Respone data will be sent to the client through the given response
 * @a characteristic proxy as a GATT characteristic notification.
 *
 * @param[in] characteristic The D-Bus proxy for the response
 *                           characteristic through which the
 *                           notification will be sent.
 * @param[in] data           The byte array to be sent.
 * @param[in] length         The number of elements in the byte
 *                           array.
 */
static bool CAGattServerSendResponseNotification(
    GattCharacteristic1 * characteristic,
    char const * data,
    size_t length)
{
    if (!gatt_characteristic1_get_notifying(characteristic))
    {
        OIC_LOG(WARNING,
                TAG,
                "Attempt to send response with notifications "
                "disabled.\n"
                "Client must enable notifications. "
                "No response was sent.");

        return false;
    }

    GVariant * const value =
        g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE,
                                  data,
                                  length,
                                  sizeof(data[0]));

    /**
     * Send the response fragment by setting the "Value" property on
     * the response characteristic, and emitting the
     * @c org.freedesktop.Dbus.Properties.PropertiesChanged signal,
     * accordingly.
     *
     * @todo Do we need to explicitly emit the @c GObject @c notify or
     *       @c org.freedesktop.Dbus.Properties.PropertiesChanged
     *       signal here?
     */
    gatt_characteristic1_set_value(characteristic, value);

    return true;
}

bool CAGattServerSendResponse(void const * method_info,
                              void const * data,
                              size_t length)
{
    assert(method_info != NULL);

    CAGattResponseInfo const * const info = method_info;

    GattCharacteristic1 * const characteristic =
        info->characteristic;

    if (!CAGattServerSendResponseNotification(characteristic,
                                              (char const *) data,
                                              length))
    {
        return false;
    }

    return true;
}
