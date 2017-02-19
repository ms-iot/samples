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
 * This file provides APIs for EDR adapter - client, server, network monitor
 * modules.
 */

#ifndef CA_EDR_INTERFACE_H_
#define CA_EDR_INTERFACE_H_

#include "caedradapter.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef OIC_EDR_SERVICE_ID
#define OIC_EDR_SERVICE_ID "12341234-1C25-481F-9DFB-59193D238280"
#endif //OIC_EDR_SERVICE_ID

typedef enum
{
    STATE_DISCONNECTED, /**< State is Disconnected. */
    STATE_CONNECTED     /**< State is Connected. */
} CAConnectedState_t;

typedef struct connected_state
{
    uint8_t address[CA_MACADDR_SIZE];
    CAConnectedState_t state;
} state_t;

/**
 * Enum for defining different server types.
 */
typedef enum
{
    CA_UNICAST_SERVER = 0,    /**< Unicast Server. */
    CA_MULTICAST_SERVER,      /**< Multicast Server. */
    CA_SECURED_UNICAST_SERVER /**< Secured Unicast Server. */
} CAAdapterServerType_t;

/**
 * Structure to maintain the information of data in message queue.
 */
typedef struct
{
    CAEndpoint_t *remoteEndpoint;       /**< Remote Endpoint. */
    uint8_t *data;                      /**< Data to be sent. */
    uint32_t dataLen;                   /**< Length of the data to be sent. */
} CAEDRData;

/**
 * Structure to maintain the adapter information and its status.
 */
typedef struct
{
    CAEndpoint_t *info;          /**< Local Connectivity Information. */
    CANetworkStatus_t status;    /**< Network Status. */
} CAEDRNetworkEvent;

/**
 * This will be used during the Receiver of network requests and response.
 * @param[in] remoteAddress EDR address of remote OIC device from which data received.
 * @param[in] data          Data received.
 * @param[in] dataLength    Length of the Data received.
 * @param[out] sentLength    Length of the sent data.
 * @pre Callback must be registered using CAEDRSetPacketReceivedCallback().
 */
typedef void (*CAEDRDataReceivedCallback)(const char *remoteAddress, const uint8_t *data,
                                          uint32_t dataLength, uint32_t *sentLength);

/**
 * This will be used during change in network status.
 * @param[in] status        Network Status of the adapter.
 */
typedef void (*CAEDRNetworkStatusCallback)(CANetworkStatus_t status);

/**
 * Callback to notify the error in the EDR adapter.
 * @param[in]  remoteAddress   Remote EDR Address.
 * @param[in]  data            data containing token, uri and coap data.
 * @param[in]  dataLength      length of data.
 * @param[in]  result          error code as defined in ::CAResult_t.
 * @pre Callback must be registered using CAEDRSetPacketReceivedCallback().
 */
typedef void (*CAEDRErrorHandleCallback)(const char *remoteAddress,
                                         const uint8_t *data,
                                         uint32_t dataLength,
                                         CAResult_t result);

/**
 * Initialize the network monitor module
 * @param[in]  threadPool   Threadpool Handle.
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_ADAPTER_NOT_ENABLED Initialization is successful, but
 * bluetooth adapter is not enabled.
 * @retval ::CA_STATUS_FAILED Operation failed.
 * @see  CAEDRTerminateNetworkMonitor().
 */
CAResult_t CAEDRInitializeNetworkMonitor(const ca_thread_pool_t threadPool);

/**
 * Deinitialize with bluetooth adapter.
 * @pre    CAEDRInitializeNetworkMonitor() should be invoked before using
 * this API.
 * @see    CAEDRInitializeNetworkMonitor().
 */
void CAEDRTerminateNetworkMonitor();

/**
 * Start Network Monitoring Process.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRStartNetworkMonitor();

/**
 * Stop Network Monitoring Process.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRStopNetworkMonitor();

/**
 * Sets the callback and Starts discovery for nearby OIC bluetooth devices.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAEDRClientSetCallbacks();

/**
 * Resetting callbacks with bluetooth framework and stop OIC device discovery.
 * @pre    CAEDRClientSetCallbacks() should be invoked before using this API.
 * @see    CAEDRClientSetCallbacks().
 */
void CAEDRClientUnsetCallbacks();

/**
 * Used to initialize the EDR client module where mutex is initialized.
 */
void CAEDRInitializeClient(ca_thread_pool_t handle);

/**
 * Destroys the Device list and mutex.
 */
void CAEDRClientTerminate();

/**
 * Closes all the client connection to peer bluetooth devices.
 */
void CAEDRClientDisconnectAll();

/**
 * Register callback to send the received packets from remote bluetooth
 * device to BTAdapter.
 *
 * @param[in]  packetReceivedCallback Callback function to register for
 * sending network packets to EDR Adapter.
 */
void CAEDRSetPacketReceivedCallback(CAEDRDataReceivedCallback packetReceivedCallback);

/**
 * Register callback for receiving local bluetooth adapter state.
 *
 * @param[in]  networkStateChangeCallback Callback function to register
 * for receiving local bluetooth adapter status.
 */
void CAEDRSetNetworkChangeCallback(CAEDRNetworkStatusCallback networkStateChangeCallback);

/**
 * set error callback to notify error in EDR adapter.
 *
 * @param[in]  errorHandleCallback Callback function to notify the error
 * in the EDR adapter.
 */
void CAEDRSetErrorHandler(CAEDRErrorHandleCallback errorHandleCallback);


/**
 * Get the local bluetooth adapter information.
 *
 * @param[out]  info Local bluetooth adapter information.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 *
 * @see CALocalConnectivity_t
 *
 */
CAResult_t CAEDRGetInterfaceInformation(CAEndpoint_t **info);

/**
 * Start RFCOMM server for given service UUID
 *
 * @param[in]  handle       Threadpool Handle.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 *
 */
CAResult_t CAEDRServerStart(ca_thread_pool_t handle);

/**
 * Stop RFCOMM server
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAEDRServerStop();

/**
 * Terminate server for EDR.
 */
void CAEDRServerTerminate();

/**
 * All received data will be notified to upper layer.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_FAILED Operation failed.
 *
 */
CAResult_t CAEDRManagerReadData();

/**
 * This function gets bluetooth adapter enable state.
 * @param[out]  state    State of the Adapter.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRGetAdapterEnableState(bool *state);

/**
 * This function sends data to specified remote bluetooth device.
 * @param[in]  remoteAddress   Remote EDR Address.
 * @param[in]  data            Data to be sent.
 * @param[in]  dataLength      Length of the data to be sent.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRClientSendUnicastData(const char *remoteAddress,
                                      const uint8_t *data,
                                      uint32_t dataLength);

/**
 * This function sends data to all bluetooth devices running OIC service.
 * @param[in]  data            Data to be sent.
 * @param[in]  dataLength      Length of the data to be sent.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRClientSendMulticastData(const uint8_t *data,
                                        uint32_t dataLength);

/**
 * This function gets bonded bluetooth device list
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRGetBondedDeviceList();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_EDR_INTERFACE_H_ */
