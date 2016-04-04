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


#ifndef CA_LEADAPTER_H_
#define CA_LEADAPTER_H_

#include "cacommon.h"
#include "caadapterinterface.h"
#include "cathreadpool.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Initialize LE connectivity interface.
 *
 * @param[in]  registerCallback Callback to register LE interfaces to
 *                              Connectivity Abstraction Layer.
 * @param[in]  reqRespCallback  Callback to notify request and response
 *                              messages from server(s) started at
 *                              Connectivity Abstraction Layer.
 * @param[in]  netCallback      Callback to notify the network additions
 *                              to Connectivity Abstraction Layer.
 * @param[in]  errorCallback    errorCallback to notify error to
 *                              connectivity common logic layer from adapter.
 * @param[in]  handle           Threadpool Handle.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAInitializeLE(CARegisterConnectivityCallback registerCallback,
                          CANetworkPacketReceivedCallback reqRespCallback,
                          CANetworkChangeCallback netCallback,
                          CAErrorHandleCallback errorCallback,
                          ca_thread_pool_t handle);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_LEADAPTER_H_ */
