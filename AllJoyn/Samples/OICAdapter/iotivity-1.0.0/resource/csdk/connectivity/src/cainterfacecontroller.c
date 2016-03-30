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
#include <inttypes.h>

#include "logger.h"
#include "oic_malloc.h"
#include "caadapterutils.h"
#include "canetworkconfigurator.h"
#include "cainterfacecontroller.h"
#include "caedradapter.h"
#include "caleadapter.h"
#include "caremotehandler.h"
#include "cathreadpool.h"
#include "caipadapter.h"
#include "cainterface.h"

#ifdef RA_ADAPTER
#include "caraadapter.h"
#endif

#ifdef TCP_ADAPTER
#include "catcpadapter.h"
#endif

#define TAG "CA_INTRFC_CNTRLR"

#define CA_MEMORY_ALLOC_CHECK(arg) {if (arg == NULL) \
    {OIC_LOG(ERROR, TAG, "memory error");goto memory_error_exit;} }

#ifdef TCP_ADAPTER
#define CA_TRANSPORT_TYPE_NUM   5
#elif RA_ADAPTER
#define CA_TRANSPORT_TYPE_NUM   4
#else
#define CA_TRANSPORT_TYPE_NUM   3
#endif

static CAConnectivityHandler_t g_adapterHandler[CA_TRANSPORT_TYPE_NUM] = {0};

static CANetworkPacketReceivedCallback g_networkPacketReceivedCallback = NULL;

static CANetworkChangeCallback g_networkChangeCallback = NULL;

static CAErrorHandleCallback g_errorHandleCallback = NULL;

static int CAGetAdapterIndex(CATransportAdapter_t cType)
{
    switch (cType)
    {
        case CA_ADAPTER_IP:
            return 0;
        case CA_ADAPTER_GATT_BTLE:
            return 1;
        case CA_ADAPTER_RFCOMM_BTEDR:
            return 2;

#ifdef RA_ADAPTER
        case CA_ADAPTER_REMOTE_ACCESS:
            return 3;
#endif

#ifdef TCP_ADAPTER
        case CA_ADAPTER_TCP:
            return 4;
#endif

        default:
            break;
    }
    return -1;
}

static void CARegisterCallback(CAConnectivityHandler_t handler, CATransportAdapter_t cType)
{
    OIC_LOG(DEBUG, TAG, "IN");

    if(handler.startAdapter == NULL ||
        handler.startListenServer == NULL ||
        handler.stopListenServer == NULL ||
        handler.startDiscoveryServer == NULL ||
        handler.sendData == NULL ||
        handler.sendDataToAll == NULL ||
        handler.GetnetInfo == NULL ||
        handler.readData == NULL ||
        handler.stopAdapter == NULL ||
        handler.terminate == NULL)
    {
        OIC_LOG(ERROR, TAG, "connectivity handler is not enough to be used!");
        return;
    }

    int index = CAGetAdapterIndex(cType);

    if (index == -1)
    {
        OIC_LOG(ERROR, TAG, "unknown connectivity type!");
        return;
    }

    g_adapterHandler[index] = handler;

    OIC_LOG_V(DEBUG, TAG, "%d type adapter, register complete!", cType);
    OIC_LOG(DEBUG, TAG, "OUT");
}

#ifdef RA_ADAPTER
CAResult_t CASetAdapterRAInfo(const CARAInfo_t *caraInfo)
{
    return CASetRAInfo(caraInfo);
}
#endif

static void CAReceivedPacketCallback(const CASecureEndpoint_t *sep,
                                     const void *data, uint32_t dataLen)
{
    OIC_LOG(DEBUG, TAG, "IN");

    if (g_networkPacketReceivedCallback != NULL)
    {
        g_networkPacketReceivedCallback(sep, data, dataLen);
    }
    else
    {
        OIC_LOG(ERROR, TAG, "network packet received callback is NULL!");
    }

    OIC_LOG(DEBUG, TAG, "OUT");
}

static void CANetworkChangedCallback(const CAEndpoint_t *info, CANetworkStatus_t status)
{
    OIC_LOG(DEBUG, TAG, "IN");

    // Call the callback.
    if (g_networkChangeCallback != NULL)
    {
        g_networkChangeCallback(info, status);
    }

    OIC_LOG(DEBUG, TAG, "OUT");
}

