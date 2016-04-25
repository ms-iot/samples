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

#include <stdio.h>
#include <string.h>
#include <jni.h>
#include <unistd.h>

#include "caleclient.h"
#include "caleserver.h"
#include "caleutils.h"
#include "caleinterface.h"
#include "caadapterutils.h"

#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "cathreadpool.h" /* for thread pool */
#include "camutex.h"
#include "uarraylist.h"
#include "org_iotivity_ca_CaLeClientInterface.h"

#define TAG PCF("CA_LE_CLIENT")

#define MICROSECS_PER_SEC 1000000

static const char METHODID_OBJECTNONPARAM[] = "()Landroid/bluetooth/BluetoothAdapter;";
static const char CLASSPATH_BT_ADAPTER[] = "android/bluetooth/BluetoothAdapter";
static const char CLASSPATH_BT_UUID[] = "java/util/UUID";
static const char CLASSPATH_BT_GATT[] = "android/bluetooth/BluetoothGatt";

JavaVM *g_jvm;
static u_arraylist_t *g_deviceList = NULL; // device list to have same UUID
static u_arraylist_t *g_gattObjectList = NULL;
static u_arraylist_t *g_deviceStateList = NULL;

static CAPacketReceiveCallback g_packetReceiveCallback = NULL;
static CABLEErrorHandleCallback g_clientErrorCallback;
static ca_thread_pool_t g_threadPoolHandle = NULL;
static jobject g_leScanCallback = NULL;
static jobject g_leGattCallback = NULL;
static jobject g_context = NULL;
static jobjectArray g_uuidList = NULL;

// it will be prevent to start send logic when adapter has stopped.
static bool g_isStartedLEClient = false;
static bool g_isStartedMulticastServer = false;
static bool g_isStartedScan = false;

static jbyteArray g_sendBuffer = NULL;
static uint32_t g_targetCnt = 0;
static uint32_t g_currentSentCnt = 0;
static bool g_isFinishedSendData = false;
static ca_mutex g_SendFinishMutex = NULL;
static ca_mutex g_threadMutex = NULL;
static ca_cond g_threadCond = NULL;
static ca_cond g_deviceDescCond = NULL;

static ca_mutex g_threadSendMutex = NULL;

static ca_mutex g_bleReqRespClientCbMutex = NULL;
static ca_mutex g_bleServerBDAddressMutex = NULL;

static ca_mutex g_deviceListMutex = NULL;
static ca_mutex g_gattObjectMutex = NULL;
static ca_mutex g_deviceStateListMutex = NULL;

static ca_mutex g_scanMutex = NULL;

static CABLEDataReceivedCallback g_CABLEClientDataReceivedCallback = NULL;

//getting jvm
void CALEClientJniInit()
{
    OIC_LOG(DEBUG, TAG, "CALEClientJniInit");
    g_jvm = (JavaVM*) CANativeJNIGetJavaVM();
}

void CALEClientJNISetContext()
{
    OIC_LOG(DEBUG, TAG, "CALEClientJNISetContext");
    g_context = (jobject) CANativeJNIGetContext();
}

CAResult_t CALECreateJniInterfaceObject()
{
    OIC_LOG(DEBUG, TAG, "CALECreateJniInterfaceObject");

    if (!g_context)
    {
        OIC_LOG(ERROR, TAG, "g_context is null");
        return CA_STATUS_FAILED;
    }

    if (!g_jvm)
    {
        OIC_LOG(ERROR, TAG, "g_jvm is null");
        return CA_STATUS_FAILED;
    }

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(INFO, TAG, "Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread has failed");
            return CA_STATUS_FAILED;
        }
        isAttached = true;
    }

    jclass jni_LEInterface = (*env)->FindClass(env, "org/iotivity/ca/CaLeClientInterface");
    if (!jni_LEInterface)
    {
        OIC_LOG(ERROR, TAG, "Could not get CaLeClientInterface class");
        goto error_exit;
    }

    jmethodID LeInterfaceConstructorMethod = (*env)->GetMethodID(env, jni_LEInterface, "<init>",
                                                                 "(Landroid/content/Context;)V");
    if (!LeInterfaceConstructorMethod)
    {
        OIC_LOG(ERROR, TAG, "Could not get CaLeClientInterface constructor method");
        goto error_exit;
    }

    (*env)->NewObject(env, jni_LEInterface, LeInterfaceConstructorMethod, g_context);
    OIC_LOG(DEBUG, TAG, "Create instance for CaLeClientInterface");

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    return CA_STATUS_OK;

error_exit:

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    return CA_STATUS_FAILED;
}

CAResult_t CALEClientInitialize(ca_thread_pool_t handle)
{
    OIC_LOG(DEBUG, TAG, "CALEClientInitialize");

    CALEClientJniInit();

    if (!g_jvm)
    {
        OIC_LOG(ERROR, TAG, "g_jvm is null");
        return CA_STATUS_FAILED;
    }

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(INFO, TAG, "Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread has failed");
            return CA_STATUS_FAILED;
        }
        isAttached = true;
    }

    CAResult_t ret = CALECheckPlatformVersion(env, 18);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "it is not supported");

        if (isAttached)
        {
            (*g_jvm)->DetachCurrentThread(g_jvm);
        }

        return ret;
    }

    g_threadPoolHandle = handle;

    ret = CALEClientInitGattMutexVaraibles();
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientInitGattMutexVaraibles has failed!");
        CALEClientTerminateGattMutexVariables();

        if (isAttached)
        {
            (*g_jvm)->DetachCurrentThread(g_jvm);
        }

        return ret;
    }

    g_deviceDescCond = ca_cond_new();

    // init mutex for send logic
    g_threadCond = ca_cond_new();

    CALEClientCreateDeviceList();
    CALEClientJNISetContext();

    ret = CALEClientCreateUUIDList();
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientCreateUUIDList has failed");

        if (isAttached)
        {
            (*g_jvm)->DetachCurrentThread(g_jvm);
        }

        return ret;
    }

    ret = CALECreateJniInterfaceObject(); /* create java caleinterface instance*/
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALECreateJniInterfaceObject has failed");

        if (isAttached)
        {
            (*g_jvm)->DetachCurrentThread(g_jvm);
        }

        return ret;
    }
    g_isStartedLEClient = true;

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    return CA_STATUS_OK;
}

void CALEClientTerminate()
{
    OIC_LOG(DEBUG, TAG, "CALEClientTerminate");

    if (!g_jvm)
    {
        OIC_LOG(ERROR, TAG, "g_jvm is null");
        return;
    }

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(INFO, TAG, "Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread has failed");
            return;
        }
        isAttached = true;
    }

    if (g_leScanCallback)
    {
        (*env)->DeleteGlobalRef(env, g_leScanCallback);
    }

    if (g_leGattCallback)
    {
        (*env)->DeleteGlobalRef(env, g_leGattCallback);
    }

    if (g_sendBuffer)
    {
        (*env)->DeleteGlobalRef(env, g_sendBuffer);
    }

    if (g_uuidList)
    {
        (*env)->DeleteGlobalRef(env, g_uuidList);
    }

    CAResult_t ret = CALEClientRemoveAllDeviceState();
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientRemoveAllDeviceState has failed");
    }

    ret = CALEClientRemoveAllScanDevices(env);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientRemoveAllScanDevices has failed");
    }

    ret = CALEClientRemoveAllGattObjs(env);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientRemoveAllGattObjs has failed");
    }

    g_isStartedMulticastServer = false;
    CALEClientSetScanFlag(false);
    CALEClientSetSendFinishFlag(false);

    CALEClientTerminateGattMutexVariables();

    ca_cond_free(g_deviceDescCond);
    ca_cond_free(g_threadCond);

    g_deviceDescCond = NULL;
    g_threadCond = NULL;

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }
}

void CALEClientSendFinish(JNIEnv *env, jobject gatt)
{
    OIC_LOG(DEBUG, TAG, "CALEClientSendFinish");
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");

    if (gatt)
    {
        CAResult_t res = CALEClientDisconnect(env, gatt);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientDisconnect has failed");
        }
    }
    CALEClientUpdateSendCnt(env);
}

CAResult_t CALEClientSendUnicastMessage(const char* address,
                                        const uint8_t* data,
                                        const uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CALEClientSendUnicastMessage(%s, %p)", address, data);
    VERIFY_NON_NULL(address, TAG, "address is null");
    VERIFY_NON_NULL(data, TAG, "data is null");

    return CALEClientSendUnicastMessageImpl(address, data, dataLen);
}

CAResult_t CALEClientSendMulticastMessage(const uint8_t* data,
                                          const uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CALEClientSendMulticastMessage(%p)", data);
    VERIFY_NON_NULL(data, TAG, "data is null");

    if (!g_jvm)
    {
        OIC_LOG(ERROR, TAG, "g_jvm is null");
        return CA_STATUS_FAILED;
    }

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(INFO, TAG, "Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread has failed");
            return CA_STATUS_FAILED;
        }
        isAttached = true;
    }

    CAResult_t ret = CALEClientSendMulticastMessageImpl(env, data, dataLen);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientSendMulticastMessageImpl has failed");
    }

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    return ret;
}

CAResult_t CALEClientStartUnicastServer(const char* address)
{
    OIC_LOG_V(DEBUG, TAG, "it is not needed in this platform (%s)", address);

    return CA_NOT_SUPPORTED;
}

CAResult_t CALEClientStartMulticastServer()
{
    OIC_LOG(DEBUG, TAG, "CALEClientStartMulticastServer");

    if (g_isStartedMulticastServer)
    {
        OIC_LOG(ERROR, TAG, "server is already started..it will be skipped");
        return CA_STATUS_FAILED;
    }

    if (!g_jvm)
    {
        OIC_LOG(ERROR, TAG, "g_jvm is null");
        return CA_STATUS_FAILED;
    }

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(INFO, TAG, "Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread has failed");
            return CA_STATUS_FAILED;
        }
        isAttached = true;
    }

    g_isStartedMulticastServer = true;
    CAResult_t ret = CALEClientStartScan();
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientStartScan has failed");
    }

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    return ret;
}

void CALEClientStopUnicastServer()
{
    OIC_LOG(DEBUG, TAG, "CALEClientStopUnicastServer");
}

void CALEClientStopMulticastServer()
{
    OIC_LOG(DEBUG, TAG, "CALEClientStopMulticastServer");
    g_isStartedMulticastServer = false;
    CAResult_t res = CALEClientStopScan();
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "CALEClientStartScan has failed");
        return;
    }
}

void CALEClientSetCallback(CAPacketReceiveCallback callback)
{
    g_packetReceiveCallback = callback;
}

void CASetBLEClientErrorHandleCallback(CABLEErrorHandleCallback callback)
{
    g_clientErrorCallback = callback;
}

CAResult_t CALEClientSendUnicastMessageImpl(const char* address, const uint8_t* data,
                                      const uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CALEClientSendUnicastMessageImpl, address: %s, data: %p", address,
              data);
    VERIFY_NON_NULL(address, TAG, "address is null");
    VERIFY_NON_NULL(data, TAG, "data is null");

    if (!g_jvm)
    {
        OIC_LOG(ERROR, TAG, "g_jvm is null");
        return CA_STATUS_FAILED;
    }

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(INFO, TAG, "Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread has failed");
            return CA_STATUS_FAILED;
        }
        isAttached = true;
    }

    ca_mutex_lock(g_threadSendMutex);

    CAResult_t ret = CA_STATUS_OK;
    if (g_context && g_deviceList)
    {
        uint32_t length = u_arraylist_length(g_deviceList);
        for (uint32_t index = 0; index < length; index++)
        {
            jobject jarrayObj = (jobject) u_arraylist_get(g_deviceList, index);
            if (!jarrayObj)
            {
                OIC_LOG(ERROR, TAG, "jarrayObj is null");
                goto error_exit;
            }

            jstring jni_setAddress = CALEGetAddressFromBTDevice(env, jarrayObj);
            if (!jni_setAddress)
            {
                OIC_LOG(ERROR, TAG, "jni_setAddress is null");
                goto error_exit;
            }

            const char* setAddress = (*env)->GetStringUTFChars(env, jni_setAddress, NULL);
            if (!setAddress)
            {
                OIC_LOG(ERROR, TAG, "setAddress is null");
                goto error_exit;
            }

            OIC_LOG_V(DEBUG, TAG, "remote device address is %s", setAddress);

            if (!strcmp(setAddress, address))
            {
                (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);

                // connect to gatt server
                ret = CALEClientStopScan();
                if (CA_STATUS_OK != ret)
                {
                    OIC_LOG(ERROR, TAG, "CALEClientStopScan has failed");
                    goto error_exit;
                }

                if (g_sendBuffer)
                {
                    (*env)->DeleteGlobalRef(env, g_sendBuffer);
                }
                jbyteArray jni_arr = (*env)->NewByteArray(env, dataLen);
                (*env)->SetByteArrayRegion(env, jni_arr, 0, dataLen, (jbyte*) data);
                g_sendBuffer = (jbyteArray)(*env)->NewGlobalRef(env, jni_arr);

                ret = CALEClientSendData(env, jarrayObj);
                if (CA_STATUS_OK != ret)
                {
                    OIC_LOG(ERROR, TAG, "CALEClientSendData in unicast is failed");
                    goto error_exit;
                }

                OIC_LOG(INFO, TAG, "wake up");
                break;
            }
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
        }
    }

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    // start LE Scan again
    ret = CALEClientStartScan();
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientStartScan has failed");
        ca_mutex_unlock(g_threadSendMutex);
        return res;
    }

    ca_mutex_unlock(g_threadSendMutex);
    OIC_LOG(INFO, TAG, "unicast - send success");
    return CA_STATUS_OK;

    // error label.
