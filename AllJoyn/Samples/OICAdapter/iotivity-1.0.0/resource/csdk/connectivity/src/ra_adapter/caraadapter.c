//*****************************************************************
//
// Copyright 2015 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//****************************************************************

#include "caraadapter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "caadapterutils.h"
#include "camutex.h"
#include "uarraylist.h"
#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "ra_xmpp.h"
#include "caremotehandler.h"
#include "cacommon.h"

/**
 * Logging tag for module name.
 */
#define RA_ADAPTER_TAG "RA_ADAP"

/**
 * Network Packet Received Callback to CA.
 */
static CANetworkPacketReceivedCallback g_networkPacketCallback = NULL;

/**
 * Network Changed Callback to CA.
 */
static CANetworkChangeCallback g_networkChangeCallback = NULL;

/**
 * Holds XMPP data information.
 */
typedef struct
{
    xmpp_context_t context;
    xmpp_handle_t handle;
    xmpp_connection_callback_t connection_callback;
    xmpp_connection_handle_t connection_handle;
    xmpp_message_context_t message_context;
    xmpp_message_callback_t message_callback;
    CANetworkStatus_t connection_status;
    xmpp_host_t     g_host;
    xmpp_identity_t g_identity;
    char jabberID[CA_RAJABBERID_SIZE];
} CARAXmppData_t;

static ca_mutex g_raadapterMutex = NULL;

static CARAXmppData_t g_xmppData = {};

static void CARANotifyNetworkChange(const char *address, CANetworkStatus_t status);

static void CARAXmppConnectedCB(void * const param, xmpp_error_code_t result,
        const char *const bound_jid,
        xmpp_connection_handle_t connection);

static void CARAXmppDisonnectedCB(void * const param, xmpp_error_code_t result,
        xmpp_connection_handle_t connection);

static void CARAXmppMessageSentCB(void * const param, xmpp_error_code_t result,
        const void *const recipient, const void *const msg, size_t messageOctets);

static void CARAXmppMessageReceivedCB(void * const param, xmpp_error_code_t result,
        const void *const sender, const void *const msg, size_t messageOctets);

void CARANotifyNetworkChange(const char *address, CANetworkStatus_t status)
{
    OIC_LOG(DEBUG, RA_ADAPTER_TAG, "CARANotifyNetworkChange IN");

    CAEndpoint_t *localEndpoint = CACreateEndpointObject(CA_DEFAULT_FLAGS,
                                CA_ADAPTER_REMOTE_ACCESS,
                                address, 0);
    if (!localEndpoint)
    {
        OIC_LOG(ERROR, RA_ADAPTER_TAG, "localEndpoint creation failed!");
        return;
    }
    CANetworkChangeCallback networkChangeCallback = g_networkChangeCallback;
    if (networkChangeCallback)
    {
        networkChangeCallback(localEndpoint, status);
    }
    else
    {
        OIC_LOG(ERROR, RA_ADAPTER_TAG, "g_networkChangeCallback is NULL");
    }

    CAFreeEndpoint(localEndpoint);

    OIC_LOG(DEBUG, RA_ADAPTER_TAG, "CARANotifyNetworkChange OUT");
}

void CARAXmppConnectedCB(void * const param, xmpp_error_code_t result,
        const char *const bound_jid,
        xmpp_connection_handle_t connection)
{
    OIC_LOG(DEBUG, RA_ADAPTER_TAG, "CARAXmppConnectedCB IN");
    CANetworkStatus_t connection_status;
    if (XMPP_ERR_OK == result)
    {
        printf("\n\n\t\t===>your jid: %s\n\n", bound_jid);

        ca_mutex_lock (g_raadapterMutex);
        OICStrcpy (g_xmppData.jabberID, CA_RAJABBERID_SIZE, bound_jid);

        g_xmppData.connection_status = CA_INTERFACE_UP;
        connection_status = CA_INTERFACE_UP;
        g_xmppData.connection_handle = connection;
        g_xmppData.message_callback.on_received = CARAXmppMessageReceivedCB;
        g_xmppData.message_callback.on_sent = CARAXmppMessageSentCB;
        g_xmppData.message_context = xmpp_message_context_create(g_xmppData.connection_handle,
                g_xmppData.message_callback);
    }
    else
    {
        g_xmppData.connection_status = CA_INTERFACE_DOWN;
        connection_status = CA_INTERFACE_DOWN;
        OIC_LOG_V(ERROR, RA_ADAPTER_TAG, "XMPP connected callback status: %d", result);
    }

    ca_mutex_unlock (g_raadapterMutex);
    // Notify network change to CA
    CARANotifyNetworkChange(bound_jid, connection_status);

    OIC_LOG(DEBUG, RA_ADAPTER_TAG, "CARAXmppConnectedCB OUT");
}

