/* ****************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
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
#include <stdint.h>

#include "catcpadapter.h"
#include "catcpinterface.h"
#include "caqueueingthread.h"
#include "caadapterutils.h"
#include "camutex.h"
#include "uarraylist.h"
#include "caremotehandler.h"
#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"

/**
 * Logging tag for module name.
 */
#define TAG "TCP_ADAP"

/**
 * Holds internal thread TCP data information.
 */
typedef struct
{
    CAEndpoint_t *remoteEndpoint;
    void *data;
    size_t dataLen;
    bool isMulticast;
} CATCPData;

#define CA_TCP_TIMEOUT 1000

#define CA_TCP_LISTEN_BACKLOG  3

/**
 * Queue handle for Send Data.
 */
static CAQueueingThread_t *g_sendQueueHandle = NULL;

/**
 * Network Packet Received Callback to CA.
 */
static CANetworkPacketReceivedCallback g_networkPacketCallback = NULL;

/**
 * Network Changed Callback to CA.
 */
static CANetworkChangeCallback g_networkChangeCallback = NULL;

/**
 * error Callback to CA adapter.
 */
static CAErrorHandleCallback g_errorCallback = NULL;

static void CATCPPacketReceivedCB(const CAEndpoint_t *endpoint,
                                  const void *data, size_t dataLength);

static CAResult_t CATCPInitializeQueueHandles();

static void CATCPDeinitializeQueueHandles();

static void CATCPSendDataThread(void *threadData);

static CATCPData *CACreateTCPData(const CAEndpoint_t *remoteEndpoint,
                                  const void *data, size_t dataLength,
                                  bool isMulticast);
void CAFreeTCPData(CATCPData *ipData);

static void CADataDestroyer(void *data, size_t size);