error_exit:

    // start LE Scan again
    ret = CALEClientStartScan();
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientStartScan has failed");
        ca_mutex_unlock(g_threadSendMutex);
        if (isAttached)
        {
            (*g_jvm)->DetachCurrentThread(g_jvm);
        }
        return res;
    }

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    if (g_clientErrorCallback)
    {
        g_clientErrorCallback(address, data, dataLen, CA_SEND_FAILED);
    }
    ca_mutex_unlock(g_threadSendMutex);
    return CA_SEND_FAILED;
}

CAResult_t CALEClientSendMulticastMessageImpl(JNIEnv *env, const uint8_t* data,
                                              const uint32_t dataLen)
{
    OIC_LOG_V(DEBUG, TAG, "CASendMulticastMessageImpl, send to, data: %p, %u", data, dataLen);
    VERIFY_NON_NULL(data, TAG, "data is null");
    VERIFY_NON_NULL(env, TAG, "env is null");

    if (!g_deviceList)
    {
        OIC_LOG(ERROR, TAG, "g_deviceList is null");
        return CA_STATUS_FAILED;
    }

    ca_mutex_lock(g_threadSendMutex);

    CALEClientSetSendFinishFlag(false);

    OIC_LOG(DEBUG, TAG, "set byteArray for data");
    if (g_sendBuffer)
    {
        (*env)->DeleteGlobalRef(env, g_sendBuffer);
    }

    if (0 == u_arraylist_length(g_deviceList))
    {
        // Wait for LE peripherals to be discovered.

        // Number of times to wait for discovery to complete.
        static size_t const RETRIES = 5;

        static uint64_t const TIMEOUT =
            2 * MICROSECS_PER_SEC;  // Microseconds

        bool devicesDiscovered = false;
        for (size_t i = 0;
             0 == u_arraylist_length(g_deviceList) && i < RETRIES;
             ++i)
        {
            if (ca_cond_wait_for(g_deviceDescCond,
                                 g_threadSendMutex,
                                 TIMEOUT) == 0)
            {
                devicesDiscovered = true;
            }
        }

        if (!devicesDiscovered)
        {
            goto error_exit;
        }
    }

    // connect to gatt server
    CAResult_t res = CALEClientStopScan();
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "CALEClientStopScan has failed");
        ca_mutex_unlock(g_threadSendMutex);
        return res;
    }
    uint32_t length = u_arraylist_length(g_deviceList);
    g_targetCnt = length;

    jbyteArray jni_arr = (*env)->NewByteArray(env, dataLen);
    (*env)->SetByteArrayRegion(env, jni_arr, 0, dataLen, (jbyte*) data);
    g_sendBuffer = (jbyteArray)(*env)->NewGlobalRef(env, jni_arr);

    for (uint32_t index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_deviceList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "jarrayObj is not available");
            continue;
        }

        res = CALEClientSendData(env, jarrayObj);
        if (res != CA_STATUS_OK)
        {
            OIC_LOG(ERROR, TAG, "BT device - send has failed");
        }

        jstring jni_address = CALEGetAddressFromBTDevice(env, jarrayObj);
        if (!jni_address)
        {
            OIC_LOG(ERROR, TAG, "CALEGetAddressFromBTDevice has failed");
            continue;
        }

        const char* address = (*env)->GetStringUTFChars(env, jni_address, NULL);
        if (!address)
        {
            OIC_LOG(ERROR, TAG, "address is not available");
            continue;
        }

        (*env)->ReleaseStringUTFChars(env, jni_address, address);
    }

    OIC_LOG(DEBUG, TAG, "connection routine is finished");

    // wait for finish to send data through "CALeGattServicesDiscoveredCallback"
    if (!g_isFinishedSendData)
    {
        ca_mutex_lock(g_threadMutex);
        ca_cond_wait(g_threadCond, g_threadMutex);
        OIC_LOG(DEBUG, TAG, "the data was sent for All devices");
        ca_mutex_unlock(g_threadMutex);
    }

    // start LE Scan again
    res = CALEClientStartScan();
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "CALEClientStartScan has failed");
        ca_mutex_unlock(g_threadSendMutex);
        return res;
    }

    ca_mutex_unlock(g_threadSendMutex);
    OIC_LOG(DEBUG, TAG, "OUT - CALEClientSendMulticastMessageImpl");
    return CA_STATUS_OK;

error_exit:
    res = CALEClientStartScan();
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "CALEClientStartScan has failed");
        ca_mutex_unlock(g_threadSendMutex);
        return res;
    }

    ca_mutex_unlock(g_threadSendMutex);
    OIC_LOG(DEBUG, TAG, "OUT - CALEClientSendMulticastMessageImpl");
    return CA_SEND_FAILED;
}

CAResult_t CALECheckSendState(const char* address)
{
    VERIFY_NON_NULL(address, TAG, "address is null");

    ca_mutex_lock(g_deviceStateListMutex);
    CALEState_t* state = CALEClientGetStateInfo(address);
    if (NULL == state)
    {
        OIC_LOG(ERROR, TAG, "state is null");
        ca_mutex_unlock(g_deviceStateListMutex);
        return CA_SEND_FAILED;
    }

    if (STATE_SEND_SUCCESS != state->sendState)
    {
        OIC_LOG(ERROR, TAG, "sendstate is not STATE_SEND_SUCCESS");
        ca_mutex_unlock(g_deviceStateListMutex);
        return CA_SEND_FAILED;
    }
    ca_mutex_unlock(g_deviceStateListMutex);
    return CA_STATUS_OK;
}

CAResult_t CALEClientSendData(JNIEnv *env, jobject device)
{
    OIC_LOG(DEBUG, TAG, "IN - CALEClientSendData");
    VERIFY_NON_NULL(device, TAG, "device is null");
    VERIFY_NON_NULL(env, TAG, "env is null");

    jstring jni_address = CALEGetAddressFromBTDevice(env, device);
    if (!jni_address)
    {
        OIC_LOG(ERROR, TAG, "CALEGetAddressFromBTDevice has failed");
        return CA_STATUS_FAILED;
    }

    const char* address = (*env)->GetStringUTFChars(env, jni_address, NULL);
    if (!address)
    {
        OIC_LOG(ERROR, TAG, "address is not available");
        return CA_STATUS_FAILED;
    }

    ca_mutex_lock(g_deviceStateListMutex);
    CALEState_t* state = CALEClientGetStateInfo(address);
    ca_mutex_unlock(g_deviceStateListMutex);
    if (!state)
    {
        OIC_LOG(DEBUG, TAG, "state is empty..start to connect LE");
        CAResult_t ret = CALEClientConnect(env, device, JNI_FALSE, g_leGattCallback);
        if (CA_STATUS_OK != ret)
        {
            OIC_LOG(ERROR, TAG, "CALEClientConnect has failed");
            (*env)->ReleaseStringUTFChars(env, jni_address, address);
            return ret;
        }
    }
    else
    {
        if (STATE_CONNECTED == state->connectedState)
        {
            OIC_LOG(INFO, TAG, "GATT has already connected");
            jobject gatt = CALEClientGetGattObjInList(env, address);
            if (!gatt)
            {
                OIC_LOG(ERROR, TAG, "CALEClientGetGattObjInList has failed");
                (*env)->ReleaseStringUTFChars(env, jni_address, address);
                return CA_STATUS_FAILED;
            }

            CAResult_t ret = CALEClientWriteCharacteristic(env, gatt);
            if (CA_STATUS_OK != ret)
            {
                OIC_LOG(ERROR, TAG, "CALEClientWriteCharacteristic has failed");
                (*env)->ReleaseStringUTFChars(env, jni_address, address);
                return ret;
            }
        }
        else
        {
            OIC_LOG(DEBUG, TAG, "start to connect LE");
            CAResult_t ret = CALEClientConnect(env, device, JNI_FALSE, g_leGattCallback);
            if (CA_STATUS_OK != ret)
            {
                OIC_LOG(ERROR, TAG, "CALEClientConnect has failed");
                (*env)->ReleaseStringUTFChars(env, jni_address, address);
                return ret;
            }
        }
    }

    (*env)->ReleaseStringUTFChars(env, jni_address, address);
    return CA_STATUS_OK;
}

jstring CALEClientGetAddressFromGattObj(JNIEnv *env, jobject gatt)
{
    VERIFY_NON_NULL_RET(gatt, TAG, "gatt is null", NULL);
    VERIFY_NON_NULL_RET(env, TAG, "env is null", NULL);

    jclass jni_cid_gattdevice_list = (*env)->FindClass(env, CLASSPATH_BT_GATT);
    if (!jni_cid_gattdevice_list)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_gattdevice_list is null");
        return NULL;
    }

    jmethodID jni_mid_getDevice = (*env)->GetMethodID(env, jni_cid_gattdevice_list, "getDevice",
                                                      "()Landroid/bluetooth/BluetoothDevice;");
    if (!jni_mid_getDevice)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getDevice is null");
        return NULL;
    }

    jobject jni_obj_device = (*env)->CallObjectMethod(env, gatt, jni_mid_getDevice);
    if (!jni_obj_device)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_device is null");
        return NULL;
    }

    jstring jni_address = CALEGetAddressFromBTDevice(env, jni_obj_device);
    if (!jni_address)
    {
        OIC_LOG(ERROR, TAG, "jni_address is null");
        return NULL;
    }

    return jni_address;
}

/**
 * BLE layer
 */
CAResult_t CALEClientGattClose(JNIEnv *env, jobject bluetoothGatt)
{
    // GATT CLOSE
    OIC_LOG(DEBUG, TAG, "Gatt Close");
    VERIFY_NON_NULL(bluetoothGatt, TAG, "bluetoothGatt is null");
    VERIFY_NON_NULL(env, TAG, "env is null");

    // get BluetoothGatt class
    OIC_LOG(DEBUG, TAG, "get BluetoothGatt class");
    jclass jni_cid_BluetoothGatt = (*env)->FindClass(env, CLASSPATH_BT_GATT);
    if (!jni_cid_BluetoothGatt)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BluetoothGatt is null");
        return CA_STATUS_FAILED;
    }

    jmethodID jni_mid_closeGatt = (*env)->GetMethodID(env, jni_cid_BluetoothGatt, "close", "()V");
    if (!jni_mid_closeGatt)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_closeGatt is null");
        return CA_STATUS_OK;
    }

    // call disconnect gatt method
    OIC_LOG(DEBUG, TAG, "request to close GATT");
    (*env)->CallVoidMethod(env, bluetoothGatt, jni_mid_closeGatt);

    if ((*env)->ExceptionCheck(env))
    {
        OIC_LOG(ERROR, TAG, "closeGATT has failed");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return CA_STATUS_FAILED;
    }

    return CA_STATUS_OK;
}

CAResult_t CALEClientStartScan()
{
    if (!g_isStartedMulticastServer)
    {
        OIC_LOG(ERROR, TAG, "server is not started yet..scan will be passed");
        return CA_STATUS_FAILED;
    }

    if (!g_isStartedLEClient)
    {
        OIC_LOG(ERROR, TAG, "LE client is not started");
        return CA_STATUS_FAILED;
    }

    if (!g_jvm)
    {
        OIC_LOG(ERROR, TAG, "g_jvm is null");
        return CA_STATUS_FAILED;
    }

    if (g_isStartedScan)
    {
        OIC_LOG(INFO, TAG, "scanning is already started");
        return CA_STATUS_OK;
    }

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(INFO, TAG, "Could not get JNIEnv pointer");

        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread has failed");
            return CA_STATUS_FAILED;
        }
        isAttached = true;
    }

    OIC_LOG(DEBUG, TAG, "CALEClientStartScan");

    CAResult_t ret = CA_STATUS_OK;
    // scan gatt server with UUID
    if (g_leScanCallback && g_uuidList)
    {
#ifdef UUID_SCAN
        ret = CALEClientStartScanWithUUIDImpl(env, g_uuidList, g_leScanCallback);
        if(CA_STATUS_OK != ret)
        {
            OIC_LOG(ERROR, TAG, "CALEClientStartScanWithUUIDImpl has failed");
        }
#else
        ret = CALEClientStartScanImpl(env, g_leScanCallback);
        if (CA_STATUS_OK != ret)
        {
            OIC_LOG(ERROR, TAG, "CALEClientStartScanImpl has failed");
        }
#endif
    }

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    return ret;
}

