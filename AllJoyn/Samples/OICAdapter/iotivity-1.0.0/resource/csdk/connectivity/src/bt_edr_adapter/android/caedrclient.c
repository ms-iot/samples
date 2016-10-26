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

#include "caedrinterface.h"
#include "caedrutils.h"
#include "caedrclient.h"
#include "logger.h"
#include "oic_malloc.h"
#include "oic_string.h"
#include "cathreadpool.h" /* for thread pool */
#include "camutex.h"
#include "uarraylist.h"
#include "caadapterutils.h"
#include "caremotehandler.h"

//#define DEBUG_MODE
#define TAG PCF("CA_EDR_CLIENT")

static const char METHODID_CONTEXTNONPARAM[] = "()Landroid/content/Context;";
static const char METHODID_OBJECTNONPARAM[] = "()Landroid/bluetooth/BluetoothAdapter;";
static const char METHODID_OUTPUTNONPARAM[] = "()Ljava/io/OutputStream;";
static const char METHODID_STRINGNONPARAM[] = "()Ljava/lang/String;";
static const char METHODID_BT_DEVICEPARAM[] =
        "(Ljava/lang/String;)Landroid/bluetooth/BluetoothDevice;";
static const char CLASSPATH_BT_ADPATER[] = "android/bluetooth/BluetoothAdapter";
static const char CLASSPATH_BT_DEVICE[] = "android/bluetooth/BluetoothDevice";
static const char CLASSPATH_BT_INTERFACE[] = "org/iotivity/ca/CaEdrInterface";
static const char CLASSPATH_BT_SOCKET[] = "android/bluetooth/BluetoothSocket";
static const char CLASSPATH_BT_UUID[] = "java/util/UUID";
static const char CLASSPATH_CONTEXT[] = "android/content/Context";
static const char CLASSPATH_OUTPUT[] = "java/io/OutputStream";

static ca_thread_pool_t g_threadPoolHandle = NULL;

static JavaVM *g_jvm;
static jobject g_context;

/**
 * @var g_mutexUnicastServer
 * @brief Mutex to synchronize unicast server
 */
static ca_mutex g_mutexUnicastServer = NULL;

/**
 * @var g_stopUnicast
 * @brief Flag to control the Receive Unicast Data Thread
 */
static bool g_stopUnicast = false;

/**
 * @var g_mutexMulticastServer
 * @brief Mutex to synchronize secure multicast server
 */
static ca_mutex g_mutexMulticastServer = NULL;

/**
 * @var g_stopMulticast
 * @brief Flag to control the Receive Multicast Data Thread
 */
static bool g_stopMulticast = false;

/**
 * @var g_stopAccept
 * @brief Flag to control the Accept Thread
 */
static bool g_stopAccept = false;

/**
 * @var g_mutexStateList
 * @brief Mutex to synchronize device state list
 */
static ca_mutex g_mutexStateList = NULL;

/**
 * @var g_mutexObjectList
 * @brief Mutex to synchronize device object list
 */
static ca_mutex g_mutexObjectList = NULL;

/**
 * @var g_edrErrorHandler
 * @brief Error callback to update error in EDR
 */
static CAEDRErrorHandleCallback g_edrErrorHandler = NULL;

typedef struct send_data
{
    char* address;
    char* data;
    uint32_t id;
} data_t;

/**
 @brief Thread context information for unicast, multicast and secured unicast server
 */
typedef struct
{
    bool *stopFlag;
    CAAdapterServerType_t type;
} CAAdapterReceiveThreadContext_t;

typedef struct
{
    bool *stopFlag;
} CAAdapterAcceptThreadContext_t;

/**
 * implement for BT-EDR adapter common method
 */
CAResult_t CAEDRGetInterfaceInformation(CAEndpoint_t **info)
{
    OIC_LOG(DEBUG, TAG, "IN - CAEDRGetInterfaceInformation");

    if (!info)
    {
        OIC_LOG(ERROR, TAG, "endpoint info is null");
        return CA_STATUS_INVALID_PARAM;
    }

    char *macAddress = NULL;
    CAResult_t ret = CAEDRGetInterfaceInfo(&macAddress);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG_V(ERROR, TAG, "Failed to get interface info [%d]", ret);
        OICFree(macAddress);
        return ret;
    }

    if (!macAddress)
    {
        OIC_LOG(ERROR, TAG, "mac address is null");
        return CA_STATUS_FAILED;
    }
    OIC_LOG_V(DEBUG, TAG, "address : %s", macAddress);

    // Create local endpoint using util function
    CAEndpoint_t *endpoint = CACreateEndpointObject(CA_DEFAULT_FLAGS, CA_ADAPTER_RFCOMM_BTEDR,
                                                    macAddress, 0);
    if (NULL == endpoint)
    {
        OIC_LOG(ERROR, TAG, "Failed to create Local Endpoint!");
        OICFree(macAddress);
        return CA_STATUS_FAILED;
    }

    // copy unicast server information
    int32_t netInfoSize = 1;
    CAEndpoint_t *netInfo = (CAEndpoint_t *)OICMalloc(sizeof(CAEndpoint_t) * netInfoSize);
    if (NULL == netInfo)
    {
        OIC_LOG(ERROR, TAG, "Invalid input..");
        OICFree(macAddress);
        CAFreeEndpoint(endpoint);
        return CA_MEMORY_ALLOC_FAILED;
    }
    *netInfo = *endpoint;
    *info = netInfo;

    OICFree(macAddress);
    CAFreeEndpoint(endpoint);

    OIC_LOG(DEBUG, TAG, "OUT - CAEDRGetInterfaceInformation");
    return CA_STATUS_OK;
}

