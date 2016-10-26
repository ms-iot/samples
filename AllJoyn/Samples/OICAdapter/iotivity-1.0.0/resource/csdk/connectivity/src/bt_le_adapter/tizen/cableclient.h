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
 * This file contains the functionalities of GATT Client. Functionalities
 * like LE device discovery, connecting to the LE device with OIC service,
 * registering to the service and there characteristics, registering to the
 * change in the charateristics, setting the value of the characteristcs
 * for the request and response will be done here.
 */

#ifndef TZ_BLE_CLIENT_H_
#define TZ_BLE_CLIENT_H_

#include <bluetooth.h>
#include <bluetooth_type.h>
#include <bluetooth_product.h>

#include "cacommon.h"
#include "caadapterutils.h"
#include "cableutil.h"
#include "caadapterinterface.h"
#include "logger.h"
#include "cathreadpool.h"
#include "caleinterface.h"
#include "oic_malloc.h"


/**
 * This is the callback which will be called after the characteristic
 * value changes happen.
 *
 * @param[in]  characteristic The attribute handle of characteristic.
 * @param[in]  value          Value of the characteristics of a service.
 * @param[in]  valueLen       length of data.
 * @param[in]  userData       The user data passed from the request function.
 */
void CABleGattCharacteristicChangedCb(bt_gatt_attribute_h characteristic,
                                      unsigned char *value,
                                      int valueLen,
                                      void *userData);
/**
 * This is the callback which will be called after the characteristics changed.
 *
 * @param[in]  result   result of write value.
 * @param[in]  userData user context.
 */
void CABleGattCharacteristicWriteCb(int result, void *userData);

/**
 * This is the callback which will be called when descriptor of
 * characteristics is found.
 *
 * @param[in]  result         The result of discovering.
 * @param[in]  format         format of descriptor.
 * @param[in]  total          The total number of descriptor in a
 *                             characteristic.
 * @param[in]  descriptor     The attribute handle of descriptor.
 * @param[in]  characteristic The attribute handle of characteristic.
 * @param[in]  userData       The user data passed from the request function.
 */
void CABleGattDescriptorDiscoveredCb(int result, unsigned char format, int total,
                                     bt_gatt_attribute_h descriptor,
                                     bt_gatt_attribute_h characteristic, void *userData);

/**
 * This is the callback which will be called after the characteristics are
 * discovered by bt_gatt_discover_characteristics().
 *
 * @param[in]  result         The result of discovering.
 * @param[in]  inputIndex     The index of characteristics in a service,
 *                             starts from 0.
 * @param[in]  total          The total number of characteristics in a service.
 * @param[in]  characteristic The attribute handle of characteristic.
 * @param[in]  userData       The user data passed from the request function.
 *
 * @return  0 on failure and 1 on success.
 */
bool CABleGattCharacteristicsDiscoveredCb(int result, int inputIndex, int total,
                                          bt_gatt_attribute_h characteristic, void *userData);

/**
 * This is the callback which will be called when we get the primary
 * services repeatedly.
 *
 * @param[in] service  The attribute handle of service. Unique identifier
 *                      for service.
 * @param[in] index    The current index of the service.
 * @param[in] count    Total number of services available in remote device.
 * @param[in] userData user data.
 *
 * @return  0 on failure and 1 on success.
 */
bool CABleGattPrimaryServiceCb(bt_gatt_attribute_h service, int index, int count,
                                   void *userData);

/**
 * This is the callback which will be called whenever there is change in
 * gatt connection with server(Connected/Disconnected)
 *
 * @param[in]  result        The result of discovering.
 * @param[in]  connected     State of connection.
 * @param[in]  remoteAddress Mac address of the remote device in which we
 *                            made connection.
 * @param[in]  userData      The user data passed from the request function.
 */
void CABleGattConnectionStateChangedCb(int result, bool connected,
                const char *remoteAddress,void *userData);

/**
 * This is the callback which will be called when the device discovery
 * state changes.
 *
 * @param[in]  result         The result of discovering.
 * @param[in]  discoveryState State of the discovery(FOUND/STARTED/ FINISHED).
 * @param[in]  discoveryInfo  Remote Device information.
 * @param[in]  userData       The user data passed from the request function.
 */
void CABtAdapterLeDeviceDiscoveryStateChangedCb(int result,
        bt_adapter_le_device_discovery_state_e discoveryState,
        bt_adapter_le_device_discovery_info_s *discoveryInfo,
        void *userData);

/**
 * Used to print device information(Util method).
 * @param[in] discoveryInfo Device information structure.
 */
void CAPrintDiscoveryInformation(const bt_adapter_le_device_discovery_info_s *discoveryInfo);

/**
 * This thread will be used to initialize the Gatt Client and start device
 * discovery.
 *        1. Set scan parameters.
 *        2. Setting neccessary callbacks for connection, characteristics
 *          changed and discovery.
 *        3. Start device discovery.
 *
 * @param[in] data Currently it will be NULL(no parameter).
 */
void CAStartBleGattClientThread(void *data);

/**
 * Used to initialize all required mutex variable for Gatt Client
 * implementation.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CAInitGattClientMutexVariables();

/**
 * Used to terminate all required mutex variable for Gatt Client implementation.
 */
