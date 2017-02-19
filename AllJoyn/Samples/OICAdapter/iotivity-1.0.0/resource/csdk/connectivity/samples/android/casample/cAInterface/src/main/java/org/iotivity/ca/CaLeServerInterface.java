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

package org.iotivity.ca;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseSettings;

public class CaLeServerInterface {

    private CaLeServerInterface() {

        caLeRegisterGattServerCallback(mGattServerCallback);
        caLeRegisterBluetoothLeAdvertiseCallback(mAdvertiseCallback);
    }

    public static void getLeGattServerCallback() {
        caLeRegisterGattServerCallback(mGattServerCallback);
    }

    public static void getBluetoothLeAdvertiseCallback() {
        caLeRegisterBluetoothLeAdvertiseCallback(mAdvertiseCallback);
    }

    private native static void caLeRegisterGattServerCallback(BluetoothGattServerCallback callback);

    private native static void caLeRegisterBluetoothLeAdvertiseCallback(AdvertiseCallback callback);

    // BluetoothGattServerCallback
    private native static void caLeGattServerConnectionStateChangeCallback(
            BluetoothDevice device, int status, int newState);

    private native static void caLeGattServerServiceAddedCallback(int status,
                                                                  BluetoothGattService service);

    private native static void caLeGattServerCharacteristicReadRequestCallback(
            BluetoothDevice device, byte[] data);

    private native static void caLeGattServerCharacteristicWriteRequestCallback(
            BluetoothDevice device, byte[] data);

    private native static void caLeGattServerNotificationSentCallback(BluetoothDevice device,
                                                                     int status);

    // AdvertiseCallback
    private native static void caLeAdvertiseStartSuccessCallback(
            AdvertiseSettings settingsInEffect);

    private native static void caLeAdvertiseStartFailureCallback(int errorCode);

    private static final BluetoothGattServerCallback mGattServerCallback =
                         new BluetoothGattServerCallback() {

        @Override
        public void onConnectionStateChange(BluetoothDevice device, int status,
                int newState) {
            super.onConnectionStateChange(device, status, newState);

            caLeGattServerConnectionStateChangeCallback(device, status, newState);
        }

        @Override
        public void onServiceAdded(int status, BluetoothGattService service) {
            super.onServiceAdded(status, service);

            caLeGattServerServiceAddedCallback(status, service);
        }

        @Override
        public void onCharacteristicReadRequest(
                BluetoothDevice device, int requestId, int offset,
                BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicReadRequest(device, requestId, offset, characteristic);

            caLeGattServerCharacteristicReadRequestCallback(device, characteristic.getValue());
        }

        @Override
        public void onCharacteristicWriteRequest(
                BluetoothDevice device, int requestId, BluetoothGattCharacteristic characteristic,
                boolean preparedWrite, boolean responseNeeded, int offset, byte[] value) {
            super.onCharacteristicWriteRequest(device, requestId, characteristic,
                    preparedWrite, responseNeeded, offset, value);

            caLeGattServerCharacteristicWriteRequestCallback(device, value);
        }

        @Override
        public void onDescriptorReadRequest(
                BluetoothDevice device,
                int requestId, int offset, BluetoothGattDescriptor descriptor) {
            super.onDescriptorReadRequest(device, requestId, offset, descriptor);
        }

        @Override
        public void onDescriptorWriteRequest(
                BluetoothDevice device, int requestId, BluetoothGattDescriptor descriptor,
                boolean preparedWrite, boolean responseNeeded, int offset,
                byte[] value) {
            super.onDescriptorWriteRequest(device, requestId, descriptor, preparedWrite,
                                           responseNeeded, offset, value);
        }

        @Override
        public void onExecuteWrite(BluetoothDevice device, int requestId, boolean execute) {
            super.onExecuteWrite(device, requestId, execute);
        }

        @Override
        public void onNotificationSent(BluetoothDevice device, int status) {
            super.onNotificationSent(device, status);

            caLeGattServerNotificationSentCallback(device, status);
        }
    };

    private static final AdvertiseCallback mAdvertiseCallback = new AdvertiseCallback() {

        @Override
        public void onStartSuccess(AdvertiseSettings settingsInEffect) {
            super.onStartSuccess(settingsInEffect);

            caLeAdvertiseStartSuccessCallback(settingsInEffect);
        }

        @Override
        public void onStartFailure(int errorCode) {
            super.onStartFailure(errorCode);

            caLeAdvertiseStartFailureCallback(errorCode);
        }
    };
}