static void CAAdapterErrorHandleCallback(const CAEndpoint_t *endpoint,
                                         const void *data, uint32_t dataLen,
                                         CAResult_t result)
{
    OIC_LOG(DEBUG, TAG, "received error from adapter in interfacecontroller");

    // Call the callback.
    if (g_errorHandleCallback != NULL)
    {
        g_errorHandleCallback(endpoint, data, dataLen, result);
    }
}

void CAInitializeAdapters(ca_thread_pool_t handle)
{
    OIC_LOG(DEBUG, TAG, "initialize adapters..");

    memset(g_adapterHandler, 0, sizeof(CAConnectivityHandler_t) * CA_TRANSPORT_TYPE_NUM);

    // Initialize adapters and register callback.
#ifdef IP_ADAPTER
    CAInitializeIP(CARegisterCallback, CAReceivedPacketCallback, CANetworkChangedCallback,
                   CAAdapterErrorHandleCallback, handle);
#endif /* IP_ADAPTER */

#ifdef EDR_ADAPTER
    CAInitializeEDR(CARegisterCallback, CAReceivedPacketCallback, CANetworkChangedCallback,
                    CAAdapterErrorHandleCallback, handle);
#endif /* EDR_ADAPTER */

#ifdef LE_ADAPTER
    CAInitializeLE(CARegisterCallback, CAReceivedPacketCallback, CANetworkChangedCallback,
                   CAAdapterErrorHandleCallback, handle);
#endif /* LE_ADAPTER */

#ifdef RA_ADAPTER
    CAInitializeRA(CARegisterCallback, CAReceivedPacketCallback, CANetworkChangedCallback,
                   handle);
#endif /* RA_ADAPTER */

#ifdef TCP_ADAPTER
    CAInitializeTCP(CARegisterCallback, CAReceivedPacketCallback, CANetworkChangedCallback,
                    CAAdapterErrorHandleCallback, handle);
#endif /* TCP_ADAPTER */
}

void CASetPacketReceivedCallback(CANetworkPacketReceivedCallback callback)
{
    OIC_LOG(DEBUG, TAG, "IN");

    g_networkPacketReceivedCallback = callback;

    OIC_LOG(DEBUG, TAG, "OUT");
}

void CASetNetworkChangeCallback(CANetworkChangeCallback callback)
{
    OIC_LOG(DEBUG, TAG, "IN");

    g_networkChangeCallback = callback;

    OIC_LOG(DEBUG, TAG, "OUT");
}

void CASetErrorHandleCallback(CAErrorHandleCallback errorCallback)
{
    OIC_LOG(DEBUG, TAG, "Set error handle callback");
    g_errorHandleCallback = errorCallback;
}

CAResult_t CAStartAdapter(CATransportAdapter_t transportType)
{
    OIC_LOG_V(DEBUG, TAG, "Start the adapter of CAConnectivityType[%d]", transportType);

    int index = CAGetAdapterIndex(transportType);

    if (index == -1)
    {
        OIC_LOG(ERROR, TAG, "unknown connectivity type!");
        return CA_STATUS_FAILED;
    }

    if (g_adapterHandler[index].startAdapter != NULL)
    {
        g_adapterHandler[index].startAdapter();
    }

    return CA_STATUS_OK;
}

void CAStopAdapter(CATransportAdapter_t transportType)
{
    OIC_LOG_V(DEBUG, TAG, "Stop the adapter of CATransportType[%d]", transportType);

    int index = CAGetAdapterIndex(transportType);

    if (index == -1)
    {
        OIC_LOG(ERROR, TAG, "unknown transport type!");
        return;
    }

    if (g_adapterHandler[index].stopAdapter != NULL)
    {
        g_adapterHandler[index].stopAdapter();
    }
}

