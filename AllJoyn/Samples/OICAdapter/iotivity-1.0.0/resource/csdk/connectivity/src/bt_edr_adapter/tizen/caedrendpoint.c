/* ****************************************************************
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
 * This file provides the APIs to send data on established RFCOMM connections.
 */

#include "caedrendpoint.h"
#include "caadapterutils.h"
#include "caedrutils.h"
#include "logger.h"

CAResult_t CAEDRSendData(int serverFD, const void *data, uint32_t dataLength)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL(data, EDR_ADAPTER_TAG, "Data is null");

    if (0 > serverFD)
    {
        OIC_LOG(ERROR, EDR_ADAPTER_TAG, "Invalid input: Negative socket id");
        return CA_STATUS_INVALID_PARAM;
    }

    int dataLen = bt_socket_send_data(serverFD, (const char *)data, dataLength);
    if (dataLen == -1)
    {
        OIC_LOG_V(ERROR, EDR_ADAPTER_TAG, "sending data failed!, soketid [%d]", serverFD);
        return CA_SOCKET_OPERATION_FAILED;
    }

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