void CAEDRClientTerminate()
{
    OIC_LOG(DEBUG, TAG, "IN");
    CAEDRTerminate();
    OIC_LOG(DEBUG, TAG, "OUT");
}

CAResult_t CAEDRManagerReadData()
{
    OIC_LOG(DEBUG, TAG, "IN");

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_NOT_SUPPORTED;
}

CAResult_t CAEDRClientSendUnicastData(const char *remoteAddress, const uint8_t *data,
                                      uint32_t dataLength)
{
    VERIFY_NON_NULL(remoteAddress, TAG, "remoteAddress is null");
    VERIFY_NON_NULL(data, TAG, "data is null");
    OIC_LOG(DEBUG, TAG, "IN");

    CAResult_t result = CAEDRSendUnicastMessage(remoteAddress, data, dataLength);
    OIC_LOG(DEBUG, TAG, "OUT");
    return result;
}

CAResult_t CAEDRClientSendMulticastData(const uint8_t *data, uint32_t dataLength)
{
    VERIFY_NON_NULL(data, TAG, "data is null");
    OIC_LOG(DEBUG, TAG, "IN");

    CAResult_t result = CAEDRSendMulticastMessage(data, dataLength);
    OIC_LOG(DEBUG, TAG, "OUT");
    return result;
}

// It will be updated when android EDR support is added
void CAEDRClientUnsetCallbacks()
{
    OIC_LOG(DEBUG, TAG, "IN");

    OIC_LOG(DEBUG, TAG, "OUT");
}

// It will be updated when android EDR support is added
void CAEDRClientDisconnectAll()
{
    OIC_LOG(DEBUG, TAG, "IN");

    OIC_LOG(DEBUG, TAG, "OUT");
}

CAResult_t CAEDRGetAdapterEnableState(bool *state)
{
    OIC_LOG(DEBUG, TAG, "IN");
    if (!g_jvm)
    {
        OIC_LOG(ERROR, TAG, "g_jvm is null");
        return CA_STATUS_INVALID_PARAM;
    }
    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(DEBUG, TAG, "CAEDRGetAdapterEnableState - Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread failed");
            return CA_STATUS_INVALID_PARAM;
        }
        isAttached = true;
    }
    jboolean ret = CAEDRNativeIsEnableBTAdapter(env);
    if (ret)
    {
        *state = true;
    }
    else
    {
        *state = false;
    }

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

void CAEDRJniInitContext()
{
    OIC_LOG(DEBUG, TAG, "CAEDRJniInitContext");

    g_context = (jobject) CANativeJNIGetContext();
}

CAResult_t CAEDRCreateJNIInterfaceObject(jobject context)
{
    JNIEnv* env;
    OIC_LOG(DEBUG, TAG, "[EDRCore] CAEDRCreateJNIInterfaceObject");

    if ((*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6) != JNI_OK)
    {
        OIC_LOG(ERROR, TAG, "[EDRCore] Could not get JNIEnv pointer");
        return CA_STATUS_FAILED;
    }

    //getApplicationContext
    jclass contextClass = (*env)->FindClass(env, CLASSPATH_CONTEXT);
    if (!contextClass)
    {
        OIC_LOG(ERROR, TAG, "[EDRCore] Could not get context object class");
        return CA_STATUS_FAILED;
    }

    jmethodID getApplicationContextMethod = (*env)->GetMethodID(env, contextClass,
                                                                "getApplicationContext",
                                                                METHODID_CONTEXTNONPARAM);
    if (!getApplicationContextMethod)
    {
        OIC_LOG(ERROR, TAG, "[EDRCore] Could not get getApplicationContext method");
        return CA_STATUS_FAILED;
    }

    //Create EDRJniInterface instance
    jclass EDRJniInterface = (*env)->FindClass(env, CLASSPATH_BT_INTERFACE);
    if (!EDRJniInterface)
    {
        OIC_LOG(ERROR, TAG, "[EDRCore] Could not get CaEdrInterface class");
        return CA_STATUS_FAILED;
    }

    jmethodID EDRInterfaceConstructorMethod = (*env)->GetMethodID(env, EDRJniInterface, "<init>",
                                                                  "(Landroid/content/Context;)V");
    if (!EDRInterfaceConstructorMethod)
    {
        OIC_LOG(ERROR, TAG, "[EDRCore] Could not get CaEdrInterface constructor method");
        return CA_STATUS_FAILED;
    }

    (*env)->NewObject(env, EDRJniInterface, EDRInterfaceConstructorMethod, context);
    OIC_LOG(DEBUG, TAG, "[EDRCore] NewObject Success");

    return CA_STATUS_OK;

}

