/******************************************************************
*
* Copyright 2014 Samsung Electronics All Rights Reserved.
*
*
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

//logger.h included first to avoid conflict with RBL library PROGMEM attribute
#include "logger.h"

#include "caleinterface.h"

#include <Arduino.h>
#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>

#include "caleadapter.h"
#include "caadapterutils.h"
#include "oic_malloc.h"

/**
 * @def TAG
 * @brief Logging tag for module name
 */
#define TAG "LENW"

/**
 * @var g_caLEDeviceStateChangedCallback
 * @brief Maintains the callback to be notified on device state changed.
 */
static CALEDeviceStateChangedCallback g_caLEDeviceStateChangedCallback = NULL;

/**
 * @var g_leAddress
 * @brief Maintains the local BLE Shield Address
 */
static unsigned char *g_leAddress = NULL;

CAResult_t CAInitializeLENetworkMonitor()
{
    OIC_LOG(DEBUG, TAG, "IN");
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

void CATerminateLENetworkMonitor()
{
    OIC_LOG(DEBUG, TAG, "IN");
    OIC_LOG(DEBUG, TAG, "OUT");
}

CAResult_t CAInitializeLEAdapter()
{
    OIC_LOG(DEBUG, TAG, "IN");
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStartLEAdapter()
{
    // Nothing to do.

    return CA_STATUS_OK;
}

CAResult_t CAGetLEAdapterState()
{
    OIC_LOG(DEBUG, TAG, "IN");
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAGetLEAddress(char **leAddress)
{
    OIC_LOG(DEBUG, TAG, "IN");
    VERIFY_NON_NULL(leAddress, TAG, "leAddress");

    g_leAddress = ble_getAddress();
    /**
     *   Below Allocated Memory will be freed by caller API
     */
    *leAddress = (char*)OICMalloc(CA_MACADDR_SIZE);
    if (NULL == *leAddress)
    {
        OIC_LOG(ERROR, TAG, "malloc fail");
        return CA_MEMORY_ALLOC_FAILED;
    }
    memcpy(*leAddress, g_leAddress, CA_MACADDR_SIZE);
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CASetLEAdapterStateChangedCb(CALEDeviceStateChangedCallback callback)
{
    OIC_LOG(DEBUG, TAG, "IN");

    g_caLEDeviceStateChangedCallback = callback;

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAUnSetLEAdapterStateChangedCb()
{
    OIC_LOG(DEBUG, TAG, "IN");

    g_caLEDeviceStateChangedCallback = NULL;

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}