CAResult_t CALEClientStartScanImpl(JNIEnv *env, jobject callback)
{
    VERIFY_NON_NULL(callback, TAG, "callback is null");
    VERIFY_NON_NULL(env, TAG, "env is null");

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return CA_ADAPTER_NOT_ENABLED;
    }

    // get default bt adapter class
    jclass jni_cid_BTAdapter = (*env)->FindClass(env, CLASSPATH_BT_ADAPTER);
    if (!jni_cid_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "getState From BTAdapter: jni_cid_BTAdapter is null");
        return CA_STATUS_FAILED;
    }

    // get remote bt adapter method
    jmethodID jni_mid_getDefaultAdapter = (*env)->GetStaticMethodID(env, jni_cid_BTAdapter,
                                                                    "getDefaultAdapter",
                                                                    METHODID_OBJECTNONPARAM);
    if (!jni_mid_getDefaultAdapter)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getDefaultAdapter is null");
        return CA_STATUS_FAILED;
    }

    // get start le scan method
    jmethodID jni_mid_startLeScan = (*env)->GetMethodID(env, jni_cid_BTAdapter, "startLeScan",
                                                        "(Landroid/bluetooth/BluetoothAdapter$"
                                                        "LeScanCallback;)Z");
    if (!jni_mid_startLeScan)
    {
        OIC_LOG(ERROR, TAG, "startLeScan: jni_mid_startLeScan is null");
        return CA_STATUS_FAILED;
    }

    // gat bt adapter object
    jobject jni_obj_BTAdapter = (*env)->CallStaticObjectMethod(env, jni_cid_BTAdapter,
                                                               jni_mid_getDefaultAdapter);
    if (!jni_obj_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "getState From BTAdapter: jni_obj_BTAdapter is null");
        return CA_STATUS_FAILED;
    }

    // call start le scan method
    jboolean jni_obj_startLeScan = (*env)->CallBooleanMethod(env, jni_obj_BTAdapter,
                                                             jni_mid_startLeScan, callback);
    if (!jni_obj_startLeScan)
    {
        OIC_LOG(ERROR, TAG, "startLeScan is failed");
        return CA_STATUS_FAILED;
    }
    else
    {
        OIC_LOG(DEBUG, TAG, "startLeScan is started");
        CALEClientSetScanFlag(true);
    }

    return CA_STATUS_OK;
}

CAResult_t CALEClientStartScanWithUUIDImpl(JNIEnv *env, jobjectArray uuids, jobject callback)
{
    VERIFY_NON_NULL(callback, TAG, "callback is null");
    VERIFY_NON_NULL(uuids, TAG, "uuids is null");
    VERIFY_NON_NULL(env, TAG, "env is null");

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return CA_ADAPTER_NOT_ENABLED;
    }

    jclass jni_cid_BTAdapter = (*env)->FindClass(env, CLASSPATH_BT_ADAPTER);
    if (!jni_cid_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "getState From BTAdapter: jni_cid_BTAdapter is null");
        return CA_STATUS_FAILED;
    }

    // get remote bt adapter method
    jmethodID jni_mid_getDefaultAdapter = (*env)->GetStaticMethodID(env, jni_cid_BTAdapter,
                                                                    "getDefaultAdapter",
                                                                    METHODID_OBJECTNONPARAM);
    if (!jni_mid_getDefaultAdapter)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getDefaultAdapter is null");
        return CA_STATUS_FAILED;
    }

    // get start le scan method
    jmethodID jni_mid_startLeScan = (*env)->GetMethodID(env, jni_cid_BTAdapter, "startLeScan",
                                                        "([Ljava/util/UUID;Landroid/bluetooth/"
                                                        "BluetoothAdapter$LeScanCallback;)Z");
    if (!jni_mid_startLeScan)
    {
        OIC_LOG(ERROR, TAG, "startLeScan: jni_mid_startLeScan is null");
        return CA_STATUS_FAILED;
    }

    // get bt adapter object
    jobject jni_obj_BTAdapter = (*env)->CallStaticObjectMethod(env, jni_cid_BTAdapter,
                                                               jni_mid_getDefaultAdapter);
    if (!jni_obj_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "getState From BTAdapter: jni_obj_BTAdapter is null");
        return CA_STATUS_FAILED;
    }

    // call start le scan method
    jboolean jni_obj_startLeScan = (*env)->CallBooleanMethod(env, jni_obj_BTAdapter,
                                                             jni_mid_startLeScan, uuids, callback);
    if (!jni_obj_startLeScan)
    {
        OIC_LOG(ERROR, TAG, "startLeScan With UUID is failed");
        return CA_STATUS_FAILED;
    }
    else
    {
        OIC_LOG(DEBUG, TAG, "startLeScan With UUID is started");
        CALEClientSetScanFlag(true);
    }

    return CA_STATUS_OK;
}

jobject CALEClientGetUUIDObject(JNIEnv *env, const char* uuid)
{
    VERIFY_NON_NULL_RET(uuid, TAG, "uuid is null", NULL);
    VERIFY_NON_NULL_RET(env, TAG, "env is null", NULL);

    // setting UUID
    jclass jni_cid_uuid = (*env)->FindClass(env, CLASSPATH_BT_UUID);
    if (!jni_cid_uuid)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_uuid is null");
        return NULL;
    }

    jmethodID jni_mid_fromString = (*env)->GetStaticMethodID(env, jni_cid_uuid, "fromString",
                                                             "(Ljava/lang/String;)"
                                                             "Ljava/util/UUID;");
    if (!jni_mid_fromString)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_fromString is null");
        return NULL;
    }

    jstring jni_uuid = (*env)->NewStringUTF(env, uuid);
    jobject jni_obj_uuid = (*env)->CallStaticObjectMethod(env, jni_cid_uuid, jni_mid_fromString,
                                                          jni_uuid);
    if (!jni_obj_uuid)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_uuid is null");
        return NULL;
    }

    return jni_obj_uuid;
}

CAResult_t CALEClientStopScan()
{
    if (!g_jvm)
    {
        OIC_LOG(ERROR, TAG, "g_jvm is null");
        return CA_STATUS_FAILED;
    }

    if (!g_isStartedScan)
    {
        OIC_LOG(INFO, TAG, "scanning is already stopped");
        return CA_STATUS_OK;
    }

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(INFO, TAG, "Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread has failed");
            return CA_STATUS_FAILED;
        }
        isAttached = true;
    }

    CAResult_t ret = CALEClientStopScanImpl(env, g_leScanCallback);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientStopScanImpl has failed");
    }
    else
    {
        CALEClientSetScanFlag(false);
    }

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    return ret;
}

void CALEClientSetScanFlag(bool flag)
{
    ca_mutex_lock(g_scanMutex);
    g_isStartedScan = flag;
    ca_mutex_unlock(g_scanMutex);
}

CAResult_t CALEClientStopScanImpl(JNIEnv *env, jobject callback)
{
    OIC_LOG(DEBUG, TAG, "CALEClientStopScanImpl");
    VERIFY_NON_NULL(callback, TAG, "callback is null");
    VERIFY_NON_NULL(env, TAG, "env is null");

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return CA_ADAPTER_NOT_ENABLED;
    }

    // get default bt adapter class
    jclass jni_cid_BTAdapter = (*env)->FindClass(env, CLASSPATH_BT_ADAPTER);
    if (!jni_cid_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "getState From BTAdapter: jni_cid_BTAdapter is null");
        return CA_STATUS_FAILED;
    }

    // get remote bt adapter method
    jmethodID jni_mid_getDefaultAdapter = (*env)->GetStaticMethodID(env, jni_cid_BTAdapter,
                                                                    "getDefaultAdapter",
                                                                    METHODID_OBJECTNONPARAM);
    if (!jni_mid_getDefaultAdapter)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getDefaultAdapter is null");
        return CA_STATUS_FAILED;
    }

    // get start le scan method
    jmethodID jni_mid_stopLeScan = (*env)->GetMethodID(env, jni_cid_BTAdapter, "stopLeScan",
                                                       "(Landroid/bluetooth/"
                                                       "BluetoothAdapter$LeScanCallback;)V");
    if (!jni_mid_stopLeScan)
    {
        OIC_LOG(ERROR, TAG, "stopLeScan: jni_mid_stopLeScan is null");
        return CA_STATUS_FAILED;
    }

    // gat bt adapter object
    jobject jni_obj_BTAdapter = (*env)->CallStaticObjectMethod(env, jni_cid_BTAdapter,
                                                               jni_mid_getDefaultAdapter);
    if (!jni_obj_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_BTAdapter is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "CALL API - request to stop LE Scan");
    // call start le scan method
    (*env)->CallVoidMethod(env, jni_obj_BTAdapter, jni_mid_stopLeScan, callback);
    if ((*env)->ExceptionCheck(env))
    {
        OIC_LOG(ERROR, TAG, "stopLeScan has failed");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return CA_STATUS_FAILED;
    }

    return CA_STATUS_OK;
}

CAResult_t CALEClientConnect(JNIEnv *env, jobject bluetoothDevice, jboolean autoconnect,
                             jobject callback)
{
    OIC_LOG(DEBUG, TAG, "GATT CONNECT");
    VERIFY_NON_NULL(env, TAG, "env is null");
    VERIFY_NON_NULL(bluetoothDevice, TAG, "bluetoothDevice is null");
    VERIFY_NON_NULL(callback, TAG, "callback is null");

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return CA_ADAPTER_NOT_ENABLED;
    }

    jstring jni_address = CALEGetAddressFromBTDevice(env, bluetoothDevice);
    if (!jni_address)
    {
        OIC_LOG(ERROR, TAG, "bleConnect: CALEGetAddressFromBTDevice is null");
        return CA_STATUS_FAILED;
    }

    // get BluetoothDevice class
    OIC_LOG(DEBUG, TAG, "get BluetoothDevice class");
    jclass jni_cid_BluetoothDevice = (*env)->FindClass(env, "android/bluetooth/BluetoothDevice");
    if (!jni_cid_BluetoothDevice)
    {
        OIC_LOG(ERROR, TAG, "bleConnect: jni_cid_BluetoothDevice is null");
        return CA_STATUS_FAILED;
    }

    // get connectGatt method
    OIC_LOG(DEBUG, TAG, "get connectGatt method");
    jmethodID jni_mid_connectGatt = (*env)->GetMethodID(env, jni_cid_BluetoothDevice, "connectGatt",
                                                        "(Landroid/content/Context;ZLandroid/"
                                                        "bluetooth/BluetoothGattCallback;)"
                                                        "Landroid/bluetooth/BluetoothGatt;");
    if (!jni_mid_connectGatt)
    {
        OIC_LOG(ERROR, TAG, "bleConnect: jni_mid_connectGatt is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "Call object method - connectGatt");
    jobject jni_obj_connectGatt = (*env)->CallObjectMethod(env, bluetoothDevice,
                                                           jni_mid_connectGatt,
                                                           NULL,
                                                           autoconnect, callback);
    if (!jni_obj_connectGatt)
    {
        OIC_LOG(ERROR, TAG, "CALL API - connectGatt was failed..it will be removed");
        CALEClientRemoveDeviceInScanDeviceList(env, jni_address);
        CALEClientUpdateSendCnt(env);
        return CA_STATUS_FAILED;
    }
    else
    {
        OIC_LOG(DEBUG, TAG, "le connecting..please wait..");
    }
    return CA_STATUS_OK;
}

CAResult_t CALEClientDisconnect(JNIEnv *env, jobject bluetoothGatt)
{
    OIC_LOG(DEBUG, TAG, "GATT DISCONNECT");
    VERIFY_NON_NULL(env, TAG, "env is null");
    VERIFY_NON_NULL(bluetoothGatt, TAG, "bluetoothGatt is null");

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return CA_ADAPTER_NOT_ENABLED;
    }

    // get BluetoothGatt class
    jclass jni_cid_BluetoothGatt = (*env)->FindClass(env, CLASSPATH_BT_GATT);
    if (!jni_cid_BluetoothGatt)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BluetoothGatt is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "get gatt disconnect method");
    jmethodID jni_mid_disconnectGatt = (*env)->GetMethodID(env, jni_cid_BluetoothGatt,
                                                           "disconnect", "()V");
    if (!jni_mid_disconnectGatt)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_disconnectGatt is null");
        return CA_STATUS_FAILED;
    }

    // call disconnect gatt method
    (*env)->CallVoidMethod(env, bluetoothGatt, jni_mid_disconnectGatt);
    if ((*env)->ExceptionCheck(env))
    {
        OIC_LOG(ERROR, TAG, "disconnect has failed");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "disconnecting Gatt...");

    return CA_STATUS_OK;
}

CAResult_t CALEClientDisconnectAll(JNIEnv *env)
{
    OIC_LOG(DEBUG, TAG, "CALEClientDisconnectAll");
    VERIFY_NON_NULL(env, TAG, "env is null");

    if (!g_gattObjectList)
    {
        OIC_LOG(ERROR, TAG, "g_gattObjectList is null");
        return CA_STATUS_FAILED;
    }

    uint32_t length = u_arraylist_length(g_gattObjectList);
    for (uint32_t index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_gattObjectList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "jarrayObj is null");
            continue;
        }
        CAResult_t res = CALEClientDisconnect(env, jarrayObj);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientDisconnect has failed");
            continue;
        }
    }

    OICFree(g_gattObjectList);
    g_gattObjectList = NULL;

    return CA_STATUS_OK;
}

CAResult_t CALEClientDiscoverServices(JNIEnv *env, jobject bluetoothGatt)
{
    VERIFY_NON_NULL(env, TAG, "env is null");
    VERIFY_NON_NULL(bluetoothGatt, TAG, "bluetoothGatt is null");

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return CA_ADAPTER_NOT_ENABLED;
    }

    // get BluetoothGatt class
    OIC_LOG(DEBUG, TAG, "get BluetoothGatt class");
    jclass jni_cid_BluetoothGatt = (*env)->FindClass(env, CLASSPATH_BT_GATT);
    if (!jni_cid_BluetoothGatt)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BluetoothGatt is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "discovery gatt services method");
    jmethodID jni_mid_discoverServices = (*env)->GetMethodID(env, jni_cid_BluetoothGatt,
                                                             "discoverServices", "()Z");
    if (!jni_mid_discoverServices)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_discoverServices is null");
        return CA_STATUS_FAILED;
    }
    // call disconnect gatt method
    OIC_LOG(DEBUG, TAG, "CALL API - request discovery gatt services");
    jboolean ret = (*env)->CallBooleanMethod(env, bluetoothGatt, jni_mid_discoverServices);
    if (!ret)
    {
        OIC_LOG(ERROR, TAG, "discoverServices has not been started");
        return CA_STATUS_FAILED;
    }

    return CA_STATUS_OK;
}

