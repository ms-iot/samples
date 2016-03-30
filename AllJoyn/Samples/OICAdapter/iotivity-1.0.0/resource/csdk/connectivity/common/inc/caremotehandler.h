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
 * This file contains common utility function for remote endpoints.
 */

#ifndef CA_REMOTE_HANDLER_H_
#define CA_REMOTE_HANDLER_H_

#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Creates a new remote endpoint from the input endpoint.
 * @param[in]   endpoint           endpoint information where the data has to be sent.
 * @return  remote endpoint created.
 */
CAEndpoint_t *CACloneEndpoint(const CAEndpoint_t *endpoint);

/**
 * Allocate CAEndpoint_t instance.
 * @param[in]   flags          Transport flag.
 * @param[in]   adapter        Adapter type.
 * @param[in]   address        Address.
 * @param[in]   port           Port.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAEndpoint_t *CACreateEndpointObject(CATransportFlags_t flags, CATransportAdapter_t adapter,
                                     const char *address, uint16_t port);
/**
 * Destroy remote endpoint.
 * @param[in]   endpoint           endpoint information where the data has to be sent.
 */
void CAFreeEndpoint(CAEndpoint_t *rep);

/**
 * duplicates the given info.
 * @param[in]   info    info object to be duplicated.
 * @param[out]  clone   info object to be modified.
 * @return      ::CA_STATUS_OK or Appropriate error code if fail to clone.
 */
CAResult_t CACloneInfo(const CAInfo_t *info, CAInfo_t *clone);

/**
 * Creates a new request information.
 * @param[in]   request           request information that needs to be duplicated.
 * @return  duplicated request info object.
 */
CARequestInfo_t *CACloneRequestInfo(const CARequestInfo_t *request);

/**
 * Destroy the request information.
 * @param[in]   request           request information that needs to be destroyed.
 */
void CADestroyRequestInfoInternal(CARequestInfo_t *request);

/**
 * Creates a new response information.
 * @param[in]   response           response information that needs to be duplicated.
 * @return  duplicated response info object.
 */
CAResponseInfo_t *CACloneResponseInfo(const CAResponseInfo_t *response);

/**
 * Destroy the response information.
 * @param[in]   response           response information that needs to be destroyed.
 */
void CADestroyResponseInfoInternal(CAResponseInfo_t *response);

/**
 * Free the error information.
 * @param[in]   errorInfo           error information to be freed.
 */
void CADestroyErrorInfoInternal(CAErrorInfo_t *errorInfo);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_REMOTE_HANDLER_H_ */

