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

#include "cableclient.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <pthread.h>
#include <gio/gio.h>

#include "camutex.h"
#include "uarraylist.h"
#include "caqueueingthread.h"
#include "caadapterutils.h"
#include "cafragmentation.h"
#include "cagattservice.h"
#include "oic_string.h"
#include "oic_malloc.h"

/**
 * Logging tag for module name.
 */
#define TZ_BLE_CLIENT_TAG "TZ_BLE_GATT_CLIENT"

/**
 * This contains the list of OIC services a client connect to.
 */
static BLEServiceList *g_bLEServiceList = NULL;

/**
 * Boolean variable to keep the state of the GATT Client.
 */
static bool g_isBleGattClientStarted = false;

/**
 * Mutex to synchronize access to BleServiceList.
 */
static ca_mutex g_bleServiceListMutex = NULL;

/**
 * Mutex to synchronize access to the requestResponse callback to be called
 *    when the data needs to be sent from GATTClient.
 */
static ca_mutex g_bleReqRespClientCbMutex = NULL;

/**
 * Mutex to synchronize access to the requestResponse callback to be called
 *    when the data needs to be sent from GATTClient.
 */
static ca_mutex g_bleClientConnectMutex = NULL;


/**
 * Mutex to synchronize the calls to be done to the platform from GATTClient
 *    interfaces from different threads.
 */
static ca_mutex g_bleClientStateMutex = NULL;

/**
 * Mutex to synchronize the Server BD Address update on client side.
 */
static ca_mutex g_bleServerBDAddressMutex = NULL;

/**
 * Condition used for notifying handler the presence of data in send queue.
 */
static ca_cond g_bleClientSendCondWait = NULL;

/**
 * Mutex to synchronize the task to be pushed to thread pool.
 */
static ca_mutex g_bleClientThreadPoolMutex = NULL;

/**
 * Maintains the callback to be notified on receival of network packets
 *    from other BLE devices
 */
static CABLEDataReceivedCallback g_bleClientDataReceivedCallback = NULL;

/**
 * callback to update the error to le adapter
 */
static CABLEErrorHandleCallback g_clientErrorCallback;

/**
 * gmainLoop to manage the threads to receive the callback from the platfrom.
 */
static GMainLoop *g_eventLoop = NULL;

/**
 * reference to threadpool.
 */
static ca_thread_pool_t g_bleClientThreadPool = NULL;

/**
 * structure to map the service attribute to BD Address.
 */
typedef struct gattService
{
    bt_gatt_attribute_h serviceInfo;         /**< bluetoth attribute for oic service*/
    char *address;                           /**< BD Address of */
} stGattServiceInfo_t;

/**
 * Remote address of Gatt Server.
 */
static char *g_remoteAddress = NULL;

void CABleGattCharacteristicChangedCb(bt_gatt_attribute_h characteristic,
                                      unsigned char *value,
                                      int valueLen,
                                      void *userData)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "Changed characteristic is  [%s]", (char *)characteristic);
    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "Changed characteristic value length [%d]", valueLen);

    ca_mutex_lock(g_bleReqRespClientCbMutex);
    if (NULL == g_bleClientDataReceivedCallback)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "gReqRespCallback is NULL!");
        ca_mutex_unlock(g_bleReqRespClientCbMutex);
        return;
    }
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "Sending data up !");

    ca_mutex_lock(g_bleServerBDAddressMutex);
    uint32_t sentLength = 0;
    g_bleClientDataReceivedCallback(g_remoteAddress, value, valueLen,
                                    &sentLength);

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "Sent data Length is %d", sentLength);
    ca_mutex_unlock(g_bleServerBDAddressMutex);

    ca_mutex_unlock(g_bleReqRespClientCbMutex);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

void CABleGattCharacteristicWriteCb(int result, void *userData)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN ");


    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT ");
}

void CABleGattDescriptorDiscoveredCb(int result, unsigned char format, int total,
                                     bt_gatt_attribute_h descriptor,
                                     bt_gatt_attribute_h characteristic, void *userData)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    stGattCharDescriptor_t *stTemp = (stGattCharDescriptor_t *)OICCalloc(1, sizeof(
                                                                         stGattCharDescriptor_t));

    VERIFY_NON_NULL_VOID(stTemp, TZ_BLE_CLIENT_TAG, "malloc failed!");

    stTemp->desc = (uint8_t *)OICMalloc(total);
    if (NULL == stTemp->desc)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "malloc failed");
        OICFree(stTemp);
        return;
    }
    memcpy(stTemp->desc, descriptor, total);

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "result[%d] format[%d] total[%d]", result, format, total);
    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "characteristic [%s]", (const char *) characteristic);


    bt_gatt_clone_attribute_handle(&(stTemp->characteristic), characteristic);
    stTemp->total = total;

    ca_mutex_lock(g_bleClientThreadPoolMutex);
    if (NULL == g_bleClientThreadPool)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "g_bleClientThreadPool is NULL");
        bt_gatt_destroy_attribute_handle(stTemp->characteristic);
        OICFree(stTemp->desc);
        OICFree(stTemp);
        ca_mutex_unlock(g_bleClientThreadPoolMutex);
        return;
    }

    CAResult_t ret = ca_thread_pool_add_task(g_bleClientThreadPool,
                                            CASetCharacteristicDescriptorValueThread,
                                            stTemp);
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "ca_thread_pool_add_task failed");
        bt_gatt_destroy_attribute_handle(stTemp->characteristic);
        OICFree(stTemp->desc);
        OICFree(stTemp);
        ca_mutex_unlock(g_bleClientThreadPoolMutex);
        return;
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG,
            "LE Client initialization flow complete");

    ca_mutex_unlock(g_bleClientThreadPoolMutex);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

