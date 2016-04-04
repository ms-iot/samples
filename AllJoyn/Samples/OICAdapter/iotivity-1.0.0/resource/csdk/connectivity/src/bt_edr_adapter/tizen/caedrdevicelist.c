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
 *
 * This file provides APIs to access the discovered bluetooth device list.
 */

#include "caedrdevicelist.h"
#include "caadapterutils.h"
#include "caedrutils.h"
#include "logger.h"


/**
 * @fn  CACreateEDRDevice
 * @brief  Creates #EDRDevice for specified remote address and uuid.
 *
 */
static CAResult_t CACreateEDRDevice(const char *deviceAddress,
                                const char *uuid,EDRDevice **device);


/**
 * @fn  CADestroyEDRDevice
 * @brief  Free all the memory associated with specified device.
 *
 */
static void CADestroyEDRDevice(EDRDevice *device);


/**
 * @fn  CADestroyEDRData
 * @brief  Free all the memory associated with specified data.
 *
 */
static void CADestroyEDRData(EDRData *data);


CAResult_t CACreateAndAddToDeviceList(EDRDeviceList **deviceList, const char *deviceAddress,
                                      const char *uuid, EDRDevice **device)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL(deviceList, EDR_ADAPTER_TAG, "Device list is null");
    VERIFY_NON_NULL(deviceAddress, EDR_ADAPTER_TAG, "Remote address is null");
    VERIFY_NON_NULL(device, EDR_ADAPTER_TAG, "Device is null");
    CAResult_t result = CACreateEDRDevice(deviceAddress, uuid, device);
    if (CA_STATUS_OK != result || NULL == *device)
    {
        OIC_LOG_V(ERROR, EDR_ADAPTER_TAG, "Invalid or Not bonded device ret[%d]", result);
        return CA_STATUS_FAILED;
    }
    result = CAAddEDRDeviceToList(deviceList, *device);
    if (CA_STATUS_OK != result)
    {
        OIC_LOG_V(ERROR, EDR_ADAPTER_TAG, "Failed to add in list ret[%d]", result);

        //Remove created EDRDevice
        CADestroyEDRDevice(*device);
        *device = NULL;

        return CA_STATUS_FAILED;
    }

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CACreateEDRDevice(const char *deviceAddress, const char *uuid, EDRDevice **device)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL(deviceAddress, EDR_ADAPTER_TAG, "Remote address is null");
    VERIFY_NON_NULL(uuid, EDR_ADAPTER_TAG, "uuid is null");
    VERIFY_NON_NULL(device, EDR_ADAPTER_TAG, "Device is null");

    *device = (EDRDevice *) OICMalloc(sizeof(EDRDevice));
    if (NULL == *device)
    {
        OIC_LOG(ERROR, EDR_ADAPTER_TAG, "Out of memory (device)!");
        return CA_MEMORY_ALLOC_FAILED;
    }

    //Copy bluetooth address
    if (deviceAddress[0])
    {
        (*device)->remoteAddress = strndup(deviceAddress, strlen(deviceAddress));
        if (NULL == (*device)->remoteAddress)
        {
            OIC_LOG(ERROR, EDR_ADAPTER_TAG, "Out of memory (remote address)!");

            OICFree(*device);
            *device = NULL;
            return CA_MEMORY_ALLOC_FAILED;
        }
    }

    //Copy OIC service uuid
    if (strlen(uuid))
    {
        (*device)->serviceUUID = strndup(uuid, strlen(uuid));
        if (NULL == (*device)->serviceUUID)
        {
            OIC_LOG_V(ERROR, EDR_ADAPTER_TAG,
                      "[createEDRDevice] Out of memory (service uuid)!");

            OICFree((*device)->remoteAddress);
            OICFree(*device);
            *device = NULL;
            return CA_MEMORY_ALLOC_FAILED;
        }
    }

    (*device)->socketFD = -1;
    (*device)->pendingDataList = NULL;
    (*device)->serviceSearched = false;

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAAddEDRDeviceToList(EDRDeviceList **deviceList, EDRDevice *device)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL(deviceList, EDR_ADAPTER_TAG, "Device list is null");
    VERIFY_NON_NULL(device, EDR_ADAPTER_TAG, "Device is null");

    EDRDeviceList *node = (EDRDeviceList *) OICMalloc(sizeof(EDRDeviceList));
    if (NULL == node)
    {
        OIC_LOG(ERROR, EDR_ADAPTER_TAG, "Out of memory (device list)!");
        return CA_MEMORY_ALLOC_FAILED;
    }

    node->device = device;
    node->next = NULL;

    if (NULL == *deviceList) //Empty list
    {
        *deviceList = node;
    }
    else //Add at front end
    {
        node->next = *deviceList;
        *deviceList = node;
    }

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CAGetEDRDevice(EDRDeviceList *deviceList,
                            const char *deviceAddress, EDRDevice **device)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL(deviceList, EDR_ADAPTER_TAG, "Device list is null");
    VERIFY_NON_NULL(deviceAddress, EDR_ADAPTER_TAG, "Remote address is null");
    VERIFY_NON_NULL(device, EDR_ADAPTER_TAG, "Device is null");

    EDRDeviceList *curNode = deviceList;
    *device = NULL;
    while (curNode != NULL)
    {
        if (!strcasecmp(curNode->device->remoteAddress, deviceAddress))
        {
            *device = curNode->device;
            return CA_STATUS_OK;
        }

        curNode = curNode->next;
    }

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT [Device not found!]");
    return CA_STATUS_FAILED;
}