static void CAEDRDestroyMutex()
{
    OIC_LOG(DEBUG, TAG, "IN");

    if (g_mutexUnicastServer)
    {
        ca_mutex_free(g_mutexUnicastServer);
        g_mutexUnicastServer = NULL;
    }

    if (g_mutexMulticastServer)
    {
        ca_mutex_free(g_mutexMulticastServer);
        g_mutexMulticastServer = NULL;
    }

    if (g_mutexStateList)
    {
        ca_mutex_free(g_mutexStateList);
        g_mutexStateList = NULL;
    }

    if (g_mutexObjectList)
    {
        ca_mutex_free(g_mutexObjectList);
        g_mutexObjectList = NULL;
    }
    OIC_LOG(DEBUG, TAG, "OUT");
}

static CAResult_t CAEDRCreateMutex()
{
    OIC_LOG(DEBUG, TAG, "IN");

    g_mutexUnicastServer = ca_mutex_new();
    if (!g_mutexUnicastServer)
    {
        OIC_LOG(ERROR, TAG, "Failed to created mutex!");
        return CA_STATUS_FAILED;
    }

    g_mutexMulticastServer = ca_mutex_new();
    if (!g_mutexMulticastServer)
    {
        OIC_LOG(ERROR, TAG, "Failed to created mutex!");

        CAEDRDestroyMutex();
        return CA_STATUS_FAILED;
    }

    g_mutexStateList = ca_mutex_new();
    if (!g_mutexStateList)
    {
        OIC_LOG(ERROR, TAG, "Failed to created mutex!");

        CAEDRDestroyMutex();
        return CA_STATUS_FAILED;
    }

    g_mutexObjectList = ca_mutex_new();
    if (!g_mutexObjectList)
    {
        OIC_LOG(ERROR, TAG, "Failed to created mutex!");

        CAEDRDestroyMutex();
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

void CAEDRInitialize(ca_thread_pool_t handle)
{
    OIC_LOG(DEBUG, TAG, "CAEDRInitialize");

    g_threadPoolHandle = handle;

    CAEDRCoreJniInit();

    CAEDRJniInitContext();

    // init mutex
    CAEDRCreateMutex();

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(DEBUG, TAG, "CAEDRInitialize - Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread failed");
            return;
        }
        isAttached = true;
    }
    jstring jni_address = CAEDRNativeGetLocalDeviceAddress(env);
    if (jni_address)
    {
        const char* localAddress = (*env)->GetStringUTFChars(env, jni_address, NULL);
        OIC_LOG_V(DEBUG, TAG, "My BT Address is %s", localAddress);
        (*env)->ReleaseStringUTFChars(env, jni_address, localAddress);
    }

    ca_mutex_lock(g_mutexStateList);
    CAEDRNativeCreateDeviceStateList();
    ca_mutex_unlock(g_mutexStateList);

    ca_mutex_lock(g_mutexObjectList);
    CAEDRNativeCreateDeviceSocketList();
    ca_mutex_unlock(g_mutexObjectList);

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    if (g_context)
    {
        CAEDRCreateJNIInterfaceObject(g_context); /* create java CaEdrInterface instance*/
    }

    OIC_LOG(DEBUG, TAG, "OUT");
}

void CAEDRTerminate()
{
    OIC_LOG(DEBUG, TAG, "CAEDRTerminate");

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(DEBUG, TAG, "CAEDRTerminate - Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread failed");
            return;
        }
        isAttached = true;
    }

    g_stopAccept = true;
    g_stopMulticast = true;
    g_stopUnicast = true;

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    if (g_context)
    {
        (*env)->DeleteGlobalRef(env, g_context);
    }

    CAEDRNativeSocketCloseToAll(env);

    // delete mutex
    CAEDRDestroyMutex();

    CAEDRNativeRemoveAllDeviceState();
    CAEDRNativeRemoveAllDeviceSocket(env);
}

void CAEDRCoreJniInit()
{
    OIC_LOG(DEBUG, TAG, "CAEdrClientJniInit");
    g_jvm = (JavaVM*) CANativeJNIGetJavaVM();
}

