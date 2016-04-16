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
import android.widget.CompoundButton;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.ToggleButton;

import org.iotivity.base.ModeType;
import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcResourceHandle;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ResourceProperty;
import org.iotivity.base.ServiceType;

import java.util.EnumSet;

/**
 * A server example for presence notification
 */
public class PresenceServer extends Activity {
    private OcResourceHandle mResourceHandle;

    private void startPresenceServer() {
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

        try {
            msg("Creating resource of type \"core.light\".");
            createResource();
            sleep(1);

            msg("Starting presence notifications.");
            OcPlatform.startPresence(OcPlatform.DEFAULT_PRESENCE_TTL);
            sleep(1);
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Error: " + e.toString());
        }
        printLine();
        enableStartStopButton();
    }

    /**
     * This function internally calls registerResource API.
     */
    private void createResource() {
        String resourceUri = "/a/light"; // URI of the resource
        String resourceTypeName = "core.light"; // resource type name.
        String resourceInterface = OcPlatform.DEFAULT_INTERFACE; // resource interface.

        try {
            // This will internally create and register the resource.
            mResourceHandle = OcPlatform.registerResource(
                    resourceUri,
                    resourceTypeName,
                    resourceInterface,
                    null, //Use default entity handler
                    EnumSet.of(ResourceProperty.DISCOVERABLE));
        } catch (OcException e) {
            msg("Resource creation was unsuccessful.");
            Log.e(TAG, e.toString());
        }
    }

    private void stopPresenceServer() {
        try {
            msg("Stopping presence notifications.");
            OcPlatform.stopPresence();

            msg("Unregister resource.");
            if (null != mResourceHandle) OcPlatform.unregisterResource(mResourceHandle);
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Error: " + e.toString());
        }

        printLine();
        enableStartStopButton();
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************

    private final static String TAG = PresenceServer.class.getSimpleName();
    private TextView mConsoleTextView;
    private ScrollView mScrollView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_presence_server);

        mConsoleTextView = (TextView) findViewById(R.id.consoleTextView);
        mConsoleTextView.setMovementMethod(new ScrollingMovementMethod());
        mScrollView = (ScrollView) findViewById(R.id.scrollView);
        mScrollView.fullScroll(View.FOCUS_DOWN);
        final ToggleButton toggleButton = (ToggleButton) findViewById(R.id.toggleButton);

        if (null == savedInstanceState) {
            toggleButton.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
                public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                    toggleButton.setEnabled(false);
                    if (isChecked) {
                        new Thread(new Runnable() {
                            public void run() {
                                startPresenceServer();
                            }
                        }).start();
                    } else {
                        new Thread(new Runnable() {
                            public void run() {
                                stopPresenceServer();
                            }
                        }).start();
                    }
                }
            });
        } else {
            String consoleOutput = savedInstanceState.getString("consoleOutputString");
            mConsoleTextView.setText(consoleOutput);
            boolean buttonCheked = savedInstanceState.getBoolean("toggleButtonChecked");
            toggleButton.setChecked(buttonCheked);
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putString("consoleOutputString", mConsoleTextView.getText().toString());
        ToggleButton toggleButton = (ToggleButton) findViewById(R.id.toggleButton);
        outState.putBoolean("toggleButtonChecked", toggleButton.isChecked());
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);

        String consoleOutput = savedInstanceState.getString("consoleOutputString");
        mConsoleTextView.setText(consoleOutput);

        final ToggleButton toggleButton = (ToggleButton) findViewById(R.id.toggleButton);
        boolean buttonCheked = savedInstanceState.getBoolean("toggleButtonChecked");
        toggleButton.setChecked(buttonCheked);
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

    private void sleep(int seconds) {
        try {
            Thread.sleep(seconds * 1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
            Log.e(TAG, e.toString());
        }
    }

    private void enableStartStopButton() {
        runOnUiThread(new Runnable() {
            public void run() {
                ToggleButton toggleButton = (ToggleButton) findViewById(R.id.toggleButton);
                toggleButton.setEnabled(true);
            }
        });
    }
}