CAResult_t CAGetEDRDeviceBySocketId(EDRDeviceList *deviceList,
                                    int32_t socketID, EDRDevice **device)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL(deviceList, EDR_ADAPTER_TAG, "Device list is null");
    VERIFY_NON_NULL(device, EDR_ADAPTER_TAG, "Device is null");
    EDRDeviceList *curNode = deviceList;
    *device = NULL;
    while (curNode != NULL)
    {
        if (curNode->device->socketFD == socketID)
        {
            *device = curNode->device;
            return CA_STATUS_OK;
        }

        curNode = curNode->next;
    }

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
    return CA_STATUS_FAILED;
}

CAResult_t CARemoveEDRDeviceFromList(EDRDeviceList **deviceList,
    const char *deviceAddress)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL(deviceList, EDR_ADAPTER_TAG, "Device list is null");
    VERIFY_NON_NULL(deviceAddress, EDR_ADAPTER_TAG, "Remote address is null");

    EDRDeviceList *curNode = NULL;
    EDRDeviceList *prevNode = NULL;

    curNode = *deviceList;
    while (curNode != NULL)
    {
        if (!strcasecmp(curNode->device->remoteAddress, deviceAddress))
        {
            if (curNode == *deviceList)
            {
                *deviceList = curNode->next;

                curNode->next = NULL;
                CADestroyEDRDeviceList(&curNode);
                return CA_STATUS_OK;
            }
            else
            {
                prevNode->next = curNode->next;

                curNode->next = NULL;
                CADestroyEDRDeviceList(&curNode);
                return CA_STATUS_OK;
            }
        }
        else
        {
            prevNode = curNode;
            curNode = curNode->next;
        }
    }

    OIC_LOG(ERROR, EDR_ADAPTER_TAG, "OUT Device not in the list !");
    return CA_STATUS_FAILED;
}

void CADestroyEDRDeviceList(EDRDeviceList **deviceList)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL_VOID(deviceList, EDR_ADAPTER_TAG, "deviceList is null");

    while (*deviceList)
    {
        EDRDeviceList *curNode = *deviceList;
        *deviceList = (*deviceList)->next;

        CADestroyEDRDevice(curNode->device);
        OICFree(curNode);
    }

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
}

void CADestroyEDRDevice(EDRDevice *device)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");
    if (device)
    {
        OICFree(device->remoteAddress);
        OICFree(device->serviceUUID);
        CADestroyEDRDataList(&device->pendingDataList);
        OICFree(device);
    }
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
}

CAResult_t CAAddEDRDataToList(EDRDataList **dataList, const void *data, uint32_t dataLength)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL(dataList, EDR_ADAPTER_TAG, "Data list is null");
    VERIFY_NON_NULL(data, EDR_ADAPTER_TAG, "Data is null");

    if (0 == dataLength)
    {
        OIC_LOG(ERROR, EDR_ADAPTER_TAG, "Invalid input: data length is zero!");
        return CA_STATUS_INVALID_PARAM;
    }

    EDRDataList *pending_data = (EDRDataList *) OICMalloc(sizeof(EDRDataList));
    if (NULL == pending_data)
    {
        OIC_LOG(ERROR, EDR_ADAPTER_TAG, "OICMalloc failed (data list)!");
        return CA_MEMORY_ALLOC_FAILED;
    }

    pending_data->data = (EDRData *) OICMalloc(sizeof(EDRData));
    if (NULL == pending_data->data)
    {
        OIC_LOG(ERROR, EDR_ADAPTER_TAG, "OICMalloc failed (data node)!");

        OICFree(pending_data);
        return CA_MEMORY_ALLOC_FAILED;
    }
    pending_data->next = NULL;

    pending_data->data->data = (void *) OICMalloc(dataLength); //data
    if (NULL == pending_data->data->data)
    {
        OIC_LOG(ERROR, EDR_ADAPTER_TAG, "OICMalloc failed (data)!");

        OICFree(pending_data->data);
        OICFree(pending_data);
        return CA_MEMORY_ALLOC_FAILED;
    }

    memcpy(pending_data->data->data, data, dataLength);
    pending_data->data->dataLength = dataLength;

    if (NULL == *dataList) //Empty list
    {
        *dataList = pending_data;
    }
    else //Add at rear end
    {
        EDRDataList *curNode = *dataList;
        while (curNode->next != NULL)
        {
            curNode = curNode->next;
        }

        curNode->next = pending_data;
    }

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

CAResult_t CARemoveEDRDataFromList(EDRDataList **dataList)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL(dataList, EDR_ADAPTER_TAG, "Data list is null");

    if (*dataList)
    {
        EDRDataList *curNode = *dataList;
        *dataList = (*dataList)->next;

        //Delete the first node
        CADestroyEDRData(curNode->data);
        OICFree(curNode);
    }

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
    return CA_STATUS_OK;
}

void CADestroyEDRDataList(EDRDataList **dataList)
{
    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "IN");

    VERIFY_NON_NULL_VOID(dataList, EDR_ADAPTER_TAG, "Data list is null");

    while (*dataList)
    {
        EDRDataList *curNode = *dataList;
        *dataList = (*dataList)->next;

        CADestroyEDRData(curNode->data);
        OICFree(curNode);
    }

    *dataList = NULL;

    OIC_LOG(DEBUG, EDR_ADAPTER_TAG, "OUT");
}

void CADestroyEDRData(EDRData *data)
{
    if (data)
    {
        OICFree(data->data);
        OICFree(data);
    }
}