CAResult_t CAEDRSendUnicastMessage(const char* address, const uint8_t* data, uint32_t dataLen)
{
    VERIFY_NON_NULL(address, TAG, "address is null");
    VERIFY_NON_NULL(data, TAG, "data is null");

    CAResult_t result = CAEDRSendUnicastMessageImpl(address, data, dataLen);
    return result;
}

CAResult_t CAEDRSendMulticastMessage(const uint8_t* data, uint32_t dataLen)
{
    VERIFY_NON_NULL(data, TAG, "data is null");
    OIC_LOG_V(DEBUG, TAG, "CAEDRSendMulticastMessage(%s)", data);

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(DEBUG, TAG, "CAEDRSendMulticastMessage - Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread failed");
            return CA_STATUS_INVALID_PARAM;
        }
        isAttached = true;
    }

    CAResult_t result = CAEDRSendMulticastMessageImpl(env, data, dataLen);
    if(CA_STATUS_OK != result)
    {
        OIC_LOG(ERROR, TAG, "CAEDRSendMulticastMessage - could not send multicast message");
        return result;
    }

    OIC_LOG(DEBUG, TAG, "sent data");

    if (isAttached)
    {
        OIC_LOG(DEBUG, TAG, "DetachCurrentThread");
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    OIC_LOG(DEBUG, TAG, "OUT - CAEDRSendMulticastMessage");
    return CA_STATUS_OK;
}

CAResult_t CAEDRGetInterfaceInfo(char **address)
{
    CAEDRGetLocalAddress(address);
    return CA_STATUS_OK;
}

void CAEDRGetLocalAddress(char **address)
{
    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(DEBUG, TAG, "CAEDRGetLocalAddress - Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread failed");
            return;
        }
        isAttached = true;
    }

    jstring jni_address = CAEDRNativeGetLocalDeviceAddress(env);
    if (jni_address)
    {
        const char* localAddress = (*env)->GetStringUTFChars(env, jni_address, NULL);
        *address = OICStrdup(localAddress);
        if (*address == NULL)
        {
            if (isAttached)
            {
                (*g_jvm)->DetachCurrentThread(g_jvm);
            }
            (*env)->ReleaseStringUTFChars(env, jni_address, localAddress);
            (*env)->DeleteLocalRef(env, jni_address);
            return;
        }

        (*env)->ReleaseStringUTFChars(env, jni_address, localAddress);
        (*env)->DeleteLocalRef(env, jni_address);
    }

    OIC_LOG_V(DEBUG, TAG, "Local Address : %s", *address);
    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }
}

CAResult_t CAEDRSendUnicastMessageImpl(const char* address, const uint8_t* data, uint32_t dataLen)
{
    VERIFY_NON_NULL(address, TAG, "address is null");
    VERIFY_NON_NULL(data, TAG, "data is null");
    OIC_LOG_V(DEBUG, TAG, "CAEDRSendUnicastMessageImpl, address: %s, data: %s", address, data);

    bool isAttached = false;
    JNIEnv* env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        OIC_LOG(DEBUG, TAG, "CAEDRSendUnicastMessageImpl - Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread failed");
            return CA_STATUS_INVALID_PARAM;
        }
        isAttached = true;
    }

    OIC_LOG(DEBUG, TAG, "[EDR][Native] set byteArray for data");

    // get bonded device list
    jobjectArray jni_arrayPairedDevices = CAEDRNativeGetBondedDevices(env);
    if (!jni_arrayPairedDevices)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] jni_arrayPairedDevices is empty");
        if (isAttached)
        {
            (*g_jvm)->DetachCurrentThread(g_jvm);
        }
        return CA_STATUS_INVALID_PARAM;
    }
    // Get information from array of devices
    jclass jni_cid_BTDevice = (*env)->FindClass(env, CLASSPATH_BT_DEVICE);
    jmethodID j_mid_getName = (*env)->GetMethodID(env, jni_cid_BTDevice, "getName",
                                                  METHODID_STRINGNONPARAM);
    jmethodID j_mid_getAddress = (*env)->GetMethodID(env, jni_cid_BTDevice, "getAddress",
                                                     METHODID_STRINGNONPARAM);

    jsize length = (*env)->GetArrayLength(env, jni_arrayPairedDevices);
    for (jsize i = 0; i < length; i++)
    {
        OIC_LOG(DEBUG, TAG, "[EDR][Native] start to check device");
        // get name, address from BT device
        jobject j_obj_device = (*env)->GetObjectArrayElement(env, jni_arrayPairedDevices, i);
        jstring j_str_name = (*env)->CallObjectMethod(env, j_obj_device, j_mid_getName);

        if (j_str_name)
        {
            const char * name = (*env)->GetStringUTFChars(env, j_str_name, NULL);
            OIC_LOG_V(DEBUG, TAG, "[EDR][Native] getBondedDevices: ~~device name is %s", name);
            (*env)->ReleaseStringUTFChars(env, j_str_name, name);
            (*env)->DeleteLocalRef(env, j_str_name);
        }

        jstring j_str_address = (*env)->CallObjectMethod(env, j_obj_device, j_mid_getAddress);
        const char * remoteAddress = (*env)->GetStringUTFChars(env, j_str_address, NULL);
        (*env)->DeleteLocalRef(env, j_obj_device);
        if (!remoteAddress)
        {
            OIC_LOG(ERROR, TAG, "[EDR][Native] remoteAddress is null");
            if (isAttached)
            {
                (*g_jvm)->DetachCurrentThread(g_jvm);
            }

            (*env)->DeleteLocalRef(env, j_str_address);
            (*env)->DeleteLocalRef(env, jni_arrayPairedDevices);
            (*env)->DeleteLocalRef(env, jni_cid_BTDevice);
            return CA_STATUS_INVALID_PARAM;
        }
        OIC_LOG_V(DEBUG, TAG,
                  "[EDR][Native] getBondedDevices: ~~device address is %s", remoteAddress);

        // find address
        if (!strcmp(remoteAddress, address))
        {
            CAResult_t res = CAEDRNativeSendData(env, remoteAddress, data, dataLen);
            (*env)->ReleaseStringUTFChars(env, j_str_address, remoteAddress);
            (*env)->DeleteLocalRef(env, j_str_address);
            if (CA_STATUS_OK != res)
            {
                (*env)->DeleteLocalRef(env, jni_arrayPairedDevices);
                (*env)->DeleteLocalRef(env, jni_cid_BTDevice);
                return res;
            }
            break;
        }
        (*env)->ReleaseStringUTFChars(env, j_str_address, remoteAddress);
        (*env)->DeleteLocalRef(env, j_str_address);
    }

    (*env)->DeleteLocalRef(env, jni_arrayPairedDevices);
    (*env)->DeleteLocalRef(env, jni_cid_BTDevice);

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    return CA_STATUS_OK;
}

