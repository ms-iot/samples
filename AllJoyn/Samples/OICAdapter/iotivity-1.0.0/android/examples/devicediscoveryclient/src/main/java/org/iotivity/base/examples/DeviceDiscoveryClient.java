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
import org.iotivity.base.OcConnectivityType;
import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

import java.util.EnumSet;
import java.util.HashMap;
import java.util.Map;

/**
 * This sample demonstrates the device discovery feature.
 * The client queries for the device related information stored by the server.
 */
public class DeviceDiscoveryClient extends Activity implements
        OcPlatform.OnDeviceFoundListener,
        OcPlatform.OnPlatformFoundListener {
    private void startDeviceDiscoveryClient() {
        Context context = this;

        PlatformConfig platformConfig = new PlatformConfig(
                context,
                ServiceType.IN_PROC,
                ModeType.CLIENT,
                "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
                0,         // Uses randomly available port
                QualityOfService.LOW
        );

        msg("Configuring platform.");
        OcPlatform.Configure(platformConfig);
        sleep(1);

        try {
            msg("Querying for platform information...");
            OcPlatform.getPlatformInfo("",
                    OcPlatform.WELL_KNOWN_PLATFORM_QUERY,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    this);
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to query for platform information");
        }
        sleep(2);

        try {
            msg("Querying for device information...");
            OcPlatform.getDeviceInfo("",
                    OcPlatform.WELL_KNOWN_DEVICE_QUERY,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    this);
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to query for device information");
        }
        sleep(2);

        enableStartButton();
        printLine();
    }

    private final static Map<String, String> PLATFORM_INFO_KEYS = new HashMap<String, String>() {{
        put("pi", "Platform ID: ");
        put("mnmn", "Manufacturer name: ");
        put("mnml", "Manufacturer url: ");
        put("mnmo", "Manufacturer Model No: ");
        put("mndt", "Manufactured Date: ");
        put("mnpv", "Manufacturer Platform Version: ");
        put("mnos", "Manufacturer OS version: ");
        put("mnhw", "Manufacturer hardware version: ");
        put("mnfv", "Manufacturer firmware version: ");
        put("mnsl", "Manufacturer support url: ");
        put("st", "Manufacturer system time: ");
    }};

    @Override
    public synchronized void onPlatformFound(OcRepresentation ocRepresentation) {
        msg("Platform Information received:");
        try {
            for (String key : PLATFORM_INFO_KEYS.keySet()) {
                msg("\t" + PLATFORM_INFO_KEYS.get(key) + ocRepresentation.getValue(key));
            }
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to read platform info values.");
        }

        printLine();
    }

    private final static Map<String, String> DEVICE_INFO_KEYS = new HashMap<String, String>() {{
        put("di", "Device ID: ");
        put("n", "Device name: ");
        put("lcv", "Spec version url: ");
        put("dmv", "Data Model: ");
    }};

    @Override
    public synchronized void onDeviceFound(OcRepresentation ocRepresentation) {
        msg("Device Information received:");
        try {
            for (String key : DEVICE_INFO_KEYS.keySet()) {
                msg("\t" + DEVICE_INFO_KEYS.get(key) + ocRepresentation.getValue(key));
            }
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to read device info values.");
        }

        printLine();
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************

    private final static String TAG = DeviceDiscoveryClient.class.getSimpleName();
    private TextView mConsoleTextView;
    private ScrollView mScrollView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_device_discovery_client);

        mConsoleTextView = (TextView) findViewById(R.id.consoleTextView);
        mConsoleTextView.setMovementMethod(new ScrollingMovementMethod());
        mScrollView = (ScrollView) findViewById(R.id.scrollView);
        mScrollView.fullScroll(View.FOCUS_DOWN);
        final Button button = (Button) findViewById(R.id.button);

        if (null == savedInstanceState) {
            button.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    button.setText("Re-start");
                    button.setEnabled(false);
                    new Thread(new Runnable() {
                        public void run() {
                            startDeviceDiscoveryClient();
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

    private void enableStartButton() {
        runOnUiThread(new Runnable() {
            public void run() {
                Button button = (Button) findViewById(R.id.button);
                button.setEnabled(true);
            }
        });
    }

    private void sleep(int seconds) {
        try {
            Thread.sleep(seconds * 1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
            Log.e(TAG, e.toString());
        }
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
