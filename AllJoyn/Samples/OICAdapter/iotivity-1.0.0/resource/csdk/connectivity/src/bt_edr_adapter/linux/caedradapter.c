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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "caedradapter.h"
#include "logger.h"

#define TAG PCF("CA")

static CANetworkPacketReceivedCallback g_edrReceivedCallback = NULL;
static ca_thread_pool_t g_threadPoolHandle = NULL;

CAResult_t CAInitializeEDR(CARegisterConnectivityCallback registerCallback,
                           CANetworkPacketReceivedCallback reqRespCallback,
                           CANetworkChangeCallback networkStateChangeCallback,
                           CAErrorHandleCallback errorCallback, ca_thread_pool_t handle)
{
    (void)networkStateChangeCallback;
    (void)errorCallback;
    OIC_LOG(DEBUG, TAG, "CAInitializeEDR");

    g_edrReceivedCallback = reqRespCallback;
    g_threadPoolHandle = handle;

    // register handlers
    CAConnectivityHandler_t handler = {
        .startAdapter = CAStartEDR,
        .startListenServer = CAStartEDRListeningServer,
        .stopListenServer = CAStopEDRListeningServer,
        .startDiscoveryServer = CAStartEDRDiscoveryServer,
        .sendData = CASendEDRUnicastData,
        .sendDataToAll = CASendEDRMulticastData,
        .GetnetInfo = CAGetEDRInterfaceInformation,
        .readData = CAReadEDRData,
        .stopAdapter = CAStopEDR,
        .terminate = CATerminateEDR
    };

    registerCallback(handler, CA_ADAPTER_RFCOMM_BTEDR);

    return CA_STATUS_OK;
}

CAResult_t CAStartEDR()
{
    OIC_LOG(DEBUG, TAG, "CAStartEDR");

    return CA_STATUS_OK;
}

CAResult_t CAStartEDRListeningServer()
{
    OIC_LOG(DEBUG, TAG, "CAStartEDRListeningServer");

    return CA_STATUS_OK;
}

CAResult_t CAStopEDRListeningServer()
{
    OIC_LOG(DEBUG, TAG, "CAStopEDRListeningServer");

    return CA_STATUS_OK;
}

CAResult_t CAStartEDRDiscoveryServer()
{
    OIC_LOG(DEBUG, TAG, "CAStartEDRDiscoveryServer");

    return CA_STATUS_OK;
}

int32_t CASendEDRUnicastData(const CAEndpoint_t *endpoint, const void *data,
    uint32_t dataLen)
{
    (void)endpoint;
    (void)data;
    (void)dataLen;
    OIC_LOG(DEBUG, TAG, "CASendEDRUnicastData");

    return -1;
}

int32_t CASendEDRMulticastData(const CAEndpoint_t *endpoint, const void *data, uint32_t dataLen)
{
    (void)endpoint;
    (void)data;
    (void)dataLen;
    OIC_LOG(DEBUG, TAG, "CASendEDRMulticastData");

    return -1;
}

CAResult_t CAGetEDRInterfaceInformation(CAEndpoint_t **info, uint32_t *size)
{
    (void)info;
    (void)size;
    OIC_LOG(DEBUG, TAG, "CAGetEDRInterfaceInformation");

    return CA_STATUS_OK;
}

CAResult_t CAReadEDRData()
{
    OIC_LOG(DEBUG, TAG, "Read EDR Data");

    return CA_STATUS_OK;
}

CAResult_t CAStopEDR()
{
    OIC_LOG(DEBUG, TAG, "CAStopEDR");

    return CA_STATUS_OK;
}

void CATerminateEDR()
{
    OIC_LOG(DEBUG, TAG, "CATerminateEDR");
}

