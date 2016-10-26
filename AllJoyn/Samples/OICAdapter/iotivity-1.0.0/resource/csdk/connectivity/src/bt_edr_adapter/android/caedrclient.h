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
 * @brief This file contains the APIs for BT EDR communications.
 */
#ifndef CA_EDR_CLIENT_H_
#define CA_EDR_CLIENT_H_

#include <stdbool.h>

#include "cacommon.h"
#include "cathreadpool.h"
#include "jni.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initialize context of application.
 */
void CAEDRJniInitContext();

/**
 * Create JNI Object.
 * @param[in]  context          context of application.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRCreateJNIInterfaceObject(jobject context);

/**
 * Initialize client for EDR.
 * @param[in]  handle           thread pool handle object.
 */
void CAEDRInitialize(ca_thread_pool_t handle);

/**
 * Terminate server for EDR.
 */
void CAEDRTerminate();

/**
 * Initialize JNI object.
 */
void CAEDRCoreJniInit();

/**
 * Send data for unicast.
 * @param[in]  address         remote address.
 * @param[in]  data            data for transmission.
 * @param[in]  dataLen         data length.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAEDRSendUnicastMessage(const char *address, const uint8_t *data, uint32_t dataLen);

/**
 * Send data for multicast.
 * @param[in]  data            data for transmission.
 * @param[in]  dataLen         data length.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAEDRSendMulticastMessage(const uint8_t *data, uint32_t dataLen);

/**
 * Get Local EDR Address.
 * @param[out]   address         local address.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAEDRGetInterfaceInfo(char **address);

/**
 * Get address from a local device.
 * @param[out]   address         local address.
 */
void CAEDRGetLocalAddress(char **address);

/**
 * Send data for unicast (implement).
 * @param[in]  address         remote address.
 * @param[in]  data            data for transmission.
 * @param[in]  dataLen         data length.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAEDRSendUnicastMessageImpl(const char *address, const uint8_t *data, uint32_t dataLen);

/**
 * Send data for multicast (implement).
 * @param[in]  env             JNI interface pointer.
 * @param[in]  data            data for transmission.
 * @param[in]  dataLen         data length.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAEDRSendMulticastMessageImpl(JNIEnv *env, const uint8_t *data, uint32_t dataLen);

/**
 * EDR Method
 */

/**
 * This function will send the data to remote device.
 * @param[in] env              JNI interface pointer.
 * @param[in] address          Remote Address.
 * @param[in] data             Data to be transmitted from EDR.
 * @param[in] dataLength       Length of data.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRNativeSendData(JNIEnv *env, const char* address, const uint8_t* data,
                               uint32_t dataLength);

/**
 * This function will connect to remote device.
 * @param[in] env              JNI interface pointer.
 * @param[in] address          Remote Address.
 * @return ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAEDRNativeConnect(JNIEnv *env, const char *address);

/**
 * This function will close socket.
 * @param[in] env              JNI interface pointer.
 * @param[in] address          Remote Address.
 */
void CAEDRNativeSocketClose(JNIEnv *env, const char *address);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_EDR_CLIENT_H_ */