bool CABleGattCharacteristicsDiscoveredCb(int result,
        int inputIndex, int total,
        bt_gatt_attribute_h characteristic, void *userData)
{

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL_RET(characteristic, TZ_BLE_CLIENT_TAG, "Param characteristic is NULL", false);

    VERIFY_NON_NULL_RET(userData, TZ_BLE_CLIENT_TAG, "Param userData is NULL", false);

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG,
              "result [%d] input_index [%d] total [%d]",
              result, inputIndex, total);

    BLEServiceInfo *bleServiceInfo = NULL;

    ca_mutex_lock(g_bleServiceListMutex);

    char *bdAddress = (char *) userData;
    CAGetBLEServiceInfo(g_bLEServiceList, bdAddress, &bleServiceInfo);

    ca_mutex_unlock(g_bleServiceListMutex);

    char *uuid = NULL;
    bt_gatt_get_service_uuid(characteristic, &uuid);

    VERIFY_NON_NULL_RET(uuid, TZ_BLE_CLIENT_TAG, "uuid is NULL", false);

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "New Characteristics[%s] of uuid[%s] is obtained",
              (char *)characteristic, uuid);

    if(0 == strcasecmp(uuid, CA_GATT_RESPONSE_CHRC_UUID)) // Server will read on this characterisctics
    {
        OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG , "Read characteristics is obtained");
        OICFree(uuid);
        CAResult_t retVal = CAAppendBLECharInfo(characteristic, BLE_GATT_READ_CHAR, bleServiceInfo);
        if (CA_STATUS_OK != retVal)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG , "CAAppendBLECharInfo failed! ");
            return false;
        }

        stGattServiceInfo_t *stTemp = (stGattServiceInfo_t *)OICCalloc(1,
                                                                      sizeof(stGattServiceInfo_t));

        VERIFY_NON_NULL_RET(stTemp, TZ_BLE_CLIENT_TAG, "calloc failed!", false);

        stTemp->address = OICStrdup(bdAddress);
        if (NULL == stTemp->address)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG , "Malloc failed!");
            OICFree(stTemp);
            return false;
        }

        bt_gatt_clone_attribute_handle(&(stTemp->serviceInfo), characteristic);

        ca_mutex_lock(g_bleClientThreadPoolMutex);
        if (NULL == g_bleClientThreadPool)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "g_bleClientThreadPool is NULL");
            bt_gatt_destroy_attribute_handle(stTemp->serviceInfo);
            OICFree(stTemp->address);
            OICFree(stTemp);
            ca_mutex_unlock(g_bleClientThreadPoolMutex);
            return false;
        }

        retVal = ca_thread_pool_add_task(g_bleClientThreadPool,
                                        CADiscoverDescriptorThread,
                                        stTemp);
        if (CA_STATUS_OK != retVal)
        {
            OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                      "ca_thread_pool_add_task failed with ret [%d]", retVal);
            bt_gatt_destroy_attribute_handle(stTemp->serviceInfo);
            OICFree(stTemp->address);
            OICFree(stTemp);
            ca_mutex_unlock(g_bleClientThreadPoolMutex);
            return false;
        }
        ca_mutex_unlock(g_bleClientThreadPoolMutex);
    }
    else if (0 == strcasecmp(uuid, CA_GATT_REQUEST_CHRC_UUID)) // Server will write on this characteristics.
    {
        OICFree(uuid);
        OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG , "Write characteristics is obtained");
        CAResult_t retVal = CAAppendBLECharInfo(characteristic, BLE_GATT_WRITE_CHAR, bleServiceInfo);
        if (CA_STATUS_OK != retVal)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG , "CAAppendBLECharInfo failed ");
            return false;
        }
    }
    else
    {
        OICFree(uuid);
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG , "service_uuid characteristics is UNKNOWN");
        return false;
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return true;
}