CAResult_t CALEClientWriteCharacteristic(JNIEnv *env, jobject gatt)
{
    VERIFY_NON_NULL(env, TAG, "env is null");
    VERIFY_NON_NULL(gatt, TAG, "gatt is null");

    // send data
    jobject jni_obj_character = CALEClientCreateGattCharacteristic(env, gatt, g_sendBuffer);
    if (!jni_obj_character)
    {
        CALEClientSendFinish(env, gatt);
        return CA_STATUS_FAILED;
    }

    CAResult_t ret = CALEClientWriteCharacteristicImpl(env, gatt, jni_obj_character);
    if (CA_STATUS_OK != ret)
    {
        CALEClientSendFinish(env, gatt);
        return ret;
    }

    return CA_STATUS_OK;
}

CAResult_t CALEClientWriteCharacteristicImpl(JNIEnv *env, jobject bluetoothGatt,
                                           jobject gattCharacteristic)
{
    OIC_LOG(DEBUG, TAG, "WRITE GATT CHARACTERISTIC");
    VERIFY_NON_NULL(env, TAG, "env is null");
    VERIFY_NON_NULL(bluetoothGatt, TAG, "bluetoothGatt is null");
    VERIFY_NON_NULL(gattCharacteristic, TAG, "gattCharacteristic is null");

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return CA_STATUS_FAILED;
    }

    // get BluetoothGatt class
    OIC_LOG(DEBUG, TAG, "get BluetoothGatt class");
    jclass jni_cid_BluetoothGatt = (*env)->FindClass(env, CLASSPATH_BT_GATT);
    if (!jni_cid_BluetoothGatt)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BluetoothGatt is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "write characteristic method");
    jmethodID jni_mid_writeCharacteristic = (*env)->GetMethodID(env, jni_cid_BluetoothGatt,
                                                                "writeCharacteristic",
                                                                "(Landroid/bluetooth/"
                                                                "BluetoothGattCharacteristic;)Z");
    if (!jni_mid_writeCharacteristic)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_writeCharacteristic is null");
        return CA_STATUS_FAILED;
    }

    // call disconnect gatt method
    OIC_LOG(DEBUG, TAG, "CALL API - request to write gatt characteristic");
    jboolean ret = (jboolean)(*env)->CallBooleanMethod(env, bluetoothGatt,
                                                       jni_mid_writeCharacteristic,
                                                       gattCharacteristic);
    if (ret)
    {
        OIC_LOG(DEBUG, TAG, "writeCharacteristic success");
    }
    else
    {
        OIC_LOG(ERROR, TAG, "writeCharacteristic has failed");
        return CA_STATUS_FAILED;
    }

    return CA_STATUS_OK;
}

CAResult_t CALEClientReadCharacteristic(JNIEnv *env, jobject bluetoothGatt)
{
    VERIFY_NON_NULL(env, TAG, "env is null");
    VERIFY_NON_NULL(bluetoothGatt, TAG, "bluetoothGatt is null");

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return CA_STATUS_FAILED;
    }

    jclass jni_cid_BluetoothGatt = (*env)->FindClass(env, CLASSPATH_BT_GATT);
    if (!jni_cid_BluetoothGatt)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BluetoothGatt is null");
        return CA_STATUS_FAILED;
    }

    jstring jni_uuid = (*env)->NewStringUTF(env, OIC_GATT_CHARACTERISTIC_RESPONSE_UUID);
    if (!jni_uuid)
    {
        OIC_LOG(ERROR, TAG, "jni_uuid is null");
        return CA_STATUS_FAILED;
    }

    jobject jni_obj_GattCharacteristic = CALEClientGetGattService(env, bluetoothGatt, jni_uuid);
    if (!jni_obj_GattCharacteristic)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_GattCharacteristic is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "read characteristic method");
    jmethodID jni_mid_readCharacteristic = (*env)->GetMethodID(env, jni_cid_BluetoothGatt,
                                                               "readCharacteristic",
                                                               "(Landroid/bluetooth/"
                                                               "BluetoothGattCharacteristic;)Z");
    if (!jni_mid_readCharacteristic)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_readCharacteristic is null");
        return CA_STATUS_FAILED;
    }

    // call disconnect gatt method
    OIC_LOG(DEBUG, TAG, "CALL API - request to read gatt characteristic");
    jboolean ret = (*env)->CallBooleanMethod(env, bluetoothGatt, jni_mid_readCharacteristic,
                                             jni_obj_GattCharacteristic);
    if (ret)
    {
        OIC_LOG(DEBUG, TAG, "readCharacteristic success");
    }
    else
    {
        OIC_LOG(ERROR, TAG, "readCharacteristic has failed");
        return CA_STATUS_FAILED;
    }

    return CA_STATUS_OK;
}

CAResult_t CALEClientSetCharacteristicNotification(JNIEnv *env, jobject bluetoothGatt,
                                                   jobject characteristic)
{
    VERIFY_NON_NULL(env, TAG, "env is null");
    VERIFY_NON_NULL(bluetoothGatt, TAG, "bluetoothGatt is null");
    VERIFY_NON_NULL(characteristic, TAG, "characteristic is null");

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return CA_ADAPTER_NOT_ENABLED;
    }

    // get BluetoothGatt class
    OIC_LOG(DEBUG, TAG, "CALEClientSetCharacteristicNotification");
    jclass jni_cid_BluetoothGatt = (*env)->FindClass(env, CLASSPATH_BT_GATT);
    if (!jni_cid_BluetoothGatt)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BluetoothGatt is null");
        return CA_STATUS_FAILED;
    }

    // set Characteristic Notification
    jmethodID jni_mid_setNotification = (*env)->GetMethodID(env, jni_cid_BluetoothGatt,
                                                            "setCharacteristicNotification",
                                                            "(Landroid/bluetooth/"
                                                            "BluetoothGattCharacteristic;Z)Z");
    if (!jni_mid_setNotification)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getService is null");
        return CA_STATUS_FAILED;
    }

    jboolean ret = (*env)->CallBooleanMethod(env, bluetoothGatt, jni_mid_setNotification,
                                             characteristic, JNI_TRUE);
    if (JNI_TRUE == ret)
    {
        OIC_LOG(DEBUG, TAG, "CALL API - setCharacteristicNotification success");
    }
    else
    {
        OIC_LOG(ERROR, TAG, "CALL API - setCharacteristicNotification has failed");
        return CA_STATUS_FAILED;
    }

    return CA_STATUS_OK;
}

jobject CALEClientGetGattService(JNIEnv *env, jobject bluetoothGatt, jstring characterUUID)
{
    VERIFY_NON_NULL_RET(env, TAG, "env is null", NULL);
    VERIFY_NON_NULL_RET(bluetoothGatt, TAG, "bluetoothGatt is null", NULL);
    VERIFY_NON_NULL_RET(characterUUID, TAG, "characterUUID is null", NULL);

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return NULL;
    }

    // get BluetoothGatt class
    OIC_LOG(DEBUG, TAG, "CALEClientGetGattService");
    jclass jni_cid_BluetoothGatt = (*env)->FindClass(env, CLASSPATH_BT_GATT);
    if (!jni_cid_BluetoothGatt)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BluetoothGatt is null");
        return NULL;
    }

    jmethodID jni_mid_getService = (*env)->GetMethodID(
            env, jni_cid_BluetoothGatt, "getService",
            "(Ljava/util/UUID;)Landroid/bluetooth/BluetoothGattService;");
    if (!jni_mid_getService)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getService is null");
        return NULL;
    }

    jobject jni_obj_service_uuid = CALEClientGetUUIDObject(env, OIC_GATT_SERVICE_UUID);
    if (!jni_obj_service_uuid)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_service_uuid is null");
        return NULL;
    }

    // get bluetooth gatt service
    OIC_LOG(DEBUG, TAG, "request to get service");
    jobject jni_obj_gattService = (*env)->CallObjectMethod(env, bluetoothGatt, jni_mid_getService,
                                                           jni_obj_service_uuid);
    if (!jni_obj_gattService)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_gattService is null");
        return NULL;
    }

    // get bluetooth gatt service class
    jclass jni_cid_BluetoothGattService = (*env)->FindClass(
            env, "android/bluetooth/BluetoothGattService");
    if (!jni_cid_BluetoothGattService)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BluetoothGattService is null");
        return NULL;
    }

    OIC_LOG(DEBUG, TAG, "get gatt getCharacteristic method");
    jmethodID jni_mid_getCharacteristic = (*env)->GetMethodID(env, jni_cid_BluetoothGattService,
                                                              "getCharacteristic",
                                                              "(Ljava/util/UUID;)"
                                                              "Landroid/bluetooth/"
                                                              "BluetoothGattCharacteristic;");
    if (!jni_mid_getCharacteristic)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getCharacteristic is null");
        return NULL;
    }

    const char* uuid = (*env)->GetStringUTFChars(env, characterUUID, NULL);
    if (!uuid)
    {
        OIC_LOG(ERROR, TAG, "uuid is null");
        return NULL;
    }

    jobject jni_obj_tx_uuid = CALEClientGetUUIDObject(env, uuid);
    if (!jni_obj_tx_uuid)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_tx_uuid is null");
        (*env)->ReleaseStringUTFChars(env, characterUUID, uuid);
        return NULL;
    }

    OIC_LOG(DEBUG, TAG, "request to get Characteristic");
    jobject jni_obj_GattCharacteristic = (*env)->CallObjectMethod(env, jni_obj_gattService,
                                                                  jni_mid_getCharacteristic,
                                                                  jni_obj_tx_uuid);

    (*env)->ReleaseStringUTFChars(env, characterUUID, uuid);
    return jni_obj_GattCharacteristic;
}

jobject CALEClientCreateGattCharacteristic(JNIEnv *env, jobject bluetoothGatt, jbyteArray data)
{
    OIC_LOG(DEBUG, TAG, "CALEClientCreateGattCharacteristic");
    VERIFY_NON_NULL_RET(env, TAG, "env is null", NULL);
    VERIFY_NON_NULL_RET(bluetoothGatt, TAG, "bluetoothGatt is null", NULL);
    VERIFY_NON_NULL_RET(data, TAG, "data is null", NULL);

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return NULL;
    }

    jstring jni_uuid = (*env)->NewStringUTF(env, OIC_GATT_CHARACTERISTIC_REQUEST_UUID);
    if (!jni_uuid)
    {
        OIC_LOG(ERROR, TAG, "jni_uuid is null");
        return NULL;
    }

    jobject jni_obj_GattCharacteristic = CALEClientGetGattService(env, bluetoothGatt, jni_uuid);
    if (!jni_obj_GattCharacteristic)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_GattCharacteristic is null");
        return NULL;
    }

    jclass jni_cid_BTGattCharacteristic = (*env)->FindClass(env, "android/bluetooth"
                                                            "/BluetoothGattCharacteristic");
    if (!jni_cid_BTGattCharacteristic)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BTGattCharacteristic is null");
        return NULL;
    }

    OIC_LOG(DEBUG, TAG, "set value in Characteristic");
    jmethodID jni_mid_setValue = (*env)->GetMethodID(env, jni_cid_BTGattCharacteristic, "setValue",
                                                     "([B)Z");
    if (!jni_mid_setValue)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_setValue is null");
        return NULL;
    }

    jboolean ret = (*env)->CallBooleanMethod(env, jni_obj_GattCharacteristic, jni_mid_setValue,
                                             data);
    if (JNI_TRUE == ret)
    {
        OIC_LOG(DEBUG, TAG, "the locally stored value has been set");
    }
    else
    {
        OIC_LOG(ERROR, TAG, "the locally stored value hasn't been set");
        return NULL;
    }

    // set Write Type
    jmethodID jni_mid_setWriteType = (*env)->GetMethodID(env, jni_cid_BTGattCharacteristic,
                                                         "setWriteType", "(I)V");
    if (!jni_mid_setWriteType)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_setWriteType is null");
        return NULL;
    }

    jfieldID jni_fid_no_response = (*env)->GetStaticFieldID(env, jni_cid_BTGattCharacteristic,
                                                            "WRITE_TYPE_NO_RESPONSE", "I");
    if (!jni_fid_no_response)
    {
        OIC_LOG(ERROR, TAG, "jni_fid_no_response is not available");
        return NULL;
    }

    jint jni_int_val = (*env)->GetStaticIntField(env, jni_cid_BTGattCharacteristic,
                                                 jni_fid_no_response);

    (*env)->CallVoidMethod(env, jni_obj_GattCharacteristic, jni_mid_setWriteType, jni_int_val);

    return jni_obj_GattCharacteristic;
}

jbyteArray CALEClientGetValueFromCharacteristic(JNIEnv *env, jobject characteristic)
{
    VERIFY_NON_NULL_RET(characteristic, TAG, "characteristic is null", NULL);
    VERIFY_NON_NULL_RET(env, TAG, "env is null", NULL);

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return NULL;
    }

    jclass jni_cid_BTGattCharacteristic = (*env)->FindClass(env, "android/bluetooth/"
                                                            "BluetoothGattCharacteristic");
    if (!jni_cid_BTGattCharacteristic)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BTGattCharacteristic is null");
        return NULL;
    }

    OIC_LOG(DEBUG, TAG, "get value in Characteristic");
    jmethodID jni_mid_getValue = (*env)->GetMethodID(env, jni_cid_BTGattCharacteristic, "getValue",
                                                     "()[B");
    if (!jni_mid_getValue)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getValue is null");
        return NULL;
    }

    jbyteArray jni_obj_data_array = (*env)->CallObjectMethod(env, characteristic,
                                                             jni_mid_getValue);
    return jni_obj_data_array;
}