CAResult_t CAEDRSendMulticastMessageImpl(JNIEnv *env, const uint8_t* data, uint32_t dataLen)
{
    VERIFY_NON_NULL(data, TAG, "data is null");
    OIC_LOG_V(DEBUG, TAG, "CASendMulticastMessageImpl, send to, data: %s, %d", data, dataLen);

    // get bonded device list
    jobjectArray jni_arrayPairedDevices = CAEDRNativeGetBondedDevices(env);
    if (!jni_arrayPairedDevices)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] jni_arrayPairedDevices is empty");
        return CA_STATUS_INVALID_PARAM;
    }
    // Get information from array of devices
    jclass jni_cid_BTDevice = (*env)->FindClass(env, CLASSPATH_BT_DEVICE);
    jmethodID j_mid_getName = (*env)->GetMethodID(env, jni_cid_BTDevice, "getName",
                                                  METHODID_STRINGNONPARAM);
    jmethodID j_mid_getAddress = (*env)->GetMethodID(env, jni_cid_BTDevice, "getAddress",
                                                     METHODID_STRINGNONPARAM);

    jsize length = (*env)->GetArrayLength(env, jni_arrayPairedDevices);
    for (jsize i = 0; i < length; i++)
    {
        // get name, address from BT device
        jobject j_obj_device = (*env)->GetObjectArrayElement(env, jni_arrayPairedDevices, i);
        jstring j_str_name = (*env)->CallObjectMethod(env, j_obj_device, j_mid_getName);

        if (j_str_name)
        {
            const char * name = (*env)->GetStringUTFChars(env, j_str_name, NULL);
            OIC_LOG_V(DEBUG, TAG, "[EDR][Native] getBondedDevices: ~~device name is %s", name);
            (*env)->ReleaseStringUTFChars(env, j_str_name, name);
            (*env)->DeleteLocalRef(env, j_str_name);
        }

        jstring j_str_address = (*env)->CallObjectMethod(env, j_obj_device, j_mid_getAddress);
        const char * remoteAddress = (*env)->GetStringUTFChars(env, j_str_address, NULL);
        (*env)->DeleteLocalRef(env, j_obj_device);
        OIC_LOG_V(DEBUG, TAG,
                  "[EDR][Native] getBondedDevices: ~~device address is %s", remoteAddress);

        // find address
        CAResult_t res = CAEDRNativeSendData(env, remoteAddress, data, dataLen);
        (*env)->ReleaseStringUTFChars(env, j_str_address, remoteAddress);
        (*env)->DeleteLocalRef(env, j_str_address);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG_V(ERROR, TAG, "CASendMulticastMessageImpl, failed to send message to : %s",
                      remoteAddress);
            g_edrErrorHandler(remoteAddress, data, dataLen, res);
            continue;
        }
    }

    (*env)->DeleteLocalRef(env, jni_arrayPairedDevices);
    (*env)->DeleteLocalRef(env, jni_cid_BTDevice);

    return CA_STATUS_OK;
}

