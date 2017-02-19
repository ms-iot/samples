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
 * This file contains the APIs for EDR adapters.
 */

#ifndef CA_EDRADAPTER_H_
#define CA_EDRADAPTER_H_

/**
 * EDR Interface AP.
 **/
#include "cacommon.h"
#include "caadapterinterface.h"
#include "cathreadpool.h" /* for thread pool */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initialize EDR Interface.
 * @param[in]   registerCallback   Callback to register EDR interface to Connectivity
 *                                 Abstraction Layer.
 * @param[in]   reqRespCallback    Callback to notify request and response messages from
 *                                 server(s) started at Connectivity Abstraction Layer.
 * @param[in]   netCallback        Callback to notify the network additions to Connectivity
 *                                 Abstraction Layer.
 * @param[in]   errorCallback      errorCallback to notify error to connectivity common logic
 *                                 layer from adapter.
 * @param[in]   handle             Threadpool Handle.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAInitializeEDR(CARegisterConnectivityCallback registerCallback,
                           CANetworkPacketReceivedCallback reqRespCallback,
                           CANetworkChangeCallback netCallback,
                           CAErrorHandleCallback errorCallback, ca_thread_pool_t handle);

/**
 * Starts EDR connectivity adapters.
 * As its peer to peer it does not require to start any servers.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAStartEDR();

/**
 * Starts listening server for receiving multicast search requests.
 * Starts  RFCOMM Server with prefixed UUID as per OIC specification.
 *
 * @return ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAStartEDRListeningServer();

/**
 * Stop listening server for receiving multicast search requests.
 * Stop  RFCOMM Server with prefixed UUID as per OIC specification.
 *
 * @return ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAStopEDRListeningServer();

/**
 * Starting discovery server for receiving multicast advertisements.
 * Starts  RFCOMM Server with prefixed UUID as per OIC specification.
 *
 * @return ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAStartEDRDiscoveryServer();

/**
 * Sends data to the peer bluetooth OIC device using the adapter connectivity.
 * @param[in]  endpoint         Remote Endpoint information (like ipaddress, port, and
 *                              connectivity type) to which the unicast data has to be sent.
 * @param[in]  data             Data to be sent.
 * @param[in]  dataLength       Size of data to be sent.
 * @return The number of bytes sent on the network. Returns -1 on error.
 *
 */
int32_t CASendEDRUnicastData(const CAEndpoint_t *endpoint, const void *data,
                             uint32_t dataLength);

/**
 * Sends multicast data to all discovered bluetooth OIC devices using the adapter.
 * @param[in]  endpoint      Remote Endpoint information (like ipaddress, port, and connectivity.
 * @param[in]  data          Data which needs to be sent to all discovered bluetooth OIC device.
 * @param[in]  dataLength    Length of data in bytes.
 * @return Number of bytes sent on the network. Returns -1 on error.
 */
int32_t CASendEDRMulticastData(const CAEndpoint_t *endpoint, const void *data,
                               uint32_t dataLength);

/**
 * Get EDR Connectivity network information.
 *
 * @param[out]  info    Array of local connectivity information structures.
 * @param[out]  size    Size of the array @info.
 *
 * @return ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAGetEDRInterfaceInformation(CAEndpoint_t **info, uint32_t *size);

/**
 * Read Synchronous API callback.
 * @return ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAReadEDRData();

/**
 * EDR Stops all RFCOMM servers and close sockets.
 * @return ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAStopEDR();

/**
 * Terminate the EDR connectivity adapter.
 * Configuration information will be deleted from further use.
 */
void CATerminateEDR();

/**
 * Initializes the adapter queues.
 * This will initiates both server and receiver adapter queues.
 * @return ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAAdapterStartQueue();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* CA_EDRADAPTER_H_ */

