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
#include "caedrutils.h"
#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "cathreadpool.h"
#include "uarraylist.h"

#define ERROR_CODE (-1)
#define TAG PCF("CA_EDR_UTILS")

static const char METHODID_OBJECTNONPARAM[] = "()Landroid/bluetooth/BluetoothAdapter;";
static const char METHODID_STRINGNONPARAM[] = "()Ljava/lang/String;";
static const char CLASSPATH_BT_ADPATER[] = "android/bluetooth/BluetoothAdapter";
static const char CLASSPATH_BT_DEVICE[] = "android/bluetooth/BluetoothDevice";
static const char CLASSPATH_BT_SOCKET[] = "android/bluetooth/BluetoothSocket";

static u_arraylist_t *g_deviceStateList = NULL;
static u_arraylist_t *g_deviceObjectList = NULL;

// get address from bluetooth socket
jstring CAEDRNativeGetAddressFromDeviceSocket(JNIEnv *env, jobject bluetoothSocketObj)
{
    if (!bluetoothSocketObj)
    {
        OIC_LOG(ERROR, TAG, "[EDR] getRemoteAddress: bluetoothSocketObj is null");
        return NULL;
    }

    jclass jni_cid_BTSocket = (*env)->FindClass(env, CLASSPATH_BT_SOCKET);
    if (!jni_cid_BTSocket)
    {
        OIC_LOG(ERROR, TAG, "[EDR] getRemoteAddress: jni_cid_BTSocket is null");
        return NULL;
    }

    jmethodID jni_mid_getRemoteDevice = (*env)->GetMethodID(
            env, jni_cid_BTSocket, "getRemoteDevice", "()Landroid/bluetooth/BluetoothDevice;");
    if (!jni_mid_getRemoteDevice)
    {
        (*env)->DeleteLocalRef(env, jni_cid_BTSocket);

        OIC_LOG(ERROR, TAG, "[EDR] getRemoteAddress: jni_mid_getRemoteDevice is null");
        return NULL;
    }

    jobject jni_obj_remoteBTDevice = (*env)->CallObjectMethod(env, bluetoothSocketObj,
                                                              jni_mid_getRemoteDevice);
    if (!jni_obj_remoteBTDevice)
    {
        (*env)->DeleteLocalRef(env, jni_cid_BTSocket);

        OIC_LOG(ERROR, TAG, "[EDR] getRemoteAddress: jni_obj_remoteBTDevice is null");
        return NULL;
    }

    jclass jni_cid_BTDevice = (*env)->FindClass(env, CLASSPATH_BT_DEVICE);
    if (!jni_cid_BTDevice)
    {
        (*env)->DeleteLocalRef(env, jni_obj_remoteBTDevice);
        (*env)->DeleteLocalRef(env, jni_cid_BTSocket);

        OIC_LOG(ERROR, TAG, "[EDR] getRemoteAddress: jni_cid_BTDevice is null");
        return NULL;
    }
    jmethodID j_mid_getAddress = (*env)->GetMethodID(env, jni_cid_BTDevice, "getAddress",
                                                     METHODID_STRINGNONPARAM);
    if (!j_mid_getAddress)
    {
        (*env)->DeleteLocalRef(env, jni_obj_remoteBTDevice);
        (*env)->DeleteLocalRef(env, jni_cid_BTDevice);
        (*env)->DeleteLocalRef(env, jni_cid_BTSocket);

        OIC_LOG(ERROR, TAG, "[EDR] getRemoteAddress: j_mid_getAddress is null");
        return NULL;
    }

    jstring j_str_address = (*env)->CallObjectMethod(env, jni_obj_remoteBTDevice, j_mid_getAddress);
    if (!j_str_address)
    {
        (*env)->DeleteLocalRef(env, jni_obj_remoteBTDevice);
        (*env)->DeleteLocalRef(env, jni_cid_BTDevice);
        (*env)->DeleteLocalRef(env, jni_cid_BTSocket);

        OIC_LOG(ERROR, TAG, "[EDR] getRemoteAddress: j_str_address is null");
        return NULL;
    }

    (*env)->DeleteLocalRef(env, jni_obj_remoteBTDevice);
    (*env)->DeleteLocalRef(env, jni_cid_BTDevice);
    (*env)->DeleteLocalRef(env, jni_cid_BTSocket);

    return j_str_address;
}