/**
 * EDR Method
 */
CAResult_t CAEDRNativeSendData(JNIEnv *env, const char *address, const uint8_t *data,
                               uint32_t dataLength)
{
    VERIFY_NON_NULL(address, TAG, "address is null");
    VERIFY_NON_NULL(data, TAG, "data is null");
    OIC_LOG_V(DEBUG, TAG, "[EDR][Native] btSendData logic start : %s, %d", data, dataLength);

    if (!CAEDRNativeIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adpater is not enable");
        return CA_STATUS_INVALID_PARAM;
    }

    if (STATE_DISCONNECTED == CAEDRIsConnectedDevice(address))
    {
        // connect before send data
        OIC_LOG(DEBUG, TAG, "[EDR][Native] connect before send data");

        CAResult_t res = CAEDRNativeConnect(env, address);
        if (CA_STATUS_OK != res)
        {
            return res;
        }
    }

    if (STATE_CONNECTED == CAEDRIsConnectedDevice(address))
    {
        if (!((*env)->ExceptionCheck(env)))
        {
            jclass jni_cid_BTsocket = (*env)->FindClass(env, CLASSPATH_BT_SOCKET);
            if (!jni_cid_BTsocket)
            {
                OIC_LOG(ERROR, TAG, "[EDR][Native] btSendData: jni_cid_BTsocket is null");
                return CA_STATUS_FAILED;
            }

            jmethodID jni_mid_getOutputStream = (*env)->GetMethodID(env, jni_cid_BTsocket,
                                                                    "getOutputStream",
                                                                    METHODID_OUTPUTNONPARAM);
            if (!jni_mid_getOutputStream)
            {
                OIC_LOG(ERROR, TAG, "[EDR][Native] btSendData: jni_mid_getOutputStream is null");
                (*env)->DeleteLocalRef(env, jni_cid_BTsocket);
                return CA_STATUS_FAILED;
            }

            OIC_LOG(DEBUG, TAG, "[EDR][Native] btSendData: Get MethodID for i/o stream");

            jobject jni_obj_socket = CAEDRNativeGetDeviceSocketBaseAddr(env, address);
            if (!jni_obj_socket)
            {
                OIC_LOG(ERROR, TAG, "[EDR][Native] btSendData: jni_socket is not available");
                (*env)->DeleteLocalRef(env, jni_cid_BTsocket);
                return CA_STATUS_FAILED;
            }

            jobject jni_obj_outputStream = (*env)->CallObjectMethod(env, jni_obj_socket,
                                                                    jni_mid_getOutputStream);
            if (!jni_obj_outputStream)
            {
                OIC_LOG(ERROR, TAG, "[EDR][Native] btSendData: jni_obj_outputStream is null");
                (*env)->DeleteLocalRef(env, jni_cid_BTsocket);
                return CA_STATUS_FAILED;
            }

            OIC_LOG(DEBUG, TAG, "[EDR][Native] btSendData: ready outputStream..");

            jclass jni_cid_OutputStream = (*env)->FindClass(env, CLASSPATH_OUTPUT);
            if (!jni_cid_OutputStream)
            {
                OIC_LOG(ERROR, TAG, "[EDR][Native] btSendData: jni_cid_OutputStream is null");
                (*env)->DeleteLocalRef(env, jni_cid_BTsocket);
                (*env)->DeleteLocalRef(env, jni_obj_outputStream);
                return CA_STATUS_FAILED;
            }

            jmethodID jni_mid_write = (*env)->GetMethodID(env, jni_cid_OutputStream, "write",
                                                          "([BII)V");
            if (!jni_mid_write)
            {
                OIC_LOG(ERROR, TAG, "[EDR][Native] btSendData: jni_mid_write is null");
                (*env)->DeleteLocalRef(env, jni_cid_BTsocket);
                (*env)->DeleteLocalRef(env, jni_obj_outputStream);
                (*env)->DeleteLocalRef(env, jni_cid_OutputStream);
                return CA_STATUS_FAILED;
            }

            jbyteArray jbuf = (*env)->NewByteArray(env, dataLength);
            (*env)->SetByteArrayRegion(env, jbuf, 0, dataLength, (jbyte*) data);

            (*env)->CallVoidMethod(env, jni_obj_outputStream, jni_mid_write, jbuf, (jint) 0,
                                   (jint) dataLength);

            (*env)->DeleteLocalRef(env, jni_cid_BTsocket);
            (*env)->DeleteLocalRef(env, jni_obj_outputStream);
            (*env)->DeleteLocalRef(env, jni_cid_OutputStream);
            (*env)->DeleteLocalRef(env, jbuf);

            if ((*env)->ExceptionCheck(env))
            {
                OIC_LOG(ERROR, TAG, "[EDR][Native] btSendData: Write Error!!!");
                (*env)->ExceptionDescribe(env);
                (*env)->ExceptionClear(env);
                return CA_STATUS_FAILED;
            }

            OIC_LOG(DEBUG, TAG, "[EDR][Native] btSendData: Write Success");
        }
        else
        {
            (*env)->ExceptionDescribe(env);
            (*env)->ExceptionClear(env);
            OIC_LOG(ERROR, TAG, "[EDR][Native] btSendData: error!!");
            return CA_STATUS_FAILED;
        }
    }
    else
    {
        OIC_LOG(DEBUG, TAG, "[EDR][Native] btSendData: BT connection is not completed!!");
    }

    return CA_STATUS_OK;
}

