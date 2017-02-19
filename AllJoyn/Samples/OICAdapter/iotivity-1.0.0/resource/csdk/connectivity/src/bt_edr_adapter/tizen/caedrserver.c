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

/**
 * @file
 *
 * This file provides the APIs to start and stop RFCOMM server.
 */

#include <string.h>
#include <bluetooth.h>

#include "caedrinterface.h"
#include "caadapterutils.h"
#include "caedrutils.h"
#include "logger.h"
#include "camutex.h"
#include "cacommon.h"
#include "caedrdevicelist.h"

static int32_t g_maxPendingConnections = 10;

/**
 * Storing RfcommserverUUID
 */
static int g_serverFD = -1;

CAResult_t CAEDRServerStart(ca_thread_pool_t handle)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    bool isRunning = false;
    bt_error_e err = bt_adapter_is_service_used(OIC_EDR_SERVICE_ID, &isRunning);
    if (BT_ERROR_NONE != err)
    {
        OIC_LOG_V(ERROR, EDR_ADAPTER_TAG,
                  "Unable to find whether service is already running or not! error num[%x]", err);
        return CA_STATUS_FAILED;
    }

    if (isRunning)
    {
        OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "Service is already running with this UUID!");
        return CA_SERVER_STARTED_ALREADY;
    }

    int socketFD = 0;
    // Registers a rfcomm socket with a specific service_uuid.
    err = bt_socket_create_rfcomm(OIC_EDR_SERVICE_ID, &socketFD);
    if (BT_ERROR_NONE != err)
    {
        OIC_LOG_V(ERROR, EDR_ADAPTER_TAG, "Failed to create rfcomm socket!, error num [%x]", err);
        return CA_STATUS_FAILED;
    }

    // Start listening and accepting
    err = bt_socket_listen_and_accept_rfcomm(socketFD, g_maxPendingConnections);
    if (BT_ERROR_NONE != err)
    {
        OIC_LOG_V(ERROR, EDR_ADAPTER_TAG, "Failed in listen rfcomm socket!, error num [%x]", err);

        bt_socket_destroy_rfcomm(socketFD);
        return CA_STATUS_FAILED;
    }

    g_serverFD = socketFD;

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAEDRServerStop()
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    if (-1 < g_serverFD)
    {
        bt_error_e err = bt_socket_destroy_rfcomm(g_serverFD);

        if (BT_ERROR_NONE != err)
        {
            OIC_LOG_V(ERROR, EDR_ADAPTER_TAG, "Failed close server socket!, error num [%x]", err);
            return CA_STATUS_FAILED;
        }
        g_serverFD = -1;
    }

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

void CAEDRServerTerminate()
{
    // This is just a dummy
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "CAEDRServerTerminate");
}

CAResult_t CAEDRManagerReadData(void)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
    return CA_NOT_SUPPORTED;
}