jstring CAEDRNativeGetLocalDeviceAddress(JNIEnv* env)
{
    jclass jni_cid_BTAdapter = (*env)->FindClass(env, CLASSPATH_BT_ADPATER);
    if (!jni_cid_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] getAddress: jni_cid_BTAdapter is null");
        return NULL;
    }

    jmethodID jni_mid_getDefaultAdapter = (*env)->GetStaticMethodID(env, jni_cid_BTAdapter,
                                                                    "getDefaultAdapter",
                                                                    METHODID_OBJECTNONPARAM);
    if (!jni_mid_getDefaultAdapter)
    {
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] getAddress: jni_mid_getDefaultAdapter is null");
        return NULL;
    }

    jmethodID jni_mid_getAddress = (*env)->GetMethodID(env, jni_cid_BTAdapter, "getAddress",
                                                       METHODID_STRINGNONPARAM);
    if (!jni_mid_getAddress)
    {
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] getAddress: jni_mid_getAddress is null");
        return NULL;
    }

    jobject jni_obj_BTAdapter = (*env)->CallStaticObjectMethod(env, jni_cid_BTAdapter,
                                                               jni_mid_getDefaultAdapter);
    if (!jni_obj_BTAdapter)
    {
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] getAddress: jni_obj_BTAdapter is null");
        return NULL;
    }

    jstring jni_str_address = (jstring)(*env)->CallObjectMethod(env, jni_obj_BTAdapter,
                                                                jni_mid_getAddress);
    if (!jni_str_address)
    {
        (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] getAddress: jni_str_address is null");
        return NULL;
    }

    (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);
    (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

    return jni_str_address;
}

jobjectArray CAEDRNativeGetBondedDevices(JNIEnv *env)
{
    jclass jni_cid_BTAdapter = (*env)->FindClass(env, CLASSPATH_BT_ADPATER);
    if (!jni_cid_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] getBondedDevices: jni_cid_BTAdapter is null");
        return NULL;
    }

    jmethodID jni_mid_getDefaultAdapter = (*env)->GetStaticMethodID(env, jni_cid_BTAdapter,
                                                                    "getDefaultAdapter",
                                                                    METHODID_OBJECTNONPARAM);
    if (!jni_mid_getDefaultAdapter)
    {
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] getBondedDevices: default adapter is null");
        return NULL;
    }

    jobject jni_obj_BTAdapter = (*env)->CallStaticObjectMethod(env, jni_cid_BTAdapter,
                                                               jni_mid_getDefaultAdapter);
    if (!jni_obj_BTAdapter)
    {
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] getBondedDevices: bluetooth adapter is null");
        return NULL;
    }

    // Get a list of currently paired devices
    jmethodID jni_mid_getBondedDevices = (*env)->GetMethodID(env, jni_cid_BTAdapter,
                                                             "getBondedDevices",
                                                             "()Ljava/util/Set;");
    if (!jni_mid_getBondedDevices)
    {
        (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] getBondedDevices: jni_mid_getBondedDevicesr is null");
        return NULL;
    }

    jobject jni_obj_setPairedDevices = (*env)->CallObjectMethod(env, jni_obj_BTAdapter,
                                                                jni_mid_getBondedDevices);
    if (!jni_obj_setPairedDevices)
    {
        (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] getBondedDevices: jni_obj_setPairedDevices is null");
        return NULL;
    }

    // Convert the set to an object array
    // object[] array = Set<BluetoothDevice>.toArray();
    jclass jni_cid_Set = (*env)->FindClass(env, "java/util/Set");
    if (!jni_cid_Set)
    {
        (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);
        (*env)->DeleteLocalRef(env, jni_obj_setPairedDevices);

        OIC_LOG(ERROR, TAG, "[EDR][Native] getBondedDevices: jni_cid_Set is null");
        return NULL;
    }
    jmethodID jni_mid_toArray = (*env)->GetMethodID(env, jni_cid_Set, "toArray",
                                                    "()[Ljava/lang/Object;");

    if (!jni_mid_toArray)
    {
        (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);
        (*env)->DeleteLocalRef(env, jni_obj_setPairedDevices);

        OIC_LOG(ERROR, TAG, "[EDR][Native] getBondedDevices: jni_mid_toArray is null");
        return NULL;
    }

    jobjectArray jni_arrayPairedDevices = (jobjectArray)(
            (*env)->CallObjectMethod(env, jni_obj_setPairedDevices, jni_mid_toArray));
    if (!jni_arrayPairedDevices)
    {
        (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);
        (*env)->DeleteLocalRef(env, jni_obj_setPairedDevices);

        OIC_LOG(ERROR, TAG, "[EDR][Native] getBondedDevices: jni_arrayPairedDevices is null");
        return NULL;
    }

    (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);
    (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);
    (*env)->DeleteLocalRef(env, jni_obj_setPairedDevices);

    return jni_arrayPairedDevices;
}