CAResult_t CAGetNetworkInfo(CAEndpoint_t **info, uint32_t *size)
{
    if (info == NULL || size == NULL)
    {
        return CA_STATUS_INVALID_PARAM;
    }

    CAEndpoint_t *tempInfo[CA_TRANSPORT_TYPE_NUM] = { 0 };
    uint32_t tempSize[CA_TRANSPORT_TYPE_NUM] = { 0 };

    CAResult_t res = CA_STATUS_FAILED;
    uint32_t resSize = 0;
    for (int index = 0; index < CA_TRANSPORT_TYPE_NUM; index++)
    {
        if (g_adapterHandler[index].GetnetInfo != NULL)
        {
            // #1. get information for each adapter
            res = g_adapterHandler[index].GetnetInfo(&tempInfo[index],
                                                     &tempSize[index]);

            // #2. total size
            if (res == CA_STATUS_OK)
            {
                resSize += tempSize[index];
            }

            OIC_LOG_V(DEBUG,
                      TAG,
                      "%d adapter network info size is %" PRIu32 " res:%d",
                      index,
                      tempSize[index],
                      res);
        }
    }

    OIC_LOG_V(DEBUG, TAG, "network info total size is %d!", resSize);

    if (resSize == 0)
    {
        if (res == CA_ADAPTER_NOT_ENABLED || res == CA_NOT_SUPPORTED)
        {
            return res;
        }
        return CA_STATUS_FAILED;
    }

    // #3. add data into result
    // memory allocation
    CAEndpoint_t *resInfo = (CAEndpoint_t *) OICCalloc(resSize, sizeof (*resInfo));
    CA_MEMORY_ALLOC_CHECK(resInfo);

    // #4. save data
    *info = resInfo;
    *size = resSize;

    for (int index = 0; index < CA_TRANSPORT_TYPE_NUM; index++)
    {
        // check information
        if (tempSize[index] == 0)
        {
            continue;
        }

        memcpy(resInfo,
               tempInfo[index],
               sizeof(*resInfo) * tempSize[index]);

        resInfo += tempSize[index];

        // free adapter data
        OICFree(tempInfo[index]);
        tempInfo[index] = NULL;
    }

    OIC_LOG(DEBUG, TAG, "each network info save success!");
    return CA_STATUS_OK;

    // memory error label.
memory_error_exit:

    for (int index = 0; index < CA_TRANSPORT_TYPE_NUM; index++)
    {

        OICFree(tempInfo[index]);
        tempInfo[index] = NULL;
    }

    return CA_MEMORY_ALLOC_FAILED;
}

