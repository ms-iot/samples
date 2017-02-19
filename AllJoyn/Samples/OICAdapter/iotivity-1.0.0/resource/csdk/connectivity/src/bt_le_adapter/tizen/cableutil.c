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

#include "cableutil.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>


#include "caadapterutils.h"
#include "cagattservice.h"
#include "oic_string.h"
#include "oic_malloc.h"

/**
 * @def TZ_BLE_CLIENT_UTIL_TAG
 * @brief Logging tag for module name
 */
#define TZ_BLE_CLIENT_UTIL_TAG "TZ_BLE_GATT_CLIENT_UTIL"

/**
 * @var g_numberOfServiceConnected
 * @brief Number of services connected.
 */
static int32_t g_numberOfServiceConnected = 0;

void CAIncrementRegisteredServiceCount()
{
    g_numberOfServiceConnected++;
}

void CADecrementRegisteredServiceCount()
{
    g_numberOfServiceConnected--;
}

void CAResetRegisteredServiceCount()
{
    g_numberOfServiceConnected = 0;
}

int32_t  CAGetRegisteredServiceCount()
{
    return g_numberOfServiceConnected ;
}

CAResult_t CACreateBLEServiceInfo(const char *bdAddress, bt_gatt_attribute_h service,
                                  BLEServiceInfo **bleServiceInfo)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "IN");

    VERIFY_NON_NULL(bdAddress, TZ_BLE_CLIENT_UTIL_TAG, "Param bdAddress is NULL");
    VERIFY_NON_NULL(service, TZ_BLE_CLIENT_UTIL_TAG, "Param service is NULL");
    VERIFY_NON_NULL(bleServiceInfo, TZ_BLE_CLIENT_UTIL_TAG, "Param bleServiceInfo is NULL");

    *bleServiceInfo = (BLEServiceInfo *) OICCalloc(1, sizeof(BLEServiceInfo));
    if (NULL == *bleServiceInfo)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_UTIL_TAG, "Malloc failed!");
        return CA_STATUS_FAILED;
    }

    (*bleServiceInfo)->bdAddress = OICStrdup(bdAddress);

    if (NULL == (*bleServiceInfo)->bdAddress)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_UTIL_TAG, "Malloc failed!");
        OICFree(*bleServiceInfo);
        return CA_STATUS_FAILED;
    }

    if (service)
    {
        int32_t ret = bt_gatt_clone_attribute_handle(&((*bleServiceInfo)->service_clone), service);

        if (BT_ERROR_NONE != ret)
        {
            OIC_LOG_V(ERROR, TZ_BLE_CLIENT_UTIL_TAG, "service handle clone failed with ret [%d]",
                      ret);
            OICFree((*bleServiceInfo)->bdAddress);
            OICFree(*bleServiceInfo);
            return CA_STATUS_FAILED;
        }
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "OUT");

    return CA_STATUS_OK;
}

CAResult_t CAAppendBLECharInfo( bt_gatt_attribute_h characteristic, CHAR_TYPE type,
                                BLEServiceInfo *bleServiceInfo)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "IN");

    VERIFY_NON_NULL(characteristic, TZ_BLE_CLIENT_UTIL_TAG, "Param characteristic is NULL");
    VERIFY_NON_NULL(bleServiceInfo, TZ_BLE_CLIENT_UTIL_TAG, "Param bleServiceInfo is NULL");

    if (BLE_GATT_READ_CHAR == type )
    {
        int ret = bt_gatt_clone_attribute_handle(&((bleServiceInfo)->read_char),
            characteristic);
        if (BT_ERROR_NONE != ret)
        {
            OIC_LOG_V(ERROR, TZ_BLE_CLIENT_UTIL_TAG, "read_char clone failed with ret [%d]",
                      ret);
            return CA_STATUS_FAILED;
        }
    }
    else  if (BLE_GATT_WRITE_CHAR == type)
    {
        int ret = bt_gatt_clone_attribute_handle(&((bleServiceInfo)->write_char),
            characteristic);
        if (BT_ERROR_NONE != ret)
        {
            OIC_LOG_V(ERROR, TZ_BLE_CLIENT_UTIL_TAG, "write_char clone failed with ret [%d]",
                      ret);
            return CA_STATUS_FAILED;
        }
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "OUT");

    return CA_STATUS_OK;
}

