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
/* Header for class org_iotivity_ca_caLeServerInterface */

#ifndef CA_Included_org_iotivity_ca_caLeServerInterface_H_
#define CA_Included_org_iotivity_ca_caLeServerInterface_H_
#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Class:     org_iotivity_ca_caLeServerInterface
 * Method:    caLeRegisterGattServerCallback
 * Signature: (Landroid/bluetooth/BluetoothGattServerCallback;)V
 */
JNIEXPORT void JNICALL

Java_org_iotivity_ca_caLeServerInterface_caLeRegisterGattServerCallback
(JNIEnv *, jobject, jobject);

/*
 * Class:     org_iotivity_ca_caLeServerInterface
 * Method:    caLeRegisterBluetoothLeAdvertiseCallback
 * Signature: (Landroid/bluetooth/le/AdvertiseCallback;)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_caLeServerInterface_caLeRegisterBluetoothLeAdvertiseCallback
(JNIEnv *, jobject, jobject);

/*
 * Class:     org_iotivity_ca_caLeServerInterface
 * Method:    caLeGattServerConnectionStateChangeCallback
 * Signature: (Landroid/bluetooth/BluetoothDevice;II)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_caLeServerInterface_caLeGattServerConnectionStateChangeCallback
(JNIEnv *, jobject, jobject, jint, jint);

/*
 * Class:     org_iotivity_ca_caLeServerInterface
 * Method:    caLeGattServerServiceAddedCallback
 * Signature: (ILandroid/bluetooth/BluetoothGattService;)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_caLeServerInterface_caLeGattServerServiceAddedCallback
(JNIEnv *, jobject, jint, jobject);

/*
 * Class:     org_iotivity_ca_caLeServerInterface
 * Method:    caLeGattServerCharacteristicReadRequestCallback
 * Signature: (Landroid/bluetooth/BluetoothDevice;IILandroid/
 * bluetooth/BluetoothGattCharacteristic;)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_caLeServerInterface_caLeGattServerCharacteristicReadRequestCallback
(JNIEnv *, jobject, jobject, jbyteArray);

/*
 * Class:     org_iotivity_ca_caLeServerInterface
 * Method:    caLeGattServerCharacteristicWriteRequestCallback
 * Signature: (Landroid/bluetooth/BluetoothDevice;ILandroid/bluetooth/
 * BluetoothGattCharacteristic;ZZI[B)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_caLeServerInterface_caLeGattServerCharacteristicWriteRequestCallback
(JNIEnv *, jobject, jobject, jbyteArray);

/*
 * Class:     org_iotivity_ca_caLeServerInterface
 * Method:    caLeGattServerNotificationSentCallback
 * Signature: (Landroid/bluetooth/BluetoothDevice;I)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_caLeServerInterface_caLeGattServerNotificationSentCallback
(JNIEnv *, jobject, jobject, jint);

/*
 * Class:     org_iotivity_ca_caLeServerInterface
 * Method:    caLeAdvertiseStartSuccessCallback
 * Signature: (Landroid/bluetooth/le/AdvertiseSettings;)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_caLeServerInterface_caLeAdvertiseStartSuccessCallback
(JNIEnv *, jobject, jobject);

/*
 * Class:     org_iotivity_ca_caLeServerInterface
 * Method:    caLeAdvertiseStartFailureCallback
 * Signature: (I)V
 */
JNIEXPORT void JNICALL
Java_org_iotivity_ca_caLeServerInterface_caLeAdvertiseStartFailureCallback
(JNIEnv *, jobject, jint);


#ifdef __cplusplus
}
#endif
#endif

