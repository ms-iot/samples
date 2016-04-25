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
 * This file contains the APIs for EDR adapters to be implemented.
 */

#ifndef CA_EDRADAPTER_SINGLETHREAD_H_
#define CA_EDRADAPTER_SINGLETHREAD_H_

/**
 * EDR Interface AP
 **/
#include "cacommon.h"
#include "caadapterinterface.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief  Initialize EDR Interface.
 * @param  registerCallback  [IN] Callback to register EDR interface to Connectivity
 *                                Abstraction Layer
 * @param  reqRespCallback   [IN] Callback to notify request and response messages from server(s)
 *                                started at Connectivity Abstraction Layer.
 * @param  netCallback       [IN] Callback to notify the network additions to Connectivity
 *                                Abstraction Layer.
 * @return #CA_STATUS_OK or Appropriate error code
 * @retval #CA_STATUS_OK Successful
 * @retval #CA_STATUS_INVALID_PARAM Invalid input parameters
 * @retval #CA_ADAPTER_NOT_ENABLED Initialization is successful, but bluetooth adapter is not
 *                                 enabled
 * @retval #CA_STATUS_FAILED Operation failed
 */
CAResult_t CAInitializeEDR(CARegisterConnectivityCallback registerCallback,
                           CANetworkPacketReceivedCallback reqRespCallback,
                           CANetworkChangeCallback netCallback);

/**
 * @brief  Starting EDR connectivity adapters. As its peer to peer it doesnot require to start
 *         any servers.
 * @return #CA_STATUS_OK or Appropriate error code
 * @retval #CA_STATUS_OK  Successful
 * @retval #CA_ADAPTER_NOT_ENABLED Bluetooth adapter is not enabled
 * @retval #CA_STATUS_FAILED Operation failed
 */
CAResult_t CAStartEDR();

/**
 * @brief  Starts listening server for receiving multicast search requests.
 *         Starts RFCOMM Server with prefixed UUID as per OIC specification.
 * @return #CA_STATUS_OK or Appropriate error code
 * @retval #CA_STATUS_OK  Successful
 * @retval #CA_SERVER_STARTED_ALREADY  Server is already started and running for the predefined
 *                                     service UUID
 * @retval #CA_STATUS_FAILED Operation failed
 */
CAResult_t CAStartEDRListeningServer();

/**
 * @brief  Stops listening server for receiving multicast search requests.
 *
 * @return #CA_STATUS_OK or Appropriate error code
 */
CAResult_t CAStopEDRListeningServer();

/**
 * @brief  Starts discovery server for receiving multicast advertisements.
 *         Starts RFCOMM Server with prefixed UUID as per OIC specification.
 * @return #CA_STATUS_OK or Appropriate error code
 * @retval #CA_STATUS_OK  Successful
 * @retval #CA_SERVER_STARTED_ALREADY Server is already started and running for the predefined
 *                                    service UUID
 * @retval #CA_STATUS_FAILED Operation failed
 */
CAResult_t CAStartEDRDiscoveryServer();

/**
 * @brief  Sends data to the peer bluetooth OIC device using the adapter connectivity.
 * @param  endpoint        [IN] Remote Endpoint information (like ipaddress, port, reference uri and
 *                              connectivity type) to which the unicast data has to be sent.
 * @param  data            [IN] Data to be sent.
 * @param  dataLength      [IN] Size of data to be sent.
 * @return Number of bytes sent on the network. Returns -1 on error.
 */
int32_t CASendEDRUnicastData(const CAEndpoint_t *remoteEndpoint, const void *data,
                              uint32_t dataLength);

/**
 * @brief  Sends multicast data to all discovered bluetooth OIC devices using the adapter
 *         connectivity.
 * @param  data         [IN]  Data which needs to be sent to all discovered bluetooth OIC device.
 * @param  dataLength   [IN]  Length of data in bytes.
 * @return Number of bytes sent on the network. Returns -1 on error.
 */
int32_t CASendEDRMulticastData(const void *data, uint32_t dataLength);

/**
 * @brief  Get EDR Connectivity network information.
 *
 * @param  info [OUT] Array of local connectivity information structures.
 * @param  size [OUT] Size of the array @info.
 *
 * @return #CA_STATUS_OK or Appropriate error code
 * @retval #CA_STATUS_OK  Successful
 * @retval #CA_STATUS_INVALID_PARAM  Invalid input parameters
 * @retval #CA_MEMORY_ALLOC_FAILED  Failed to allocate memory
 * @retval #CA_STATUS_FAILED Operation failed
 * @remarks info is allocated in this API and should be freed by the caller.
 */
CAResult_t CAGetEDRInterfaceInformation(CAEndpoint_t **info, uint32_t *size);

/**
 * @brief  Read Synchronous API callback.
 * @return #CA_STATUS_OK on success otherwise proper error code.
 * @retval #CA_STATUS_OK  Successful
 * @retval #CA_STATUS_FAILED Operation failed
 */
CAResult_t CAReadEDRData();

/**
 * @brief  EDR Stops all RFCOMM servers and close sockets.
 * @return #CA_STATUS_OK or Appropriate error code
 */
CAResult_t CAStopEDR();

/**
 * @brief  Terminate the EDR connectivity adapter.
 * Configuration information will be deleted from further use.
 * @return NONE
 */
void CATerminateEDR();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* CA_EDRADAPTER_SINGLETHREAD_H_ */