void CARAXmppDisonnectedCB(void * const param, xmpp_error_code_t result,
        xmpp_connection_handle_t connection)
{
    OIC_LOG(DEBUG, RA_ADAPTER_TAG, "CARAXmppDisonnectedCB IN");
    char jabberID[CA_RAJABBERID_SIZE];
    ca_mutex_lock (g_raadapterMutex);

    g_xmppData.connection_status = CA_INTERFACE_DOWN;
    xmpp_message_context_destroy(g_xmppData.message_context);
    OICStrcpy (jabberID, CA_RAJABBERID_SIZE, g_xmppData.jabberID);

    ca_mutex_unlock (g_raadapterMutex);

    // Notify network change to CA
    CARANotifyNetworkChange(jabberID, CA_INTERFACE_DOWN);

    OIC_LOG(DEBUG, RA_ADAPTER_TAG, "CARAXmppDisonnectedCB OUT");
}

void CARAXmppMessageSentCB(void * const param, xmpp_error_code_t result,
        const void *const recipient, const void *const msg, size_t messageOctets)
{
    OIC_LOG_V(DEBUG, RA_ADAPTER_TAG, "Sending message to %s has result %d",
        recipient, result);
}

void CARAXmppMessageReceivedCB(void * const param, xmpp_error_code_t result,
        const void *const sender, const void *const msg, size_t messageOctets)
{
    if (g_networkPacketCallback)
    {
        VERIFY_NON_NULL_VOID(sender, RA_ADAPTER_TAG, "sender is NULL");
        VERIFY_NON_NULL_VOID(msg,    RA_ADAPTER_TAG, "message is NULL");

        OIC_LOG_V (ERROR, RA_ADAPTER_TAG, "Message received from %s", sender);
        OIC_LOG_V (ERROR, RA_ADAPTER_TAG, "Message reception result %d", result);

        CAEndpoint_t *endPoint = CACreateEndpointObject(CA_DEFAULT_FLAGS,
                        CA_ADAPTER_REMOTE_ACCESS, sender, 0);
        if (!endPoint)
        {
            OIC_LOG(ERROR, RA_ADAPTER_TAG, "EndPoint creation failed!");
            return;
        }

        void *buf = OICMalloc(messageOctets);
        if (!buf)
        {
            OIC_LOG(ERROR, RA_ADAPTER_TAG, "Memory alloc of message failed!");
            CAFreeEndpoint(endPoint);
            return;
        }
        memcpy(buf, msg, messageOctets);
        CANetworkPacketReceivedCallback networkPacketCallback = g_networkPacketCallback;
        if (networkPacketCallback)
        {
            g_networkPacketCallback(endPoint, buf, messageOctets);
        }

        CAFreeEndpoint (endPoint);
    }
    else
    {
        OIC_LOG_V (ERROR, RA_ADAPTER_TAG, "No callback for RA  received message found");
    }
}

