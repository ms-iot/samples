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

/**
 * @file
 * This file contains the APIs for TCP Adapter.
 */
#ifndef CA_TCP_ADAPTER_H_
#define CA_TCP_ADAPTER_H_

#include "cacommon.h"
#include "caadapterinterface.h"
#include "cathreadpool.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * TCP Server Information for IPv4 TCP transport
 */
typedef struct
{
    char addr[MAX_ADDR_STR_SIZE_CA];    /**< TCP Server address */
    CASocket_t u4tcp;                   /**< TCP Server port */
} CATCPServerInfo_t;

/**
 * API to initialize TCP Interface.
 * @param[in] registerCallback      Callback to register TCP interfaces to
 *                                  Connectivity Abstraction Layer.
 * @param[in] networkPacketCallback Callback to notify request and
 *                                  response messages from server(s)
 *                                  started at Connectivity Abstraction Layer.
 * @param[in] netCallback           Callback to notify the network additions
 *                                  to Connectivity Abstraction Layer.
 * @param[in] errorCallback         Callback to notify the network errors to
 *                                  Connectivity Abstraction Layer.
 * @param[in] handle                Threadpool Handle.
 * @return  ::CA_STATUS_OK or Appropriate error code
 */
CAResult_t CAInitializeTCP(CARegisterConnectivityCallback registerCallback,
                           CANetworkPacketReceivedCallback networkPacketCallback,
                           CANetworkChangeCallback netCallback,
                           CAErrorHandleCallback errorCallback, ca_thread_pool_t handle);

/**
 * Start TCP Interface adapter.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAStartTCP();

/**
 * Start listening server for receiving connect requests.
 * Transport Specific Behavior:
 * TCP Starts Listening Server on a particular interface and prefixed port
 * number and as per OIC Specification.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAStartTCPListeningServer();

/**
 * Stops listening server from receiving connect requests.
 * Transport Specific Behavior:
 * TCP Stops Listening Server on a particular interface and prefixed port
 * number and as per OIC Specification.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAStopTCPListeningServer();

/**
 * Start discovery servers for receiving advertisements.
 * Transport Specific Behavior:
 * TCP Starts Discovery server on a particular interface and prefixed port
 * number as per OIC Specification.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAStartTCPDiscoveryServer();

/**
 * Sends data to the endpoint using the adapter connectivity.
 * @param[in]   endpoint       Remote Endpoint information (like ipaddress,
 *                              port, reference uri and transport type) to
 *                              which the unicast data has to be sent.
 * @param[in]   data           Data which is required to be sent.
 * @param[in]   dataLen        Size of data to be sent.
 * @note  dataLen must be > 0.
 * @return  The number of bytes sent on the network, or -1 upon error.
 */
int32_t CASendTCPUnicastData(const CAEndpoint_t *endpoint,
                             const void *data, uint32_t dataLen);

/**
 * Send Multicast data to the endpoint using the TCP connectivity.
 * @param[in]   endpoint       Remote Endpoint information (like ipaddress,
 *                              port)
 * @param[in]   data           Data which is required to be sent.
 * @param[in]   dataLen        Size of data to be sent.
 * @note  dataLen must be > 0.
 * @return  The number of bytes sent on the network, or -1 upon error.
 */
int32_t CASendTCPMulticastData(const CAEndpoint_t *endpoint,
                               const void *data, uint32_t dataLen);

/**
 * Get TCP Connectivity network information.
 * @param[out]   info        Local connectivity information structures.
 * @note info is allocated in this API and should be freed by the caller.
 * @param[out]   size        Number of local connectivity structures.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAGetTCPInterfaceInformation(CAEndpoint_t **info, uint32_t *size);

/**
 * Read Synchronous API callback.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAReadTCPData();

/**
 * Stops Unicast, servers and close the sockets.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAStopTCP();

/**
 * Terminate the TCP connectivity adapter.
 * Configuration information will be deleted from further use.
 */
void CATerminateTCP();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // CA_TCP_ADAPTER_H_