void CATerminateGattClientMutexVariables();

/**
 * Used to clear NonOICDeviceList.
 */
void CAClearNonOICDeviceList();

/**
 * Used to set scan parameter of starting discovery.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CABleGattSetScanParameter();

/**
 * Used to register required callbacks to BLE platform(connection,
 * discovery, etc).
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CABleGattSetCallbacks();

/**
 * Used to unset all the registerd callbacks to BLE platform.
 */
void CABleGattUnSetCallbacks();

/**
 * Used to watch all the changes happening in characteristics of the service.
 *
 * @param[in] service The attribute handle of the service.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CABleGattWatchCharacteristicChanges(bt_gatt_attribute_h service);

/**
 * Used to unwatch characteristics changes using
 * bt_gatt_unwatch_characteristic_changes().
 */
void CABleGattUnWatchCharacteristicChanges();

/**
 * Used to start LE discovery for BLE  devices.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CABleGattStartDeviceDiscovery();

/**
 * Used to stop LE discovery for BLE  devices.
 */
void CABleGattStopDeviceDiscovery();

/**
 * This is the thread  which will be used for making gatt connection with
 * remote devices.
 * @param[in] remoteAddress MAC address of remote device to connect.
 */
void CAGattConnectThread (void *remoteAddress);

/**
 * Used to do connection with remote device.
 *
 * @param[in] remoteAddress Remote address inwhich we wants to connect with.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CABleGattConnect(const char *remoteAddress);

/**
 * Used to do disconnection with remote device.
 * @param[in] remoteAddress Remote address inwhich we wants to disconnect with.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CABleGattDisConnect(const char *remoteAddress);

/**
 * This is thread which will be spawned for discovering ble services. Once
 * called discover api, then it will be terminated.
 * @param[in] remoteAddress Mac address of the remote device in which we
 *                           want to search services.
 */
void CADiscoverBLEServicesThread (void *remoteAddress);

/**
 * Used to discover the services that is advertised by Gatt Server
 * asynchronously.
 *
 * @param[in] remoteAddress MAC address of remote device in which we want
 *                           to discover the services.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CABleGattDiscoverServices(const char *remoteAddress);

/**
 * This is the thread which will be used for finding characteristic of a
 * service.
 *
 * @param[in]  stServiceInfo Service Information which contains the remote
 *                            address, service handle and characteristic handle.
 */
void CADiscoverCharThread(void *stServiceInfo);

/**
 * Used to discover characteristics of service using
 * bt_gatt_discover_characteristics() api.
 *
 * @param[in] service        The attribute handle for service.
 * @param[in] remoteAddress  Remote address inwhich we wants to discover
 *                            characteristics of given service handle.
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CABleGattDiscoverCharacteristics(bt_gatt_attribute_h service,
                    const char *remoteAddress);

/**
 * This is the thread which will be used for finding descriptor of
 * characteristic.
 *
 * @param[in]  stServiceInfo Service Information which contains the remote
 *                            address, service handle and characteristic handle.
 */
void CADiscoverDescriptorThread(void *stServiceInfo);

/**
 * This is thread which will be used for calling
 * CASetCharacteristicDescriptorValue() api.
 *
 * @param[in] service        The attribute handle for characteristics.
 * @param[in] remoteAddress  Remote address inwhich we wants to discover
 *                            descriptor of given char handle.
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CABleGattDiscoverDescriptor(bt_gatt_attribute_h service,
                const char *remoteAddress);

/**
 * This is thread which will be used for calling
 * CASetCharacteristicDescriptorValue() api.
 *
 * @param[in]  stServiceInfo Service Information which contains the remote
 *                            address, service handle and characteristic handle.
 */
void CASetCharacteristicDescriptorValueThread(void *stServiceInfo);

/**
 * Used to set characteristic descriptor value using
 * bt_gatt_set_characteristic_desc_value_request() api.
 * @param[in]  stGattCharDescriptorInfo Structure which contains char
 *                                       handle and descriptor handle.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CASetCharacteristicDescriptorValue
            (stGattCharDescriptor_t *stGattCharDescriptorInfo);

/**
 * Used to enqueue the message into sender queue using
 * CAAdapterEnqueueMessage() and make signal to the thread to process.
 *
 * @param[in]  remoteEndpoint Remote device information.
 * @param[in]  data           Data to be sent to remote device.
 * @param[in]  dataLen        Length of data..
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CABleClientSenderQueueEnqueueMessage
                            (const CAEndpoint_t *remoteEndpoint,
                                                const uint8_t *data, uint32_t dataLen);

/**
 * This is the thread which will be used for processing sender queue.
 */
void CABleClientSenderQueueProcessor();

/**
 * Synchronous function for reading characteristic value.
 *
 * @return ::CA_STATUS_OK or Appropriate error code.
 * @retval ::CA_STATUS_OK  Successful.
 * @retval ::CA_STATUS_INVALID_PARAM  Invalid input arguments.
 * @retval ::CA_STATUS_FAILED Operation failed.
 */
CAResult_t CALEReadDataFromLEClient();

#endif /* TZ_BLE_CLIENT_H_ */