CAResult_t CAInitializeRA(CARegisterConnectivityCallback registerCallback,
                                CANetworkPacketReceivedCallback networkPacketCallback,
                                CANetworkChangeCallback netCallback, ca_thread_pool_t handle)
{
    OIC_LOG(DEBUG, RA_ADAPTER_TAG, "CAInitializeRA IN");
    if (!registerCallback || !networkPacketCallback || !netCallback || !handle)
    {
        return CA_STATUS_INVALID_PARAM;
    }

    g_networkChangeCallback = netCallback;
    g_networkPacketCallback = networkPacketCallback;

    CAConnectivityHandler_t raHandler = {};
    raHandler.startAdapter = CAStartRA;
    raHandler.startListenServer = CAStartRAListeningServer;
    raHandler.stopListenServer = CAStopRAListeningServer;
    raHandler.startDiscoveryServer = CAStartRADiscoveryServer;
    raHandler.sendData = CASendRAUnicastData;
    raHandler.sendDataToAll = CASendRAMulticastData;
    raHandler.GetnetInfo = CAGetRAInterfaceInformation;
    raHandler.readData = CAReadRAData;
    raHandler.stopAdapter = CAStopRA;
    raHandler.terminate = CATerminateRA;
    registerCallback(raHandler, CA_ADAPTER_REMOTE_ACCESS);

    return CA_STATUS_OK;
}

CAResult_t CASetRAInfo(const CARAInfo_t *caraInfo)
{
    if (!caraInfo)
    {
        return CA_STATUS_INVALID_PARAM;
    }
    xmpp_identity_init(&g_xmppData.g_identity, caraInfo->username, caraInfo->password,
            caraInfo->user_jid, XMPP_TRY_IN_BAND_REGISTER);
    xmpp_host_init(&g_xmppData.g_host, caraInfo->hostname, caraInfo->port,
            caraInfo->xmpp_domain, XMPP_PROTOCOL_XMPP);
    return CA_STATUS_OK;
}

void CATerminateRA()
{
    CAStopRA();
}

CAResult_t CAStartRA()
{
    if (g_xmppData.handle.abstract_handle)
    {
        OIC_LOG(WARNING, RA_ADAPTER_TAG, "RA adapter already started");
        return CA_STATUS_OK;
    }

    OIC_LOG(DEBUG, RA_ADAPTER_TAG, PCF("Starting RA adapter"));

    g_raadapterMutex = ca_mutex_new ();
    if (!g_raadapterMutex)
    {
        OIC_LOG (ERROR, RA_ADAPTER_TAG, PCF("Memory allocation for mutex failed."));
        return CA_MEMORY_ALLOC_FAILED;
    }

    ca_mutex_lock (g_raadapterMutex);

    xmpp_context_init(&g_xmppData.context);
    g_xmppData.handle = xmpp_startup(&g_xmppData.context);

    // Wire up connection callbacks and call API to connect to XMPP server
    g_xmppData.connection_callback.on_connected = CARAXmppConnectedCB;
    g_xmppData.connection_callback.on_disconnected = CARAXmppDisonnectedCB;

    xmpp_error_code_t ret = xmpp_connect(g_xmppData.handle, &g_xmppData.g_host,
            &g_xmppData.g_identity, g_xmppData.connection_callback);

    // Destroy host and identity structures as they are only
    // required to establish initial connection
    xmpp_identity_destroy(&g_xmppData.g_identity);
    xmpp_host_destroy(&g_xmppData.g_host);

    ca_mutex_unlock (g_raadapterMutex);

    if (XMPP_ERR_OK != ret)
    {
        OIC_LOG_V(ERROR, RA_ADAPTER_TAG, "Failed to init XMPP connection status: %d",
            ret);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, RA_ADAPTER_TAG, "RA adapter started succesfully");
    return CA_STATUS_OK;
}

CAResult_t CAStopRA()
{
    OIC_LOG(DEBUG, RA_ADAPTER_TAG, PCF("Stopping RA adapter"));

    xmpp_error_code_t ret = xmpp_close(g_xmppData.connection_handle);
    if (XMPP_ERR_OK != ret)
    {
        OIC_LOG_V(ERROR, RA_ADAPTER_TAG, "Failed to close XMPP connection, status: %d",
            ret);
        return CA_STATUS_FAILED;
    }

    xmpp_shutdown_xmpp(g_xmppData.handle);
    xmpp_context_destroy(&g_xmppData.context);
    ca_mutex_free (g_raadapterMutex);
    g_raadapterMutex = NULL;

    OIC_LOG(DEBUG, RA_ADAPTER_TAG, PCF("Stopped RA adapter successfully"));
    return CA_STATUS_OK;
}

