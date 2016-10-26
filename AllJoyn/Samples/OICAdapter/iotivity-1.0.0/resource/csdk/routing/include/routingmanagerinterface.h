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
 * This file is an interface of routing manager and resource interospection layer.
 */
#ifndef ROUTING_MANAGER_INTERFACE_H_
#define ROUTING_MANAGER_INTERFACE_H_

#include "routingmanager.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Creates a Gateway resource and initializes the observer List.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMInitGatewayResource();

/**
 * Send multicast discover request for the Gateway resource.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMDiscoverGatewayResource();

/**
 * Send observe request for the gateway device hosting the Gateway resource.
 * @param[in]   devAddr   Device address of the Gateway device hosting
 *                        the gateway resource.
 * @param[in]   payload   Payload to be sent with the request.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMSendObserveRequest(const OCDevAddr *devAddr, OCRepPayload *payload);

/**
 * Send delete request to all the neighbor nodes.
 * @param[in]   devAddr   Device address of the Gateway device hosting
 *                        the gateway resource.
 * @param[in]   payload   Payload to be sent with the request.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMSendDeleteRequest(const OCDevAddr *devAddr, OCRepPayload *payload);

/**
 * Send a response for GET/OBSERVE request received for Gateway resource.
 * @param[in]   request     Request Received.
 * @param[in]   resource    Resource Handle.
 * @param[in]   payload     Payload containing Gateway Entries.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMSendResponse(const OCServerRequest *request, const OCResource *resource,
                             const OCRepPayload *payload);

/**
 * Send notification for list of observers except a particular observer.
 * @param[in]   obsId     Observer who shouldn't be sent notification.
 * @param[in]   obsLen    Length of Observer ID list.
 * @param[in]   payload   Payload containing Gateway Entries.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMSendNotificationForListofObservers(OCObservationId *obsId, uint8_t obsLen,
                                                   const OCRepPayload *payload);

/**
 * Adds an observer to the RI stack.
 * @param[in]   request     Request handle.
 * @param[out]  obsID       Observer ID generated for the requester.
 * @return  ::OC_STACK_OK or Appropriate error code.
 */
OCStackResult RMAddObserverToStack(const OCServerRequest *request, OCObservationId *obsID);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ROUTING_MANAGER_INTERFACE_H_ */