bool CABleGattPrimaryServiceCb(bt_gatt_attribute_h service, int index, int count,
                               void *userData)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL_RET(service, TZ_BLE_CLIENT_TAG, "Param service is NULL", false);

    VERIFY_NON_NULL_RET(userData, TZ_BLE_CLIENT_TAG, "Param userData is NULL", false);

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "Service info [%s] index [%d] count [%d]", (char *)service,
              index, count);

    CAResult_t result = CAVerifyOICServiceByServiceHandle(service);

    if (CA_STATUS_OK == result)
    {
        OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "Its OIC service");

        OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG , "Registering to watch characteristics changes");

        result = CABleGattWatchCharacteristicChanges(service);
        if (CA_STATUS_OK != result)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG,
                      "CABleGattWatchCharacteristicChanges failed!");
            return false;
        }

        stGattServiceInfo_t *stTemp = (stGattServiceInfo_t *)OICCalloc(1,
                                                                      sizeof(stGattServiceInfo_t));
        VERIFY_NON_NULL_RET(stTemp, TZ_BLE_CLIENT_TAG, "Calloc Failed", false);

        char *bdAddress = (char *)userData;

        stTemp->address = OICStrdup(bdAddress);
        if (NULL == stTemp->address)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG , "Malloc failed!");
            OICFree(stTemp);
            return false;
        }

        BLEServiceInfo *bleServiceInfo = NULL;

        result = CACreateBLEServiceInfo(bdAddress, service, &bleServiceInfo);
        if (CA_STATUS_OK != result)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG , "CACreateBLEServiceInfo failed! ");
            OICFree(stTemp->address);
            OICFree(stTemp);
            OICFree(bleServiceInfo);
            return false;
        }
        if (NULL == bleServiceInfo )
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG , " bleServiceInfo is NULL");
            OICFree(stTemp->address);
            OICFree(stTemp);
            OICFree(bleServiceInfo);
            return false;
        }

        OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG ,
                  " serviceInfo remote address [%s]", bleServiceInfo->bdAddress);

        ca_mutex_lock(g_bleServiceListMutex);
        result = CAAddBLEServiceInfoToList(&g_bLEServiceList, bleServiceInfo);
        ca_mutex_unlock(g_bleServiceListMutex);
        if (CA_STATUS_OK != result)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG , "CAAddBLEServiceInfoToList failed!");
            OICFree(stTemp->address);
            OICFree(stTemp);
            CAFreeBLEServiceInfo(bleServiceInfo);
            return false;
        }


        ca_mutex_lock(g_bleClientThreadPoolMutex);
        if (NULL == g_bleClientThreadPool)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "g_bleClientThreadPool is NULL");
            OICFree(stTemp->address);
            OICFree(stTemp);
            ca_mutex_lock(g_bleServiceListMutex);
            CARemoveBLEServiceInfoToList(&g_bLEServiceList, bleServiceInfo,
                                         bleServiceInfo->bdAddress);
            ca_mutex_unlock(g_bleServiceListMutex);
            ca_mutex_unlock(g_bleClientThreadPoolMutex);
            return false;
        }
        bt_gatt_clone_attribute_handle(&(stTemp->serviceInfo), service);

        result = ca_thread_pool_add_task(g_bleClientThreadPool,
                                        CADiscoverCharThread,
                                        stTemp);
        if (CA_STATUS_OK != result)
        {
            OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                      "ca_thread_pool_add_task failed with ret [%d]", result);
            OICFree(stTemp->address);
            OICFree(stTemp);
            ca_mutex_lock(g_bleServiceListMutex);
            CARemoveBLEServiceInfoToList(&g_bLEServiceList, bleServiceInfo,
                                         bleServiceInfo->bdAddress);
            ca_mutex_unlock(g_bleServiceListMutex);
            ca_mutex_unlock(g_bleClientThreadPoolMutex);
            return false;
        }
        ca_mutex_unlock(g_bleClientThreadPoolMutex);
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT ");
    return true;;
}

void CABleGattConnectionStateChangedCb(int result, bool connected,
                                       const char *remoteAddress, void *userData)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN ");

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "CABleGattConnectionStateChangedCb result[%d] ", result);

    VERIFY_NON_NULL_VOID(remoteAddress, TZ_BLE_CLIENT_TAG, "remote address is NULL");

    CAResult_t ret = CA_STATUS_FAILED;
    if (!connected)
    {
        OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "DisConnected from [%s] ", remoteAddress);

        ret = CABleGattStartDeviceDiscovery();
        if (CA_STATUS_OK != ret)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "CABleGattStartDeviceDiscovery failed");
            return;
        }
    }
    else
    {
        OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "Connected to [%s] ", remoteAddress);

        ca_mutex_lock(g_bleServerBDAddressMutex);

        g_remoteAddress = OICStrdup(remoteAddress);

        ca_mutex_unlock(g_bleServerBDAddressMutex);

        VERIFY_NON_NULL_VOID(g_remoteAddress, TZ_BLE_CLIENT_TAG, "Malloc failed");

        char *addr = OICStrdup(remoteAddress);

        ca_mutex_lock(g_bleClientThreadPoolMutex);
        if (NULL == g_bleClientThreadPool)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "g_bleClientThreadPool is NULL");
            OICFree(addr);

            ca_mutex_lock(g_bleServerBDAddressMutex);
            OICFree(g_remoteAddress);
            ca_mutex_unlock(g_bleServerBDAddressMutex);

            ca_mutex_unlock(g_bleClientThreadPoolMutex);
            return;
        }

        ret = ca_thread_pool_add_task(g_bleClientThreadPool, CADiscoverBLEServicesThread,
                                     addr);
        if (CA_STATUS_OK != ret)
        {
            OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG, "ca_thread_pool_add_task failed with ret [%d]", ret);
            OICFree(addr);

            ca_mutex_lock(g_bleServerBDAddressMutex);
            OICFree(g_remoteAddress);
            ca_mutex_unlock(g_bleServerBDAddressMutex);

            ca_mutex_unlock(g_bleClientThreadPoolMutex);
            return;
        }
        ca_mutex_unlock(g_bleClientThreadPoolMutex);
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