int32_t CASendRAUnicastData(const CAEndpoint_t *remoteEndpoint, const void *data,
                                  uint32_t dataLength)
{
    if (!remoteEndpoint || !data)
    {
        OIC_LOG(ERROR, RA_ADAPTER_TAG, "Invalid parameter!");
        return -1;
    }

    if (0 == dataLength)
    {
        OIC_LOG(ERROR, RA_ADAPTER_TAG, "Data length is 0!");
        return 0;
    }

    OIC_LOG_V(ERROR, RA_ADAPTER_TAG, "Sending unicast data to %s", remoteEndpoint->addr);
    ca_mutex_lock (g_raadapterMutex);

    if (CA_INTERFACE_UP != g_xmppData.connection_status)
    {
        OIC_LOG(ERROR, RA_ADAPTER_TAG, "Unable to send XMPP message, RA not connected");
        ca_mutex_unlock (g_raadapterMutex);
        return -1;
    }

    xmpp_error_code_t res = xmpp_send_message(g_xmppData.message_context,
            remoteEndpoint->addr, data, dataLength,
            XMPP_MESSAGE_TRANSMIT_DEFAULT);
    if (XMPP_ERR_OK != res)
    {
        OIC_LOG_V(ERROR, RA_ADAPTER_TAG, "Unable to send XMPP message, status: %d", res);
        ca_mutex_unlock (g_raadapterMutex);
        return -1;
    }
    ca_mutex_unlock (g_raadapterMutex);

    OIC_LOG_V(INFO, RA_ADAPTER_TAG, "Successfully dispatched bytes[%d] to addr[%s]",
            dataLength, remoteEndpoint->addr);

    return dataLength;
}

CAResult_t CAGetRAInterfaceInformation(CAEndpoint_t **info, uint32_t *size)
{
    VERIFY_NON_NULL(info, RA_ADAPTER_TAG, "info is NULL");
    VERIFY_NON_NULL(size, RA_ADAPTER_TAG, "size is NULL");

    ca_mutex_lock (g_raadapterMutex);

    if (CA_INTERFACE_UP != g_xmppData.connection_status)
    {
        OIC_LOG(ERROR, RA_ADAPTER_TAG, "Failed to get interface info, RA not Connected");
        ca_mutex_unlock (g_raadapterMutex);
        return CA_ADAPTER_NOT_ENABLED;
    }

    ca_mutex_unlock (g_raadapterMutex);

    CAEndpoint_t *localEndpoint = CACreateEndpointObject(CA_DEFAULT_FLAGS,
                                 CA_ADAPTER_REMOTE_ACCESS,
                                 g_xmppData.jabberID, 0);

    *size = 1;
    *info = localEndpoint;

    return CA_STATUS_OK;
}

int32_t CASendRAMulticastData(const CAEndpoint_t *endpoint,
                    const void *data, uint32_t dataLength)
{
    OIC_LOG(INFO, RA_ADAPTER_TAG, "RA adapter does not support sending multicast data");
    return 0;
}

CAResult_t CAStartRAListeningServer()
{
    OIC_LOG(INFO, RA_ADAPTER_TAG, "RA adapter does not support listening for multicast data");
    return CA_NOT_SUPPORTED;
}

CAResult_t CAStopRAListeningServer()
{
    OIC_LOG(INFO, RA_ADAPTER_TAG, "RA adapter does not support listening for multicast data");
    return CA_NOT_SUPPORTED;
}

CAResult_t CAStartRADiscoveryServer()
{
    OIC_LOG(INFO, RA_ADAPTER_TAG, "RA adapter does not support discovery of multicast servers");
    return CA_NOT_SUPPORTED;
}

CAResult_t CAReadRAData()
{
    OIC_LOG(INFO, RA_ADAPTER_TAG, "Read data is not implemented for the RA adapter");
    return CA_NOT_SUPPORTED;
}
