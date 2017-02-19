/******************************************************************
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

#include <jni.h>
#include <stdio.h>
#include <android/log.h>

#include "caleutils.h"
#include "logger.h"
#include "oic_malloc.h"
#include "cathreadpool.h"
#include "uarraylist.h"
#include "caadapterutils.h"

#define TAG PCF("CA_LE_UTILS")

#define METHODID_OBJECTNONPARAM   "()Landroid/bluetooth/BluetoothAdapter;"
#define METHODID_STRINGNONPARAM   "()Ljava/lang/String;"
#define CLASSPATH_BT_ADPATER "android/bluetooth/BluetoothAdapter"

jobject CALEGetUuidFromString(JNIEnv *env, const char* uuid)
{
    VERIFY_NON_NULL_RET(uuid, TAG, "uuid is null", NULL);
    VERIFY_NON_NULL_RET(env, TAG, "env is null", NULL);

    OIC_LOG(DEBUG, TAG, "CALEGetUuidFromString");

    jclass jni_cid_UUID = (*env)->FindClass(env, "java/util/UUID");
    if (!jni_cid_UUID)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_UUID is not available");
        return NULL;
    }

    jmethodID jni_mid_fromString = (*env)->GetStaticMethodID(env, jni_cid_UUID, "fromString",
                                                             "(Ljava/lang/String;)"
                                                             "Ljava/util/UUID;");
    if (!jni_mid_fromString)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_fromString is not available");
        return NULL;
    }

    jstring str_uuid = (*env)->NewStringUTF(env, uuid);
    if (!str_uuid)
    {
        OIC_LOG(ERROR, TAG, "str_uuid is not available");
        return NULL;
    }

    jobject jni_obj_uuid = (*env)->CallStaticObjectMethod(env, jni_cid_UUID, jni_mid_fromString,
                                                          str_uuid);
    if (!jni_obj_uuid)
    {
        OIC_LOG(ERROR, TAG, "Fail to get jni uuid object");
        return NULL;
    }

    return jni_obj_uuid;
}

jobject CALEGetParcelUuid(JNIEnv *env, jobject uuid)
{
    OIC_LOG(DEBUG, TAG, "CALEGetParcelUuid");
    VERIFY_NON_NULL_RET(env, TAG, "env is null", NULL);
    VERIFY_NON_NULL_RET(uuid, TAG, "uuid is null", NULL);

    jclass jni_cid_ParcelUuid = (*env)->FindClass(env, "android/os/ParcelUuid");
    if (!jni_cid_ParcelUuid)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_ParcelUuid is not available");
        return NULL;
    }

    jmethodID jni_mid_ParcelUuid = (*env)->GetMethodID(env, jni_cid_ParcelUuid, "<init>",
                                                       "(Ljava/util/UUID;)V");
    if (!jni_mid_ParcelUuid)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_ParcelUuid is not available");
        return NULL;
    }

    jobject jni_ParcelUuid = (*env)->NewObject(env, jni_cid_ParcelUuid, jni_mid_ParcelUuid, uuid);
    if (!jni_ParcelUuid)
    {
        OIC_LOG(ERROR, TAG, "Fail to get jni ParcelUuid");
        return NULL;
    }

    return jni_ParcelUuid;
}

bool CALEIsBondedDevice(JNIEnv *env, jobject bluetoothDevice)
{
    VERIFY_NON_NULL_RET(env, TAG, "env is null", false);
    VERIFY_NON_NULL_RET(bluetoothDevice, TAG, "bluetoothDevice is null", false);

    jclass jni_cid_device_list = (*env)->FindClass(env, "android/bluetooth/BluetoothDevice");
    if (!jni_cid_device_list)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_device_list is null");
        return false;
    }

    jmethodID jni_mid_getBondState = (*env)->GetMethodID(env, jni_cid_device_list, "getBondState",
                                                         "()I");
    if (!jni_mid_getBondState)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getBondState is null");
        return false;
    }

    jint jni_bondState = (jint)(*env)->CallIntMethod(env, bluetoothDevice, jni_mid_getBondState);

    OIC_LOG_V(DEBUG, TAG, "bond state is %d", jni_bondState);

    if (BOND_BONDED == jni_bondState)
    {
        OIC_LOG(DEBUG, TAG, "remote device is bonded");
        return true;
    }
    else
    {
        OIC_LOG(DEBUG, TAG, "remote device is not bonded");
        return false;
    }

    return false;
}

jobjectArray CALEGetBondedDevices(JNIEnv *env)
{
    VERIFY_NON_NULL_RET(env, TAG, "env is null", NULL);

    jclass jni_cid_BTAdapter = (*env)->FindClass(env, CLASSPATH_BT_ADPATER);
    if (!jni_cid_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "getBondedDevices: jni_cid_BTAdapter is null");
        return NULL;
    }

    jmethodID jni_mid_getDefaultAdapter = (*env)->GetStaticMethodID(env, jni_cid_BTAdapter,
                                                                    "getDefaultAdapter",
                                                                    METHODID_OBJECTNONPARAM);

    jobject jni_obj_BTAdapter = (*env)->CallStaticObjectMethod(env, jni_cid_BTAdapter,
                                                               jni_mid_getDefaultAdapter);
    if (!jni_obj_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "getBondedDevices: bluetooth adapter is null");
        return NULL;
    }

    // Get a list of currently paired devices
    jmethodID jni_mid_getBondedDevices = (*env)->GetMethodID(env, jni_cid_BTAdapter,
                                                             "getBondedDevices",
                                                             "()Ljava/util/Set;");
    if (!jni_mid_getBondedDevices)
    {
        OIC_LOG(ERROR, TAG, "getBondedDevices: jni_mid_getBondedDevicesr is null");
        return NULL;
    }

    jobject jni_obj_setPairedDevices = (*env)->CallObjectMethod(env, jni_obj_BTAdapter,
                                                                jni_mid_getBondedDevices);
    if (!jni_obj_setPairedDevices)
    {
        OIC_LOG(ERROR, TAG, "getBondedDevices: jni_obj_setPairedDevices is null");
        return NULL;
    }

    jclass jni_cid_Set = (*env)->FindClass(env, "java/util/Set");
    if (!jni_cid_Set)
    {
        OIC_LOG(ERROR, TAG, "getBondedDevices : jni_cid_Set is null");
        return NULL;
    }

    jmethodID jni_mid_toArray = (*env)->GetMethodID(env, jni_cid_Set, "toArray",
                                                    "()[Ljava/lang/Object;");
    if (!jni_mid_toArray)
    {
        OIC_LOG(ERROR, TAG, "getBondedDevices: jni_mid_toArray is null");
        return NULL;
    }

    jobjectArray jni_arrayPairedDevices = (jobjectArray)(
            (*env)->CallObjectMethod(env, jni_obj_setPairedDevices, jni_mid_toArray));
    if (!jni_arrayPairedDevices)
    {
        OIC_LOG(ERROR, TAG, "getBondedDevices: jni_arrayPairedDevices is null");
        return NULL;
    }

    return jni_arrayPairedDevices;
}

jint CALEGetBTStateOnInfo(JNIEnv *env)
{
    VERIFY_NON_NULL_RET(env, TAG, "env is null", -1);

    jclass jni_cid_BTAdapter = (*env)->FindClass(env, CLASSPATH_BT_ADPATER);
    if (!jni_cid_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "getBTStateOnInfo: jni_cid_BTAdapter is null");
        return -1;
    }

    jfieldID jni_fid_stateon = (*env)->GetStaticFieldID(env, jni_cid_BTAdapter, "STATE_ON", "I");
    if (!jni_fid_stateon)
    {
        OIC_LOG(ERROR, TAG, "get_field_state is not available");
        return -1;
    }

    jint jni_int_val = (*env)->GetStaticIntField(env, jni_cid_BTAdapter, jni_fid_stateon);
    OIC_LOG_V(DEBUG, TAG, "bluetooth.STATE_ON state integer value : %d", jni_int_val);

    return jni_int_val;
}

CAResult_t CALECheckPlatformVersion(JNIEnv *env, uint16_t level)
{
    jint jni_int_sdk = CALEGetBuildVersion(env);
    if (jni_int_sdk < level)
    {
        OIC_LOG(ERROR, TAG, "it is not supported");
        return CA_NOT_SUPPORTED;
    }

    return CA_STATUS_OK;
}

jint CALEGetBuildVersion(JNIEnv *env)
{
    VERIFY_NON_NULL_RET(env, TAG, "env is null", -1);

    // VERSION is a nested class within android.os.Build (hence "$" rather than "/")
    jclass jni_cls_version = (*env)->FindClass(env, "android/os/Build$VERSION");
    if (!jni_cls_version)
    {
        OIC_LOG(ERROR, TAG, "jni_cls_version is null");
        return -1;
    }

    jfieldID jni_fid_sdk = (*env)->GetStaticFieldID(env, jni_cls_version, "SDK_INT", "I");
    if (!jni_fid_sdk)
    {
        OIC_LOG(ERROR, TAG, "jni_fid_sdk is null");
        return -1;
    }

    jint jni_int_sdk = (*env)->GetStaticIntField(env, jni_cls_version, jni_fid_sdk);
    OIC_LOG_V(DEBUG, TAG, "sdk version is %d", jni_int_sdk);

    return jni_int_sdk;
}

jint CALEGetBuildVersionCodeForName(JNIEnv *env, const char* versionName)
{
    VERIFY_NON_NULL_RET(env, TAG, "env is null", -1);
    VERIFY_NON_NULL_RET(versionName, TAG, "versionName is null", -1);

    // VERSION is a nested class within android.os.Build (hence "$" rather than "/")
    jclass jni_cls_version = (*env)->FindClass(env, "android/os/Build$VERSION_CODES");
    if (!jni_cls_version)
    {
        OIC_LOG(ERROR, TAG, "jni_cls_version is null");
        return -1;
    }

    jfieldID jni_fid_version = (*env)->GetStaticFieldID(env, jni_cls_version, versionName, "I");
    if (!jni_fid_version)
    {
        OIC_LOG(ERROR, TAG, "jni_fid_version is null");
        return -1;
    }

    jint jni_int_version = (*env)->GetStaticIntField(env, jni_cls_version, jni_fid_version);
    OIC_LOG_V(DEBUG, TAG, "version [%s] is %d",versionName, jni_int_version);

    return jni_int_version;
}

jboolean CALEIsEnableBTAdapter(JNIEnv *env)
{
    VERIFY_NON_NULL_RET(env, TAG, "env is null", JNI_FALSE);

    jclass jni_cid_BTAdapter = (*env)->FindClass(env, CLASSPATH_BT_ADPATER);
    if (!jni_cid_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BTAdapter: jni_cid_BTAdapter is null");
        return JNI_FALSE;
    }

    jmethodID jni_mid_getDefaultAdapter = (*env)->GetStaticMethodID(env, jni_cid_BTAdapter,
                                                                    "getDefaultAdapter",
                                                                    METHODID_OBJECTNONPARAM);
    if (!jni_mid_getDefaultAdapter)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getDefaultAdapter is null");
        return JNI_FALSE;
    }

    jobject jni_obj_BTAdapter = (*env)->CallStaticObjectMethod(env, jni_cid_BTAdapter,
                                                               jni_mid_getDefaultAdapter);
    if (!jni_obj_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_BTAdapter is null");
        return JNI_FALSE;
    }

    // isEnable()
    jmethodID jni_mid_isEnable = (*env)->GetMethodID(env, jni_cid_BTAdapter, "isEnabled", "()Z");
    if (!jni_mid_isEnable)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_isEnable is null");
        return JNI_FALSE;
    }

    jboolean jni_isEnable = (*env)->CallBooleanMethod(env, jni_obj_BTAdapter, jni_mid_isEnable);
    OIC_LOG_V(DEBUG, TAG, "adapter state is %d", jni_isEnable);

    return jni_isEnable;
}

jstring CALEGetAddressFromBTDevice(JNIEnv *env, jobject bluetoothDevice)
{
    OIC_LOG(DEBUG, TAG, "IN - CALEGetAddressFromBTDevice");
    VERIFY_NON_NULL_RET(env, TAG, "env is null", NULL);
    VERIFY_NON_NULL_RET(bluetoothDevice, TAG, "bluetoothDevice is null", NULL);

    jclass jni_cid_device_list = (*env)->FindClass(env, "android/bluetooth/BluetoothDevice");
    if (!jni_cid_device_list)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_device_list is null");
        return NULL;
    }

    jmethodID jni_mid_getAddress = (*env)->GetMethodID(env, jni_cid_device_list, "getAddress",
                                                       "()Ljava/lang/String;");
    if (!jni_mid_getAddress)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getAddress is null");
        return NULL;
    }

    jstring jni_address = (jstring)(*env)->CallObjectMethod(env, bluetoothDevice,
                                                            jni_mid_getAddress);
    if (!jni_address)
    {
        OIC_LOG(ERROR, TAG, "jni_address is null");
        return NULL;
    }

    OIC_LOG(DEBUG, TAG, "OUT - CALEGetAddressFromBTDevice");
    return jni_address;
}
