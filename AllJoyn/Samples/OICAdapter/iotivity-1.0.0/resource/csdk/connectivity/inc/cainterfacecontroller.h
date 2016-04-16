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
 * This file contains common utility function for CA transport adaptors.
 */

#ifndef CA_INTERFACE_CONTROLLER_H_
#define CA_INTERFACE_CONTROLLER_H_

#include "caadapterinterface.h"

#ifndef SINGLE_THREAD
#include "cathreadpool.h" /* for thread pool */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef SINGLE_THREAD
/**
 * Initializes different adapters based on the compilation flags.
 */
void CAInitializeAdapters();
#else
/**
 * Initializes different adapters based on the compilation flags.
 * @param[in]   handle           thread pool handle created by message handler
 *                               for different adapters.
 */
void CAInitializeAdapters(ca_thread_pool_t handle);
#endif

/**
 * Set the received packets callback for message handler.
 * @param[in]   callback         message handler callback to receive packets
 *                               from different adapters.
 */
void CASetPacketReceivedCallback(CANetworkPacketReceivedCallback callback);

/**
 * Set the error handler callback for message handler.
 * @param[in]   errorCallback    error handler callback from adapters
 */
void CASetErrorHandleCallback(CAErrorHandleCallback errorCallback);

/**
 * Set the network status changed callback for message handler.
 * @param[in]   callback         message handler network status callback
 *                               to receive network changes.
 */
void CASetNetworkChangeCallback(CANetworkChangeCallback callback);

/**
 * Starting different connectivity adapters based on the network selection.
 * @param[in]   transportType    interested network for starting.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAStartAdapter(CATransportAdapter_t transportType);

/**
 * Stopping different connectivity adapters based on the network un-selection.
 * @param[in]   transportType    network type that want to stop.
 */
void CAStopAdapter(CATransportAdapter_t transportType);

#ifdef RA_ADAPTER
/**
 * Set Remote Access information for XMPP Client.
 * @param[in]   caraInfo         remote access info..
 *
 * @return  CA_STATUS_OK
 */
CAResult_t CASetAdapterRAInfo(const CARAInfo_t *caraInfo);
#endif

/**
 * Get network information such as ipaddress and mac information.
 * @param[out]   info           connectivity information
 *                                  such as ipaddress and mac information.
 * @param[out]   size           number of connectivity information structures.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAGetNetworkInfo(CAEndpoint_t **info, uint32_t *size);

/**
 * Sends unicast data to the remote endpoint.
 * @param[in]   endpoint       endpoint information where the data has to be sent.
 * @param[in]   data           data that needs to be sent.
 * @param[in]   length         length of the data that needs to be sent.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CASendUnicastData(const CAEndpoint_t *endpoint, const void *data, uint32_t length);

/**
 * Sends multicast data to all endpoints in the network.
 * @param[in]   endpoint       endpoint information where the data has to be sent.
 * @param[in]   data           data that needs to be sent.
 * @param[in]   length         length of the data that needs to be sent.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */

CAResult_t CASendMulticastData(const CAEndpoint_t *endpoint, const void *data, uint32_t length);

/**
 * Start listening servers to receive search requests from clients.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAStartListeningServerAdapters();

/**
 * Stop listening servers to receive search requests from clients.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAStopListeningServerAdapters();

/**
 * Start discovery servers to receive advertisements from server.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAStartDiscoveryServerAdapters();

/**
 * Terminates the adapters which are initialized during the initialization.
 */
void CATerminateAdapters();

#ifdef SINGLE_THREAD
/**
 * Checks for available data and reads it.
 * @return   ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CAReadData();
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_INTERFACE_CONTROLLER_H_ */

