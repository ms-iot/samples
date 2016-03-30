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
 * This file contains the util function for LE adapter. This maintains the
 * list of services an individual GATT Client connected to and operations on
 * that list, such as getting the service info with BD address or with
 * position etc. This is mainly useful for the multicast transmission of
 * data where client needs to have the info of all the services to which it
 * is connected.
 */

#ifndef TZ_BLE_UTIL_H_
#define TZ_BLE_UTIL_H_

#include <bluetooth.h>

#include "cacommon.h"

/**
 * Information regarding the GATTServer.
 *
 * This structure holds the infomation about the GATTServer
 * in the service and the characteristic level.
 */
typedef struct
{
    bt_gatt_attribute_h service_clone; /**< GATT attribute handler for the OIC service. */
    bt_gatt_attribute_h read_char;     /**< GATT attribute handler for OIC read characteristic.
                                            Server will read.*/
    bt_gatt_attribute_h write_char;    /**< GATT attribute handler for OIC write characteristic.
                                            server will write*/
    char *bdAddress;                   /**< BD address where OIC service is running. */
} BLEServiceInfo;

/**
 * List of the BLEServiceInfo structures.
 *
 * A list of BLEServiceInfo and gives the info about all the
 * the registered services from the client side.
 */
typedef struct _BLEServiceList
{
    BLEServiceInfo *serviceInfo;    /**< BLEServiceInfo structure from an OIC Server. */
    struct _BLEServiceList *next;   /**< Next reference to the List. */
} BLEServiceList;

/**
 * Different characteristics types.
 *
 * This provides information of different characteristics
 * which will be added to OIC service.
 */
typedef enum
{
    BLE_GATT_WRITE_CHAR = 0, /**< write_char This will be used to get the unicast response. */
    BLE_GATT_READ_CHAR,      /**< read_char This will be used update value to OIC server. */
    BLE_GATT_NOTIFY_CHAR     /**< Reserved char for the time being. */
} CHAR_TYPE;

/**
 * Stores the information required to set the descriptor value of the Service.
 */
typedef struct gattCharDescriptor
{
    bt_gatt_attribute_h characteristic; /**< The attribute handle of descriptor. */
    uint8_t *desc;                      /**< Descriptor handle of characteristic, in byte array. */
    int total;                          /**< The total number of descriptor in a characteristic. */
} stGattCharDescriptor_t;

/**
 * Used to increment the registered service count.
 */
void CAIncrementRegisteredServiceCount();

/**
 * Used to decrement the registered service count.
 */
void CADecrementRegisteredServiceCount();

/**
 * Used to reset the registered service count.
 */
void CAResetRegisteredServiceCount();

/**
 * Used to get the total registered service count.
 * @return  Total registered service count.
 */
int32_t  CAGetRegisteredServiceCount();

/**
 * Used to create BLEServiceInfo structure with server handler and BD address will be
 * created.
 * @param[in] bdAddress        BD address of the device where GATTServer is running.
 * @param[in] service          service attribute handler.
 * @param[in] bleServiceInfo   Pointer where serviceInfo structure needs to be stored.
 *                             Memory will be allocated here and needs to be cleared by caller.
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CACreateBLEServiceInfo(const char *bdAddress, bt_gatt_attribute_h service,
                                  BLEServiceInfo **bleServiceInfo);

/**
 * Used to append the characteristic info to the already created serviceInfo structure.
 *
 * @param[in] characteristic   Charecteristic attribute handler.
 * @param[in] type             Specifies whether its BLE_GATT_READ_CHAR or BLE_GATT_WRITE_CHAR
 * @param[in] bleServiceInfo   Pointer where serviceInfo structure needs to be appended with
 *                             char info.
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAAppendBLECharInfo(bt_gatt_attribute_h characteristic, CHAR_TYPE type,
                               BLEServiceInfo *bleServiceInfo);

/**
 * Used to add the ServiceInfo structure to the Service List.
 *
 * @param[in] serviceList      Pointer to the ble service list which holds the info of list of
 *                             service registered from client.
 * @param[in] bleServiceInfo   Pointer where serviceInfo structure needs to be appended with
 *                             char info.
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAAddBLEServiceInfoToList(BLEServiceList **serviceList,
                BLEServiceInfo *bleServiceInfo);

/**
 * Used to remove the ServiceInfo structure from the Service List.
 *
 * @param[in] serviceList      Pointer to the ble service list which holds the info of list of
 *                             service registered from client.
 * @param[in] bleServiceInfo   Pointer where serviceInfo structure needs to be appended with
 *                             char info.
 * @param[in] bdAddress        BD address of the device where GATTServer is disconnected.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CARemoveBLEServiceInfoToList(BLEServiceList **serviceList,
                                        BLEServiceInfo *bleServiceInfo,
                                        const char *bdAddress);

/**
 * Used to get the serviceInfo from the list.
 *
 * @param[in] serviceList       Pointer to the ble service list which holds the info of list
 *                              of service registered from client.
 * @param[in] bdAddress         BD address of the device where GATTServer information is required.
 * @param[out] bleServiceInfo   Pointer where serviceInfo structure needs to provide the service
 *                              and char info.
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAGetBLEServiceInfo(BLEServiceList *serviceList, const char *bdAddress,
                               BLEServiceInfo **bleServiceInfo);

/**
 * Used to get the serviceInfo from the list by position.
 *
 * @param[in] serviceList      Pointer to the ble service list which holds the info of list
 *                             of service registered from client.
 * @param[in] position         The service information of particular position in the list.
 * @param[out] bleServiceInfo  Pointer where serviceInfo structure needs to provide the service
 *                             and char info.
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAGetBLEServiceInfoByPosition(BLEServiceList *serviceList, int32_t position,
                                         BLEServiceInfo **bleServiceInfo);

/**
 * Used to clear BLE service list.
 *
 * @param[in]  serviceList   Pointer to the ble service list which holds the info of list of
 *                           service registered from client.
 */
void CAFreeBLEServiceList(BLEServiceList *serviceList);

/**
 * Used to get remove particular BLE service info from list.
 * @param[in] serviceinfo   Pointer to the structure which needs to be cleared.
 */
void CAFreeBLEServiceInfo(BLEServiceInfo *bleServiceInfo);

/**
 * Used to check whether found handle is OIC service handle or not.
 *
 * @param[in] serviceHandle   Discovered service handle(unique identifier for service).
 * @return  STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAVerifyOICServiceByServiceHandle(bt_gatt_attribute_h serviceHandle);

/**
 * Used to check whether UUID of the discovered device is OIC service or not.
 *
 * @param[in]  serviceUUID   Service UUID.
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAVerifyOICServiceByUUID(const char* serviceUUID);

/**
 * Used to get the Error message.
 * @param[in] err   Error code(::bt_error_e).
 * @return  Error string corresponding to the BT error code.
 */
const char *CABTGetErrorMsg(bt_error_e err);

#endif /* TZ_BLE_UTIL_H_ */