void CABtAdapterLeDeviceDiscoveryStateChangedCb(int result,
        bt_adapter_le_device_discovery_state_e discoveryState,
        bt_adapter_le_device_discovery_info_s *discoveryInfo,
        void *userData)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    if (NULL  == discoveryInfo && BT_ADAPTER_LE_DEVICE_DISCOVERY_FOUND == discoveryState)
    {
        OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "discoveryInfo is NULL");
        return;
    }

    if (BT_ADAPTER_LE_DEVICE_DISCOVERY_FOUND != discoveryState)
    {
        OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG,
                  " LE Discovery state is [%s]",
          discoveryState == BT_ADAPTER_LE_DEVICE_DISCOVERY_STARTED ? "Started" : "Finished");
    }
    else
    {
        CAPrintDiscoveryInformation(discoveryInfo);

        if (discoveryInfo->service_uuid == NULL)
        {
            OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "NO UUIDS from device");
        }
        else
        {
            for (int32_t i = discoveryInfo->service_count - 1; i >= 0; i--)
            {
                OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "uuid[%d]: [%s]",
                          i, discoveryInfo->service_uuid[i]);
                CAResult_t res = CAVerifyOICServiceByUUID(discoveryInfo->service_uuid[i]);
                if (CA_STATUS_OK == res)
                {
                    char *addr = OICStrdup(discoveryInfo->remote_address);
                    VERIFY_NON_NULL_VOID(addr, TZ_BLE_CLIENT_TAG, "Malloc failed");

                    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG,
                              "Trying to do Gatt connection to [%s]", addr);

                    ca_mutex_lock(g_bleClientThreadPoolMutex);
                    if (NULL == g_bleClientThreadPool)
                    {
                        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "g_bleClientThreadPool is NULL");
                        OICFree(addr);
                        ca_mutex_unlock(g_bleClientThreadPoolMutex);
                        return;
                    }

                    CAResult_t ret = ca_thread_pool_add_task(g_bleClientThreadPool,
                                                  CAGattConnectThread, addr);
                    if (CA_STATUS_OK != ret)
                    {
                        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                                  "ca_thread_pool_add_task failed with ret [%d]", ret);
                        OICFree(addr);
                        ca_mutex_unlock(g_bleClientThreadPoolMutex);
                        return;
                    }
                    ca_mutex_unlock(g_bleClientThreadPoolMutex);
                    if (discoveryInfo->adv_data_len > 31 || discoveryInfo->scan_data_len > 31)
                    {
                        bt_adapter_le_stop_device_discovery();
                        return;
                    }
                    break;  // Found the OIC Service. No need to verify remaining services.
                }
            }
        }
    }
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}


void CAPrintDiscoveryInformation(const bt_adapter_le_device_discovery_info_s *discoveryInfo)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    if (NULL == discoveryInfo)
    {
        OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "discoveryInfo is NULL ");
        return;
    }

    if (discoveryInfo->remote_address)
    {
        OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "Remote Address [%s]", discoveryInfo->remote_address);
    }

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG,
              " Adv data len [%d] Scan data len[%d]RSSI [%d] Addr_type [%d] ",
              discoveryInfo->adv_data_len, discoveryInfo->scan_data_len, discoveryInfo->rssi,
              discoveryInfo->address_type);
    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG,
              " Number of services present in device [%s] is [%d]",
              discoveryInfo->remote_address, discoveryInfo->service_count);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

void CASetLEClientThreadPoolHandle(ca_thread_pool_t handle)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    ca_mutex_lock(g_bleClientThreadPoolMutex);
    g_bleClientThreadPool = handle;
    ca_mutex_unlock(g_bleClientThreadPoolMutex);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

void CASetLEReqRespClientCallback(CABLEDataReceivedCallback callback)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    ca_mutex_lock(g_bleReqRespClientCbMutex);

    g_bleClientDataReceivedCallback = callback;

    ca_mutex_unlock(g_bleReqRespClientCbMutex);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}


void CASetBLEClientErrorHandleCallback(CABLEErrorHandleCallback callback)
{
    g_clientErrorCallback = callback;
}

CAResult_t CAStartLEGattClient()
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    CAResult_t retVal = CAInitGattClientMutexVariables();

    if (CA_STATUS_OK != retVal)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "CAInitGattClientMutexVariables failed!");
        CATerminateGattClientMutexVariables();
        return CA_STATUS_FAILED;
    }

    ca_mutex_lock(g_bleClientThreadPoolMutex);
    if (NULL == g_bleClientThreadPool)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "gBleServerThreadPool is NULL");
        CATerminateGattClientMutexVariables();
        ca_mutex_unlock(g_bleClientThreadPoolMutex);
        return CA_STATUS_FAILED;
    }

    retVal = ca_thread_pool_add_task(g_bleClientThreadPool, CAStartBleGattClientThread,
                                     NULL);
    if (CA_STATUS_OK != retVal)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "ca_thread_pool_add_task failed");
        CATerminateGattClientMutexVariables();
        ca_mutex_unlock(g_bleClientThreadPoolMutex);
        return CA_STATUS_FAILED;
    }
    ca_mutex_unlock(g_bleClientThreadPoolMutex);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

