/*
 * //******************************************************************
 * //
 * // Copyright 2015 Intel Corporation.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //      http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */
package org.iotivity.base.examples;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ScrollView;
import android.widget.TextView;

import org.iotivity.base.ModeType;
import org.iotivity.base.OcDeviceInfo;
import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcPlatformInfo;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

/**
 * This sample demonstrates platform and device discovery feature.
 * The server sets the platform and device related info. which can be later retrieved by a client.
 */
public class DeviceDiscoveryServer extends Activity {

    private void startDeviceDiscoveryServer() {
        Context context = this;

        PlatformConfig platformConfig = new PlatformConfig(
                context,
                ServiceType.IN_PROC,
                ModeType.SERVER,
                "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
                0,         // Uses randomly available port
                QualityOfService.LOW
        );

        msg("Configuring platform.");
        OcPlatform.Configure(platformConfig);

        OcDeviceInfo deviceInfo = new OcDeviceInfo("myDeviceName");
        try {
            msg("Registering device info");
            OcPlatform.registerDeviceInfo(deviceInfo);
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to register device info.");
        }

        OcPlatformInfo platformInfo = new OcPlatformInfo(
                "myPlatformId",             //Platform ID
                "myManufactName",           //Manufacturer Name
                "www.myurl.com",            //Manufacturer URL
                "myModelNumber",            //Model Number
                "myDateOfManufacture",      //Date of Manufacture
                "myPlatformVersion",        //Platform Version
                "Manufacturer OS version",  //Operating System Version
                "myHardwareVersion",        //Hardware Version
                "myFirmwareVersion",        //Firmware Version
                "www.mysupporturl.com",     //Support URL
                String.valueOf(System.currentTimeMillis()) // System Time
        );
        try {
            msg("Registering platform info");
            OcPlatform.registerPlatformInfo(platformInfo);
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to register platform info.");
        }

        msg("Waiting for the requests...");
        printLine();
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************

    private final static String TAG = DeviceDiscoveryServer.class.getSimpleName();
    private TextView mConsoleTextView;
    private ScrollView mScrollView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device_discovery_server);

        mConsoleTextView = (TextView) findViewById(R.id.consoleTextView);
        mConsoleTextView.setMovementMethod(new ScrollingMovementMethod());
        mScrollView = (ScrollView) findViewById(R.id.scrollView);
        mScrollView.fullScroll(View.FOCUS_DOWN);
        final Button button = (Button) findViewById(R.id.button);

        if (null == savedInstanceState) {
            button.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    button.setEnabled(false);
                    new Thread(new Runnable() {
                        public void run() {
                            startDeviceDiscoveryServer();
                        }
                    }).start();
                }
            });
        } else {
            String consoleOutput = savedInstanceState.getString("consoleOutputString");
            mConsoleTextView.setText(consoleOutput);
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putString("consoleOutputString", mConsoleTextView.getText().toString());
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);

        String consoleOutput = savedInstanceState.getString("consoleOutputString");
        mConsoleTextView.setText(consoleOutput);
    }

    private void msg(final String text) {
        runOnUiThread(new Runnable() {
            public void run() {
                mConsoleTextView.append("\n");
                mConsoleTextView.append(text);
                mScrollView.fullScroll(View.FOCUS_DOWN);
            }
        });
        Log.i(TAG, text);
    }

    private void printLine() {
        msg("------------------------------------------------------------------------");
    }
}