CAResult_t CALEClientCreateUUIDList()
{
    if (!g_jvm)
    {
        OIC_LOG(ERROR, TAG, "g_jvm is null");
        return CA_STATUS_FAILED;
    }

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(INFO, TAG, "Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread has failed");
            return CA_STATUS_FAILED;
        }
        isAttached = true;
    }

    // create new object array
    jclass jni_cid_uuid_list = (*env)->FindClass(env, CLASSPATH_BT_UUID);
    if (!jni_cid_uuid_list)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_uuid_list is null");
        goto error_exit;
    }

    jobjectArray jni_obj_uuid_list = (jobjectArray)(*env)->NewObjectArray(env, 1,
                                                                          jni_cid_uuid_list, NULL);
    if (!jni_obj_uuid_list)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_uuid_list is null");
        goto error_exit;
    }

    // make uuid list
    jobject jni_obj_uuid = CALEClientGetUUIDObject(env, OIC_GATT_SERVICE_UUID);
    if (!jni_obj_uuid)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_uuid is null");
        goto error_exit;
    }
    (*env)->SetObjectArrayElement(env, jni_obj_uuid_list, 0, jni_obj_uuid);

    g_uuidList = (jobjectArray)(*env)->NewGlobalRef(env, jni_obj_uuid_list);

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    return CA_STATUS_OK;

    // error label.
error_exit:

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }
    return CA_STATUS_FAILED;
}

CAResult_t CALEClientSetUUIDToDescriptor(JNIEnv *env, jobject bluetoothGatt,
                                         jobject characteristic)
{
    VERIFY_NON_NULL(env, TAG, "env is null");
    VERIFY_NON_NULL(bluetoothGatt, TAG, "bluetoothGatt is null");
    VERIFY_NON_NULL(characteristic, TAG, "characteristic is null");

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        return CA_ADAPTER_NOT_ENABLED;
    }

    OIC_LOG(DEBUG, TAG, "CALEClientSetUUIDToDescriptor");
    jclass jni_cid_BTGattCharacteristic = (*env)->FindClass(env, "android/bluetooth/"
                                                            "BluetoothGattCharacteristic");
    if (!jni_cid_BTGattCharacteristic)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_BTGattCharacteristic is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "set value in Characteristic");
    jmethodID jni_mid_getDescriptor = (*env)->GetMethodID(env, jni_cid_BTGattCharacteristic,
                                                          "getDescriptor",
                                                          "(Ljava/util/UUID;)Landroid/bluetooth/"
                                                          "BluetoothGattDescriptor;");
    if (!jni_mid_getDescriptor)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_getDescriptor is null");
        return CA_STATUS_FAILED;
    }

    jobject jni_obj_cc_uuid = CALEClientGetUUIDObject(env, OIC_GATT_CHARACTERISTIC_CONFIG_UUID);
    if (!jni_obj_cc_uuid)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_cc_uuid is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "request to get descriptor");
    jobject jni_obj_descriptor = (*env)->CallObjectMethod(env, characteristic,
                                                          jni_mid_getDescriptor, jni_obj_cc_uuid);
    if (!jni_obj_descriptor)
    {
        OIC_LOG(ERROR, TAG, "jni_obj_descriptor is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "set value in descriptor");
    jclass jni_cid_descriptor = (*env)->FindClass(env,
                                                  "android/bluetooth/BluetoothGattDescriptor");
    if (!jni_cid_descriptor)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_descriptor is null");
        return CA_STATUS_FAILED;
    }

    jmethodID jni_mid_setValue = (*env)->GetMethodID(env, jni_cid_descriptor, "setValue", "([B)Z");
    if (!jni_mid_setValue)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_setValue is null");
        return CA_STATUS_FAILED;
    }

    jfieldID jni_fid_NotiValue = (*env)->GetStaticFieldID(env, jni_cid_descriptor,
                                                          "ENABLE_NOTIFICATION_VALUE", "[B");
    if (!jni_fid_NotiValue)
    {
        OIC_LOG(ERROR, TAG, "jni_fid_NotiValue is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "get ENABLE_NOTIFICATION_VALUE");

    jboolean jni_setvalue = (*env)->CallBooleanMethod(
            env, jni_obj_descriptor, jni_mid_setValue,
            (jbyteArray)(*env)->GetStaticObjectField(env, jni_cid_descriptor, jni_fid_NotiValue));
    if (jni_setvalue)
    {
        OIC_LOG(DEBUG, TAG, "setValue success");
    }
    else
    {
        OIC_LOG(ERROR, TAG, "setValue has failed");
        return CA_STATUS_FAILED;
    }

    jclass jni_cid_gatt = (*env)->FindClass(env, "android/bluetooth/BluetoothGatt");
    if (!jni_cid_gatt)
    {
        OIC_LOG(ERROR, TAG, "jni_cid_gatt is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "write Descriptor in gatt object");
    jmethodID jni_mid_writeDescriptor = (*env)->GetMethodID(env, jni_cid_gatt, "writeDescriptor",
                                                            "(Landroid/bluetooth/"
                                                            "BluetoothGattDescriptor;)Z");
    if (!jni_mid_writeDescriptor)
    {
        OIC_LOG(ERROR, TAG, "jni_mid_writeDescriptor is null");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "request to write descriptor");
    jboolean jni_ret = (*env)->CallBooleanMethod(env, bluetoothGatt, jni_mid_writeDescriptor,
                                                 jni_obj_descriptor);
    if (jni_ret)
    {
        OIC_LOG(DEBUG, TAG, "writeDescriptor success");
    }
    else
    {
        OIC_LOG(ERROR, TAG, "writeDescriptor has failed");
        return CA_STATUS_FAILED;
    }

    return CA_STATUS_OK;
}

void CALEClientCreateScanDeviceList(JNIEnv *env)
{
    OIC_LOG(DEBUG, TAG, "CALEClientCreateScanDeviceList");
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");

    ca_mutex_lock(g_deviceListMutex);
    // create new object array
    if (g_deviceList == NULL)
    {
        OIC_LOG(DEBUG, TAG, "Create device list");

        g_deviceList = u_arraylist_create();
    }
    ca_mutex_unlock(g_deviceListMutex);
}

CAResult_t CALEClientAddScanDeviceToList(JNIEnv *env, jobject device)
{
    OIC_LOG(DEBUG, TAG, "IN - CALEClientAddScanDeviceToList");
    VERIFY_NON_NULL(device, TAG, "device is null");
    VERIFY_NON_NULL(env, TAG, "env is null");

    ca_mutex_lock(g_deviceListMutex);

    if (!g_deviceList)
    {
        OIC_LOG(ERROR, TAG, "gdevice_list is null");
        ca_mutex_unlock(g_deviceListMutex);
        return CA_STATUS_FAILED;
    }

    jstring jni_remoteAddress = CALEGetAddressFromBTDevice(env, device);
    if (!jni_remoteAddress)
    {
        OIC_LOG(ERROR, TAG, "jni_remoteAddress is null");
        ca_mutex_unlock(g_deviceListMutex);
        return CA_STATUS_FAILED;
    }

    const char* remoteAddress = (*env)->GetStringUTFChars(env, jni_remoteAddress, NULL);
    if (!remoteAddress)
    {
        OIC_LOG(ERROR, TAG, "remoteAddress is null");
        ca_mutex_unlock(g_deviceListMutex);
        return CA_STATUS_FAILED;
    }

    if (!CALEClientIsDeviceInScanDeviceList(env, remoteAddress))
    {
        jobject gdevice = (*env)->NewGlobalRef(env, device);
        u_arraylist_add(g_deviceList, gdevice);
        ca_cond_signal(g_deviceDescCond);
        OIC_LOG(DEBUG, TAG, "Set Object to Array as Element");
    }
    (*env)->ReleaseStringUTFChars(env, jni_remoteAddress, remoteAddress);

    ca_mutex_unlock(g_deviceListMutex);

    OIC_LOG(DEBUG, TAG, "OUT - CALEClientAddScanDeviceToList");
    return CA_STATUS_OK;
}

bool CALEClientIsDeviceInScanDeviceList(JNIEnv *env, const char* remoteAddress)
{
    OIC_LOG(DEBUG, TAG, "IN - CALEClientIsDeviceInScanDeviceList");
    VERIFY_NON_NULL_RET(env, TAG, "env is null", NULL);
    VERIFY_NON_NULL_RET(remoteAddress, TAG, "remoteAddress is null", true);

    if (!g_deviceList)
    {
        OIC_LOG(DEBUG, TAG, "g_deviceList is null");
        return true;
    }

    uint32_t length = u_arraylist_length(g_deviceList);
    for (uint32_t index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_deviceList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "jarrayObj is null");
            return true;
        }

        jstring jni_setAddress = CALEGetAddressFromBTDevice(env, jarrayObj);
        if (!jni_setAddress)
        {
            OIC_LOG(ERROR, TAG, "jni_setAddress is null");
            return true;
        }

        const char* setAddress = (*env)->GetStringUTFChars(env, jni_setAddress, NULL);
        if (!setAddress)
        {
            OIC_LOG(ERROR, TAG, "setAddress is null");
            return true;
        }

        if (!strcmp(remoteAddress, setAddress))
        {
            OIC_LOG(DEBUG, TAG, "the device is already set");
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            return true;
        }

        (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
    }

    OIC_LOG(DEBUG, TAG, "OUT - CALEClientIsDeviceInScanDeviceList");
    OIC_LOG(DEBUG, TAG, "there are no the device in list. we can add");

    return false;
}

CAResult_t CALEClientRemoveAllScanDevices(JNIEnv *env)
{
    OIC_LOG(DEBUG, TAG, "CALEClientRemoveAllScanDevices");
    VERIFY_NON_NULL(env, TAG, "env is null");

    ca_mutex_lock(g_deviceListMutex);

    if (!g_deviceList)
    {
        OIC_LOG(ERROR, TAG, "g_deviceList is null");
        ca_mutex_unlock(g_deviceListMutex);
        return CA_STATUS_FAILED;
    }

    uint32_t length = u_arraylist_length(g_deviceList);
    for (uint32_t index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_deviceList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "jarrayObj is null");
            continue;
        }
        (*env)->DeleteGlobalRef(env, jarrayObj);
    }

    OICFree(g_deviceList);
    g_deviceList = NULL;

    ca_mutex_unlock(g_deviceListMutex);
    return CA_STATUS_OK;
}

CAResult_t CALEClientRemoveDeviceInScanDeviceList(JNIEnv *env, jstring address)
{
    OIC_LOG(DEBUG, TAG, "CALEClientRemoveDeviceInScanDeviceList");
    VERIFY_NON_NULL(address, TAG, "address is null");
    VERIFY_NON_NULL(env, TAG, "env is null");

    ca_mutex_lock(g_deviceListMutex);

    if (!g_deviceList)
    {
        OIC_LOG(ERROR, TAG, "g_deviceList is null");
        ca_mutex_unlock(g_deviceListMutex);
        return CA_STATUS_FAILED;
    }

    uint32_t length = u_arraylist_length(g_deviceList);
    for (uint32_t index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_deviceList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "jarrayObj is null");
            ca_mutex_unlock(g_deviceListMutex);
            return CA_STATUS_FAILED;
        }

        jstring jni_setAddress = CALEGetAddressFromBTDevice(env, jarrayObj);
        if (!jni_setAddress)
        {
            OIC_LOG(ERROR, TAG, "jni_setAddress is null");
            ca_mutex_unlock(g_deviceListMutex);
            return CA_STATUS_FAILED;
        }

        const char* setAddress = (*env)->GetStringUTFChars(env, jni_setAddress, NULL);
        if (!setAddress)
        {
            OIC_LOG(ERROR, TAG, "setAddress is null");
            ca_mutex_unlock(g_deviceListMutex);
            return CA_STATUS_FAILED;
        }

        const char* remoteAddress = (*env)->GetStringUTFChars(env, address, NULL);
        if (!remoteAddress)
        {
            OIC_LOG(ERROR, TAG, "remoteAddress is null");
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            ca_mutex_unlock(g_deviceListMutex);
            return CA_STATUS_FAILED;
        }

        if (!strcmp(setAddress, remoteAddress))
        {
            OIC_LOG_V(DEBUG, TAG, "remove object : %s", remoteAddress);
            (*env)->DeleteGlobalRef(env, jarrayObj);
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            (*env)->ReleaseStringUTFChars(env, address, remoteAddress);

            if (NULL == u_arraylist_remove(g_deviceList, index))
            {
                OIC_LOG(ERROR, TAG, "List removal failed.");
                ca_mutex_unlock(g_deviceListMutex);
                return CA_STATUS_FAILED;
            }
            ca_mutex_unlock(g_deviceListMutex);
            return CA_STATUS_OK;
        }
        (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
        (*env)->ReleaseStringUTFChars(env, address, remoteAddress);
    }

    ca_mutex_unlock(g_deviceListMutex);
    OIC_LOG(DEBUG, TAG, "There are no object in the device list");

    return CA_STATUS_OK;
}

/**
 * Gatt Object List
 */