jint CAEDRNativeGetBTStateOnInfo(JNIEnv *env)
{
    jclass jni_cid_BTAdapter = (*env)->FindClass(env, CLASSPATH_BT_ADPATER);
    if (!jni_cid_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] getBTStateOnInfo: jni_cid_BTAdapter is null");
        return ERROR_CODE;
    }

    jfieldID jni_fid_stateon = (*env)->GetStaticFieldID(env, jni_cid_BTAdapter, "STATE_ON", "I");
    if (jni_fid_stateon == 0)
    {
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] get_field_state is 0");
        return ERROR_CODE;
    }
    jint jni_int_val = (*env)->GetStaticIntField(env, jni_cid_BTAdapter, jni_fid_stateon);

    OIC_LOG_V(DEBUG, TAG, "[EDR][Native] bluetooth state integer value : %d", jni_int_val);

    (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

    return jni_int_val;
}

jboolean CAEDRNativeIsEnableBTAdapter(JNIEnv *env)
{
    jclass jni_cid_BTAdapter = (*env)->FindClass(env, CLASSPATH_BT_ADPATER);
    if (!jni_cid_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] jni_cid_BTAdapter: jni_cid_BTAdapter is null");
        return JNI_FALSE;
    }

    jmethodID jni_mid_getDefaultAdapter = (*env)->GetStaticMethodID(env, jni_cid_BTAdapter,
                                                                    "getDefaultAdapter",
                                                                    METHODID_OBJECTNONPARAM);
    if (!jni_mid_getDefaultAdapter)
    {
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] jni_mid_getDefaultAdapter is null");
        return JNI_FALSE;
    }

    jobject jni_obj_BTAdapter = (*env)->CallStaticObjectMethod(env, jni_cid_BTAdapter,
                                                               jni_mid_getDefaultAdapter);
    if (!jni_obj_BTAdapter)
    {
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] jni_obj_BTAdapter is null");
        return JNI_FALSE;
    }

    // isEnable()
    jmethodID jni_mid_isEnable = (*env)->GetMethodID(env, jni_cid_BTAdapter, "isEnabled", "()Z");
    if (!jni_mid_isEnable)
    {
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);
        (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);

        OIC_LOG(ERROR, TAG, "[EDR][Native] jni_mid_isEnable is null");
        return JNI_FALSE;
    }

    jboolean jni_isEnable = (*env)->CallBooleanMethod(env, jni_obj_BTAdapter, jni_mid_isEnable);
    OIC_LOG_V(DEBUG, TAG, "[EDR][Native] adapter state is %d", jni_isEnable);

    (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);
    (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);

    return jni_isEnable;
}

