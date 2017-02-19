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

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

public class CaEdrInterface {

    private CaEdrInterface(Context context) {

        registerIntentFilter(context);
    }

    private static IntentFilter registerIntentFilter(Context context) {
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        context.registerReceiver(mReceiver, filter);
        return filter;
    }

    // Network Monitor
    private native static void caEdrStateChangedCallback(int state);

    private native static void caEdrBondStateChangedCallback(String addr);

    private static final BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {

            String action = intent.getAction();

            if (action != null && action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {

                int state =
                        intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);

                // STATE_ON:12, STATE_OFF:10
                if (state == BluetoothAdapter.STATE_ON || state == BluetoothAdapter.STATE_OFF)
                {
                    caEdrStateChangedCallback(state);
                }
            }

            if (action != null && action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED)) {

                int bondState = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE,
                                                   BluetoothDevice.ERROR);

                if (bondState == BluetoothDevice.BOND_NONE) {
                    if ((intent.getIntExtra(BluetoothDevice.EXTRA_PREVIOUS_BOND_STATE,
                                            BluetoothDevice.ERROR)
                                                == BluetoothDevice.BOND_BONDED)) {
                        BluetoothDevice device
                            = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

                        caEdrBondStateChangedCallback(device.getAddress());
                    }
                }
            }
        }
    };
}