CAResult_t CALEClientAddGattobjToList(JNIEnv *env, jobject gatt)
{
    OIC_LOG(DEBUG, TAG, "CALEClientAddGattobjToList");
    VERIFY_NON_NULL(env, TAG, "env is null");
    VERIFY_NON_NULL(gatt, TAG, "gatt is null");

    ca_mutex_lock(g_gattObjectMutex);

    jstring jni_remoteAddress = CALEClientGetAddressFromGattObj(env, gatt);
    if (!jni_remoteAddress)
    {
        OIC_LOG(ERROR, TAG, "jni_remoteAddress is null");
        ca_mutex_unlock(g_gattObjectMutex);
        return CA_STATUS_FAILED;
    }

    const char* remoteAddress = (*env)->GetStringUTFChars(env, jni_remoteAddress, NULL);
    if (!remoteAddress)
    {
        OIC_LOG(ERROR, TAG, "remoteAddress is null");
        ca_mutex_unlock(g_gattObjectMutex);
        return CA_STATUS_FAILED;
    }

    if (!CALEClientIsGattObjInList(env, remoteAddress))
    {
        jobject newGatt = (*env)->NewGlobalRef(env, gatt);
        u_arraylist_add(g_gattObjectList, newGatt);
        OIC_LOG(DEBUG, TAG, "Set GATT Object to Array as Element");
    }

    (*env)->ReleaseStringUTFChars(env, jni_remoteAddress, remoteAddress);
    ca_mutex_unlock(g_gattObjectMutex);
    return CA_STATUS_OK;
}

bool CALEClientIsGattObjInList(JNIEnv *env, const char* remoteAddress)
{
    OIC_LOG(DEBUG, TAG, "CALEClientIsGattObjInList");
    VERIFY_NON_NULL(env, TAG, "env is null");
    VERIFY_NON_NULL_RET(remoteAddress, TAG, "remoteAddress is null", true);

    uint32_t length = u_arraylist_length(g_gattObjectList);
    for (uint32_t index = 0; index < length; index++)
    {

        jobject jarrayObj = (jobject) u_arraylist_get(g_gattObjectList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "jarrayObj is null");
            return true;
        }

        jstring jni_setAddress = CALEClientGetAddressFromGattObj(env, jarrayObj);
        if (!jni_setAddress)
        {
            OIC_LOG(ERROR, TAG, "jni_setAddress is null");
            return true;
        }

        const char* setAddress = (*env)->GetStringUTFChars(env, jni_setAddress, NULL);
        if (!setAddress)
        {
            OIC_LOG(ERROR, TAG, "setAddress is null");
            return true;
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

    OIC_LOG(DEBUG, TAG, "There are no GATT object in list. it can be added");
    return false;
}

jobject CALEClientGetGattObjInList(JNIEnv *env, const char* remoteAddress)
{
    OIC_LOG(DEBUG, TAG, "CALEClientGetGattObjInList");
    VERIFY_NON_NULL_RET(env, TAG, "env is null", NULL);
    VERIFY_NON_NULL_RET(remoteAddress, TAG, "remoteAddress is null", NULL);

    ca_mutex_lock(g_gattObjectMutex);
    uint32_t length = u_arraylist_length(g_gattObjectList);
    for (uint32_t index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_gattObjectList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "jarrayObj is null");
            ca_mutex_unlock(g_gattObjectMutex);
            return NULL;
        }

        jstring jni_setAddress = CALEClientGetAddressFromGattObj(env, jarrayObj);
        if (!jni_setAddress)
        {
            OIC_LOG(ERROR, TAG, "jni_setAddress is null");
            ca_mutex_unlock(g_gattObjectMutex);
            return NULL;
        }

        const char* setAddress = (*env)->GetStringUTFChars(env, jni_setAddress, NULL);
        if (!setAddress)
        {
            OIC_LOG(ERROR, TAG, "setAddress is null");
            ca_mutex_unlock(g_gattObjectMutex);
            return NULL;
        }

        if (!strcmp(remoteAddress, setAddress))
        {
            OIC_LOG(DEBUG, TAG, "the device is already set");
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            ca_mutex_unlock(g_gattObjectMutex);
            return jarrayObj;
        }
        (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
    }

    ca_mutex_unlock(g_gattObjectMutex);
    OIC_LOG(DEBUG, TAG, "There are no the gatt object in list");
    return NULL;
}

CAResult_t CALEClientRemoveAllGattObjs(JNIEnv *env)
{
    OIC_LOG(DEBUG, TAG, "CALEClientRemoveAllGattObjs");
    VERIFY_NON_NULL(env, TAG, "env is null");

    ca_mutex_lock(g_gattObjectMutex);
    if (!g_gattObjectList)
    {
        OIC_LOG(ERROR, TAG, "g_gattObjectList is null");
        ca_mutex_unlock(g_gattObjectMutex);
        return CA_STATUS_FAILED;
    }

    uint32_t length = u_arraylist_length(g_gattObjectList);
    for (uint32_t index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_gattObjectList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "jarrayObj is null");
            continue;
        }
        (*env)->DeleteGlobalRef(env, jarrayObj);
    }

    OICFree(g_gattObjectList);
    g_gattObjectList = NULL;
    ca_mutex_unlock(g_gattObjectMutex);
    return CA_STATUS_OK;
}

CAResult_t CALEClientRemoveGattObj(JNIEnv *env, jobject gatt)
{
    OIC_LOG(DEBUG, TAG, "CALEClientRemoveGattObj");
    VERIFY_NON_NULL(gatt, TAG, "gatt is null");
    VERIFY_NON_NULL(env, TAG, "env is null");

    ca_mutex_lock(g_gattObjectMutex);
    if (!g_gattObjectList)
    {
        OIC_LOG(ERROR, TAG, "g_gattObjectList is null");
        ca_mutex_unlock(g_gattObjectMutex);
        return CA_STATUS_FAILED;
    }

    uint32_t length = u_arraylist_length(g_gattObjectList);
    for (uint32_t index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_gattObjectList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "jarrayObj is null");
            ca_mutex_unlock(g_gattObjectMutex);
            return CA_STATUS_FAILED;
        }

        jstring jni_setAddress = CALEClientGetAddressFromGattObj(env, jarrayObj);
        if (!jni_setAddress)
        {
            OIC_LOG(ERROR, TAG, "jni_setAddress is null");
            ca_mutex_unlock(g_gattObjectMutex);
            return CA_STATUS_FAILED;
        }

        const char* setAddress = (*env)->GetStringUTFChars(env, jni_setAddress, NULL);
        if (!setAddress)
        {
            OIC_LOG(ERROR, TAG, "setAddress is null");
            ca_mutex_unlock(g_gattObjectMutex);
            return CA_STATUS_FAILED;
        }

        jstring jni_remoteAddress = CALEClientGetAddressFromGattObj(env, gatt);
        if (!jni_remoteAddress)
        {
            OIC_LOG(ERROR, TAG, "jni_remoteAddress is null");
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            ca_mutex_unlock(g_gattObjectMutex);
            return CA_STATUS_FAILED;
        }

        const char* remoteAddress = (*env)->GetStringUTFChars(env, jni_remoteAddress, NULL);
        if (!remoteAddress)
        {
            OIC_LOG(ERROR, TAG, "remoteAddress is null");
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            ca_mutex_unlock(g_gattObjectMutex);
            return CA_STATUS_FAILED;
        }

        if (!strcmp(setAddress, remoteAddress))
        {
            OIC_LOG_V(DEBUG, TAG, "remove object : %s", remoteAddress);
            (*env)->DeleteGlobalRef(env, jarrayObj);
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            (*env)->ReleaseStringUTFChars(env, jni_remoteAddress, remoteAddress);

            if (NULL == u_arraylist_remove(g_gattObjectList, index))
            {
                OIC_LOG(ERROR, TAG, "List removal failed.");
                ca_mutex_unlock(g_gattObjectMutex);
                return CA_STATUS_FAILED;
            }
            ca_mutex_unlock(g_gattObjectMutex);
            return CA_STATUS_OK;
        }
        (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
        (*env)->ReleaseStringUTFChars(env, jni_remoteAddress, remoteAddress);
    }

    ca_mutex_unlock(g_gattObjectMutex);
    OIC_LOG(DEBUG, TAG, "there are no target object");
    return CA_STATUS_OK;
}

CAResult_t CALEClientRemoveGattObjForAddr(JNIEnv *env, jstring addr)
{
    OIC_LOG(DEBUG, TAG, "CALEClientRemoveGattObjForAddr");
    VERIFY_NON_NULL(addr, TAG, "addr is null");
    VERIFY_NON_NULL(env, TAG, "env is null");

    ca_mutex_lock(g_gattObjectMutex);
    if (!g_gattObjectList)
    {
        OIC_LOG(ERROR, TAG, "g_gattObjectList is null");
        ca_mutex_unlock(g_gattObjectMutex);
        return CA_STATUS_FAILED;
    }

    uint32_t length = u_arraylist_length(g_gattObjectList);
    for (uint32_t index = 0; index < length; index++)
    {
        jobject jarrayObj = (jobject) u_arraylist_get(g_gattObjectList, index);
        if (!jarrayObj)
        {
            OIC_LOG(ERROR, TAG, "jarrayObj is null");
            ca_mutex_unlock(g_gattObjectMutex);
            return CA_STATUS_FAILED;
        }

        jstring jni_setAddress = CALEClientGetAddressFromGattObj(env, jarrayObj);
        if (!jni_setAddress)
        {
            OIC_LOG(ERROR, TAG, "jni_setAddress is null");
            ca_mutex_unlock(g_gattObjectMutex);
            return CA_STATUS_FAILED;
        }

        const char* setAddress = (*env)->GetStringUTFChars(env, jni_setAddress, NULL);
        if (!setAddress)
        {
            OIC_LOG(ERROR, TAG, "setAddress is null");
            ca_mutex_unlock(g_gattObjectMutex);
            return CA_STATUS_FAILED;
        }

        const char* remoteAddress = (*env)->GetStringUTFChars(env, addr, NULL);
        if (!remoteAddress)
        {
            OIC_LOG(ERROR, TAG, "remoteAddress is null");
            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            ca_mutex_unlock(g_gattObjectMutex);
            return CA_STATUS_FAILED;
        }

        if (!strcmp(setAddress, remoteAddress))
        {
            OIC_LOG_V(DEBUG, TAG, "remove object : %s", remoteAddress);
            (*env)->DeleteGlobalRef(env, jarrayObj);

            (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
            (*env)->ReleaseStringUTFChars(env, addr, remoteAddress);
            if (NULL == u_arraylist_remove(g_gattObjectList, index))
            {
                OIC_LOG(ERROR, TAG, "List removal failed.");
                ca_mutex_unlock(g_gattObjectMutex);
                return CA_STATUS_FAILED;
            }
            ca_mutex_unlock(g_gattObjectMutex);
            return CA_STATUS_OK;
        }
        (*env)->ReleaseStringUTFChars(env, jni_setAddress, setAddress);
        (*env)->ReleaseStringUTFChars(env, addr, remoteAddress);
    }

    ca_mutex_unlock(g_gattObjectMutex);
    OIC_LOG(DEBUG, TAG, "there are no target object");
    return CA_STATUS_FAILED;
}

/**
 * BT State List
 */

CAResult_t CALEClientUpdateDeviceState(const char* address, uint32_t connectedState,
                                       uint16_t notificationState, uint16_t sendState)
{
    VERIFY_NON_NULL(address, TAG, "address is null");

    CALEState_t *newstate = (CALEState_t*) OICMalloc(sizeof(CALEState_t));
    if (!newstate)
    {
        OIC_LOG(ERROR, TAG, "out of memory");
        return CA_MEMORY_ALLOC_FAILED;
    }

    if (strlen(address) > CA_MACADDR_SIZE)
    {
        OIC_LOG(ERROR, TAG, "address is not proper");
        OICFree(newstate);
        return CA_STATUS_FAILED;
    }

    OICStrcpy(newstate->address, sizeof(newstate->address), address);
    newstate->connectedState = connectedState;
    newstate->notificationState = notificationState;
    newstate->sendState = sendState;
    return CALEClientAddDeviceStateToList(newstate);
}

CAResult_t CALEClientAddDeviceStateToList(CALEState_t* state)
{
    VERIFY_NON_NULL(state, TAG, "state is null");

    ca_mutex_lock(g_deviceStateListMutex);

    if (!g_deviceStateList)
    {
        OIC_LOG(ERROR, TAG, "gdevice_list is null");
        ca_mutex_unlock(g_deviceStateListMutex);
        return CA_STATUS_FAILED;
    }

    if (CALEClientIsDeviceInList(state->address))
    {
        CALEState_t* curState = CALEClientGetStateInfo(state->address);
        if(!curState)
        {
            OIC_LOG(ERROR, TAG, "curState is null");
            ca_mutex_unlock(g_deviceStateListMutex);
            return CA_STATUS_FAILED;
        }

        if (STATE_CHARACTER_NO_CHANGE == state->notificationState)
        {
            state->notificationState = curState->notificationState;
        }

        // delete previous state for update new state
        CAResult_t res = CALEClientRemoveDeviceState(state->address);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientRemoveDeviceState has failed");
            ca_mutex_unlock(g_deviceStateListMutex);
            return res;
        }
    }
    u_arraylist_add(g_deviceStateList, state); // update new state
    OIC_LOG_V(DEBUG, TAG, "Set State Info to List : %d, %d",
              state->connectedState, state->notificationState);

    ca_mutex_unlock(g_deviceStateListMutex);
    return CA_STATUS_OK;
}