jstring CAEDRNativeGetAddressFromBTDevice(JNIEnv *env, jobject bluetoothDevice)
{
    if (!bluetoothDevice)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] bluetoothDevice is null");
        return NULL;
    }
    jclass jni_cid_device_list = (*env)->FindClass(env, "android/bluetooth/BluetoothDevice");
    if (!jni_cid_device_list)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] jni_cid_device_list is null");
        return NULL;
    }

    jmethodID jni_mid_getAddress = (*env)->GetMethodID(env, jni_cid_device_list, "getAddress",
                                                       METHODID_STRINGNONPARAM);
    if (!jni_mid_getAddress)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] jni_mid_getAddress is null");
        return NULL;
    }

    jstring jni_address = (jstring)(*env)->CallObjectMethod(env, bluetoothDevice,
                                                            jni_mid_getAddress);
    if (!jni_address)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] jni_address is null");
        return NULL;
    }
    return jni_address;
}

/**
 * BT State List
 */
void CAEDRNativeCreateDeviceStateList()
{
    OIC_LOG(DEBUG, TAG, "[EDR][Native] CAEDRNativeCreateDeviceStateList");

    // create new object array
    if (NULL == g_deviceStateList)
    {
        OIC_LOG(DEBUG, TAG, "Create device list");

        g_deviceStateList = u_arraylist_create();
    }
}

void CAEDRUpdateDeviceState(CAConnectedState_t state, const char *address)
{
    if (!address)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] address is null");
        return;
    }
    state_t *newstate = (state_t*) OICCalloc(1, sizeof(state_t));
    if (!newstate)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] newstate is null");
        return;
    }
    OICStrcpy((char*) newstate->address, sizeof(newstate->address), address);
    newstate->state = state;

    CAEDRNativeAddDeviceStateToList(newstate);
}

void CAEDRNativeAddDeviceStateToList(state_t *state)
{
    if (!state)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] device is null");
        return;
    }

    if (!g_deviceStateList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdevice_list is null");
        return;
    }

    if (CAEDRNativeIsDeviceInList((const char*) state->address))
    {
        // delete previous state for update new state
        CAEDRNativeRemoveDevice((const char*) state->address);
    }
    u_arraylist_add(g_deviceStateList, state); // update new state
    OIC_LOG_V(DEBUG, TAG, "Set State Info to List : %d", state->state);
}

bool CAEDRNativeIsDeviceInList(const char* remoteAddress)
{

    if (!remoteAddress)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] remoteAddress is null");
        return false;
    }
    jint index;
    jint length = u_arraylist_length(g_deviceStateList);
    for (index = 0; index < length; index++)
    {
        state_t* state = (state_t*) u_arraylist_get(g_deviceStateList, index);
        if (!state)
        {
            OIC_LOG(ERROR, TAG, "[EDR][Native] state_t object is null");
            return false;
        }

        if (!strcmp(remoteAddress, (const char*) state->address))
        {
            OIC_LOG(DEBUG, TAG, "the device is already set");
            return true;
        }
        else
        {
            continue;
        }
    }

    OIC_LOG(DEBUG, TAG, "there are no the device in list.");
    return false;
}

void CAEDRNativeRemoveAllDeviceState()
{
    OIC_LOG(DEBUG, TAG, "CAEDRNativeRemoveAllDevices");

    if (!g_deviceStateList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdeviceStateList is null");
        return;
    }

    jint index;
    jint length = u_arraylist_length(g_deviceStateList);
    for (index = 0; index < length; index++)
    {
        state_t* state = (state_t*) u_arraylist_get(g_deviceStateList, index);
        if (!state)
        {
            OIC_LOG(DEBUG, TAG, "[EDR][Native] jarrayObj is null");
            continue;
        }
        OICFree(state);
    }

    OICFree(g_deviceStateList);
    g_deviceStateList = NULL;
    return;
}

void CAEDRNativeRemoveDevice(const char *remoteAddress)
{
    OIC_LOG(DEBUG, TAG, "CAEDRNativeRemoveDeviceforStateList");

    if (!g_deviceStateList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdeviceStateList is null");
        return;
    }
    if (!remoteAddress)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] remoteAddress is null");
        return;
    }

    jint index;
    jint length = u_arraylist_length(g_deviceStateList);
    for (index = 0; index < length; index++)
    {
        state_t* state = (state_t*) u_arraylist_get(g_deviceStateList, index);
        if (!state)
        {
            OIC_LOG(DEBUG, TAG, "[EDR][Native] state_t object is null");
            continue;
        }

        if (!strcmp((const char*) state->address, remoteAddress))
        {
            OIC_LOG_V(DEBUG, TAG, "[EDR][Native] remove state : %s", remoteAddress);
            OICFree(state);

            u_arraylist_remove(g_deviceStateList, index);
            break;
        }
    }
    return;
}