CAResult_t CATCPInitializeQueueHandles()
{
    OIC_LOG(DEBUG, TAG, "IN");

    // Check if the message queue is already initialized
    if (g_sendQueueHandle)
    {
        OIC_LOG(DEBUG, TAG, "send queue handle is already initialized!");
        return CA_STATUS_OK;
    }

    // Create send message queue
    g_sendQueueHandle = OICMalloc(sizeof(CAQueueingThread_t));
    if (!g_sendQueueHandle)
    {
        OIC_LOG(ERROR, TAG, "Memory allocation failed!");
        return CA_MEMORY_ALLOC_FAILED;
    }

    if (CA_STATUS_OK != CAQueueingThreadInitialize(g_sendQueueHandle,
                                (const ca_thread_pool_t)caglobals.tcp.threadpool,
                                CATCPSendDataThread, CADataDestroyer))
    {
        OIC_LOG(ERROR, TAG, "Failed to Initialize send queue thread");
        OICFree(g_sendQueueHandle);
        g_sendQueueHandle = NULL;
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

void CATCPDeinitializeQueueHandles()
{
    OIC_LOG(DEBUG, TAG, "IN");

    CAQueueingThreadDestroy(g_sendQueueHandle);
    OICFree(g_sendQueueHandle);
    g_sendQueueHandle = NULL;

    OIC_LOG(DEBUG, TAG, "OUT");
}

void CATCPConnectionStateCB(const char *ipAddress, CANetworkStatus_t status)
{
    (void)ipAddress;
    (void)status;
    OIC_LOG(DEBUG, TAG, "IN");
}

void CATCPPacketReceivedCB(const CAEndpoint_t *endpoint, const void *data,
                           size_t dataLength)
{
    OIC_LOG(DEBUG, TAG, "IN");

    VERIFY_NON_NULL_VOID(endpoint, TAG, "ipAddress is NULL");
    VERIFY_NON_NULL_VOID(data, TAG, "data is NULL");

    OIC_LOG_V(DEBUG, TAG, "Address: %s, port:%d", endpoint->addr, endpoint->port);

    if (g_networkPacketCallback)
    {
        g_networkPacketCallback(endpoint, data, dataLength);
    }
    OIC_LOG(DEBUG, TAG, "OUT");
}

void CATCPErrorHandler(const CAEndpoint_t *endpoint, const void *data,
                       size_t dataLength, CAResult_t result)
{
    OIC_LOG(DEBUG, TAG, "IN");

    VERIFY_NON_NULL_VOID(endpoint, TAG, "endpoint is NULL");

    VERIFY_NON_NULL_VOID(data, TAG, "data is NULL");

    if (g_errorCallback)
    {
        g_errorCallback(endpoint, data, dataLength, result);
    }

    OIC_LOG(DEBUG, TAG, "OUT");
}

static void CAInitializeTCPGlobals()
{
    caglobals.tcp.selectTimeout = CA_TCP_TIMEOUT;
    caglobals.tcp.listenBacklog = CA_TCP_LISTEN_BACKLOG;
    caglobals.tcp.svrlist = NULL;

    CATransportFlags_t flags = 0;
    if (caglobals.client)
    {
        flags |= caglobals.clientFlags;
    }
    if (caglobals.server)
    {
        flags |= caglobals.serverFlags;
    }

    caglobals.tcp.ipv4tcpenabled = flags & CA_IPV4;
}

CAResult_t CAInitializeTCP(CARegisterConnectivityCallback registerCallback,
                           CANetworkPacketReceivedCallback networkPacketCallback,
                           CANetworkChangeCallback netCallback,
                           CAErrorHandleCallback errorCallback, ca_thread_pool_t handle)
{
    OIC_LOG(DEBUG, TAG, "IN");
    VERIFY_NON_NULL(registerCallback, TAG, "registerCallback");
    VERIFY_NON_NULL(networkPacketCallback, TAG, "networkPacketCallback");
    VERIFY_NON_NULL(netCallback, TAG, "netCallback");
    VERIFY_NON_NULL(handle, TAG, "thread pool handle");

    g_networkChangeCallback = netCallback;
    g_networkPacketCallback = networkPacketCallback;
    g_errorCallback = errorCallback;

    CAInitializeTCPGlobals();
    caglobals.tcp.threadpool = handle;

    CATCPSetPacketReceiveCallback(CATCPPacketReceivedCB);
    CATCPSetErrorHandler(CATCPErrorHandler);

    CAConnectivityHandler_t TCPHandler = {
        .startAdapter = CAStartTCP,
        .startListenServer = CAStartTCPListeningServer,
        .stopListenServer = CAStopTCPListeningServer,
        .startDiscoveryServer = CAStartTCPDiscoveryServer,
        .sendData = CASendTCPUnicastData,
        .sendDataToAll = CASendTCPMulticastData,
        .GetnetInfo = CAGetTCPInterfaceInformation,
        .readData = CAReadTCPData,
        .stopAdapter = CAStopTCP,
        .terminate = CATerminateTCP };
    registerCallback(TCPHandler, CA_ADAPTER_TCP);

    OIC_LOG(INFO, TAG, "OUT IntializeTCP is Success");
    return CA_STATUS_OK;
}

CAResult_t CAStartTCP()
{
    OIC_LOG(DEBUG, TAG, "IN");

    if (CA_STATUS_OK != CATCPInitializeQueueHandles())
    {
        OIC_LOG(ERROR, TAG, "Failed to Initialize Queue Handle");
        CATerminateTCP();
        return CA_STATUS_FAILED;
    }

    // Start send queue thread
    if (CA_STATUS_OK != CAQueueingThreadStart(g_sendQueueHandle))
    {
        OIC_LOG(ERROR, TAG, "Failed to Start Send Data Thread");
        return CA_STATUS_FAILED;
    }

    CAResult_t ret = CATCPStartServer((const ca_thread_pool_t)caglobals.tcp.threadpool);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG_V(ERROR, TAG, "Failed to start server![%d]", ret);
        return ret;
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStartTCPListeningServer()
{
    OIC_LOG(DEBUG, TAG, "IN");

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStopTCPListeningServer()
{
    OIC_LOG(DEBUG, TAG, "IN");

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStartTCPDiscoveryServer()
{
    OIC_LOG(DEBUG, TAG, "IN");

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

static size_t CAQueueTCPData(bool isMulticast, const CAEndpoint_t *endpoint,
                             const void *data, size_t dataLength)
{
    OIC_LOG(DEBUG, TAG, "IN");

    VERIFY_NON_NULL_RET(endpoint, TAG, "remoteEndpoint", -1);
    VERIFY_NON_NULL_RET(data, TAG, "data", -1);

    if (0 == dataLength)
    {
        OIC_LOG(ERROR, TAG, "Invalid Data Length");
        return -1;
    }

    VERIFY_NON_NULL_RET(g_sendQueueHandle, TAG, "sendQueueHandle", -1);

    // Create TCPData to add to queue
    CATCPData *TCPData = CACreateTCPData(endpoint, data, dataLength, isMulticast);
    if (!TCPData)
    {
        OIC_LOG(ERROR, TAG, "Failed to create ipData!");
        return -1;
    }
    // Add message to send queue
    CAQueueingThreadAddData(g_sendQueueHandle, TCPData, sizeof(CATCPData));

    OIC_LOG(DEBUG, TAG, "OUT");
    return dataLength;
}

int32_t CASendTCPUnicastData(const CAEndpoint_t *endpoint,
                             const void *data, uint32_t dataLength)
{
    OIC_LOG(DEBUG, TAG, "IN");
    return CAQueueTCPData(false, endpoint, data, dataLength);
}

int32_t CASendTCPMulticastData(const CAEndpoint_t *endpoint,
                               const void *data, uint32_t dataLength)
{
    OIC_LOG(DEBUG, TAG, "IN");
    return CAQueueTCPData(true, endpoint, data, dataLength);
}

CAResult_t CAReadTCPData()
{
    OIC_LOG(DEBUG, TAG, "IN");
    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStopTCP()
{
    OIC_LOG(DEBUG, TAG, "IN");

    if (g_sendQueueHandle && g_sendQueueHandle->threadMutex)
    {
        CAQueueingThreadStop(g_sendQueueHandle);
    }

    CATCPDeinitializeQueueHandles();

    CATCPStopServer();

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

void CATerminateTCP()
{
    OIC_LOG(DEBUG, TAG, "IN");

    CATCPSetPacketReceiveCallback(NULL);

    CATCPDeinitializeQueueHandles();

    OIC_LOG(DEBUG, TAG, "OUT");
}

void CATCPSendDataThread(void *threadData)
{
    OIC_LOG(DEBUG, TAG, "IN");

    CATCPData *TCPData = (CATCPData *) threadData;
    if (!TCPData)
    {
        OIC_LOG(DEBUG, TAG, "Invalid TCP data!");
        return;
    }

    if (TCPData->isMulticast)
    {
        //Processing for sending multicast
        OIC_LOG(DEBUG, TAG, "Send Multicast Data is called, not supported");
        return;
    }
    else
    {
        //Processing for sending unicast
        CATCPSendData(TCPData->remoteEndpoint, TCPData->data, TCPData->dataLen, false);
    }

    OIC_LOG(DEBUG, TAG, "OUT");
}

CATCPData *CACreateTCPData(const CAEndpoint_t *remoteEndpoint, const void *data,
                           size_t dataLength, bool isMulticast)
{
    VERIFY_NON_NULL_RET(data, TAG, "TCPData is NULL", NULL);

    CATCPData *TCPData = (CATCPData *) OICMalloc(sizeof(CATCPData));
    if (!TCPData)
    {
        OIC_LOG(ERROR, TAG, "Memory allocation failed!");
        return NULL;
    }

    TCPData->remoteEndpoint = CACloneEndpoint(remoteEndpoint);
    TCPData->data = (void *) OICMalloc(dataLength);
    if (!TCPData->data)
    {
        OIC_LOG(ERROR, TAG, "Memory allocation failed!");
        CAFreeTCPData(TCPData);
        return NULL;
    }

    memcpy(TCPData->data, data, dataLength);
    TCPData->dataLen = dataLength;

    TCPData->isMulticast = isMulticast;

    return TCPData;
}

void CAFreeTCPData(CATCPData *TCPData)
{
    VERIFY_NON_NULL_VOID(TCPData, TAG, "TCPData is NULL");

    CAFreeEndpoint(TCPData->remoteEndpoint);
    OICFree(TCPData->data);
    OICFree(TCPData);
}

void CADataDestroyer(void *data, size_t size)
{
    if (size < sizeof(CATCPData))
    {
        OIC_LOG_V(ERROR, TAG, "Destroy data too small %p %d", data, size);
    }
    CATCPData *TCPData = (CATCPData *) data;

    CAFreeTCPData(TCPData);
}
