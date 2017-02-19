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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

public class CaLeClientInterface {

    private static String SERVICE_UUID = "ADE3D529-C784-4F63-A987-EB69F70EE816";
    private static String TAG          = "Sample_Service : CaLeClientInterface";

    private CaLeClientInterface(Context context) {

        caLeRegisterLeScanCallback(mLeScanCallback);
        caLeRegisterGattCallback(mGattCallback);

        registerIntentFilter(context);
    }

    public static void getLeScanCallback() {
        caLeRegisterLeScanCallback(mLeScanCallback);
    }

    public static void getLeGattCallback() {
        caLeRegisterGattCallback(mGattCallback);
    }

    private static IntentFilter registerIntentFilter(Context context) {
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        context.registerReceiver(mReceiver, filter);
        return filter;
    }

    private native static void caLeRegisterLeScanCallback(BluetoothAdapter.LeScanCallback callback);

    private native static void caLeRegisterGattCallback(BluetoothGattCallback callback);

    // BluetoothAdapter.LeScanCallback
    private native static void caLeScanCallback(BluetoothDevice device);

    // BluetoothGattCallback
    private native static void caLeGattConnectionStateChangeCallback(
            BluetoothGatt gatt, int status, int newState);

    private native static void caLeGattServicesDiscoveredCallback(BluetoothGatt gatt, int status);

    private native static void caLeGattCharacteristicWriteCallback(
            BluetoothGatt gatt, byte[] data, int status);

    private native static void caLeGattCharacteristicChangedCallback(
            BluetoothGatt gatt, byte[] data);

    private native static void caLeGattDescriptorWriteCallback(BluetoothGatt gatt, int status);

    private native static void caLeGattReliableWriteCompletedCallback(BluetoothGatt gatt,
                                                                     int status);

    private native static void caLeGattReadRemoteRssiCallback(BluetoothGatt gatt, int rssi,
                                                             int status);

    // Network Monitor
    private native static void caLeStateChangedCallback(int state);

    // bond state
    private native static void caLeBondStateChangedCallback(String address);

    // Callback
    private static BluetoothAdapter.LeScanCallback mLeScanCallback =
                   new BluetoothAdapter.LeScanCallback() {

        @Override
        public void onLeScan(BluetoothDevice device, int rssi, byte[] scanRecord) {

            try {
                List<UUID> uuids = getUuids(scanRecord);
                for (UUID uuid : uuids) {
                    Log.d(TAG, "UUID : " + uuid.toString());
                    if(uuid.toString().contains(SERVICE_UUID.toLowerCase())) {
                        Log.d(TAG, "we found that has the Device");
                        caLeScanCallback(device);
                    }
                }
            } catch(UnsatisfiedLinkError e) {

            }
        }
    };

    private static List<UUID> getUuids(final byte[] scanRecord) {
        List<UUID> uuids = new ArrayList<UUID>();

        int offset = 0;
        while (offset < (scanRecord.length - 2)) {
            int len = scanRecord[offset++];
            if (len == 0)
                break;

            int type = scanRecord[offset++];

            switch (type) {
            case 0x02:
            case 0x03:
                while (len > 1) {
                    int uuid16 = scanRecord[offset++];
                    uuid16 += (scanRecord[offset++] << 8);
                    len -= 2;
                    uuids.add(UUID.fromString(String.format(
                            "%08x-0000-1000-8000-00805f9b34fb", uuid16)));
                }
                break;
            case 0x06:
            case 0x07:
                while (len >= 16) {
                    try {
                        ByteBuffer buffer = ByteBuffer.wrap(scanRecord, offset++, 16).
                                                            order(ByteOrder.LITTLE_ENDIAN);
                        long mostSigBits = buffer.getLong();
                        long leastSigBits = buffer.getLong();
                        uuids.add(new UUID(leastSigBits, mostSigBits));
                    } catch (IndexOutOfBoundsException e) {
                        Log.e(TAG, e.toString());
                        continue;
                    } finally {
                        offset += 15;
                        len -= 16;
                    }
                }
                break;
            default:
                offset += (len - 1);
                break;
            }
        }
        return uuids;
    }

    private static final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {

        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            super.onConnectionStateChange(gatt, status, newState);

            caLeGattConnectionStateChangeCallback(gatt, status, newState);
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            super.onServicesDiscovered(gatt, status);

            caLeGattServicesDiscoveredCallback(gatt, status);
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicRead(gatt, characteristic, status);
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic, int status) {
            super.onCharacteristicWrite(gatt, characteristic, status);

            caLeGattCharacteristicWriteCallback(gatt, characteristic.getValue(), status);
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                BluetoothGattCharacteristic characteristic) {
            super.onCharacteristicChanged(gatt, characteristic);

            caLeGattCharacteristicChangedCallback(gatt, characteristic.getValue());
        }

        @Override
        public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor,
                int status) {
            super.onDescriptorRead(gatt, descriptor, status);
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor,
                int status) {
            super.onDescriptorWrite(gatt, descriptor, status);

            caLeGattDescriptorWriteCallback(gatt, status);
        }

        @Override
        public void onReliableWriteCompleted(BluetoothGatt gatt, int status) {
            super.onReliableWriteCompleted(gatt, status);
        }

        @Override
        public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
            super.onReadRemoteRssi(gatt, rssi, status);
        }
    };

    private static final BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {

            String action = intent.getAction();

            if (action != null && action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {

                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
                                               BluetoothAdapter.ERROR);

                if (state == BluetoothAdapter.STATE_ON || state == BluetoothAdapter.STATE_OFF)
                {
                    caLeStateChangedCallback(state);
                }
            }

            if (action != null && action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED)) {

                int bondState = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE,
                                                   BluetoothDevice.ERROR);

                if (bondState == BluetoothDevice.BOND_NONE) {
                    if ((intent.getIntExtra(BluetoothDevice.EXTRA_PREVIOUS_BOND_STATE,
                            BluetoothDevice.ERROR) == BluetoothDevice.BOND_BONDED)) {
                            BluetoothDevice device = intent
                                .getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

                        caLeBondStateChangedCallback(device.getAddress());
                    }
                }
            }
        }
    };
}