CAConnectedState_t CAEDRIsConnectedDevice(const char *remoteAddress)
{
    OIC_LOG(DEBUG, TAG, "CAEDRIsConnectedDevice");

    if (!remoteAddress)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] remoteAddress is null");
        return STATE_DISCONNECTED;
    }

    if (!g_deviceStateList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdeviceStateList is null");
        return STATE_DISCONNECTED;
    }

    jint index;
    jint length = u_arraylist_length(g_deviceStateList);
    for (index = 0; index < length; index++)
    {
        state_t* state = (state_t*) u_arraylist_get(g_deviceStateList, index);
        if (!state)
        {
            OIC_LOG(DEBUG, TAG, "[EDR][Native] state_t object is null");
            continue;
        }

        if (!strcmp((const char*) state->address, remoteAddress))
        {
            OIC_LOG(DEBUG, TAG, "[EDR][Native] check whether it is connected or not");

            return state->state;
        }
    }
    return STATE_DISCONNECTED;
}

/**
 * Device Socket Object List
 */
void CAEDRNativeCreateDeviceSocketList()
{
    OIC_LOG(DEBUG, TAG, "[EDR][Native] CAEDRNativeCreateDeviceSocketList");

    // create new object array
    if (NULL == g_deviceObjectList)
    {
        OIC_LOG(DEBUG, TAG, "Create Device object list");

        g_deviceObjectList = u_arraylist_create();
    }
}

void CAEDRNativeAddDeviceSocketToList(JNIEnv *env, jobject deviceSocket)
{
    OIC_LOG(DEBUG, TAG, "[EDR][Native] CANativeAddDeviceobjToList");

    if (!deviceSocket)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] Device is null");
        return;
    }

    if (!g_deviceObjectList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdeviceObjectList is null");
        return;
    }

    jstring jni_remoteAddress = CAEDRNativeGetAddressFromDeviceSocket(env, deviceSocket);
    if (!jni_remoteAddress)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] jni_remoteAddress is null");
        return;
    }

    const char* remoteAddress = (*env)->GetStringUTFChars(env, jni_remoteAddress, NULL);

    if (!CAEDRNativeIsDeviceSocketInList(env, remoteAddress))
    {
        jobject gDeviceSocker = (*env)->NewGlobalRef(env, deviceSocket);
        u_arraylist_add(g_deviceObjectList, gDeviceSocker);
        OIC_LOG(DEBUG, TAG, "Set Socket Object to Array");
    }
    (*env)->ReleaseStringUTFChars(env, jni_remoteAddress, remoteAddress);
    (*env)->DeleteLocalRef(env, jni_remoteAddress);
}

bool CAEDRNativeIsDeviceSocketInList(JNIEnv *env, const char* remoteAddress)
{
    OIC_LOG(DEBUG, TAG, "[EDR][Native] CANativeIsDeviceObjInList");

    if (!remoteAddress)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] remoteAddress is null");
        return false;
    }
    jint index;
    jint length = u_arraylist_length(g_deviceStateList);
    for (index = 0; index < length; index++)
    {

        jobject jarrayObj = (jobject) u_arraylist_get(g_deviceObjectList, index);
        if (!jarrayObj)
        {
            OIC_LOG(DEBUG, TAG, "[EDR][Native] jarrayObj is null");
            return false;
        }

        jstring jni_setAddress = CAEDRNativeGetAddressFromDeviceSocket(env, jarrayObj);
        (*env)->DeleteLocalRef(env, jarrayObj);
        if (!jni_setAddress)
        {
            OIC_LOG(DEBUG, TAG, "[EDR][Native] jni_setAddress is null");
            return false;
        }

        const char* setAddress = (*env)->GetStringUTFChars(env, jni_setAddress, NULL);
        if (!setAddress)
        {
            OIC_LOG(DEBUG, TAG, "[EDR][Native] setAddress is null");
            return false;
        }

        if (!strcmp(remoteAddress, setAddress))
        {
            OIC_LOG(DEBUG, TAG, "the device is already set");
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            return true;
        }
        else
        {
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            continue;
        }
    }

    OIC_LOG(DEBUG, TAG, "there are no the Device obejct in list. we can add");
    return false;
}

