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
 * This file contains the APIs related to the GATT Server functionalities.
 * Creation of the GattServer with the characteristics. Enabling the
 * advertisement and updating the characteristics for the response and
 * notifying the change of characteristcs for the request will be done here.
 * LE adapter will interact with this sub module.
 */

#ifndef TZ_BLE_SERVER_H_
#define TZ_BLE_SERVER_H_

#include "caadapterinterface.h"
#include "logger.h"
#include "cathreadpool.h"
#include "caleinterface.h"

/**
 * This is thread which will be used for creating ble service and advertise ble service.
 * 1. Create New OIC Service 2. Add two read & write characteristics to service.
 * 3. Register Service     4. Advertise service.
 *
 * @param[in] data  Currently it will be NULL.
 */
void CAStartBleGattServerThread(void *data);

/**
 * Used to initialize gatt service using _bt_gatt_init_service api.
 *
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAInitBleGattService();

/**
 * Used to de-initialize gatt service using _bt_gatt_deinit_service api.
 *
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CADeInitBleGattService();

/**
 * Used to initialize all required mutex variables for GATT server implementation.
 *
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAInitGattServerMutexVariables();


/**
 * Used to terminate all required mutex variables for GATT server implementation.
 */
void CATerminateGattServerMutexVariables();

/**
 * Used to add new OIC service in GATT server using bt_gatt_add_service api and
 * internally store service path(outparam) in global variable.
 *
 * @param[in]  serviceUUID  unique identifier for BLE OIC service.
 *
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAAddNewBleServiceInGattServer(const char *serviceUUID);

/**
 * Used to remove already registered service from Gatt Server using
 * bt_gatt_remove_service api.
 * @param[in] svcPath  unique identifier for BLE OIC service which is outparam of
 *                     bt_gatt_add_service api.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CARemoveBleServiceFromGattServer(const char *svcPath);

/**
 * Used to remove all the registered service from Gatt Server using
 * bt_gatt_delete_services api.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CARemoveAllBleServicesFromGattServer();

/**
 * Used to register the service in Gatt Server using bt_gatt_register_service api.
 *
 * @param[in] svcPath  unique identifier for BLE OIC service which is outparam of
 *                     bt_gatt_add_service api.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CARegisterBleServicewithGattServer(const char *svcPath);

/**
 * Used to add new characteristics(Read/Write) to the service in Gatt Server using
 * bt_gatt_add_characteristic api.
 * @param[in] svcPath         Service path to which this characteristic belongs to.
 * @param[in] charUUID        Gatt characteristic uuid.
 * @param[in] charValue       Gatt characteristic value.
 * @param[in] charValueLen    Characteristic value length.
 * @param[in] read            Boolean variable for checking whether read characteristics or
 *                            write characteristics.
 * @return  ::CA_STATUS_OK or Appropriate error code.
 */
CAResult_t CAAddNewCharacteristicsToGattServer(const char *svcPath, const char *charUUID,
                                               const uint8_t *charValue, int charValueLen,
                                               bool read);

/**
 * Used to remove characteristics(Read/Write) from the service in Gatt Server.
 *
 * @param[in]  charPath   Characteristic path registered on the interface and unique identifier
 *                        for added characteristics.
 *
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CARemoveCharacteristicsFromGattServer(const char *charPath);

/**
 * This is the callback which will be called when client update one of the characteristics
 * with data.
 * @param[in]  charPath       characteristic path registered on the interface and unique
 *                            identifier for added characteristics.
 * @param[in]  charValue      data which is send by client.
 * @param[in]  charValueLen   length of the data.
 * @param[in]  remoteAddress  remote device bluetooth address in which data is received.
 * @param[in]  userData       user data.

 */
void CABleGattRemoteCharacteristicWriteCb(char *charPath, unsigned char *charValue,
                                          int charValueLen, const char  *remoteAddress,
                                          void *userData);

/**
 * This is the callback which will be called whenever there is change in gatt connection
 * with Client(Connected/Disconnected).
 *
 * @param[in]  result         The result of discovering.
 * @param[in]  connected      State of connection.
 * @param[in]  remoteAddress  Mac address of the remote device in which we made connection.
 * @param[in]  userData       The user data passed from the request function.

 */
void CABleGattServerConnectionStateChangedCb(int result, bool connected,
                                             const char *remoteAddress, void *userData);

/**
 * Synchronous function for reading characteristic value.
 *
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CALEReadDataFromLEServer();

/**
 * Used to enqueue the message into sender queue using CAAdapterEnqueueMessage and make
 * signal to the thread to process.
 *
 * @param[in]  remoteEndpoint  Remote device information.
 * @param[in]  data            Data to be sent to remote device.
 * @param[in]  dataLen         Length of data.
 *
 * @return  ::CA_STATUS_OK or Appropriate error code.
 * @retval  ::CA_STATUS_OK  Successful.
 * @retval  ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval  ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CABleServerSenderQueueEnqueueMessage
                (const CAEndpoint_t *remoteEndpoint, const uint8_t *data, uint32_t dataLen);

/**
 * This is the thread which will be used for processing receiver queue.
 */
void *CABleServerSenderQueueProcessor();

#endif /* TZ_BLE_SERVER_H_ */