void CAStartBleGattClientThread(void *data)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    ca_mutex_lock(g_bleClientStateMutex);

    if (true  == g_isBleGattClientStarted)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "Gatt Client is already running!!");
        ca_mutex_unlock(g_bleClientStateMutex);
        return;
    }

    CAResult_t  ret = CABleGattSetScanParameter();
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "CABleSetScanParameter Failed");
        ca_mutex_unlock(g_bleClientStateMutex);
        CATerminateLEGattClient();
        return;
    }

    ret = CABleGattSetCallbacks();
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "CABleGattSetCallbacks Failed");
        ca_mutex_unlock(g_bleClientStateMutex);
        CATerminateLEGattClient();
        return;
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "Starting LE device discovery");

    ret = CABleGattStartDeviceDiscovery();
    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "bt_adapter_le_start_device_discovery Failed");
        ca_mutex_unlock(g_bleClientStateMutex);
        CATerminateLEGattClient();
        return;
    }

    g_isBleGattClientStarted = true;

    ca_mutex_unlock(g_bleClientStateMutex);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "Giveing the control to threadPool");

    GMainContext *thread_context = g_main_context_new();

    g_eventLoop = g_main_loop_new(thread_context, FALSE);

    g_main_context_push_thread_default(thread_context);

    g_main_loop_run(g_eventLoop);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

void CAStopLEGattClient()
{
    OIC_LOG(DEBUG,  TZ_BLE_CLIENT_TAG, "IN");

    ca_mutex_lock(g_bleClientStateMutex);

    if (false == g_isBleGattClientStarted)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "Gatt Client is not running to stop");
        ca_mutex_unlock(g_bleClientStateMutex);
        return;
    }

    CABleGattUnSetCallbacks();

    CABleGattUnWatchCharacteristicChanges();

    CABleGattStopDeviceDiscovery();

    g_isBleGattClientStarted = false;

    GMainContext  *context_event_loop = NULL;
    // Required for waking up the thread which is running in gmain loop
    if (NULL != g_eventLoop)
    {
        context_event_loop = g_main_loop_get_context(g_eventLoop);
    }
    if (context_event_loop)
    {
        OIC_LOG_V(DEBUG,  TZ_BLE_CLIENT_TAG, "g_eventLoop context %x", context_event_loop);
        g_main_context_wakeup(context_event_loop);

        // Kill g main loops and kill threads.
        g_main_loop_quit(g_eventLoop);
    }
    else
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "g_eventLoop context is NULL");
    }

    ca_mutex_unlock(g_bleClientStateMutex);

    OIC_LOG(DEBUG,  TZ_BLE_CLIENT_TAG, "OUT");
}

void CATerminateLEGattClient()
{
    OIC_LOG(DEBUG,  TZ_BLE_CLIENT_TAG, "IN");
    ca_mutex_lock(g_bleClientStateMutex);

    ca_mutex_lock(g_bleServerBDAddressMutex);

    OICFree(g_remoteAddress);

    ca_mutex_unlock(g_bleServerBDAddressMutex);

    ca_mutex_lock(g_bleServiceListMutex);
    CAFreeBLEServiceList(g_bLEServiceList);
    g_bLEServiceList = NULL;
    ca_mutex_unlock(g_bleServiceListMutex);

    CAResetRegisteredServiceCount();

    ca_mutex_unlock(g_bleClientStateMutex);

    CATerminateGattClientMutexVariables();

    OIC_LOG(DEBUG,  TZ_BLE_CLIENT_TAG, "OUT");
}

CAResult_t CAInitGattClientMutexVariables()
{
    OIC_LOG(DEBUG,  TZ_BLE_CLIENT_TAG, "IN");
    if (NULL == g_bleClientStateMutex)
    {
        g_bleClientStateMutex = ca_mutex_new();
        if (NULL == g_bleClientStateMutex)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "ca_mutex_new failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_bleServiceListMutex)
    {
        g_bleServiceListMutex = ca_mutex_new();
        if (NULL == g_bleServiceListMutex)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "ca_mutex_new failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_bleReqRespClientCbMutex)
    {
        g_bleReqRespClientCbMutex = ca_mutex_new();
        if (NULL == g_bleReqRespClientCbMutex)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "ca_mutex_new failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_bleClientThreadPoolMutex)
    {
        g_bleClientThreadPoolMutex = ca_mutex_new();
        if (NULL == g_bleClientThreadPoolMutex)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "ca_mutex_new failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_bleClientConnectMutex)
    {
        g_bleClientConnectMutex = ca_mutex_new();
        if (NULL == g_bleClientConnectMutex)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "ca_mutex_new failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_bleClientSendCondWait)
    {
        g_bleClientSendCondWait = ca_cond_new();
        if (NULL == g_bleClientSendCondWait)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "ca_cond_new failed");
            return CA_STATUS_FAILED;
        }
    }

    if (NULL == g_bleServerBDAddressMutex)
    {
        g_bleServerBDAddressMutex = ca_mutex_new();
        if (NULL == g_bleServerBDAddressMutex)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "ca_mutex_new failed");
            return CA_STATUS_FAILED;
        }
    }

    OIC_LOG(DEBUG,  TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

void CATerminateGattClientMutexVariables()
{
    OIC_LOG(DEBUG,  TZ_BLE_CLIENT_TAG, "IN");

    ca_mutex_free(g_bleClientStateMutex);
    g_bleClientStateMutex = NULL;

    ca_mutex_free(g_bleServiceListMutex);
    g_bleServiceListMutex = NULL;

    ca_mutex_free(g_bleReqRespClientCbMutex);
    g_bleReqRespClientCbMutex = NULL;

    ca_mutex_free(g_bleClientConnectMutex);
    g_bleClientConnectMutex = NULL;

    ca_mutex_free(g_bleClientThreadPoolMutex);
    g_bleClientThreadPoolMutex = NULL;

    ca_mutex_free(g_bleServerBDAddressMutex);
    g_bleServerBDAddressMutex = NULL;

    ca_cond_free(g_bleClientSendCondWait);
    g_bleClientSendCondWait = NULL;


    OIC_LOG(DEBUG,  TZ_BLE_CLIENT_TAG, "OUT");
}

CAResult_t CABleGattSetScanParameter()
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    bt_adapter_le_scan_params_s scan_param = { 0, };
    scan_param.type = BT_ADAPTER_LE_PASSIVE_SCAN;
    scan_param.interval = 1560;
    scan_param.window = 160;

    int ret = bt_adapter_le_set_scan_parameter(&scan_param);
    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG, "bt_adapter_le_set_scan_parameter Failed with ret [%d]", ret);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CABleGattSetCallbacks()
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    int ret = bt_gatt_set_connection_state_changed_cb(CABleGattConnectionStateChangedCb, NULL);
    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                  "bt_gatt_set_connection_state_changed_cb Failed with return as [%s ]",
                  CABTGetErrorMsg(ret));
        return CA_STATUS_FAILED;
    }

    ret = bt_adapter_le_set_device_discovery_state_changed_cb(
              CABtAdapterLeDeviceDiscoveryStateChangedCb, NULL);
    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                  "bt_adapter_le_set_device_discovery_state_changed_cb Failed with return as [%s ]",
                  CABTGetErrorMsg(ret));;
        return CA_STATUS_FAILED;
    }

    ret = bt_gatt_set_characteristic_changed_cb(CABleGattCharacteristicChangedCb, NULL);
    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG, "bt_gatt_set_characteristic_changed_cb Failed as [%s ]",
                  CABTGetErrorMsg(ret));
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