CAResult_t CAEDRNativeConnect(JNIEnv *env, const char *address)
{
    VERIFY_NON_NULL(address, TAG, "address is null");
    OIC_LOG(DEBUG, TAG, "[EDR][Native] btConnect..");

    if (!CAEDRNativeIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adpater is not enable");
        return CA_STATUS_INVALID_PARAM;
    }

    jclass jni_cid_BTAdapter = (*env)->FindClass(env, CLASSPATH_BT_ADPATER);
    if (!jni_cid_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_cid_BTAdapter is null");
        return CA_STATUS_FAILED;
    }

    // get BTadpater
    jmethodID jni_mid_getDefaultAdapter = (*env)->GetStaticMethodID(env, jni_cid_BTAdapter,
                                                                    "getDefaultAdapter",
                                                                    METHODID_OBJECTNONPARAM);
    if (!jni_mid_getDefaultAdapter)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_mid_getDefaultAdapter is null");
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);
        return CA_STATUS_FAILED;
    }

    jobject jni_obj_BTAdapter = (*env)->CallStaticObjectMethod(env, jni_cid_BTAdapter,
                                                               jni_mid_getDefaultAdapter);
    if (!jni_obj_BTAdapter)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_obj_BTAdapter is null");
        (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);
        return CA_STATUS_FAILED;
    }

    // get remote bluetooth device
    jmethodID jni_mid_getRemoteDevice = (*env)->GetMethodID(env, jni_cid_BTAdapter,
                                                            "getRemoteDevice",
                                                            METHODID_BT_DEVICEPARAM);
    (*env)->DeleteLocalRef(env, jni_cid_BTAdapter);
    if (!jni_mid_getRemoteDevice)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_mid_getRemoteDevice is null");
        (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);
        return CA_STATUS_FAILED;
    }

    jstring jni_address = (*env)->NewStringUTF(env, address);
    jobject jni_obj_remoteBTDevice = (*env)->CallObjectMethod(env, jni_obj_BTAdapter,
                                                              jni_mid_getRemoteDevice, jni_address);
    (*env)->DeleteLocalRef(env, jni_address);
    (*env)->DeleteLocalRef(env, jni_obj_BTAdapter);
    if (!jni_obj_remoteBTDevice)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_obj_remoteBTDevice is null");
        return CA_STATUS_FAILED;
    }

    // get create Rfcomm Socket method ID
    jclass jni_cid_BluetoothDevice = (*env)->FindClass(env, CLASSPATH_BT_DEVICE);
    if (!jni_cid_BluetoothDevice)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_cid_BluetoothDevice is null");
        (*env)->DeleteLocalRef(env, jni_obj_remoteBTDevice);
        return CA_STATUS_FAILED;
    }

    jmethodID jni_mid_createSocket = (*env)->GetMethodID(
            env, jni_cid_BluetoothDevice, "createInsecureRfcommSocketToServiceRecord",
            "(Ljava/util/UUID;)Landroid/bluetooth/BluetoothSocket;");
    (*env)->DeleteLocalRef(env, jni_cid_BluetoothDevice);
    if (!jni_mid_createSocket)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_mid_createSocket is null");
        (*env)->DeleteLocalRef(env, jni_obj_remoteBTDevice);
        return CA_STATUS_FAILED;
    }

    // setting UUID
    jclass jni_cid_uuid = (*env)->FindClass(env, CLASSPATH_BT_UUID);
    if (!jni_cid_uuid)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_cid_uuid is null");
        (*env)->DeleteLocalRef(env, jni_obj_remoteBTDevice);
        return CA_STATUS_FAILED;
    }

    jmethodID jni_mid_fromString = (*env)->GetStaticMethodID(
            env, jni_cid_uuid, "fromString", "(Ljava/lang/String;)Ljava/util/UUID;");
    if (!jni_mid_fromString)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_mid_fromString is null");
        (*env)->DeleteLocalRef(env, jni_cid_uuid);
        (*env)->DeleteLocalRef(env, jni_obj_remoteBTDevice);
        return CA_STATUS_FAILED;
    }

    jstring jni_uuid = (*env)->NewStringUTF(env, OIC_EDR_SERVICE_ID);
    if (!jni_uuid)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_uuid is null");
        (*env)->DeleteLocalRef(env, jni_cid_uuid);
        (*env)->DeleteLocalRef(env, jni_obj_remoteBTDevice);
        return CA_STATUS_FAILED;
    }
    jobject jni_obj_uuid = (*env)->CallStaticObjectMethod(env, jni_cid_uuid, jni_mid_fromString,
                                                          jni_uuid);
    (*env)->DeleteLocalRef(env, jni_cid_uuid);
    (*env)->DeleteLocalRef(env, jni_uuid);
    if (!jni_obj_uuid)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_obj_uuid is null");
        (*env)->DeleteLocalRef(env, jni_obj_remoteBTDevice);
        return CA_STATUS_FAILED;
    }
    // create socket
    jobject jni_obj_BTSocket = (*env)->CallObjectMethod(env, jni_obj_remoteBTDevice,
                                                        jni_mid_createSocket, jni_obj_uuid);
    (*env)->DeleteLocalRef(env, jni_obj_uuid);
    (*env)->DeleteLocalRef(env, jni_obj_remoteBTDevice);
    if (!jni_obj_BTSocket)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_obj_BTSocket is null");
        return CA_STATUS_FAILED;
    }

    // connect
    jclass jni_cid_BTSocket = (*env)->FindClass(env, CLASSPATH_BT_SOCKET);
    if (!jni_cid_BTSocket)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_cid_BTSocket is null");
        (*env)->DeleteLocalRef(env, jni_obj_BTSocket);
        return CA_STATUS_FAILED;
    }

    jmethodID jni_mid_connect = (*env)->GetMethodID(env, jni_cid_BTSocket, "connect", "()V");
    (*env)->DeleteLocalRef(env, jni_cid_BTSocket);
    if (!jni_mid_connect)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_mid_connect is null");
        (*env)->DeleteLocalRef(env, jni_obj_BTSocket);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "[EDR][Native] btConnect: initiating connection...");
    (*env)->CallVoidMethod(env, jni_obj_BTSocket, jni_mid_connect);

    if ((*env)->ExceptionCheck(env))
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: Connect is Failed!!!");
        (*env)->ExceptionDescribe(env);
        (*env)->ExceptionClear(env);
        return CA_STATUS_FAILED;
    }

    // set socket to list
    jobject jni_socket = (*env)->NewGlobalRef(env, jni_obj_BTSocket);
    if (!jni_socket)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] btConnect: jni_socket is null");
        (*env)->DeleteLocalRef(env, jni_obj_BTSocket);
        return CA_STATUS_FAILED;
    }
    ca_mutex_lock(g_mutexObjectList);
    CAEDRNativeAddDeviceSocketToList(env, jni_socket);
    (*env)->DeleteGlobalRef(env, jni_socket);
    (*env)->DeleteLocalRef(env, jni_obj_BTSocket);
    ca_mutex_unlock(g_mutexObjectList);

    // update state
    ca_mutex_lock(g_mutexStateList);
    CAEDRUpdateDeviceState(STATE_CONNECTED, address);
    ca_mutex_unlock(g_mutexStateList);

    OIC_LOG(DEBUG, TAG, "[EDR][Native] btConnect: connected");

    return CA_STATUS_OK;
}

