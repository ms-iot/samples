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

import org.iotivity.base.ErrorCode;
import org.iotivity.base.ModeType;
import org.iotivity.base.OcConnectivityType;
import org.iotivity.base.OcException;
import org.iotivity.base.OcHeaderOption;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResource;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ServiceType;

import java.util.EnumSet;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * GroupClient
 */
public class GroupClient extends Activity implements
        OcPlatform.OnResourceFoundListener,
        OcResource.OnGetListener {

    private OcResource mFoundCollectionResource;

    /**
     * A local method to configure and initialize platform and then search for the collection
     * resources
     */
    private synchronized void startGroupClient() {
        Context context = this;

        PlatformConfig platformConfig = new PlatformConfig(
                context,
                ServiceType.IN_PROC,
                ModeType.CLIENT,
                "0.0.0.0", 0,
                QualityOfService.LOW
        );

        msg("Configuring platform.");
        OcPlatform.Configure(platformConfig);

        msg("Find all resources of type \"a.collection\".");
        try {
            OcPlatform.findResource(
                    "",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=a.collection",
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    this
            );
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to invoke find resource API");
        }

        printLine();
    }

    /**
     * An event handler to be executed whenever a "findResource" request completes successfully
     *
     * @param foundResource found resource
     */
    @Override
    public synchronized void onResourceFound(OcResource foundResource) {
        if (null == foundResource) {
            msg("Found resource is invalid");
            return;
        }
        if (null != mFoundCollectionResource) {
            msg("Found another resource, ignoring");
            return;
        }

        // Get the resource URI
        String resourceUri = foundResource.getUri();
        // Get the resource host address
        String hostAddress = foundResource.getHost();
        msg("\tURI of the resource: " + resourceUri);
        msg("\tHost address of the resource: " + hostAddress);
        // Get the resource types
        msg("\tList of resource types: ");
        for (String resourceType : foundResource.getResourceTypes()) {
            msg("\t\t" + resourceType);
        }
        msg("\tList of resource interfaces:");
        for (String resourceInterface : foundResource.getResourceInterfaces()) {
            msg("\t\t" + resourceInterface);
        }
        msg("\tList of resource connectivity types:");
        for (OcConnectivityType connectivityType : foundResource.getConnectivityTypeSet()) {
            msg("\t\t" + connectivityType);
        }

        if (resourceUri.equals("/core/a/collection")) {
            mFoundCollectionResource = foundResource;

            msg("Getting representation of a collection resource...");

            Map<String, String> queryParams = new HashMap<>();
            try {
                mFoundCollectionResource.get(
                        "",
                        OcPlatform.DEFAULT_INTERFACE,
                        queryParams,
                        this
                );
            } catch (OcException e) {
                Log.e(TAG, e.toString());
                msg("Error occurred while invoking \"get\" API");
            }
        }

        printLine();
        enableStartStopButton();
    }

    /**
     * An event handler to be executed whenever a "get" request completes successfully
     *
     * @param list           list of the header options
     * @param representation representation of a resource
     */
    @Override
    public void onGetCompleted(List<OcHeaderOption> list, OcRepresentation representation) {
        msg("Representation of a light collection resource:");
        for (OcRepresentation childRepresentation : representation.getChildren()) {
            msg("\t\tURI: " + childRepresentation.getUri());
        }
    }

    /**
     * An event handler to be executed whenever a "get" request fails
     *
     * @param throwable exception
     */
    @Override
    public synchronized void onGetFailed(Throwable throwable) {
        if (throwable instanceof OcException) {
            OcException ocEx = (OcException) throwable;
            Log.e(TAG, ocEx.toString());
            ErrorCode errCode = ocEx.getErrorCode();
            //do something based on errorCode
            msg("Error code: " + errCode);
        }
        msg("Failed to get representation of a found collection resource");
    }

    /**
     * A local method to reset group client
     */
    private synchronized void stopGroupClient() {
        mFoundCollectionResource = null;

        msg("Group Client is reset.");
        printLine();

        enableStartStopButton();
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************

    private final static String TAG = GroupClient.class.getSimpleName();
    private TextView mConsoleTextView;
    private ScrollView mScrollView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_group_client);

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
                                startGroupClient();
                            }
                        }).start();
                    } else {
                        new Thread(new Runnable() {
                            public void run() {
                                stopGroupClient();
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
