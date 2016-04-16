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
 * This file contains the APIs for BT LE communications.
 */
#ifndef CA_LE_UTILS_H_
#define CA_LE_UTILS_H_

#include "cacommon.h"
#include "cathreadpool.h"
#include "cagattservice.h"
#include "uarraylist.h"
#include "jni.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Service UUID */
static const char OIC_GATT_SERVICE_UUID[] = CA_GATT_SERVICE_UUID;
static const char OIC_GATT_CHARACTERISTIC_REQUEST_UUID[] = CA_GATT_REQUEST_CHRC_UUID;
static const char OIC_GATT_CHARACTERISTIC_RESPONSE_UUID[] = CA_GATT_RESPONSE_CHRC_UUID;
static const char OIC_GATT_CHARACTERISTIC_CONFIG_UUID[] = "00002902-0000-1000-8000-00805f9b34fb";

static const jint GATT_SUCCESS = 0;

static const jint BOND_BONDED = 12;
static const jint BOND_BONDING = 11;
static const jint BOND_NONE = 10;

/**
 * get uuid(jni object) from uuid(character).
 * @param[in]   env              JNI interface pointer.
 * @param[in]   uuid             uuid(character).
 * @return  uuid(jni object).
 */
jobject CALEGetUuidFromString(JNIEnv *env, const char* uuid);

/**
 * get parcel uuid object.
 * @param[in]   env              JNI interface pointer.
 * @param[in]   uuid             uuid (jni object).
 * @return  parcel uuid object.
 */
jobject CALEGetParcelUuid(JNIEnv *env, jobject uuid);

/**
 * get address from a local device.
 * @param[in]   env              JNI interface pointer.
 * @return  local address.
 */
jstring CALEGetLocalDeviceAddress(JNIEnv *env);

/**
 * get bonded list.
 * @param[in]   env              JNI interface pointer.
 * @return  bonded list.
 */
jobjectArray CALEGetBondedDevices(JNIEnv *env);

/**
 * get constants information of bluetooth state-on.
 * @param[in]   env              JNI interface pointer.
 * @return  constants information of bluetooth state-on.
 */
jint CALEGetBTStateOnInfo(JNIEnv *env);

/**
 * check this device can be supported as BLE client or server.
 * @param[in]   env              JNI interface pointer.
 * @param[in]   level            Android API Level to support.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALECheckPlatformVersion(JNIEnv *env, uint16_t level);

/**
 * get constants information of android.os.Build.VERSION.SDK_INT.
 * @param[in]   env              JNI interface pointer.
 * @return  constants information of android.os.Build.VERSION.SDK_INT.
 */
jint CALEGetBuildVersion(JNIEnv *env);

/**
 * get constants information of android.os.Build.VERSION_CODES.[VersionName].
 * @param[in]   env              JNI interface pointer.
 * @param[in]   versionName      version name (.., KITKAT, LOLLIPOP, ..).
 * @return  constants information of android.os.Build.VERSION_CODES.[VersionName].
 */
jint CALEGetBuildVersionCodeForName(JNIEnv *env, const char* versionName);

/**
 * get bluetooth adapter state information.
 * @param[in]   env              JNI interface pointer.
 * @return  JNI_TRUE if the local adapter is turned on.
 */
jboolean CALEIsEnableBTAdapter(JNIEnv *env);

/**
 * get address from remote device.
 * @param[in]   env              JNI interface pointer.
 * @param[in]   bluetoothDevice  bluetooth device object.
 * @return  remote address.
 */
jstring CALEGetAddressFromBTDevice(JNIEnv *env, jobject bluetoothDevice);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_LE_UTILS_H_ */
