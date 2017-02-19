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
import org.iotivity.base.OcConnectivityType;
import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcPresenceHandle;
import org.iotivity.base.OcPresenceStatus;
import org.iotivity.base.OcResource;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

import java.util.EnumSet;

/**
 * A client example for presence notification
 */
public class PresenceClient extends Activity implements
        OcPlatform.OnResourceFoundListener,
        OcPlatform.OnPresenceListener {
    private final static String TAG = PresenceClient.class.getSimpleName();
    private OcResource mResource;
    private OcPresenceHandle mPresenceHandle;
    private TextView mConsoleTextView;
    private ScrollView mScrollView;

    private void startPresenceClient() {
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

        try {
            msg("Finding Resource...");
            OcPlatform.findResource("", OcPlatform.WELL_KNOWN_QUERY,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT), this);
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to find resource(s). ");
        }
        printLine();
        enableStartStopButton();
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************

    @Override
    public synchronized void onResourceFound(OcResource foundResource) {
        String resourceUri = foundResource.getUri();
        if (null != mResource || !resourceUri.equals("/a/light")) {
            msg("Found resource, ignoring");
            return;
        }

        msg("Found Resource");
        String hostAddress = foundResource.getHost();
        msg("\tResource URI : " + resourceUri);
        msg("\tResource Host : " + hostAddress);
        msg("\tResource Interfaces : ");
        for (String resInterface : foundResource.getResourceInterfaces()) {
            msg("\t\t" + resInterface);
        }
        msg("\tResource Type : ");
        for (String resTypes : foundResource.getResourceTypes()) {
            msg("\t\t" + resTypes);
        }

        try {
            msg("Subscribing to unicast address:" + hostAddress);
            OcPlatform.subscribePresence(hostAddress,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT), this);
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to subscribe to unicast address:" + hostAddress);
        }
        mResource = foundResource;
        printLine();
    }

    @Override
    public void onPresence(OcPresenceStatus ocPresenceStatus, int nonce, String hostAddress) {
        msg("Received presence notification from : " + hostAddress);
        switch (ocPresenceStatus) {
            case OK:
                msg("Nonce# " + nonce);
                break;
            case STOPPED:
                msg("Presence Stopped");
                break;
            case TIMEOUT:
                msg("Presence Timeout");
                break;
            case DO_NOT_HANDLE:
                msg("Presence Do Not Handle");
                break;
        }
    }

    private void stopPresenceClient() {
        if (null != mPresenceHandle) {
            try {
                msg("Unsubscribing from the presence notifications.");
                OcPlatform.unsubscribePresence(mPresenceHandle);
                mPresenceHandle = null;
            } catch (OcException e) {
                Log.e(TAG, e.toString());
                msg("Failed to unsubscribe from the presence notifications" + e.toString());
            }
        }
        mResource = null;
        printLine();
        enableStartStopButton();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_presence_client);

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
                                startPresenceClient();
                            }
                        }).start();
                    } else {
                        new Thread(new Runnable() {
                            public void run() {
                                stopPresenceClient();
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

    private void enableStartStopButton() {
        runOnUiThread(new Runnable() {
            public void run() {
                ToggleButton toggleButton = (ToggleButton) findViewById(R.id.toggleButton);
                toggleButton.setEnabled(true);
            }
        });
    }
}