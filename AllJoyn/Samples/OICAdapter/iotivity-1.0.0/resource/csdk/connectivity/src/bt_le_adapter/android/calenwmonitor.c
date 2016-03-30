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
#include "logger.h"
#include "calenwmonitor.h"
#include "caleclient.h"
#include "caleserver.h"
#include "caleutils.h"
#include "caleinterface.h"
#include "caadapterutils.h"

#include "camutex.h"

#include "org_iotivity_ca_CaLeClientInterface.h"

#define TAG PCF("CA_LE_MONITOR")

#define BT_STATE_ON (12)
#define BT_STATE_OFF (10)

static JavaVM *g_jvm;

/**
 * @var gCALEDeviceStateChangedCallback
 * @brief Maintains the callback to be notified on device state changed.
 */
static CALEDeviceStateChangedCallback gCALEDeviceStateChangedCallback = NULL;

/**
 * @var gCALEDeviceStateChangedCbMutex
 * @brief Mutex to synchronize access to the deviceStateChanged Callback when the state
 *           of the LE adapter gets change.
 */
static ca_mutex gCALEDeviceStateChangedCbMutex = NULL;

//getting context
void CALENetworkMonitorJNISetContext()
{
    OIC_LOG(DEBUG, TAG, "CALENetworkMonitorJNISetContext - it is not supported");
}

//getting jvm
void CALENetworkMonitorJniInit()
{
    OIC_LOG(DEBUG, TAG, "CALENetworkMonitorJniInit");
    g_jvm = CANativeJNIGetJavaVM();
}

void CALESetNetStateCallback(CALEDeviceStateChangedCallback callback)
{
    OIC_LOG(DEBUG, TAG, "CALESetNetStateCallback");
    gCALEDeviceStateChangedCallback = callback;
}

CAResult_t CAInitializeLEAdapter()
{
    OIC_LOG(DEBUG, TAG, "IN");

    CALENetworkMonitorJNISetContext();
    CALENetworkMonitorJniInit();

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAStartLEAdapter()
{
    // Nothing to do.

    return CA_STATUS_OK;
}

CAResult_t CAInitLENwkMonitorMutexVaraibles()
{
    OIC_LOG(DEBUG, TAG, "IN");
    if (NULL == gCALEDeviceStateChangedCbMutex)
    {
        gCALEDeviceStateChangedCbMutex = ca_mutex_new();
        if (NULL == gCALEDeviceStateChangedCbMutex)
        {
            OIC_LOG(ERROR, TAG, "ca_mutex_new has failed");
            return CA_STATUS_FAILED;
        }
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;

}

void CATerminateLENwkMonitorMutexVaraibles()
{
    OIC_LOG(DEBUG, TAG, "IN");

    ca_mutex_free(gCALEDeviceStateChangedCbMutex);
    gCALEDeviceStateChangedCbMutex = NULL;

    OIC_LOG(DEBUG, TAG, "OUT");
}

CAResult_t CAGetLEAdapterState()
{
    OIC_LOG(DEBUG, TAG, "IN");

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
        OIC_LOG(DEBUG, TAG, "Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            OIC_LOG(ERROR, TAG, "AttachCurrentThread has failed");
            return CA_STATUS_FAILED;
        }
        isAttached = true;
    }

    if (!CALEIsEnableBTAdapter(env))
    {
        OIC_LOG(ERROR, TAG, "BT adapter is not enabled");
        if (isAttached)
        {
            (*g_jvm)->DetachCurrentThread(g_jvm);
        }
        return CA_ADAPTER_NOT_ENABLED;
    }

    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
    }

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAInitializeLENetworkMonitor()
{
    OIC_LOG(DEBUG, TAG, "IN");

    CAResult_t res = CAInitLENwkMonitorMutexVaraibles();
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "CAInitLENwkMonitorMutexVaraibles has failed");
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TAG, "OUT");

    return CA_STATUS_OK;

}

void CATerminateLENetworkMonitor()
{
    OIC_LOG(DEBUG, TAG, "IN");

    CATerminateLENwkMonitorMutexVaraibles();

    OIC_LOG(DEBUG, TAG, "OUT");
}

CAResult_t CASetLEAdapterStateChangedCb(CALEDeviceStateChangedCallback callback)
{
    OIC_LOG(DEBUG, TAG, "IN");

    OIC_LOG(DEBUG, TAG, "Setting CALEDeviceStateChangedCallback");

    ca_mutex_lock(gCALEDeviceStateChangedCbMutex);
    CALESetNetStateCallback(callback);
    ca_mutex_unlock(gCALEDeviceStateChangedCbMutex);

    OIC_LOG(DEBUG, TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAUnSetLEAdapterStateChangedCb()
{
    OIC_LOG(DEBUG, TAG, "it is not required in this platform");
    return CA_STATUS_OK;
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaLeClientInterface_caLeStateChangedCallback(JNIEnv *env, jobject obj,
                                                                   jint status)
{
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");
    VERIFY_NON_NULL_VOID(obj, TAG, "obj is null");

    OIC_LOG(DEBUG, TAG, "CaLeClientInterface - Network State Changed");

    if (!gCALEDeviceStateChangedCallback)
    {
        OIC_LOG_V(ERROR, TAG, "gNetworkChangeCb is null", status);
        return;
    }

    if (BT_STATE_ON == status) // STATE_ON:12
    {
        CANetworkStatus_t newStatus = CA_INTERFACE_UP;
        CALEClientCreateDeviceList();
        CALEServerCreateCachedDeviceList();

        CAResult_t res = CALEClientStartScan();
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientStartScan has failed");
        }

        res = CALEStartAdvertise();
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEStartAdvertise has failed");
        }

        gCALEDeviceStateChangedCallback(newStatus);
    }
    else if (BT_STATE_OFF == status) // STATE_OFF:10
    {
        // remove obj for client
        CAResult_t res = CALEClientRemoveAllGattObjs(env);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientRemoveAllGattObjs has failed");
        }

        res = CALEClientRemoveAllScanDevices(env);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientRemoveAllScanDevices has failed");
        }

        res = CALEClientRemoveAllDeviceState();
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEClientRemoveAllDeviceState has failed");
        }

        // remove obej for server
        res = CALEServerRemoveAllDevices(env);
        if (CA_STATUS_OK != res)
        {
            OIC_LOG(ERROR, TAG, "CALEServerRemoveAllDevices has failed");
        }

        CALEClientSetScanFlag(false);

        CANetworkStatus_t newStatus = CA_INTERFACE_DOWN;
        gCALEDeviceStateChangedCallback(newStatus);
    }
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_CaLeClientInterface_caLeBondStateChangedCallback(JNIEnv *env, jobject obj,
                                                                       jstring addr)
{
    OIC_LOG(DEBUG, TAG, "CaLeClientInterface - Bond State Changed");
    VERIFY_NON_NULL_VOID(env, TAG, "env is null");
    VERIFY_NON_NULL_VOID(obj, TAG, "obj is null");
    VERIFY_NON_NULL_VOID(addr, TAG, "addr is null");

    // remove obj for client
    CAResult_t res = CALEClientRemoveGattObjForAddr(env, addr);
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "CANativeRemoveGattObjForAddr has failed");
    }

    res = CALEClientRemoveDeviceInScanDeviceList(env, addr);
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "CALEClientRemoveDeviceInScanDeviceList has failed");
    }

    // remove obej for server
    res = CALEServerRemoveDevice(env, addr);
    if (CA_STATUS_OK != res)
    {
        OIC_LOG(ERROR, TAG, "CALEServerRemoveDevice has failed");
    }

}