void CABleGattUnSetCallbacks()
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    bt_gatt_unset_characteristic_changed_cb();

    bt_gatt_unset_connection_state_changed_cb();

    bt_adapter_le_unset_device_discovery_state_changed_cb();

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

CAResult_t CABleGattWatchCharacteristicChanges(bt_gatt_attribute_h service)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    int ret = bt_gatt_watch_characteristic_changes(service);
    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                  "bt_gatt_watch_characteristic_changes failed  with [%s]",
                  CABTGetErrorMsg(ret));

        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

void CABleGattUnWatchCharacteristicChanges()
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    int32_t count = CAGetRegisteredServiceCount();

    for (int32_t index = 0; index < count; index++)
    {
        BLEServiceInfo *bleServiceInfo = NULL;

        ca_mutex_lock(g_bleServiceListMutex);

        CAResult_t  result = CAGetBLEServiceInfoByPosition(g_bLEServiceList, index, &bleServiceInfo);
        if (CA_STATUS_OK == result && NULL != bleServiceInfo
            && NULL != bleServiceInfo->service_clone)
        {
            bt_gatt_unwatch_characteristic_changes(bleServiceInfo->service_clone);
            OIC_LOG(INFO, TZ_BLE_CLIENT_TAG, "bt_gatt_unwatch_characteristic_changes done");
        }

        ca_mutex_unlock(g_bleServiceListMutex);
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

CAResult_t CABleGattStartDeviceDiscovery()
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");
    bool isDiscovering = false;

    int ret = bt_adapter_le_is_discovering(&isDiscovering);
    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "bt_adapter_le_is_discovering Failed");
        return CA_STATUS_FAILED;
    }

    if(!isDiscovering)
    {
        ret = bt_adapter_le_start_device_discovery();
        if (BT_ERROR_NONE != ret)
        {
            OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG, "bt_adapter_le_start_device_discovery Failed Ret: %d, %x", ret, ret);
            return CA_STATUS_FAILED;
        }
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

void CABleGattStopDeviceDiscovery()
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    bool isDiscovering = false;

    int ret = bt_adapter_le_is_discovering(&isDiscovering);
    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "bt_adapter_le_is_discovering Failed");
        return;
    }

    if(isDiscovering)
    {
        ret = bt_adapter_le_stop_device_discovery();
        if (BT_ERROR_NONE != ret)
        {
            OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "bt_adapter_le_stop_device_discovery Failed");
            return;
        }
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

void CAGattConnectThread (void *remoteAddress)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN ");

    VERIFY_NON_NULL_VOID(remoteAddress, TZ_BLE_CLIENT_TAG, "remote address is NULL");

    char *address  = (char *)remoteAddress;

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "remote address is [%s]", address);

    CAResult_t result = CABleGattConnect(address);

    if (CA_STATUS_OK != result)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG, "bt_gatt_connect failed for [%s]", address);
    }

    OICFree(address);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

CAResult_t CABleGattConnect(const char *remoteAddress)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL_RET(remoteAddress, TZ_BLE_CLIENT_TAG,
                        "remote address is NULL", CA_STATUS_FAILED);

    //Because of the platform issue, we added. Once platform is stablized, then it will be removed
    sleep(1);

    ca_mutex_lock(g_bleClientConnectMutex);

    int ret = bt_gatt_connect(remoteAddress, true);

    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG, "bt_gatt_connect Failed with ret value [%s] ",
                  CABTGetErrorMsg(ret));
        ca_mutex_unlock(g_bleClientConnectMutex);
        return CA_STATUS_FAILED;
    }
    ca_mutex_unlock(g_bleClientConnectMutex);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CABleGattDisConnect(const char *remoteAddress)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL_RET(remoteAddress, TZ_BLE_CLIENT_TAG,
                        "remote address is NULL", CA_STATUS_FAILED);

    int32_t ret = bt_gatt_disconnect(remoteAddress);

    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG, "bt_gatt_disconnect Failed with ret value [%d] ",
                  ret);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

