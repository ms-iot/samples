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
 * This file contains the APIs for BT communications.
 */
#ifndef CA_EDR_UTILS_H_
#define CA_EDR_UTILS_H_

#include "cacommon.h"
#include "cathreadpool.h"
#include "uarraylist.h"
#include "caedrinterface.h"
#include "jni.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Get address from device socket.
 * @param[in]   env                 JNI interface pointer.
 * @param[in]   bluetoothSocketObj  bluetooth socket object.
 * @return  Bluetooth device address.
 */
jstring CAEDRNativeGetAddressFromDeviceSocket(JNIEnv *env, jobject bluetoothSocketObj);

/**
 * Get local device address.
 * @param[in]   env                 JNI interface pointer.
 * @return  Bluetooth device address.
 */
jstring CAEDRNativeGetLocalDeviceAddress(JNIEnv *env);

/**
 * Get bonded devices list.
 * @param[in]   env                 JNI interface pointer.
 * @return  Bonded devices list.
 */
jobjectArray CAEDRNativeGetBondedDevices(JNIEnv *env);

/**
 * Get Bluetooth device state.
 * @param[in]   env                 JNI interface pointer.
 * @return  Bluetooth device state.
 */
jint CAEDRNativeGetBTStateOnInfo(JNIEnv *env);

/**
 * Check the BT Adapter enable.
 * @param[in]   env                 JNI interface pointer.
 * @return  JNI_TRUE or JNI_FALSE.
 */
jboolean CAEDRNativeIsEnableBTAdapter(JNIEnv *env);

/**
 * Get address from BT device.
 * @param[in]   env                 JNI interface pointer.
 * @param[in]   bluetoothDevice     bluetooth socket object.
 * @return  Bluetooth device address.
 */
jstring CAEDRNativeGetAddressFromBTDevice(JNIEnv *env, jobject bluetoothDevice);

/**
 * This function will create the device state list.
 */
void CAEDRNativeCreateDeviceStateList();

/**
 * Update connection state of device.
 * @param[in]  state            connection state.
 * @param[in]  address          remote address.
 */
void CAEDRUpdateDeviceState(CAConnectedState_t state, const char *address);

/**
 * Add device object to the list.
 * @param[in]  state            connection state object.
 */
void CAEDRNativeAddDeviceStateToList(state_t *state);

/**
 * Check whether the device exist in the list or not.
 * @param[in]  remoteAddress    remote address.
 * @return true or false.
 */
bool CAEDRNativeIsDeviceInList(const char *remoteAddress);

/**
 * Close all socket.
 * @param[in]   env       JNI interface pointer.
 */
void CAEDRNativeSocketCloseToAll(JNIEnv *env);

/**
 * Remove all device objects in the list.
 */
void CAEDRNativeRemoveAllDeviceState();

/**
 * Remove target device in the list.
 * @param[in]   remoteAddress    remote address.
 */
void CAEDRNativeRemoveDevice(const char *remoteAddress);

/**
 * Get current device connection state.
 * @param[in]   remoteAddress    remote address.
 * @return  STATE_DISCONNECTED or STATE_CONNECTED.
 */
CAConnectedState_t CAEDRIsConnectedDevice(const char *remoteAddress);

/**
 * This function will create the device socket list.
 */
void CAEDRNativeCreateDeviceSocketList();

/**
 * Add device object to the list.
 * @param[in]  env              JNI interface pointer.
 * @param[in]  deviceSocket     device socket object.
 */
void CAEDRNativeAddDeviceSocketToList(JNIEnv *env, jobject deviceSocket);

/**
 * Add device object to the list.
 * @param[in]  env              JNI interface pointer.
 * @param[in]  remoteAddress    remote address.
 * @return true or false.
 */
bool CAEDRNativeIsDeviceSocketInList(JNIEnv *env, const char *remoteAddress);

/**
 * Add device object to the list.
 * @param[in]  env              JNI interface pointer.
 */
void CAEDRNativeRemoveAllDeviceSocket(JNIEnv *env);

/**
 * Add device object to the list.
 * @param[in]  env              JNI interface pointer.
 * @param[in]  deviceSocket     device socket object.
 */
void CAEDRNativeRemoveDeviceSocket(JNIEnv *env, jobject deviceSocket);

/**
 * Remove device socket.
 * @param[in]   env             JNI interface pointer.
 * @param[in]   address         remote address.
 */
void CAEDRNativeRemoveDeviceSocketBaseAddr(JNIEnv *env, jstring address);

/**
 * Get device socket object from the list.
 * @param[in]  idx              index of device list.
 * @return Device socket object or NULL.
 */
jobject CAEDRNativeGetDeviceSocket(uint32_t idx);

/**
 * Get device socket address.
 * @param[in]   env             JNI interface pointer.
 * @param[in]   remoteAddress   remote address.
 * @return  Device socket object or NULL.
 */
jobject CAEDRNativeGetDeviceSocketBaseAddr(JNIEnv *env, const char* remoteAddress);

/**
 * Get length of device socket list.
 * @return length of list.
 */
uint32_t CAEDRGetSocketListLength();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_EDR_UTILS_H_ */

