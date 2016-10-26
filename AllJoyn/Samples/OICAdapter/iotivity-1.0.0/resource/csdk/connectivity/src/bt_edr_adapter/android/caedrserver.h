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
 * This file contains the APIs for BT EDR communications.
 */
#ifndef CA_EDR_SERVER_H_
#define CA_EDR_SERVER_H_

#include <stdbool.h>

#include "cacommon.h"
#include "cathreadpool.h"
#include "jni.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*CAPacketReceiveCallback)(const char *address, const char *data);

/**
 * Initialize JNI object.
 */
void CAEDRServerJniInit();

/**
 * Initialize server for EDR.
 * @param[in]   handle           thread pool handle object.
 */
void CAEDRServerInitialize(ca_thread_pool_t handle);

/**
 * Start Accept Thread.
 */
void CAEDRServerStartAcceptThread();

/**
 * Start unicast server.
 * @param[in]   isSecured       unicast server type.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRStartUnicastServer(bool isSecured);

/**
 * Start multicast server.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRStartMulticastServer();

/**
 * Stop unicast server.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRStopUnicastServer();

/**
 * Stop multicast server.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRStopMulticastServer();

/**
 * This function will read the data from remote device.
 * @param[in]  env              JNI interface pointer.
 * @param[in]  id               index of remote address.
 * @param[in]  type             EDR server type.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRNativeReadData(JNIEnv *env, uint32_t id, CAAdapterServerType_t type);

/**
 * Start Listen Task.
 * @param[in]   env             JNI interface pointer.
 */
void CANativeStartListenTask(JNIEnv *env);

/**
 * This function will listen the connection from remote device.
 * @param[in]  env              JNI interface pointer.
 * @return server socket object or NULL.
 */
jobject CAEDRNativeListen(JNIEnv *env);

/**
 * This function will listen the connection from remote device.
 * @param[in]  env              JNI interface pointer.
 * @param[in]  socket           server socket object.
 * @return JNI_TRUE or JNI_FALSE.
 */
jboolean CAEDRIsConnectedForSocket(JNIEnv *env, jobject socket);

/**
 * This function will accept the connection from remote device.
 * @param[in]  env                  JNI interface pointer.
 * @param[in]  severSocketObject    server socket object.
 */
void CAEDRNativeAccept(JNIEnv *env, jobject severSocketObject);

/**
 * Remove all device objects in the list.
 * @param[in]   env    JNI interface pointer.
 */
void CAEDRNatvieCloseServerTask(JNIEnv* env);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_EDR_SERVER_H_ */