CAResult_t CAAddBLEServiceInfoToList(BLEServiceList **serviceList,
    BLEServiceInfo *bleServiceInfo)
{

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "IN");

    VERIFY_NON_NULL(serviceList, TZ_BLE_CLIENT_UTIL_TAG, "Param serviceList is NULL");
    VERIFY_NON_NULL(bleServiceInfo, TZ_BLE_CLIENT_UTIL_TAG, "Param bleServiceInfo is NULL");

    BLEServiceList *node = (BLEServiceList *) OICCalloc(1, sizeof(BLEServiceList));
    if (NULL == node)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_UTIL_TAG, "Malloc failed!");
        return CA_STATUS_FAILED;
    }

    node->serviceInfo = bleServiceInfo;
    node->next = NULL;

    if (*serviceList == NULL)   // Empty list
    {
        *serviceList = node;
    }
    else     // Add at front end
    {
        node->next = *serviceList;
        *serviceList = node;
    }

    CAIncrementRegisteredServiceCount();

    OIC_LOG_V(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "Device [%s] added to list",
        bleServiceInfo->bdAddress);

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "OUT");

    return CA_STATUS_OK;
}

CAResult_t CARemoveBLEServiceInfoToList(BLEServiceList **serviceList,
                                        BLEServiceInfo *bleServiceInfo,
                                        const char *bdAddress)
{

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "IN");

    VERIFY_NON_NULL(serviceList, TZ_BLE_CLIENT_UTIL_TAG, "Param serviceList is NULL");
    VERIFY_NON_NULL(*serviceList, TZ_BLE_CLIENT_UTIL_TAG, "Param *serviceList is NULL");
    VERIFY_NON_NULL(bdAddress, TZ_BLE_CLIENT_UTIL_TAG, "Param bdAddress is NULL");

    BLEServiceList *prev = NULL;
    BLEServiceList *cur = *serviceList;
    while (cur != NULL)
    {
        if (!strcasecmp(cur->serviceInfo->bdAddress, bdAddress))
        {
            if (cur == *serviceList)
            {
                *serviceList = cur->next;

                cur->next = NULL;
                CAFreeBLEServiceList(cur);
                CADecrementRegisteredServiceCount();
                OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "OUT");
                return CA_STATUS_OK;
            }
            else
            {
                prev->next = cur->next;

                cur->next = NULL;
                CAFreeBLEServiceList(cur);
                CADecrementRegisteredServiceCount();
                OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "OUT");
                return CA_STATUS_OK;
            }
        }
        else
        {
            prev = cur;
            cur = cur->next;
        }
    }
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, " OUT");
    return CA_STATUS_FAILED;
}

CAResult_t CAGetBLEServiceInfo(BLEServiceList *serviceList, const char *bdAddress,
                               BLEServiceInfo **bleServiceInfo)
{

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "IN");

    VERIFY_NON_NULL(serviceList, TZ_BLE_CLIENT_UTIL_TAG, "Param serviceList is NULL");
    VERIFY_NON_NULL(bleServiceInfo, TZ_BLE_CLIENT_UTIL_TAG, "Param bleServiceInfo is NULL");
    VERIFY_NON_NULL(bdAddress, TZ_BLE_CLIENT_UTIL_TAG, "Param bdAddress is NULL");


    BLEServiceList *cur = serviceList;
    *bleServiceInfo = NULL;
    while (cur != NULL)
    {
        if (!strcasecmp(cur->serviceInfo->bdAddress, bdAddress))
        {
            *bleServiceInfo = cur->serviceInfo;
            OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "OUT");
            return CA_STATUS_OK;
        }

        cur = cur->next;
    }

    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, " OUT");
    return CA_STATUS_FAILED;
}

CAResult_t CAGetBLEServiceInfoByPosition(BLEServiceList *serviceList, int32_t position,
        BLEServiceInfo **bleServiceInfo)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "IN");

    VERIFY_NON_NULL(serviceList, TZ_BLE_CLIENT_UTIL_TAG, "Param serviceList is NULL");
    VERIFY_NON_NULL(bleServiceInfo, TZ_BLE_CLIENT_UTIL_TAG, "Param bleServiceInfo is NULL");

    if (0 > position)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_UTIL_TAG, "Position Invalid input !");
        return CA_STATUS_INVALID_PARAM;
    }

    *bleServiceInfo = NULL;
    int32_t count = 0;
    BLEServiceList *cur = serviceList;
    while (cur != NULL)
    {
        if (position == count)
        {
            *bleServiceInfo = cur->serviceInfo;
            OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "OUT");
            return CA_STATUS_OK;
        }
        count++;
        cur = cur->next;
    }
    return CA_STATUS_FAILED;
}

void CAFreeBLEServiceList(BLEServiceList *serviceList)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "IN");
    while (serviceList)
    {
        BLEServiceList *temp = serviceList;
        serviceList = serviceList->next;
        CAFreeBLEServiceInfo(temp->serviceInfo);
        OICFree(temp);
    }
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "OUT");
}

void CAFreeBLEServiceInfo(BLEServiceInfo *bleServiceInfo)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "IN");
    if (bleServiceInfo)
    {
        if (bleServiceInfo->bdAddress)
        {
            bt_gatt_disconnect(bleServiceInfo->bdAddress);
            OICFree(bleServiceInfo->bdAddress);
            bt_gatt_destroy_attribute_handle(bleServiceInfo->service_clone);
            bt_gatt_destroy_attribute_handle(bleServiceInfo->read_char);
            bt_gatt_destroy_attribute_handle(bleServiceInfo->write_char);
        }
        OICFree(bleServiceInfo);
    }
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "OUT");
}