void CADiscoverBLEServicesThread (void *remoteAddress)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL_VOID(remoteAddress, TZ_BLE_CLIENT_TAG, "remote address is NULL");

    char *address  = (char *)remoteAddress;

    CAResult_t result = CABleGattDiscoverServices(address);
    if (CA_STATUS_OK != result)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "CABleGattDiscoverServices failed");
    }

    OICFree(address);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT ");
}

CAResult_t CABleGattDiscoverServices(const char *remoteAddress)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL_RET(remoteAddress, TZ_BLE_CLIENT_TAG,
                        "remote address is NULL", CA_STATUS_FAILED);

    char *addr = OICStrdup(remoteAddress);
    VERIFY_NON_NULL_RET(addr, TZ_BLE_CLIENT_TAG, "Malloc failed", CA_STATUS_FAILED);

    int32_t ret = bt_gatt_foreach_primary_services(remoteAddress, CABleGattPrimaryServiceCb,
                  (void *)addr); // addr memory will be free in callback.
    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                  "bt_gatt_foreach_primary_services Failed with ret value [%d] ", ret);
        OICFree(addr);
        return CA_STATUS_FAILED;
    }
    else
    {
        OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG,
                  "bt_gatt_foreach_primary_services success for address [%s]", remoteAddress);
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

void CADiscoverCharThread(void *stServiceInfo)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL_VOID(stServiceInfo, TZ_BLE_CLIENT_TAG, "stServiceInfo is NULL");

    stGattServiceInfo_t *stTemp  = (stGattServiceInfo_t *)stServiceInfo;

    VERIFY_NON_NULL_VOID(stTemp->address, TZ_BLE_CLIENT_TAG, "stTemp->address is NULL");

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "remote address [%s]", stTemp->address);

    CAResult_t  result = CABleGattDiscoverCharacteristics(stTemp->serviceInfo, stTemp->address);
    if (CA_STATUS_OK != result)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG , "CABleGattDiscoverCharacteristics failed!");
        bt_gatt_destroy_attribute_handle(stTemp->serviceInfo);
        OICFree(stTemp->address);
        OICFree(stTemp);
        return;
    }
    bt_gatt_destroy_attribute_handle(stTemp->serviceInfo);
    OICFree(stTemp->address);
    OICFree(stTemp);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

CAResult_t CABleGattDiscoverCharacteristics(bt_gatt_attribute_h service,
        const char *remoteAddress)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL_RET(service, TZ_BLE_CLIENT_TAG, "service is NULL", CA_STATUS_FAILED);

    VERIFY_NON_NULL_RET(remoteAddress, TZ_BLE_CLIENT_TAG, "remoteAddress is NULL", CA_STATUS_FAILED);

    char *addr = OICStrdup(remoteAddress);
    VERIFY_NON_NULL_RET(addr, TZ_BLE_CLIENT_TAG, "Malloc failed", CA_STATUS_FAILED);

    int32_t ret = bt_gatt_discover_characteristics(service, CABleGattCharacteristicsDiscoveredCb,
                  (void *)addr); // addr will be freed in callback.
    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                  "bt_gatt_discover_characteristics failed with error [%d]", ret);
        OICFree(addr);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

void CADiscoverDescriptorThread(void *stServiceInfo)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, " IN");

    VERIFY_NON_NULL_VOID(stServiceInfo, TZ_BLE_CLIENT_TAG, "stServiceInfo is NULL");

    stGattServiceInfo_t *stTemp  = (stGattServiceInfo_t *)stServiceInfo;

    CAResult_t result = CABleGattDiscoverDescriptor(stTemp->serviceInfo, NULL);
    if (CA_STATUS_OK != result)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG,
                  "bt_gatt_discover_characteristic_descriptor failed");
        bt_gatt_destroy_attribute_handle(stTemp->serviceInfo);
        OICFree(stTemp->address);
        OICFree(stTemp);
        return;
    }

    bt_gatt_destroy_attribute_handle(stTemp->serviceInfo);
    OICFree(stTemp->address);
    OICFree(stTemp);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

CAResult_t CABleGattDiscoverDescriptor(bt_gatt_attribute_h service, const char *remoteAddress)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL_RET(service, TZ_BLE_CLIENT_TAG, "service is NULL", CA_STATUS_FAILED);

    int ret = bt_gatt_discover_characteristic_descriptor(service,
                  CABleGattDescriptorDiscoveredCb, NULL);
    if (BT_ERROR_NONE != ret)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                  "bt_gatt_discover_characteristic_descriptor failed with returns[%s]",
                  CABTGetErrorMsg(ret));
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

