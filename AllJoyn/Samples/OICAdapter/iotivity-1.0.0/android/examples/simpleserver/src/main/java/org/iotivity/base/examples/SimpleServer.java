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
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

import java.util.LinkedList;
import java.util.List;

/**
 * SimpleServer
 * <p/>
 * SimpleServer is a sample OIC server application.
 * It creates a Light and waits for the incoming client calls to handle
 * various request scenarios.
 */
public class SimpleServer extends Activity {

    List<Light> lights = new LinkedList<>();

    /**
     * A local method to configure and initialize platform, and then create a light resource.
     */
    private void startSimpleServer() {
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

        createNewLightResource("/a/light", "John's light");

        msg("Waiting for the requests...");
        printLine();

        enableStartStopButton();
    }

    public void createNewLightResource(String resourceUri, String resourceName){
        msg("Creating a light");
        Light light = new Light(
                resourceUri,     //URI
                resourceName,    //name
                false,           //state
                0                //power
        );
        msg(light.toString());
        light.setContext(this);

        msg("Registering light as a resource");
        try {
            light.registerResource();
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to register a light resource");
        }
        lights.add(light);
    }

    private void stopSimpleServer() {
        for (Light light : lights) {
            try {
                light.unregisterResource();
            } catch (OcException e) {
                Log.e(TAG, e.toString());
                msg("Failed to unregister a light resource");
            }
        }
        lights.clear();

        msg("All created resources have been unregistered");
        printLine();
        enableStartStopButton();
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************

    private final static String TAG = SimpleServer.class.getSimpleName();
    private MessageReceiver mMessageReceiver = new MessageReceiver();
    private TextView mConsoleTextView;
    private ScrollView mScrollView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_simple_server);

        registerReceiver(mMessageReceiver,
                new IntentFilter("org.iotivity.base.examples.simpleserver"));

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
                                startSimpleServer();
                            }
                        }).start();
                    } else {
                        new Thread(new Runnable() {
                            public void run() {
                                stopSimpleServer();
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
    public void onDestroy() {
        super.onDestroy();
        onStop();
    }

    @Override
    protected void onStop() {
        //unregisterReceiver(mMessageReceiver);
        super.onStop();
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

    public class MessageReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String message = intent.getStringExtra("message");
            msg(message);
        }
    }
}