void CAEDRNativeSocketCloseToAll(JNIEnv *env)
{
    OIC_LOG(DEBUG, TAG, "[EDR][Native] CAEDRNativeSocketCloseToAll");

    if (!g_deviceObjectList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdeviceObjectList is null");
        return;
    }

    jclass jni_cid_BTSocket = (*env)->FindClass(env, CLASSPATH_BT_SOCKET);
    if (!jni_cid_BTSocket)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] close: jni_cid_BTSocket is null");
        return;
    }

    jmethodID jni_mid_close = (*env)->GetMethodID(env, jni_cid_BTSocket, "close", "()V");
    if (!jni_mid_close)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] close: jni_mid_close is null");
        return;
    }

    jint index;
    jint length = u_arraylist_length(g_deviceStateList);
    for (index = 0; index < length; index++)
    {
        jobject jni_obj_socket = (jobject) u_arraylist_get(g_deviceObjectList, index);
        if (!jni_obj_socket)
        {
            OIC_LOG(ERROR, TAG, "[EDR][Native] socket obj is null");
            return;
        }

        (*env)->CallVoidMethod(env, jni_obj_socket, jni_mid_close);

        if ((*env)->ExceptionCheck(env))
        {
            OIC_LOG(ERROR, TAG, "[EDR][Native] close: close is Failed!!!");
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
            return;
        }
    }
}

void CAEDRNativeRemoveAllDeviceSocket(JNIEnv *env)
{
    OIC_LOG(DEBUG, TAG, "CANativeRemoveAllDeviceObjsList");

    if (!g_deviceObjectList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdeviceObjectList is null");
        return;
    }

    jint index;
    jint length = u_arraylist_length(g_deviceStateList);
    for (index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_deviceObjectList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "[EDR][Native] jarrayObj is null");
            return;
        }
        (*env)->DeleteGlobalRef(env, jarrayObj);
    }

    OICFree(g_deviceObjectList);
    g_deviceObjectList = NULL;
    return;
}

void CAEDRNativeRemoveDeviceSocket(JNIEnv *env, jobject deviceSocket)
{
    OIC_LOG(DEBUG, TAG, "CAEDRNativeRemoveDeviceSocket");

    if (!g_deviceObjectList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdeviceObjectList is null");
        return;
    }

    jint index;
    jint length = u_arraylist_length(g_deviceStateList);
    for (index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_deviceObjectList, index);
        if (!jarrayObj)
        {
            OIC_LOG(DEBUG, TAG, "[EDR][Native] jarrayObj is null");
            continue;
        }

        jstring jni_setAddress = CAEDRNativeGetAddressFromDeviceSocket(env, jarrayObj);
        if (!jni_setAddress)
        {
            OIC_LOG(DEBUG, TAG, "[EDR][Native] jni_setAddress is null");
            continue;
        }

        jstring jni_remoteAddress = CAEDRNativeGetAddressFromDeviceSocket(env, deviceSocket);
        if (!jni_remoteAddress)
        {
            OIC_LOG(DEBUG, TAG, "[EDR][Native] jni_remoteAddress is null");
            continue;
        }

        const char* setAddress = (*env)->GetStringUTFChars(env, jni_setAddress, NULL);
        const char* remoteAddress = (*env)->GetStringUTFChars(env, jni_remoteAddress, NULL);

        if (!strcmp(setAddress, remoteAddress))
        {
            OIC_LOG_V(DEBUG, TAG, "[EDR][Native] remove object : %s", remoteAddress);
            (*env)->DeleteGlobalRef(env, jarrayObj);
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            (*env)->ReleaseStringUTFChars(env, jni_remoteAddress, remoteAddress);

            u_arraylist_remove(g_deviceObjectList, index);
            break;
        }
        (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
        (*env)->ReleaseStringUTFChars(env, jni_remoteAddress, remoteAddress);
    }

    OIC_LOG(DEBUG, TAG, "[EDR][Native] there are no target object");
    return;
}