CAResult_t CAVerifyOICServiceByUUID(const char* serviceUUID)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "IN");

    VERIFY_NON_NULL(serviceUUID, TZ_BLE_CLIENT_UTIL_TAG, "Param serviceHandle is NULL");

    if (strcasecmp(serviceUUID, CA_GATT_SERVICE_UUID) != 0)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_UTIL_TAG, "It is not OIC service!");
        return CA_STATUS_FAILED;
    }
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAVerifyOICServiceByServiceHandle(bt_gatt_attribute_h serviceHandle)
{
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "IN");

    VERIFY_NON_NULL(serviceHandle, TZ_BLE_CLIENT_UTIL_TAG, "Param serviceHandle is NULL");

    char *uuid = NULL;
    int ret = bt_gatt_get_service_uuid(serviceHandle, &uuid);

    if (0 != ret || NULL == uuid)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_UTIL_TAG, "bt_gatt_get_service_uuid failed !");
        return CA_STATUS_FAILED;
    }

    if (strcasecmp(uuid, CA_GATT_SERVICE_UUID) != 0)
    {
        OIC_LOG(ERROR, TZ_BLE_CLIENT_UTIL_TAG, "It is not OIC service!");
        OICFree(uuid);
        return CA_STATUS_FAILED;
    }

    OICFree(uuid);
    OIC_LOG(DEBUG, TZ_BLE_CLIENT_UTIL_TAG, "OUT");
    return CA_STATUS_OK;
}

const char *CABTGetErrorMsg(bt_error_e err)
{
    const char *errStr = NULL;

    switch (err)
    {
        case BT_ERROR_NONE:
            errStr = "BT_ERROR_NONE";
            break;
        case BT_ERROR_CANCELLED:
            errStr = "BT_ERROR_CANCELLED";
            break;
        case BT_ERROR_INVALID_PARAMETER:
            errStr = "BT_ERROR_INVALID_PARAMETER";
            break;
        case BT_ERROR_OUT_OF_MEMORY:
            errStr = "BT_ERROR_OUT_OF_MEMORY";
            break;
        case BT_ERROR_RESOURCE_BUSY:
            errStr = "BT_ERROR_RESOURCE_BUSY";
            break;
        case BT_ERROR_TIMED_OUT:
            errStr = "BT_ERROR_TIMED_OUT";
            break;
        case BT_ERROR_NOW_IN_PROGRESS:
            errStr = "BT_ERROR_NOW_IN_PROGRESS";
            break;
        case BT_ERROR_NOT_INITIALIZED:
            errStr = "BT_ERROR_NOT_INITIALIZED";
            break;
        case BT_ERROR_NOT_ENABLED:
            errStr = "BT_ERROR_NOT_ENABLED";
            break;
        case BT_ERROR_ALREADY_DONE:
            errStr = "BT_ERROR_ALREADY_DONE";
            break;
        case BT_ERROR_OPERATION_FAILED:
            errStr = "BT_ERROR_OPERATION_FAILED";
            break;
        case BT_ERROR_NOT_IN_PROGRESS:
            errStr = "BT_ERROR_NOT_IN_PROGRESS";
            break;
        case BT_ERROR_REMOTE_DEVICE_NOT_BONDED:
            errStr = "BT_ERROR_REMOTE_DEVICE_NOT_BONDED";
            break;
        case BT_ERROR_AUTH_REJECTED:
            errStr = "BT_ERROR_AUTH_REJECTED";
            break;
        case BT_ERROR_AUTH_FAILED:
            errStr = "BT_ERROR_AUTH_FAILED";
            break;
        case BT_ERROR_REMOTE_DEVICE_NOT_FOUND:
            errStr = "BT_ERROR_REMOTE_DEVICE_NOT_FOUND";
            break;
        case BT_ERROR_SERVICE_SEARCH_FAILED:
            errStr = "BT_ERROR_SERVICE_SEARCH_FAILED";
            break;
        case BT_ERROR_REMOTE_DEVICE_NOT_CONNECTED:
            errStr = "BT_ERROR_REMOTE_DEVICE_NOT_CONNECTED";
            break;
        case BT_ERROR_PERMISSION_DENIED:
            errStr = "BT_ERROR_PERMISSION_DENIED";
            break;
        case BT_ERROR_SERVICE_NOT_FOUND:
            errStr = "BT_ERROR_SERVICE_NOT_FOUND";
            break;
        case BT_ERROR_NOT_SUPPORTED:
            errStr = "BT_ERROR_NOT_SUPPORTED";
            break;
        default:
            errStr = "NOT Defined";
            break;
    }

    return errStr;
}


