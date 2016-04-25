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
 * This file contains the APIs for Resource Model to use.
 */

#ifndef CA_INTERFACE_H_
#define CA_INTERFACE_H_

/**
 * Connectivity Abstraction Interface APIs.
 */
#include "cacommon.h"
#include "casecurityinterface.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Callback function type for request delivery.
 * @param[out]   object       Endpoint object from which the request is received.
 *                            It contains endpoint address based on the connectivity type.
 * @param[out]   requestInfo  Info for resource model to understand about the request.
 */
typedef void (*CARequestCallback)(const CAEndpoint_t *object,
                                  const CARequestInfo_t *requestInfo);

/**
 * Callback function type for response delivery.
 * @param[out]   object           Endpoint object from which the response is received.
 * @param[out]   responseInfo     Identifier which needs to be mapped with response.
 */
typedef void (*CAResponseCallback)(const CAEndpoint_t *object,
                                   const CAResponseInfo_t *responseInfo);
/**
 * Callback function type for error.
 * @param[out]   object           remote device information.
 * @param[out]   errorInfo        CA Error information.
 */
typedef void (*CAErrorCallback)(const CAEndpoint_t *object,
                                const CAErrorInfo_t *errorInfo);

/**
 * Initialize the connectivity abstraction module.
 * It will initialize adapters, thread pool and other modules based on the platform
 * compilation options.
 *
 * @return  ::CA_STATUS_OK or ::CA_STATUS_FAILED or ::CA_MEMORY_ALLOC_FAILED
 */
CAResult_t CAInitialize();

/**
 * Terminate the connectivity abstraction module.
 * All threads, data structures are destroyed for next initializations.
 */
void CATerminate();

/**
 * Starts listening servers.
 * This API is used by resource hosting server for listening multicast requests.
 * Based on the adapters configurations, different kinds of servers are started.
 * @return  ::CA_STATUS_OK or ::CA_STATUS_FAILED
 */
CAResult_t CAStartListeningServer();

/**
 * Stops the server from receiving the multicast traffic. This is used by sleeping
 * device to not receives the multicast traffic.
 * @return  ::CA_STATUS_OK or ::CA_STATUS_FAILED
 */
CAResult_t CAStopListeningServer();

/**
 * Starts discovery servers.
 * This API is used by resource required clients for listening multicast requests.
 * Based on the adapters configurations, different kinds of servers are started.
 * @return  ::CA_STATUS_OK or ::CA_STATUS_FAILED
 */
CAResult_t CAStartDiscoveryServer();

/**
 * Register request callbacks and response callbacks.
 *          Requests and responses are delivered these callbacks.
 * @param[in]   ReqHandler    Request callback ( for GET,PUT ..etc).
 * @param[in]   RespHandler   Response Handler Callback.
 * @see     CARequestCallback
 * @see     CAResponseCallback
 * @see     CAErrorCallback
 */
void CARegisterHandler(CARequestCallback ReqHandler, CAResponseCallback RespHandler,
                       CAErrorCallback ErrorHandler);

/**
 * Create an endpoint description.
 * @param[in]   flags                 how the adapter should be used.
 * @param[in]   adapter               which adapter to use.
 * @param[in]   addr                  string representation of address.
 * @param[in]   port                  port (for IP_ADAPTER).
 * @param[in]   endpoint              Endpoint which contains the above.
 * @return  ::CA_STATUS_OK or ::CA_STATUS_FAILED
 * @remark  The created Remote endpoint can be freed using CADestroyEndpoint().
 * @see     CADestroyEndpoint
 */
CAResult_t CACreateEndpoint(CATransportFlags_t flags,
                            CATransportAdapter_t adapter,
                            const char *addr,
                            uint16_t port,
                            CAEndpoint_t **object);

/**
 * Destroy the remote endpoint created.
 * @param[in]   object   Remote Endpoint object created with CACreateEndpoint.
 */
void CADestroyEndpoint(CAEndpoint_t *object);

/**
 * Generating the token for matching the request and response.
 * @param[in]   token            Token for the request.
 * @param[in]   tokenLength      length of the token.
 * @return  ::CA_STATUS_OK or ::CA_STATUS_FAILED or
 *          ::CA_MEMORY_ALLOC_FAILED or ::CA_STATUS_NOT_INITIALIZED
 * @remark  Token memory is destroyed by the caller using CADestroyToken().
 * @see     CADestroyToken
 */
CAResult_t CAGenerateToken(CAToken_t *token, uint8_t tokenLength);

/**
 * Destroy the token generated by CAGenerateToken.
 * @param[in]   token    token to be freed.
 */
void CADestroyToken(CAToken_t token);

/**
 * Send control Request on a resource.
 * @param[in]   object       Endpoint where the payload need to be sent.
 *                           This endpoint is delivered with Request or response callback.
 * @param[in]   requestInfo  Information for the request.
 * @return  ::CA_STATUS_OK ::CA_STATUS_FAILED ::CA_MEMORY_ALLOC_FAILED
 */
CAResult_t CASendRequest(const CAEndpoint_t *object, const CARequestInfo_t *requestInfo);

/**
 * Send the response.
 * @param[in]   object           Endpoint where the payload need to be sent.
 *                               This endpoint is delivered with Request or response callback.
 * @param[in]   responseInfo     Information for the response.
 * @return  ::CA_STATUS_OK or  ::CA_STATUS_FAILED or ::CA_MEMORY_ALLOC_FAILED
 */
CAResult_t CASendResponse(const CAEndpoint_t *object, const CAResponseInfo_t *responseInfo);

/**
 * Select network to use.
 * @param[in]   interestedNetwork    Connectivity Type enum.
 * @return  ::CA_STATUS_OK or ::CA_NOT_SUPPORTED or
 *          ::CA_STATUS_FAILED or ::CA_NOT_SUPPORTED
 */
CAResult_t CASelectNetwork(CATransportAdapter_t interestedNetwork);

/**
 * Select network to unuse.
 * @param[in]   nonInterestedNetwork     Connectivity Type enum.
 * @return  ::CA_STATUS_OK or ::CA_NOT_SUPPORTED or ::CA_STATUS_FAILED
 */
CAResult_t CAUnSelectNetwork(CATransportAdapter_t nonInterestedNetwork);

/**
 * Get network information.
 * It should be destroyed by the caller as it Get Information.
 * @param[out]   info     LocalConnectivity objects
 * @param[out]   size     No Of Array objects
 * @return  ::CA_STATUS_OK or ::CA_STATUS_FAILED or
 *          ::CA_STATUS_INVALID_PARAM or ::CA_MEMORY_ALLOC_FAILED
 */
CAResult_t CAGetNetworkInformation(CAEndpoint_t **info, uint32_t *size);

/**
 * To Handle the Request or Response.
 * @return   ::CA_STATUS_OK
 */
CAResult_t CAHandleRequestResponse();

#ifdef RA_ADAPTER
/**
 * Set Remote Access information for XMPP Client.
 * @param[in]   caraInfo          remote access info.
 *
 * @return  ::CA_STATUS_OK
 */
CAResult_t CASetRAInfo(const CARAInfo_t *caraInfo);
#endif



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_INTERFACE_H_ */