void CAEDRNativeRemoveDeviceSocketBaseAddr(JNIEnv *env, jstring address)
{
    OIC_LOG(DEBUG, TAG, "CAEDRNativeRemoveDeviceSocket");

    if (!g_deviceObjectList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdeviceObjectList is null");
        return;
    }

    jint index;
    jint length = u_arraylist_length(g_deviceStateList);
    for (index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_deviceObjectList, index);
        if (!jarrayObj)
        {
            OIC_LOG(DEBUG, TAG, "[EDR][Native] jarrayObj is null");
            continue;
        }

        jstring jni_setAddress = CAEDRNativeGetAddressFromDeviceSocket(env, jarrayObj);
        if (!jni_setAddress)
        {
            OIC_LOG(ERROR, TAG, "[EDR][Native] jni_setAddress is null");
            continue;
        }
        const char* setAddress = (*env)->GetStringUTFChars(env, jni_setAddress, NULL);
        const char* remoteAddress = (*env)->GetStringUTFChars(env, address, NULL);

        if (!strcmp(setAddress, remoteAddress))
        {
            OIC_LOG_V(ERROR, TAG, "[EDR][Native] remove object : %s", remoteAddress);
            (*env)->DeleteGlobalRef(env, jarrayObj);
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            (*env)->ReleaseStringUTFChars(env, address, remoteAddress);

            u_arraylist_remove(g_deviceObjectList, index);
            break;
        }
        (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
        (*env)->ReleaseStringUTFChars(env, address, remoteAddress);
    }

    OIC_LOG(DEBUG, TAG, "[EDR][Native] there are no target object");
    return;
}

jobject CAEDRNativeGetDeviceSocket(uint32_t idx)
{
    OIC_LOG(DEBUG, TAG, "CAEDRNativeGetDeviceSocket");

    if (!g_deviceObjectList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdeviceObjectList is null");
        return NULL;
    }

    jobject jarrayObj = (jobject) u_arraylist_get(g_deviceObjectList, idx);
    if (!jarrayObj)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] jarrayObj is not available");
        return NULL;
    }
    return jarrayObj;
}

jobject CAEDRNativeGetDeviceSocketBaseAddr(JNIEnv *env, const char* remoteAddress)
{
    OIC_LOG(DEBUG, TAG, "CAEDRNativeGetDeviceSocket");

    if (!g_deviceObjectList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdeviceObjectList is null");
        return NULL;
    }

    jint index;
    jint length = u_arraylist_length(g_deviceStateList);
    for (index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_deviceObjectList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "[EDR][Native] jarrayObj is null");
            continue;
        }

        jstring jni_setAddress = CAEDRNativeGetAddressFromDeviceSocket(env, jarrayObj);
        if (!jni_setAddress)
        {
            OIC_LOG(ERROR, TAG, "[EDR][Native] jni_setAddress is null");
            continue;
        }
        const char* setAddress = (*env)->GetStringUTFChars(env, jni_setAddress, NULL);

        if (!strcmp(setAddress, remoteAddress))
        {
            OIC_LOG_V(ERROR, TAG, "[EDR][Native] remove object : %s", remoteAddress);
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            (*env)->DeleteLocalRef(env, jni_setAddress);
            return jarrayObj;
        }
        (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
        (*env)->DeleteLocalRef(env, jni_setAddress);
    }

    return NULL;
}

uint32_t CAEDRGetSocketListLength()
{
    if (!g_deviceObjectList)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] gdeviceObjectList is null");
        return 0;
    }

    uint32_t length = u_arraylist_length(g_deviceObjectList);

    return length;
}
