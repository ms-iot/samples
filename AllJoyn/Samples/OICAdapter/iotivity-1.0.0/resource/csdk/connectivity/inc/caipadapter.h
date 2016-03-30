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
 * This file contains the APIs for IP Adapter.
 */
#ifndef CA_IP_ADAPTER_H_
#define CA_IP_ADAPTER_H_

#include "cacommon.h"
#include "caadapterinterface.h"
#include "cathreadpool.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * API to initialize IP Interface.
 * @param[in] registerCallback      Callback to register IP interfaces to
 *                                   Connectivity Abstraction Layer.
 * @param[in] networkPacketCallback Callback to notify request and
 *                                   response messages from server(s)
 *                                   started at Connectivity Abstraction Layer.
 * @param[in] netCallback           Callback to notify the network additions
 *                                   to Connectivity Abstraction Layer.
 * @param[in] errorCallback         Callback to notify the network errors to
 *                                   Connectivity Abstraction Layer.
 * @param[in] handle                Threadpool Handle.
 * @return  ::CA_STATUS_OK or Appropriate error code
 */
CAResult_t CAInitializeIP(CARegisterConnectivityCallback registerCallback,
                          CANetworkPacketReceivedCallback networkPacketCallback,
                          CANetworkChangeCallback netCallback,
                          CAErrorHandleCallback errorCallback, ca_thread_pool_t handle);

/**
 * Start IP Interface adapter.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAStartIP();

/**
 * Start listening server for receiving multicast search requests.
 * Transport Specific Behavior:
 * IP Starts Multicast Server on a particular interface and prefixed port
 * number and as per OIC Specification.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAStartIPListeningServer();

/**
 * Stop listening server from receiving multicast search requests.
 * Transport Specific Behavior:
 * IP closes open multicast socket for a particular interface.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAStopIPListeningServer();

/**
 * Start discovery servers for receiving multicast advertisements.
 * Transport Specific Behavior:
 * IP Starts multicast server on a particular interface and prefixed port
 * number as per OIC Specification.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAStartIPDiscoveryServer();

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
int32_t CASendIPUnicastData(const CAEndpoint_t *endpoint, const void *data,
                            uint32_t dataLen);

/**
 * Send Multicast data to the endpoint using the IP connectivity.
 * @param[in]   endpoint       Remote Endpoint information (like ipaddress,
 *                              port)
 * @param[in]   data           Data which is required to be sent.
 * @param[in]   dataLen        Size of data to be sent.
 * @note  dataLen must be > 0.
 * @return  The number of bytes sent on the network, or -1 upon error.
 */
int32_t CASendIPMulticastData(const CAEndpoint_t *endpoint, const void *data, uint32_t dataLen);

/**
 * Get IP Connectivity network information.
 * @param[out]   info        Local connectivity information structures.
 * @note info is allocated in this API and should be freed by the caller.
 * @param[out]   size        Number of local connectivity structures.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAGetIPInterfaceInformation(CAEndpoint_t **info, uint32_t *size);

/**
 * Read Synchronous API callback.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAReadIPData();

/**
 * Stops Unicast, Multicast servers and close the sockets.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAStopIP();

/**
 * Terminate the IP connectivity adapter.
 * Configuration information will be deleted from further use.
 */
void CATerminateIP();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // CA_IP_ADAPTER_H_