void CAEDRNativeSocketClose(JNIEnv *env, const char *address)
{
    VERIFY_NON_NULL_VOID(address, TAG, "address is null");

    jclass jni_cid_BTSocket = (*env)->FindClass(env, "android/bluetooth/BluetoothSocket");
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

    jobject jni_obj_socket = CAEDRNativeGetDeviceSocketBaseAddr(env, address);
    if (!jni_obj_socket)
    {
        OIC_LOG(ERROR, TAG, "[EDR][Native] close: jni_obj_socket is not available");
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

    // remove socket to list
    CAEDRNativeRemoveDeviceSocket(env, jni_obj_socket);

    // update state
    ca_mutex_lock(g_mutexStateList);
    CAEDRUpdateDeviceState(STATE_DISCONNECTED, address);
    ca_mutex_unlock(g_mutexStateList);

    OIC_LOG(DEBUG, TAG, "[EDR][Native] close: disconnected");
}

void CAEDRInitializeClient(ca_thread_pool_t handle)
{
    OIC_LOG(DEBUG, TAG, "IN");
    CAEDRInitialize(handle);
    OIC_LOG(DEBUG, TAG, "OUT");
}

void CAEDRSetErrorHandler(CAEDRErrorHandleCallback errorHandleCallback)
{
    g_edrErrorHandler = errorHandleCallback;
}