void CASetCharacteristicDescriptorValueThread(void *stServiceInfo)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL_VOID(stServiceInfo, TZ_BLE_CLIENT_TAG, "stServiceInfo is NULL");

    stGattCharDescriptor_t *stTemp  = (stGattCharDescriptor_t *)stServiceInfo;

    CAResult_t  result = CASetCharacteristicDescriptorValue(stTemp);
    if (CA_STATUS_OK != result)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG , "CASetCharacteristicDescriptorValue failed!");
        bt_gatt_destroy_attribute_handle(stTemp->characteristic);
        OICFree(stTemp->desc);
        OICFree(stTemp);
        return;
    }
    bt_gatt_destroy_attribute_handle(stTemp->characteristic);
    OICFree(stTemp->desc);
    OICFree(stTemp);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
}

CAResult_t CASetCharacteristicDescriptorValue(stGattCharDescriptor_t *stGattCharDescInfo)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    unsigned char noti[4] = {0,};

    char *strUUID = (char *)OICCalloc(5, sizeof(char));

    VERIFY_NON_NULL_RET(strUUID, TZ_BLE_CLIENT_TAG, "calloc failed", CA_STATUS_FAILED);

    snprintf(strUUID, 4, "%x%x", stGattCharDescInfo->desc[3], stGattCharDescInfo->desc[2]);
    noti[0] = stGattCharDescInfo->desc[0];
    noti[1] = stGattCharDescInfo->desc[1];
    noti[2] = 0x01;
    noti[3] = 0x00;

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "desc x0 [%x]", stGattCharDescInfo->desc[0]);
    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "desc x1 [%x]", stGattCharDescInfo->desc[1]);
    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "desc x2 [%x]", stGattCharDescInfo->desc[2]);
    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "desc x3 [%x]", stGattCharDescInfo->desc[3]);


    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "CA_GATT_CONFIGURATION_DESC_UUID strUUID is [%s]",
              strUUID);
    //if (!strncmp(strUUID, CA_GATT_CONFIGURATION_DESC_UUID, 2))
    {
        OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "setting notification/indication for descriptor");

        int ret =  bt_gatt_set_characteristic_desc_value_request(
                           stGattCharDescInfo->characteristic,
                           noti,  4, CABleGattCharacteristicWriteCb);
        if (BT_ERROR_NONE != ret)
        {
            OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                      "bt_gatt_set_characteristic_desc_value_request failed with return[%s]",
                      CABTGetErrorMsg(ret));
            OICFree(strUUID);
            return CA_STATUS_FAILED;
        }
    }
    OICFree(strUUID);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t  CAUpdateCharacteristicsToGattServer(const char *remoteAddress,
        const uint8_t *data, uint32_t dataLen,
        CALETransferType_t type, int32_t position)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL(data, TZ_BLE_CLIENT_TAG, "data is NULL");

    if (0 >= dataLen)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "dataLen is less than or equal zero. Invalid input!");
        return CA_STATUS_INVALID_PARAM;
    }

    BLEServiceInfo *bleServiceInfo = NULL;

    CAResult_t ret =  CA_STATUS_FAILED;

    ca_mutex_lock(g_bleServiceListMutex);
    if ( LE_UNICAST == type)
    {
        VERIFY_NON_NULL(remoteAddress, TZ_BLE_CLIENT_TAG, "remoteAddress is NULL");

        ret = CAGetBLEServiceInfo(g_bLEServiceList, remoteAddress, &bleServiceInfo);
    }
    else if ( LE_MULTICAST == type)
    {
        ret = CAGetBLEServiceInfoByPosition(g_bLEServiceList, position, &bleServiceInfo);
    }
    ca_mutex_unlock(g_bleServiceListMutex);

    if (CA_STATUS_OK != ret)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "CAGetBLEServiceInfoByPosition is failed");
        return CA_STATUS_FAILED;
    }

    VERIFY_NON_NULL(bleServiceInfo, TZ_BLE_CLIENT_TAG, "bleServiceInfo is NULL");

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "Updating the data of length [%u] to [%s] ", dataLen,
              bleServiceInfo->bdAddress);

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_TAG, "Updating to write char [%s]",
              bleServiceInfo->read_char);

    int result = bt_gatt_set_characteristic_value(bleServiceInfo->write_char, (unsigned char *)data,
                     dataLen);
    if (BT_ERROR_NONE != result)
    {
        OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                  "bt_gatt_set_characteristic_value Failed with return val [%d]",
                  result);
        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t  CAUpdateCharacteristicsToAllGattServers(const uint8_t *data,
            uint32_t dataLen)
{
    OIC_LOG(DEBUG,  TZ_BLE_CLIENT_TAG, "IN");

    VERIFY_NON_NULL(data, TZ_BLE_CLIENT_TAG, "data is NULL");

    if (0 >= dataLen)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_TAG, "dataLen is less than or equal zero. Invalid input !");
        return CA_STATUS_INVALID_PARAM;
    }

    int numOfServersConnected = CAGetRegisteredServiceCount();

    for (int32_t pos = 0; pos < numOfServersConnected; pos++)
    {
        /*remoteAddress will be NULL.
          Since we have to send to all destinations. pos will be used for getting remote address.
         */
        CAResult_t  ret = CAUpdateCharacteristicsToGattServer(NULL, data, dataLen, LE_MULTICAST, pos);

        if (CA_STATUS_OK != ret)
        {
            OIC_LOG_V(ERROR, TZ_BLE_CLIENT_TAG,
                      "CAUpdateCharacteristicsToGattServer Failed with return val [%d] ", ret);
            g_clientErrorCallback(NULL, data, dataLen, ret);
            continue;
        }
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_TAG, "OUT ");
    return CA_STATUS_OK;
}
