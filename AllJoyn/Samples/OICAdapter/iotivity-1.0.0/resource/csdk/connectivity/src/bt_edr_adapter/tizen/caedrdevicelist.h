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
 * This file provides APIs to manage discovered bluetooth device list.
 */

#ifndef CA_EDR_DEVICE_LIST_H_
#define CA_EDR_DEVICE_LIST_H_

#include "cacommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Structure to maintain the data needs to send to peer Bluetooth device.
 */
typedef struct
{
    void *data;             /**< Data to be sent to peer Bluetooth device. */
    uint32_t dataLength;    /**< Length of the data. */
} EDRData;

/**
 * Structure to maintain list of data needs to send to peer Bluetooth device.
 */
typedef struct _EDRDataList
{
    EDRData *data;            /**< Data to be sent to peer Bluetooth device. */
    struct _EDRDataList *next;/**< Reference to next data in list. */
} EDRDataList;

/**
 * Structure to maintain information of peer Bluetooth device.
 */
typedef struct
{
    char *remoteAddress;        /**< Address of peer Bluetooth device. */
    char *serviceUUID;          /**< OIC service UUID running in peer Bluetooth device. */
    int socketFD;           /**< RfComm connection socket FD. */
    EDRDataList *pendingDataList;/**< List of data needs to send to peer Bluetooth device. */
    bool serviceSearched;   /**< Flag to indicate the status of service search. */
} EDRDevice;

/**
 * Structure to maintain list of peer Bluetooth device information.
 */
typedef struct _EDRDeviceList
{
    EDRDevice *device;            /**< Bluetooth device information. */
    struct _EDRDeviceList *next;  /**< Reference to next device information. */
} EDRDeviceList;

/**
 * Creates ::EDRDevice for specified remote address and uuid and to device list.
 *
 * @param[in/out]  deviceList       Device list which created device add to.
 * @param[in]      deviceAddress    Bluetooth device address.
 * @param[in]      uuid             Service uuid.
 * @param[in]      device           Created ::EDRDevice.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM Invalid input parameters.
 * @retval ::CA_STATUS_FAILED Failed to create device and add to list.
 */
CAResult_t CACreateAndAddToDeviceList(EDRDeviceList **deviceList, const char *deviceAddress,
                                      const char *uuid, EDRDevice **device);

/**
 * Insert device to specified list.
 *
 * @param[in/out]  deviceList        Device list to which specifed @device to be added.
 * @param[in]      device            Device to be added to list.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM Invalid input parameters.
 * @retval ::CA_MEMORY_ALLOC_FAILED Memory allocation failed.
 */
CAResult_t CAAddEDRDeviceToList(EDRDeviceList **deviceList, EDRDevice *device);

/**
 * Get the device from list which matches specified device address.
 *
 * @param[in]   deviceList     Device list to search for the device.
 * @param[in]   deviceAddress  Device address used for matching.
 * @param[out]  device         ::EDRDevice which has matching device address.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM Invalid input parameters.
 * @retval ::CA_STATUS_FAILED Device is not found in the list.
 */
CAResult_t CAGetEDRDevice(EDRDeviceList *deviceList,
                           const char *deviceAddress, EDRDevice **device);

/**
 * Get the device from list which matches specified RFCOMM socket id.
 *
 * @param[in]   deviceList  Device list to search for the device.
 * @param[in]   socketID    RFCOMM socket id.
 * @param[out]  device      ::EDRDevice which has matching RFCOMM socket id .
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM Invalid input parameters.
 * @retval ::CA_STATUS_FAILED Device is not found in the list.
 */
CAResult_t CAGetEDRDeviceBySocketId(EDRDeviceList *deviceList, int32_t socketID,
                                    EDRDevice **device);

/**
 * Remove and delete the device matching specified device address from list.
 *
 * @param[in/out]  deviceList        Device list to search for the device.
 * @param[in]      deviceAddress     Bluetooth device address.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM Invalid input parameters.
 * @retval ::CA_STATUS_FAILED Device is not found in the list.
 */
CAResult_t CARemoveEDRDeviceFromList(EDRDeviceList **deviceList,
                                    const char *deviceAddress);

/**
 * Destroy the specified device list. Removes and delete all the devices in the list.
 * @param[in/out]  deviceList      Device list to be destroyed.
 */
void CADestroyEDRDeviceList(EDRDeviceList **deviceList);

/**
 * Insert data to specified list.
 *
 * @param[in]  dataList        Data list to which data will be add.
 * @param[in]  data            Data to be stored.
 * @param[in]  dataLength      Length of the data.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM Invalid input parameters.
 * @retval ::CA_MEMORY_ALLOC_FAILED Memory allocation failed.
 */
CAResult_t CAAddEDRDataToList(EDRDataList **dataList, const void *data, uint32_t dataLength);

/**
 * Remove and delete data from front end of list.
 * @param[in/out]  dataList      Data list from which data will be removed.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM Invalid input parameters.
 */
CAResult_t CARemoveEDRDataFromList(EDRDataList **dataList);

/**
 * Destroy the specified data list. Removes and deletes all the data in the list.
 * @param[in]  dataList      Data list to be destroyed.
 */
void CADestroyEDRDataList(EDRDataList **dataList);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_EDR_DEVICE_LIST_H_ */


