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
 * This file contains the APIs for BT LE communications.
 */
#ifndef CA_LESERVER_H_
#define CA_LESERVER_H_

#include "cacommon.h"
#include "cathreadpool.h"
#include "uarraylist.h"
#include "jni.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Callback to be notified on reception of any data from remote devices.
 * @param[in]  address           MAC address of remote device.
 * @param[in]  data              Data received from remote device.
 * @pre  Callback must be registered using CALEServerSetCallback(CAPacketReceiveCallback callback).
 */
typedef void (*CAPacketReceiveCallback)(const char *address, const uint8_t *data);

/**
 * initialize server for BLE.
 * @param[in]   handle           thread pool handle object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerInitialize(ca_thread_pool_t handle);

/**
 * terminate client for BLE.
 */
void CALEServerTerminate();

/**
 * send data for unicast (interface).
 * @param[in]   address          remote address.
 * @param[in]   data             data for transmission.
 * @param[in]   dataLen          data length.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerSendUnicastMessage(const char *address, const uint8_t *data, uint32_t dataLen);

/**
 * send data for multicast (interface).
 * @param[in]   data             data for transmission.
 * @param[in]   dataLen          data length.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerSendMulticastMessage(const uint8_t *data, uint32_t dataLen);

/**
 * start multicast server (start advertise).
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerStartMulticastServer();

/**
 * stop multicast server (stop discovery).
 * @return  None
 */
CAResult_t CALEServerStopMulticastServer();

/**
 * set this callback for receiving data packets from peer devices.
 * @param[in]   callback         callback to be notified on reception of
 *                               unicast/multicast data packets.
 */
void CALEServerSetCallback(CAPacketReceiveCallback callback);

/**
 * send data for unicast (implement).
 * @param[in]   env              JNI interface pointer.
 * @param[in]   address          remote address.
 * @param[in]   data             data for transmission.
 * @param[in]   dataLen          data length.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerSendUnicastMessageImpl(JNIEnv *env, const char *address, const uint8_t *data,
                                            uint32_t dataLen);

/**
 * send data for multicast (implement)
 * @param[in]   env              JNI interface pointer
 * @param[in]   data             data for transmission
 * @param[in]   dataLen          data length
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerSendMulticastMessageImpl(JNIEnv *env, const uint8_t *data, uint32_t dataLen);

/**
 * set context of application.
 */
void CALEServerJNISetContext();

/**
 * initialize JNI object.
 */
void CALeServerJniInit();

/**
 * create interface object and initialize the object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerCreateJniInterfaceObject();

/**
 * start advertise in gatt server.
 */
CAResult_t CALEStartAdvertise();

/**
 * start advertise in gatt server.
 * @param[in]   env                JNI interface pointer.
 * @param[in]   advertiseCallback  callback to be notified on reception of
 *                                 advertisement result.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerStartAdvertise(JNIEnv *env, jobject advertiseCallback);

/**
 * stop advertise in gatt server.
 * @param[in]   env                JNI interface pointer.
 * @param[in]   advertiseCallback  callback to be notified on reception of
 *                                 advertisement result.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerStopAdvertise(JNIEnv *env, jobject advertiseCallback);

/**
 * open a gatt server.
 * @param[in]   env                JNI interface pointer.
 * @return  gatt server object.
 */
jobject CALEServerOpenGattServer(JNIEnv *env);

/**
 * create gatt service
 * @param[in]   env                JNI interface pointer
 * @return  gatt service object
 */
jobject CALEServerCreateGattService(JNIEnv *env);

/**
 * add a descriptor to the characteristic.
 * @param[in]   env                JNI interface pointer.
 * @param[in]   characteristic     Characteristic object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerAddDescriptor(JNIEnv *env, jobject characteristic);

/**
 * create gatt service.
 * @param[in]   env                  JNI interface pointer.
 * @param[in]   bluetoothGattServer  Bluetooth Gatt Server object.
 * @param[in]   bluetoothGattService Bluetooth Gatt Service object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerAddGattService(JNIEnv *env, jobject bluetoothGattServer,
                                    jobject bluetoothGattService);

/**
 * start gatt server.
 * @param[in]   env                  JNI interface pointer.
 * @param[in]   gattServerCallback   callback to be notified on reception of
 *                                   state change of gatt server.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerStartGattServer(JNIEnv *env, jobject gattServerCallback);

/**
 * close gatt server.
 * @param[in]   env                  JNI interface pointer.
 * @param[in]   bluetoothGattServer  Gatt Server object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerGattClose(JNIEnv *env, jobject bluetoothGattServer);
/**
 * send data.
 * @param[in]   env                  JNI interface pointer.
 * @param[in]   bluetoothDevice      bluetooth device object.
 * @param[in]   data                 data which send.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerSend(JNIEnv *env, jobject bluetoothDevice, jstring data);

/**
 * set data in BluetoothGattCharacteristic.
 * @param[in]   env                  JNI interface pointer.
 * @param[in]   responseData         data to set in characteristic object.
 * @return  BluetoothGattCharacteristic object.
 */
jobject CALEServerSetResponseData(JNIEnv *env, jbyteArray responseData);

/**
 * send data through notifyCharacteristicChanged api of android.
 * @param[in]   env                  JNI interface pointer.
 * @param[in]   device               bluetooth device object.
 * @param[in]   responseData         data which send.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerSendResponseData(JNIEnv *env, jobject device, jobject responseData);

/**
 * send a response to a write request to a remote device.
 * @param[in]   env                  JNI interface pointer.
 * @param[in]   device               bluetooth device object.
 * @param[in]   requestId            the id of request.
 * @param[in]   status               the status of the request to be sent to the remote devices.
 * @param[in]   offset               value offset for partial write response.
 * @param[in]   value                the value of the attribute that written (optional).
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerSendResponse(JNIEnv *env, jobject device, jint requestId, jint status,
                                  jint offset, jbyteArray value);

/**
 * connect BLE to remote device form gatt server.
 * @param[in]   env                  JNI interface pointer.
 * @param[in]   bluetoothDevice      bluetooth device object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerConnect(JNIEnv *env, jobject bluetoothDevice);

/**
 * disconnect LE for all devices.
 * @param[in]   env                  JNI interface pointer.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerDisconnectAllDevices(JNIEnv *env);

/**
 * disconnect LE to remote device form gatt server.
 * @param[in]   env                  JNI interface pointer.
 * @param[in]   bluetoothDevice      bluetooth device object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerDisconnect(JNIEnv *env, jobject bluetoothDevice);

/**
 * create connected device list.
 */
void CALEServerCreateCachedDeviceList();

/**
 * check whether the device exist in the list or not.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   remoteAddress         remote address.
 * @return  true or false.
 */
bool CALEServerIsDeviceInList(JNIEnv *env, const char* remoteAddress);

/**
 * add device object to the list (connected device list).
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   device                bluetooth device object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerAddDeviceToList(JNIEnv *env, jobject device);

/**
 * remove all devices objects in the list (connected device list).
 * @param[in]   env                   JNI interface pointer.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerRemoveAllDevices(JNIEnv *env);

/**
 * remove target device in the list (connected device list).
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   address               target address to remove.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerRemoveDevice(JNIEnv *env, jstring address);

/**
 * initialize mutex.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerInitMutexVaraibles();

/**
 * terminate mutex.
 */
void CALEServerTerminateMutexVaraibles();

/**
 * initialize condition.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEServerInitConditionVaraibles();

/**
 * terminate condition.
 */
void CALEServerTerminateConditionVaraibles();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_LESERVER_H_ */