bool CALEClientIsDeviceInList(const char* remoteAddress)
{
    VERIFY_NON_NULL_RET(remoteAddress, TAG, "remoteAddress is null", false);

    if (!g_deviceStateList)
    {
        OIC_LOG(ERROR, TAG, "g_deviceStateList is null");
        return false;
    }

    uint32_t length = u_arraylist_length(g_deviceStateList);
    for (uint32_t index = 0; index < length; index++)
    {
        CALEState_t* state = (CALEState_t*) u_arraylist_get(g_deviceStateList, index);
        if (!state)
        {
            OIC_LOG(ERROR, TAG, "CALEState_t object is null");
            return false;
        }

        if (!strcmp(remoteAddress, state->address))
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

CAResult_t CALEClientRemoveAllDeviceState()
{
    OIC_LOG(DEBUG, TAG, "CALENativeRemoveAllDevices");

    ca_mutex_lock(g_deviceStateListMutex);
    if (!g_deviceStateList)
    {
        OIC_LOG(ERROR, TAG, "g_deviceStateList is null");
        ca_mutex_unlock(g_deviceStateListMutex);
        return CA_STATUS_FAILED;
    }

    uint32_t length = u_arraylist_length(g_deviceStateList);
    for (uint32_t index = 0; index < length; index++)
    {
        CALEState_t* state = (CALEState_t*) u_arraylist_get(g_deviceStateList, index);
        if (!state)
        {
            OIC_LOG(ERROR, TAG, "jarrayObj is null");
            continue;
        }
        OICFree(state);
    }

    OICFree(g_deviceStateList);
    g_deviceStateList = NULL;
    ca_mutex_unlock(g_deviceStateListMutex);

    return CA_STATUS_OK;
}

CAResult_t CALEClientRemoveDeviceState(const char* remoteAddress)
{
    OIC_LOG(DEBUG, TAG, "CALEClientRemoveDeviceState");
    VERIFY_NON_NULL(remoteAddress, TAG, "remoteAddress is null");

    if (!g_deviceStateList)
    {
        OIC_LOG(ERROR, TAG, "g_deviceStateList is null");
        return CA_STATUS_FAILED;
    }

    uint32_t length = u_arraylist_length(g_deviceStateList);
    for (uint32_t index = 0; index < length; index++)
    {
        CALEState_t* state = (CALEState_t*) u_arraylist_get(g_deviceStateList, index);
        if (!state)
        {
            OIC_LOG(ERROR, TAG, "CALEState_t object is null");
            continue;
        }

        if (!strcmp(state->address, remoteAddress))
        {
            OIC_LOG_V(DEBUG, TAG, "remove state : %s", remoteAddress);
            OICFree(state);

            if (NULL == u_arraylist_remove(g_deviceStateList, index))
            {
                OIC_LOG(ERROR, TAG, "List removal failed.");
                return CA_STATUS_FAILED;
            }

            return CA_STATUS_OK;
        }
    }

    return CA_STATUS_FAILED;
}

CALEState_t* CALEClientGetStateInfo(const char* remoteAddress)
{
    OIC_LOG(DEBUG, TAG, "CALEClientGetStateInfo");
    VERIFY_NON_NULL_RET(remoteAddress, TAG, "remoteAddress is null", NULL);

    if (!g_deviceStateList)
    {
        OIC_LOG(ERROR, TAG, "g_deviceStateList is null");
        return NULL;
    }

    uint32_t length = u_arraylist_length(g_deviceStateList);
    for (uint32_t index = 0; index < length; index++)
    {
        CALEState_t* state = (CALEState_t*) u_arraylist_get(g_deviceStateList, index);
        if (!state)
        {
            OIC_LOG(ERROR, TAG, "CALEState_t object is null");
            continue;
        }

        if (!strcmp(state->address, remoteAddress))
        {
            OIC_LOG_V(DEBUG, TAG, "get state : %s", remoteAddress);
            return state;
        }
    }
    return NULL;
}

bool CALEClientIsConnectedDevice(const char* remoteAddress)
{
    OIC_LOG(DEBUG, TAG, "CALEClientIsConnectedDevice");
    VERIFY_NON_NULL_RET(remoteAddress, TAG, "remoteAddress is null", false);

    ca_mutex_lock(g_deviceStateListMutex);
    if (!g_deviceStateList)
    {
        OIC_LOG(ERROR, TAG, "g_deviceStateList is null");
        ca_mutex_unlock(g_deviceStateListMutex);
        return false;
    }

    uint32_t length = u_arraylist_length(g_deviceStateList);
    for (uint32_t index = 0; index < length; index++)
    {
        CALEState_t* state = (CALEState_t*) u_arraylist_get(g_deviceStateList, index);
        if (!state)
        {
            OIC_LOG(ERROR, TAG, "CALEState_t object is null");
            continue;
        }

        if (!strcmp(state->address, remoteAddress))
        {
            OIC_LOG(DEBUG, TAG, "check whether it is connected or not");

            if (STATE_CONNECTED == state->connectedState)
            {
                ca_mutex_unlock(g_deviceStateListMutex);
                return true;
            }
            else
            {
                ca_mutex_unlock(g_deviceStateListMutex);
                return false;
            }
        }
    }
    ca_mutex_unlock(g_deviceStateListMutex);
    return false;
}

bool CALEClientIsSetCharacteristic(const char* remoteAddress)
{
    OIC_LOG(DEBUG, TAG, "CALEClientIsSetCharacteristic");
    VERIFY_NON_NULL_RET(remoteAddress, TAG, "remoteAddress is null", false);

    ca_mutex_lock(g_deviceStateListMutex);
    if (!g_deviceStateList)
    {
        OIC_LOG(ERROR, TAG, "g_deviceStateList is null");
        ca_mutex_unlock(g_deviceStateListMutex);
        return false;
    }

    uint32_t length = u_arraylist_length(g_deviceStateList);
    for (uint32_t index = 0; index < length; index++)
    {
        CALEState_t* state = (CALEState_t*) u_arraylist_get(g_deviceStateList, index);
        if (!state)
        {
            OIC_LOG(ERROR, TAG, "CALEState_t object is null");
            continue;
        }

        if (!strcmp(state->address, remoteAddress))
        {
            OIC_LOG_V(DEBUG, TAG, "check whether it was set or not:%d", state->notificationState);

            if (STATE_CHARACTER_SET == state->notificationState)
            {
                ca_mutex_unlock(g_deviceStateListMutex);
                return true;
            }
            else
            {
                ca_mutex_unlock(g_deviceStateListMutex);
                return false;
            }
        }
    }

    ca_mutex_unlock(g_deviceStateListMutex);
    return false;
}

void CALEClientCreateDeviceList()
{
    OIC_LOG(DEBUG, TAG, "CALEClientCreateDeviceList");

    // create new object array
    if (!g_gattObjectList)
    {
        OIC_LOG(DEBUG, TAG, "Create g_gattObjectList");

        g_gattObjectList = u_arraylist_create();
    }

    if (!g_deviceStateList)
    {
        OIC_LOG(DEBUG, TAG, "Create g_deviceStateList");

        g_deviceStateList = u_arraylist_create();
    }

    if (!g_deviceList)
    {
        OIC_LOG(DEBUG, TAG, "Create g_deviceList");

        g_deviceList = u_arraylist_create();
    }
}

/**
 * Check Sent Count for remove g_sendBuffer
 */
void CALEClientUpdateSendCnt(JNIEnv *env)
{
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");
    // mutex lock
    ca_mutex_lock(g_threadMutex);

    g_currentSentCnt++;

    if (g_targetCnt <= g_currentSentCnt)
    {
        g_targetCnt = 0;
        g_currentSentCnt = 0;

        if (g_sendBuffer)
        {
            (*env)->DeleteGlobalRef(env, g_sendBuffer);
            g_sendBuffer = NULL;
        }
        // notity the thread
        ca_cond_signal(g_threadCond);
        CALEClientSetSendFinishFlag(true);
        OIC_LOG(DEBUG, TAG, "set signal for send data");
    }
    // mutex unlock
    ca_mutex_unlock(g_threadMutex);
}

CAResult_t CALEClientInitGattMutexVaraibles()
{
    OIC_LOG(DEBUG, TAG, "IN");

    if (NULL == g_bleReqRespClientCbMutex)
    {
        g_bleReqRespClientCbMutex = ca_mutex_new();
        if (NULL == g_bleReqRespClientCbMutex)
        {
            OIC_LOG(ERROR, TAG, "ca_mutex_new has failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_bleServerBDAddressMutex)
    {
        g_bleServerBDAddressMutex = ca_mutex_new();
        if (NULL == g_bleServerBDAddressMutex)
        {
            OIC_LOG(ERROR, TAG, "ca_mutex_new has failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_threadMutex)
    {
        g_threadMutex = ca_mutex_new();
        if (NULL == g_threadMutex)
        {
            OIC_LOG(ERROR, TAG, "ca_mutex_new has failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_threadSendMutex)
    {
        g_threadSendMutex = ca_mutex_new();
        if (NULL == g_threadSendMutex)
        {
            OIC_LOG(ERROR, TAG, "ca_mutex_new has failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_deviceListMutex)
    {
        g_deviceListMutex = ca_mutex_new();
        if (NULL == g_deviceListMutex)
        {
            OIC_LOG(ERROR, TAG, "ca_mutex_new has failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_gattObjectMutex)
    {
        g_gattObjectMutex = ca_mutex_new();
        if (NULL == g_gattObjectMutex)
        {
            OIC_LOG(ERROR, TAG, "ca_mutex_new has failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_deviceStateListMutex)
    {
        g_deviceStateListMutex = ca_mutex_new();
        if (NULL == g_deviceStateListMutex)
        {
            OIC_LOG(ERROR, TAG, "ca_mutex_new has failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_SendFinishMutex)
    {
        g_SendFinishMutex = ca_mutex_new();
        if (NULL == g_SendFinishMutex)
        {
            OIC_LOG(ERROR, TAG, "ca_mutex_new has failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_scanMutex)
    {
        g_scanMutex = ca_mutex_new();
        if (NULL == g_scanMutex)
        {
            OIC_LOG(ERROR, TAG, "ca_mutex_new has failed");
            return CA_STATUS_FAILED;
        }
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

void CALEClientTerminateGattMutexVariables()
{
    OIC_LOG(DEBUG, TAG, "IN");

    ca_mutex_free(g_bleReqRespClientCbMutex);
    g_bleReqRespClientCbMutex = NULL;

    ca_mutex_free(g_bleServerBDAddressMutex);
    g_bleServerBDAddressMutex = NULL;

    ca_mutex_free(g_threadMutex);
    g_threadMutex = NULL;

    ca_mutex_free(g_threadSendMutex);
    g_threadSendMutex = NULL;

    ca_mutex_free(g_deviceListMutex);
    g_deviceListMutex = NULL;

    ca_mutex_free(g_SendFinishMutex);
    g_SendFinishMutex = NULL;

    ca_mutex_free(g_scanMutex);
    g_scanMutex = NULL;

    OIC_LOG(DEBUG, TAG, "OUT");
}

void CALEClientSetSendFinishFlag(bool flag)
{
    OIC_LOG_V(DEBUG, TAG, "g_isFinishedSendData is %d", flag);

    ca_mutex_lock(g_SendFinishMutex);
    g_isFinishedSendData = flag;
    ca_mutex_unlock(g_SendFinishMutex);
}

/**
 * adapter common
 */

CAResult_t CAStartLEGattClient()
{
    CAResult_t res = CALEClientStartMulticastServer();
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "CALEClientStartMulticastServer has failed");
    }
    else
    {
        g_isStartedLEClient = true;
    }

    return res;
}

void CAStopLEGattClient()
{
    OIC_LOG(DEBUG, TAG, "CAStopBLEGattClient");

    if (!g_jvm)
    {
        OIC_LOG(ERROR, TAG, "g_jvm is null");
        return;
    }

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(INFO, TAG, "Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread has failed");
            return;
        }
        isAttached = true;
    }

    CAResult_t ret = CALEClientDisconnectAll(env);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientDisconnectAll has failed");
    }

    ret = CALEClientStopScan();
    if(CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TAG, "CALEClientStopScan has failed");
    }

    ca_cond_signal(g_threadCond);

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

}

void CATerminateLEGattClient()
{
    OIC_LOG(DEBUG, TAG, "Terminate GATT Client");
    CALEClientTerminate();
}

CAResult_t  CAUpdateCharacteristicsToGattServer(const char *remoteAddress, const uint8_t  *data,
                                                uint32_t dataLen, CALETransferType_t type,
                                                int32_t position)
{
    OIC_LOG(DEBUG, TAG, "call CALEClientSendUnicastMessage");
    VERIFY_NON_NULL(data, TAG, "data is null");
    VERIFY_NON_NULL(remoteAddress, TAG, "remoteAddress is null");

    if (LE_UNICAST != type || position < 0)
    {
        OIC_LOG(ERROR, TAG, "this request is not unicast");
        return CA_STATUS_INVALID_PARAM;
    }

    return CALEClientSendUnicastMessage(remoteAddress, data, dataLen);
}

CAResult_t CAUpdateCharacteristicsToAllGattServers(const uint8_t *data, uint32_t dataLen)
{
    OIC_LOG(DEBUG, TAG, "call CALEClientSendMulticastMessage");
    VERIFY_NON_NULL(data, TAG, "data is null");

    return CALEClientSendMulticastMessage(data, dataLen);
}

void CASetLEReqRespClientCallback(CABLEDataReceivedCallback callback)
{
    OIC_LOG(DEBUG, TAG, "IN");

    ca_mutex_lock(g_bleReqRespClientCbMutex);
    g_CABLEClientDataReceivedCallback = callback;
    ca_mutex_unlock(g_bleReqRespClientCbMutex);

    OIC_LOG(DEBUG, TAG, "OUT");
}

void CASetLEClientThreadPoolHandle(ca_thread_pool_t handle)
{
    OIC_LOG(DEBUG, TAG, "IN");

    CALEClientInitialize(handle);

    OIC_LOG(DEBUG, TAG, "OUT");
}

CAResult_t CAGetLEAddress(char **local_address)
{
    VERIFY_NON_NULL(local_address, TAG, "local_address");
    OIC_LOG(INFO, TAG, "CAGetLEAddress is not support");
    return CA_NOT_SUPPORTED;
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaLeClientInterface_caLeRegisterLeScanCallback(JNIEnv *env, jobject obj,
                                                                    jobject callback)
{
    OIC_LOG(DEBUG, TAG, "CaLeRegisterLeScanCallback");
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");
    VERIFY_NON_NULL_VOID(obj, TAG, "obj is null");
    VERIFY_NON_NULL_VOID(callback, TAG, "callback is null");

    g_leScanCallback = (*env)->NewGlobalRef(env, callback);
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaLeClientInterface_caLeRegisterGattCallback(JNIEnv *env, jobject obj,
                                                                  jobject callback)
{
    OIC_LOG(DEBUG, TAG, "CaLeRegisterGattCallback");
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");
    VERIFY_NON_NULL_VOID(obj, TAG, "obj is null");
    VERIFY_NON_NULL_VOID(callback, TAG, "callback is null");

    g_leGattCallback = (*env)->NewGlobalRef(env, callback);
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaLeClientInterface_caLeScanCallback(JNIEnv *env, jobject obj,
                                                          jobject device)
{
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");
    VERIFY_NON_NULL_VOID(obj, TAG, "obj is null");
    VERIFY_NON_NULL_VOID(device, TAG, "device is null");

    CAResult_t res = CALEClientAddScanDeviceToList(env, device);
    if (CA_STATUS_OK != res)
    {
        OIC_LOG_V(ERROR, TAG, "CALEClientAddScanDeviceToList has failed : %d", res);
    }
}

/*
 * Class:     org_iotivity_ca_jar_caleinterface
 * Method:    CALeGattConnectionStateChangeCallback
 * Signature: (Landroid/bluetooth/BluetoothGatt;II)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaLeClientInterface_caLeGattConnectionStateChangeCallback(JNIEnv *env,
                                                                                jobject obj,
                                                                                jobject gatt,
                                                                                jint status,
                                                                                jint newstate)
{
    OIC_LOG_V(DEBUG, TAG, "CALeGattConnectionStateChangeCallback - status %d, newstate %d", status,
            newstate);
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");
    VERIFY_NON_NULL_VOID(obj, TAG, "obj is null");
    VERIFY_NON_NULL_VOID(gatt, TAG, "gatt is null");

    if (GATT_SUCCESS == status && STATE_CONNECTED == newstate) // le connected
    {
        jstring jni_address = CALEClientGetAddressFromGattObj(env, gatt);
        if (!jni_address)
        {
            goto error_exit;
        }

        const char* address = (*env)->GetStringUTFChars(env, jni_address, NULL);
        if (address)
        {
            CAResult_t res = CALEClientUpdateDeviceState(address, STATE_CONNECTED,
                                                         STATE_CHARACTER_NO_CHANGE,
                                                         STATE_SEND_NONE);
            if (CA_STATUS_OK != res)
            {
                OIC_LOG(ERROR, TAG, "CALEClientUpdateDeviceState has failed");
                (*env)->ReleaseStringUTFChars(env, jni_address, address);
                goto error_exit;
            }
            (*env)->ReleaseStringUTFChars(env, jni_address, address);
        }

        CAResult_t res = CALEClientAddGattobjToList(env, gatt);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientAddGattobjToList has failed");
            goto error_exit;
        }

        res = CALEClientDiscoverServices(env, gatt);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientDiscoverServices has failed");
            goto error_exit;
        }
    }
    else // le disconnected
    {
        CAResult_t res = CALEClientStartScan();
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientStartScan has failed");
            goto error_exit;
        }

        jstring jni_address = CALEClientGetAddressFromGattObj(env, gatt);
        if (!jni_address)
        {
            OIC_LOG(ERROR, TAG, "CALEClientGetAddressFromGattObj has failed");
            goto error_exit;
        }

        const char* address = (*env)->GetStringUTFChars(env, jni_address, NULL);
        if (address)
        {
            res = CALEClientRemoveDeviceState(address);
            if (CA_STATUS_OK != res)
            {
                OIC_LOG(ERROR, TAG, "CALEClientRemoveDeviceState has failed");
                goto error_exit;
            }

            res = CALEClientRemoveGattObjForAddr(env, jni_address);
            if (CA_STATUS_OK != res)
            {
                OIC_LOG(ERROR, TAG, "CALEClientRemoveGattObjForAddr has failed");
                goto error_exit;
            }

            (*env)->ReleaseStringUTFChars(env, jni_address, address);
        }

        res = CALEClientGattClose(env, gatt);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientGattClose has failed");
        }
    }
    return;

    // error label.
error_exit:

    CALEClientSendFinish(env, gatt);
    return;
}

/*
 * Class:     org_iotivity_ca_jar_caleinterface
 * Method:    CALeGattServicesDiscoveredCallback
 * Signature: (Landroid/bluetooth/BluetoothGatt;I)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaLeClientInterface_caLeGattServicesDiscoveredCallback(JNIEnv *env,
                                                                             jobject obj,
                                                                             jobject gatt,
                                                                             jint status)
{
    OIC_LOG_V(DEBUG, TAG, "CALeGattServicesDiscoveredCallback - status %d: ", status);
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");
    VERIFY_NON_NULL_VOID(obj, TAG, "obj is null");
    VERIFY_NON_NULL_VOID(gatt, TAG, "gatt is null");

    if (0 != status) // discovery error
    {
        CALEClientSendFinish(env, gatt);
        return;
    }

    jstring jni_address = CALEClientGetAddressFromGattObj(env, gatt);
    if (!jni_address)
    {
        CALEClientSendFinish(env, gatt);
        return;
    }

    const char* address = (*env)->GetStringUTFChars(env, jni_address, NULL);
    if (!address)
    {
        CALEClientSendFinish(env, gatt);
        return;
    }

    if (!CALEClientIsSetCharacteristic(address))
    {
        jstring jni_uuid = (*env)->NewStringUTF(env, OIC_GATT_CHARACTERISTIC_RESPONSE_UUID);
        if (!jni_uuid)
        {
            OIC_LOG(ERROR, TAG, "jni_uuid is null");
            goto error_exit;
        }

        jobject jni_obj_GattCharacteristic = CALEClientGetGattService(env, gatt, jni_uuid);
        if (!jni_obj_GattCharacteristic)
        {
            OIC_LOG(ERROR, TAG, "jni_obj_GattCharacteristic is null");
            goto error_exit;
        }

        CAResult_t res = CALEClientSetCharacteristicNotification(env, gatt,
                                                                 jni_obj_GattCharacteristic);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientSetCharacteristicNotification has failed");
            goto error_exit;
        }

        res = CALEClientSetUUIDToDescriptor(env, gatt, jni_obj_GattCharacteristic);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(INFO, TAG, "Descriptor of the uuid is not found");
            CAResult_t res = CALEClientWriteCharacteristic(env, gatt);
            if (CA_STATUS_OK != res)
            {
                OIC_LOG(ERROR, TAG, "CALEClientWriteCharacteristic has failed");
                goto error_exit;
            }
        }

        res = CALEClientUpdateDeviceState(address, STATE_CONNECTED, STATE_CHARACTER_SET,
                                          STATE_SEND_NONE);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientUpdateDeviceState has failed");
            goto error_exit;
        }
    }
    else
    {
        CAResult_t res = CALEClientWriteCharacteristic(env, gatt);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientWriteCharacteristic has failed");
            goto error_exit;
        }
    }
    (*env)->ReleaseStringUTFChars(env, jni_address, address);
    return;

    // error label.
error_exit:
    (*env)->ReleaseStringUTFChars(env, jni_address, address);
    CALEClientSendFinish(env, gatt);
    return;
}

/*
 * Class:     org_iotivity_ca_jar_caleinterface
 * Method:    CALeGattCharacteristicWritjclasseCallback
 * Signature: (Landroid/bluetooth/BluetoothGatt;Landroid/bluetooth/BluetoothGattCharacteristic;I)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaLeClientInterface_caLeGattCharacteristicWriteCallback(
        JNIEnv *env, jobject obj, jobject gatt, jbyteArray data,
        jint status)
{
    OIC_LOG_V(DEBUG, TAG, "CALeGattCharacteristicWriteCallback - status : %d", status);
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");
    VERIFY_NON_NULL_VOID(obj, TAG, "obj is null");
    VERIFY_NON_NULL_VOID(gatt, TAG, "gatt is null");

    jboolean isCopy;
    char* wroteData = (char*) (*env)->GetByteArrayElements(env, data, &isCopy);

    OIC_LOG_V(DEBUG, TAG, "CALeGattCharacteristicWriteCallback - write data : %s", wroteData);

    // send success & signal
    jstring jni_address = CALEClientGetAddressFromGattObj(env, gatt);
    if (!jni_address)
    {
        goto error_exit;
    }

    const char* address = (*env)->GetStringUTFChars(env, jni_address, NULL);
    if (!address)
    {
        goto error_exit;
    }

    if (GATT_SUCCESS != status) // error case
    {
        OIC_LOG(ERROR, TAG, "send failure");
        CAResult_t res = CALEClientUpdateDeviceState(address, STATE_CONNECTED, STATE_CHARACTER_SET,
                                                     STATE_SEND_FAILED);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientUpdateDeviceState has failed");
        }

        if (g_clientErrorCallback)
        {
            jint length = (*env)->GetArrayLength(env, data);
            g_clientErrorCallback(address, data, length, CA_SEND_FAILED);
        }

        CALEClientSendFinish(env, gatt);
    }
    else
    {
        OIC_LOG(DEBUG, TAG, "send success");
        CAResult_t res = CALEClientUpdateDeviceState(address, STATE_CONNECTED, STATE_CHARACTER_SET,
                                                     STATE_SEND_SUCCESS);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientUpdateDeviceState has failed");
        }
        CALEClientUpdateSendCnt(env);
    }

    (*env)->ReleaseStringUTFChars(env, jni_address, address);
    return;

    // error label.
error_exit:

    CALEClientSendFinish(env, gatt);
    return;
}

/*
 * Class:     org_iotivity_ca_jar_caleinterface
 * Method:    CALeGattCharacteristicChangedCallback
 * Signature: (Landroid/bluetooth/BluetoothGatt;Landroid/bluetooth/BluetoothGattCharacteristic;)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaLeClientInterface_caLeGattCharacteristicChangedCallback(
        JNIEnv *env, jobject obj, jobject gatt, jbyteArray data)
{
    OIC_LOG(DEBUG, TAG, "CALeGattCharacteristicChangedCallback");
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");
    VERIFY_NON_NULL_VOID(obj, TAG, "obj is null");
    VERIFY_NON_NULL_VOID(gatt, TAG, "gatt is null");
    VERIFY_NON_NULL_VOID(data, TAG, "data is null");

    // get Byte Array and convert to uint8_t*
    jint length = (*env)->GetArrayLength(env, data);

    jboolean isCopy;
    jbyte *jni_byte_responseData = (jbyte*) (*env)->GetByteArrayElements(env, data, &isCopy);

    OIC_LOG_V(DEBUG, TAG, "CALeGattCharacteristicChangedCallback - raw data received : %p",
            jni_byte_responseData);

    uint8_t* receivedData = OICMalloc(length);
    if (!receivedData)
    {
        OIC_LOG(ERROR, TAG, "receivedData is null");
        return;
    }

    memcpy(receivedData, jni_byte_responseData, length);
    (*env)->ReleaseByteArrayElements(env, data, jni_byte_responseData, JNI_ABORT);

    jstring jni_address = CALEClientGetAddressFromGattObj(env, gatt);
    if (!jni_address)
    {
        OIC_LOG(ERROR, TAG, "jni_address is null");
        OICFree(receivedData);
        return;
    }

    const char* address = (*env)->GetStringUTFChars(env, jni_address, NULL);
    if (!address)
    {
        OIC_LOG(ERROR, TAG, "address is null");
        OICFree(receivedData);
        return;
    }

    OIC_LOG_V(DEBUG, TAG, "CALeGattCharacteristicChangedCallback - data. : %p, %d",
              receivedData, length);

    ca_mutex_lock(g_bleServerBDAddressMutex);
    uint32_t sentLength = 0;
    g_CABLEClientDataReceivedCallback(address, receivedData, length,
                                      &sentLength);
    ca_mutex_unlock(g_bleServerBDAddressMutex);

    (*env)->ReleaseStringUTFChars(env, jni_address, address);
}

/*
 * Class:     org_iotivity_ca_jar_caleinterface
 * Method:    CALeGattDescriptorWriteCallback
 * Signature: (Landroid/bluetooth/BluetoothGatt;Landroid/bluetooth/BluetoothGattDescriptor;I)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaLeClientInterface_caLeGattDescriptorWriteCallback(JNIEnv *env, jobject obj,
                                                                         jobject gatt,
                                                                         jint status)
{
    OIC_LOG_V(DEBUG, TAG, "CALeGattDescriptorWriteCallback - status %d: ", status);
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");
    VERIFY_NON_NULL_VOID(obj, TAG, "obj is null");
    VERIFY_NON_NULL_VOID(gatt, TAG, "gatt is null");

    CAResult_t res = CALEClientWriteCharacteristic(env, gatt);
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "CALEClientWriteCharacteristic has failed");
        goto error_exit;
    }
    return;

// error label.
error_exit:

    CALEClientSendFinish(env, gatt);
    return;
}
