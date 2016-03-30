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
#ifndef CA_LECLIENT_H_
#define CA_LECLIENT_H_

#include "cacommon.h"
#include "cathreadpool.h"
#include "uarraylist.h"
#include "jni.h"

#ifdef __cplusplus
extern "C"
{
#endif

static const uint16_t STATE_CHARACTER_SET = 2;
static const uint16_t STATE_CHARACTER_UNSET = 1;
static const uint16_t STATE_CHARACTER_NO_CHANGE = 0;

static const uint16_t STATE_SEND_NONE = 0;
static const uint16_t STATE_SEND_SUCCESS = 1;
static const uint16_t STATE_SEND_FAILED = 2;

static const jint STATE_CONNECTED = 2;
static const jint STATE_DISCONNECTED = 0;

typedef struct le_state_info
{
    char address[CA_MACADDR_SIZE];
    jint connectedState;
    uint16_t notificationState;
    uint16_t sendState;
} CALEState_t;

/**
 * Callback to be notified on reception of any data from remote devices.
 * @param[in]  address                MAC address of remote device.
 * @param[in]  data                   Data received from remote device.
 * @pre  Callback must be registered using CALESetCallback(CAPacketReceiveCallback callback)
 */
typedef void (*CAPacketReceiveCallback)(const char *address, const uint8_t *data);

/**
 * initialize JNI object.
 */
void CALEClientJniInit();

/**
 * set context of application.
 */
void CALEClientJNISetContext();

/**
 * create interface object and initialize the object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientCreateJniInterfaceObject();

/**
 * initialize client for BLE.
 * @param[in]   handle                thread pool handle object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientInitialize(ca_thread_pool_t handle);

/**
 * terminate client for BLE.
 */
void CALEClientTerminate();

/**
 * for destroy sending routine.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   gatt                  Gatt profile object.
 */
void CALEClientSendFinish(JNIEnv *env, jobject gatt);

/**
 * send data for unicast (interface).
 * @param[in]   address               remote address.
 * @param[in]   data                  data for transmission.
 * @param[in]   dataLen               data length.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientSendUnicastMessage(const char *address, const uint8_t *data,
                                        const uint32_t dataLen);

/**
 * send data for multicast (interface).
 * @param[in]   data                  data for transmission.
 * @param[in]   dataLen               data length.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientSendMulticastMessage(const uint8_t *data, const uint32_t dataLen);

/**
 * start unicast server.
 * @param[in]   address               remote address.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientStartUnicastServer(const char *address);

/**
 * start multicast server (start discovery).
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientStartMulticastServer();

/**
 * stop unicast server.
 */
void CALEClientStopUnicastServer();

/**
 * stop multicast server (stop discovery).
 */
void CALEClientStopMulticastServer();

/**
 * set this callback for receiving data packets from peer devices.
 * @param[in]   callback              callback to be notified on reception of
 *                                    unicast/multicast data packets.
 */
void CALEClientSetCallback(CAPacketReceiveCallback callback);

/**
 * send data for unicast (implement).
 * @param[in]   address               remote address.
 * @param[in]   data                  data for transmission.
 * @param[in]   dataLen               data length.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientSendUnicastMessageImpl(const char *address, const uint8_t *data,
                                            const uint32_t dataLen);

/**
 * send data for multicast (implement).
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   data                  data for transmission.
 * @param[in]   dataLen               data length.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientSendMulticastMessageImpl(JNIEnv *env, const uint8_t *data,
                                              const uint32_t dataLen);

/**
 * check whether it is connected or not with remote address.
 * @param[in]   address               remote address.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALECheckSendState(const char* address);

/**
 * send data to remote device.
 * if it isn't connected yet. connect LE before try to send data.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   device                bluetooth device object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientSendData(JNIEnv *env, jobject device);

/**
 * get address from bluetooth gatt object.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   gatt                  Gatt profile object.
 * @return  bluetooth address.
 */
jstring CALEClientGetAddressFromGattObj(JNIEnv *env, jobject gatt);

/**
 * get remote address from bluetooth socket object.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   bluetoothSocketObj    bluetooth socket.
 * @return  bluetooth address.
 */
jstring CALEClientGetRemoteAddress(JNIEnv *env, jobject bluetoothSocketObj);

/**
 * close gatt.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   bluetoothGatt         gatt profile object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientGattClose(JNIEnv *env, jobject bluetoothGatt);

/**
 * start to scan whole bluetooth devices (interface).
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientStartScan();

/**
 * start to scan whole bluetooth devices (implement).
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   callback              callback to receive device object by scanning.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientStartScanImpl(JNIEnv *env, jobject callback);

/**
 * start to scan target bluetooth devices for service uuid (implement).
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   uuids                 target UUID.
 * @param[in]   callback              callback to receive device object by scanning.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientStartScanWithUUIDImpl(JNIEnv *env, jobjectArray uuids,
                                           jobject callback);

/**
 * get uuid object.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   uuid                  uuid.
 * @return  uuid object.
 */
jobject CALEClientGetUUIDObject(JNIEnv *env, const char *uuid);

/**
 * stop scan (interface).
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientStopScan();

/**
 * set ble scanning flag.
 * @param[in]   flag        scan flag.
 */
void CALEClientSetScanFlag(bool flag);

/**
 * stop scan (implement).
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   callback              callback to receive device object by scanning.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientStopScanImpl(JNIEnv *env, jobject callback);

/**
 * connect to gatt server hosted.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   bluetoothDevice       bluetooth Device object.
 * @param[in]   autoconnect           whether to directly connect to the remote device(false) or
 *                                     to automatically connect as soon as the remote device
 *                                     becomes available.
 * @param[in]   callback              callback for connection state change.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientConnect(JNIEnv *env, jobject bluetoothDevice, jboolean autoconnect,
                             jobject callback);

/**
 * disconnect to gatt server by a target device.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   bluetoothGatt         Gatt profile object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientDisconnect(JNIEnv *env, jobject bluetoothGatt);

/**
 * disconnect to gatt server by whole devices.
 * @param[in]   env                   JNI interface pointer.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientDisconnectAll(JNIEnv *env);

/**
 * start discovery server.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   bluetoothGatt         Gatt profile object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientDiscoverServices(JNIEnv *env, jobject bluetoothGatt);

/**
 * create GattCharacteristic and call CALEClientWriteCharacteristicImpl
 * for request to write gatt characteristic.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   gatt                  Gatt profile object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientWriteCharacteristic(JNIEnv *env, jobject gatt);

/**
 * request to write gatt characteristic.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   bluetoothGatt         Gatt profile object.
 * @param[in]   gattCharacteristic    characteristic object that contain data to send.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientWriteCharacteristicImpl(JNIEnv *env, jobject bluetoothGatt,
                                             jobject gattCharacteristic);

/**
 * request to read gatt characteristic.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   bluetoothGatt         Gatt profile object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientReadCharacteristic(JNIEnv *env, jobject bluetoothGatt);

/**
 * enable notification for a target device.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   bluetoothGatt         Gatt profile object.
 * @param[in]   characteristic        Characteristic object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientSetCharacteristicNotification(JNIEnv *env, jobject bluetoothGatt,
                                                  jobject characteristic);

/**
 * create gatt characteristic object.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   bluetoothGatt         Gatt profile object.
 * @param[in]   data                  for make Characteristic with data.
 * @return  Gatt Characteristic object.
 */
jobject CALEClientCreateGattCharacteristic(JNIEnv *env, jobject bluetoothGatt, jbyteArray data);

/**
 * get gatt service.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   bluetoothGatt         Gatt profile object.
 * @param[in]   characterUUID         for make BluetoothGattCharacteristic object.
 * @return  Gatt Service.
 */
jobject CALEClientGetGattService(JNIEnv *env, jobject bluetoothGatt, jstring characterUUID);

/**
 * get value from characteristic.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   characteristic        Characteristic object.
 * @return  value in characteristic.
 */
jbyteArray CALEClientGetValueFromCharacteristic(JNIEnv *env, jobject characteristic);

/**
 * create UUID List.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientCreateUUIDList();

/**
 * set UUID to descriptor.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   bluetoothGatt         Gatt profile object.
 * @param[in]   characteristic        Characteristic object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientSetUUIDToDescriptor(JNIEnv *env, jobject bluetoothGatt,
                                         jobject characteristic);

/**
 * add device object to scan device list.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   device                bluetooth device object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientAddScanDeviceToList(JNIEnv *env, jobject device);

/**
 * check whether the device exist in list or not.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   remoteAddress         remote address.
 * @return  true or false.
 */
bool CALEClientIsDeviceInScanDeviceList(JNIEnv *env, const char *remoteAddress);

/**
 * remove all devices in scan device list.
 * @param[in]   env                   JNI interface pointer.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientRemoveAllScanDevices(JNIEnv *env);

/**
 * remove target device in scan device list.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   remoteAddress         remote address.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientRemoveDeviceInScanDeviceList(JNIEnv *env, jstring remoteAddress);

/**
 * add gatt object to gatt object list.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   gatt                  Gatt profile object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientAddGattobjToList(JNIEnv *env, jobject gatt);

/**
 * check whether the gatt object exist in list or not.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   remoteAddress         remote address.
 * @return  true or false.
 */
bool CALEClientIsGattObjInList(JNIEnv *env, const char *remoteAddress);

/**
 * get the gatt object.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   remoteAddress         remote address.
 * @return  gatt object.
 */
jobject CALEClientGetGattObjInList(JNIEnv *env, const char* remoteAddress);

/**
 * remove all gatt objects in gatt object list.
 * @param[in]   env                   JNI interface pointer.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientRemoveAllGattObjs(JNIEnv *env);

/**
 * remove target device in gatt object list.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   gatt                  Gatt profile object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientRemoveGattObj(JNIEnv *env, jobject gatt);

/**
 * remove gatt object of target device for address in gatt object list.
 * @param[in]   env                   JNI interface pointer.
 * @param[in]   gatt                  Gatt profile object.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientRemoveGattObjForAddr(JNIEnv *env, jstring addr);

/**
 * update new state information.
 * @param[in]   address               remote address.
 * @param[in]   connectedState        connection state.
 * @param[in]   notificationState     whether characteristic notification already set or not.
 * @param[in]   sendState             whether sending was success or not.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientUpdateDeviceState(const char* address, uint32_t connectedState,
                                       uint16_t notificationState, uint16_t sendState);

/**
 * add new state to state list.
 * @param[in]   state                 new state.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientAddDeviceStateToList(CALEState_t* state);

/**
 * check whether the remote address is existed or not.
 * @param[in]   address               remote address.
 * @return  true or false.
 */
bool CALEClientIsDeviceInList(const char *remoteAddress);

/**
 * remove all device states.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientRemoveAllDeviceState();

/**
 * remove the device state for a remote device.
 * @param[in]   remoteAddress         remote address.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientRemoveDeviceState(const char* remoteAddress);

/**
 * get state information for a remote device.
 * @param[in]   remoteAddress         remote address.
 * @return  CALEState_t.
 */
CALEState_t* CALEClientGetStateInfo(const char* remoteAddress);

/**
 * check whether the remote address is connected or not.
 * @param[in]   remoteAddress         remote address.
 * @return  true or false.
 */
bool CALEClientIsConnectedDevice(const char* remoteAddress);

/**
 * check whether the remote address set CharacteristicNotification or not.
 * @param[in]   remoteAddress         remote address.
 * @return  true or false.
 */
bool CALEClientIsSetCharacteristic(const char* remoteAddress);

/**
 * create scan device list.
 */
void CALEClientCreateDeviceList();

/**
 * update the counter which data is sent to remote device.
 * @param[in]   env                   JNI interface pointer.
 */
void CALEClientUpdateSendCnt(JNIEnv *env);

/**
 * initialize mutex.
 * @return  ::CA_STATUS_OK or ERROR CODES (::CAResult_t error codes in cacommon.h).
 */
CAResult_t CALEClientInitGattMutexVaraibles();

/**
 * terminate mutex.
 */
void CALEClientTerminateGattMutexVariables();

/**
 * set send finish flag.
 * @param[in]   flag        finish flag.
 */
void CALEClientSetSendFinishFlag(bool flag);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CA_LECLIENT_H_ */