CAResult_t CASendUnicastData(const CAEndpoint_t *endpoint, const void *data, uint32_t length)
{
    OIC_LOG(DEBUG, TAG, "IN");

    if (endpoint == NULL)
    {
        OIC_LOG(DEBUG, TAG, "Invalid endpoint");
        return CA_STATUS_INVALID_PARAM;
    }

    CATransportAdapter_t type = endpoint->adapter;

    int index = CAGetAdapterIndex(type);

    if (index == -1)
    {
        OIC_LOG(ERROR, TAG, "unknown transport type!");
        return CA_STATUS_INVALID_PARAM;
    }

    int32_t sentDataLen = 0;

    if (g_adapterHandler[index].sendData != NULL)
    {
        sentDataLen = g_adapterHandler[index].sendData(endpoint, data, length);
    }

    if (sentDataLen != (int)length)
    {
        OIC_LOG(ERROR, TAG, "error in sending data. Error will be reported in adapter");
#ifdef SINGLE_THREAD
        //in case of single thread, no error handler. Report error immediately
        return CA_SEND_FAILED;
#endif
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CASendMulticastData(const CAEndpoint_t *endpoint, const void *data, uint32_t length)
{
    OIC_LOG(DEBUG, TAG, "IN");

    u_arraylist_t *list = CAGetSelectedNetworkList();
    if (!list)
    {
        OIC_LOG(DEBUG, TAG, "No selected network");
        return CA_SEND_FAILED;
    }

    CATransportFlags_t requestedAdapter = endpoint->adapter ? endpoint->adapter : CA_ALL_ADAPTERS;

    for (uint32_t i = 0; i < u_arraylist_length(list); i++)
    {
        void* ptrType = u_arraylist_get(list, i);

        if(ptrType == NULL)
        {
            continue;
        }

        CATransportAdapter_t connType = *(CATransportAdapter_t *)ptrType;
        if ((connType & requestedAdapter) == 0)
        {
            continue;
        }

        int index = CAGetAdapterIndex(connType);

        if (index == -1)
        {
            OIC_LOG(DEBUG, TAG, "unknown connectivity type!");
            continue;
        }

        uint32_t sentDataLen = 0;

        if (g_adapterHandler[index].sendDataToAll != NULL)
        {
            void *payload = (void *) OICMalloc(length);
            if (!payload)
            {
                OIC_LOG(ERROR, TAG, "Out of memory!");
                return CA_MEMORY_ALLOC_FAILED;
            }
            memcpy(payload, data, length);
            sentDataLen = g_adapterHandler[index].sendDataToAll(endpoint, payload, length);
            OICFree(payload);
        }

        if (sentDataLen != length)
        {
            OIC_LOG(ERROR, TAG, "sendDataToAll failed! Error will be reported from adapter");
#ifdef SINGLE_THREAD
            //in case of single thread, no error handler. Report error immediately
            return CA_SEND_FAILED;
#endif
        }
    }

    OIC_LOG(DEBUG, TAG, "OUT");

    return CA_STATUS_OK;
}

CAResult_t CAStartListeningServerAdapters()
{
    OIC_LOG(DEBUG, TAG, "IN");

    u_arraylist_t *list = CAGetSelectedNetworkList();
    if (!list)
    {
        OIC_LOG(ERROR, TAG, "No selected network");
        return CA_STATUS_FAILED;
    }

    for (uint32_t i = 0; i < u_arraylist_length(list); i++)
    {
        void* ptrType = u_arraylist_get(list, i);

        if(ptrType == NULL)
        {
            continue;
        }

        CATransportAdapter_t connType = *(CATransportAdapter_t *)ptrType;

        int index = CAGetAdapterIndex(connType);
        if (index == -1)
        {
            OIC_LOG(ERROR, TAG, "unknown connectivity type!");
            continue;
        }

        if (g_adapterHandler[index].startListenServer != NULL)
        {
            g_adapterHandler[index].startListenServer();
        }
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStopListeningServerAdapters()
{
    OIC_LOG(DEBUG, TAG, "IN");

    u_arraylist_t *list = CAGetSelectedNetworkList();
    if (!list)
    {
        OIC_LOG(ERROR, TAG, "No selected network");
        return CA_STATUS_FAILED;
    }

    for (uint32_t i = 0; i < u_arraylist_length(list); i++)
    {
        void* ptrType = u_arraylist_get(list, i);
        if(ptrType == NULL)
        {
            continue;
        }

        CATransportAdapter_t connType = *(CATransportAdapter_t *)ptrType;

        int index = CAGetAdapterIndex(connType);
        if (index == -1)
        {
            OIC_LOG(ERROR, TAG, "unknown connectivity type!");
            continue;
        }

        if (g_adapterHandler[index].stopListenServer != NULL)
        {
            g_adapterHandler[index].stopListenServer();
        }
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStartDiscoveryServerAdapters()
{
    OIC_LOG(DEBUG, TAG, "IN");

    u_arraylist_t *list = CAGetSelectedNetworkList();

    if (!list)
    {
        OIC_LOG(ERROR, TAG, "No selected network");
        return CA_STATUS_FAILED;
    }

    for (uint32_t i = 0; i < u_arraylist_length(list); i++)
    {
        void* ptrType = u_arraylist_get(list, i);

        if(ptrType == NULL)
        {
            continue;
        }

        CATransportAdapter_t connType = *(CATransportAdapter_t *)ptrType;

        int index = CAGetAdapterIndex(connType);

        if (index == -1)
        {
            OIC_LOG(DEBUG, TAG, "unknown connectivity type!");
            continue;
        }

        if (g_adapterHandler[index].startDiscoveryServer != NULL)
        {
            g_adapterHandler[index].startDiscoveryServer();
        }
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

void CATerminateAdapters()
{
    OIC_LOG(DEBUG, TAG, "IN");

    uint32_t index;
    for (index = 0; index < CA_TRANSPORT_TYPE_NUM; index++)
    {
        if (g_adapterHandler[index].terminate != NULL)
        {
            g_adapterHandler[index].terminate();
        }
    }

    OIC_LOG(DEBUG, TAG, "OUT");
}

#ifdef SINGLE_THREAD
CAResult_t CAReadData()
{
    OIC_LOG(DEBUG, TAG, "IN");
    u_arraylist_t *list = CAGetSelectedNetworkList();

    if (!list)
    {
        return CA_STATUS_FAILED;
    }

    uint8_t i = 0;
    for (i = 0; i < u_arraylist_length(list); i++)
    {
        void *ptrType = u_arraylist_get(list, i);
        if (NULL == ptrType)
        {
            OIC_LOG(ERROR, TAG, "get list fail");
            return CA_STATUS_FAILED;
        }

        CATransportAdapter_t connType = *(CATransportAdapter_t *) ptrType;

        int index = CAGetAdapterIndex(connType);

        if (-1 == index)
        {
            OIC_LOG(DEBUG, TAG, "unknown connectivity type!");
            continue;
        }

        if (g_adapterHandler[index].readData != NULL)
        {
            g_adapterHandler[index].readData();
        }
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}
#endif

