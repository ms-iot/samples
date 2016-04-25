/* *****************************************************************
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
 *
 * This file contains the APIs for adapters to be implemented.
 */

#ifndef CA_ADAPTER_INTERFACE_H_
#define CA_ADAPTER_INTERFACE_H_

#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Starting connectivity adapters and each adapter have transport specific behavior.
 * Transport Specific Behavior:
 * WIFI/ETH connectivity Starts unicast server on  all available IPs and defined
 * port number as per specification.
 * EDR will not start any specific servers.
 * LE will not start any specific servers.
 * @return ::CA_STATUS_OK or ::CA_STATUS_FAILED
 *  ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
typedef CAResult_t (*CAAdapterStart)();

/**
 * Starting listening server for receiving multicast search requests
 * Transport Specific Behavior:
 * WIFI/ETH Starts multicast server on  all available IPs and defined
 * port number and as per specification.
 * EDR  Starts RFCOMM Server with prefixed UUID as per specification.
 * LE Start GATT Server with prefixed UUID and Characteristics as per OIC Specification.
 * @return ::CA_STATUS_OK or ::CA_STATUS_FAILED
 * ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
typedef CAResult_t (*CAAdapterStartListeningServer)();

/**
 * Stopping listening server to not receive multicast search requests
 * Transport Specific Behavior:
 * WIFI/ETH Stops multicast server on  all available IPs. This is required for the
 * thin device that call this function once all local resources are pushed to the
 * resource directory.
 * @return ::CA_STATUS_OK or ::CA_STATUS_FAILED
 * ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
typedef CAResult_t (*CAAdapterStopListeningServer)();

/**
 * for starting discovery servers for receiving multicast advertisements
 * Transport Specific Behavior:
 * WIFI/ETH Starts multicast server on all available IPs and defined port
 * number as per OIC Specification.
 * EDR Starts RFCOMM Server with prefixed UUID as per OIC Specification.
 * LE Starts GATT Server with prefixed UUID and Characteristics as per OIC Specification.
 * @return ::CA_STATUS_OK or ::CA_STATUS_FAILED
 * ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
typedef CAResult_t (*CAAdapterStartDiscoveryServer)();

/**
 * Sends data to the endpoint using the adapter connectivity.
 * Note: length must be > 0.
 * @param[in]   endpoint        Remote Endpoint information (like ipaddress , port,
 * reference uri and connectivity type) to which the unicast data has to be sent.
 * @param[in]   data            Data which required to be sent.
 * @param[in]   dataLen         Size of data to be sent.
 * @return The number of bytes sent on the network. Return value equal to -1 indicates error.
 */
typedef int32_t (*CAAdapterSendUnicastData)(const CAEndpoint_t *endpoint,
                                            const void *data, uint32_t dataLen);

/**
 * Sends Multicast data to the endpoint using the adapter connectivity.
 * Note: length must be > 0.
 * @param[in]   endpoint        Remote Endpoint information (like ipaddress , port,
 * @param[in]   data            Data which required to be sent.
 * @param[in]   dataLen         Size of data to be sent.
 * @return The number of bytes sent on the network. Return value equal to -1 indicates error.
 */
typedef int32_t (*CAAdapterSendMulticastData)(const CAEndpoint_t *endpoint,
        const void *data, uint32_t dataLen);

/**
 * Get Network Information.
 * @param[out]   info           Local connectivity information structures
 * @param[out]   size           Number of local connectivity structures.
 * @return ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h)
 */
typedef CAResult_t (*CAAdapterGetNetworkInfo)(CAEndpoint_t **info, uint32_t *size);

/**
 * Read Synchronous API callback.
 * @return ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h)
 */
typedef CAResult_t (*CAAdapterReadData)();

/**
 * Stopping the adapters and close socket connections.
 * Transport Specific Behavior:
 * WIFI/ETH Stops all listening servers and close sockets.
 * EDR Stops all RFCOMM servers and close sockets.
 * LE Stops all GATT servers and close sockets.
 * @return CA_STATUS_OK or ERROR CODES ( CAResult_t error codes in cacommon.h)
 */
typedef CAResult_t (*CAAdapterStop)();

/**
 * Terminate the connectivity adapter.Configuration information will be deleted from
 * further use. Freeing Memory of threadpool and mutexs and cleanup will be done.
 */
typedef void (*CAAdapterTerminate)();

/**
 * Connectivity handler information for adapter.
 */
typedef struct
{
    /** Start Transport specific functions. */
    CAAdapterStart startAdapter;

    /** Listening Server function address. */
    CAAdapterStartListeningServer startListenServer;

    /** Stops receiving the multicast traffic. */
    CAAdapterStopListeningServer stopListenServer;

    /** Discovery Server function address. **/
    CAAdapterStartDiscoveryServer startDiscoveryServer;

    /** Unicast data function address. **/
    CAAdapterSendUnicastData sendData;

    /** Multicast data function address. **/
    CAAdapterSendMulticastData sendDataToAll;

    /** Get Networking information. **/
    CAAdapterGetNetworkInfo GetnetInfo;

    /** Read Data function address. **/
    CAAdapterReadData readData;

    /** Stop Transport specific functions. */
    CAAdapterStop stopAdapter;

    /** Terminate function address stored in this pointer. **/
    CAAdapterTerminate terminate;

} CAConnectivityHandler_t;

/**
 * This will be used during the registration of adapters call backs to the common logic.
 * @see ::CAConnectivityHandler_t , ::CATransportAdapter_t
 */
typedef void (*CARegisterConnectivityCallback)(CAConnectivityHandler_t handler,
        CATransportAdapter_t cType);

/**
 * This will be used during the receive of network requests and response.
 * @see SendUnicastData(), SendMulticastData()
 */
typedef void (*CANetworkPacketReceivedCallback)(const CASecureEndpoint_t *sep,
                                            const void *data, uint32_t dataLen);

/**
 * This will be used to notify network changes to the connectivity common logic layer.
 * @see SendUnicastData(), SendMulticastData()
 */
typedef void (*CANetworkChangeCallback)(const CAEndpoint_t *info, CANetworkStatus_t status);

/**
 * This will be used to notify error result to the connectivity common logic layer.
 */
typedef void (*CAErrorHandleCallback)(const CAEndpoint_t *endpoint,
                                      const void *data, uint32_t dataLen,
                                      CAResult_t result);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* CA_ADAPTER_INTERFACE_H_ */

