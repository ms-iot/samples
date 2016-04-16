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
import org.iotivity.base.OcResource;
import org.iotivity.base.OcResourceHandle;
import org.iotivity.base.PlatformConfig;
import org.iotivity.base.QualityOfService;
import org.iotivity.base.ResourceProperty;
import org.iotivity.base.ServiceType;

import java.util.EnumSet;
import java.util.LinkedList;
import java.util.List;

/**
 * GroupServer
 */
public class GroupServer extends Activity implements OcPlatform.OnResourceFoundListener {

    private OcResourceHandle mCollectionResourceHandle;
    private List<OcResourceHandle> mProxyResourceHandleList = new LinkedList<>();

    /**
     * A local method to configure and initialize platform, register a collection resource
     * and then search for the light resources.In addition it creates a local light resource and
     * adds it to the collection.
     */
    private synchronized void startGroupServer() {
        Context context = this;

        PlatformConfig platformConfig = new PlatformConfig(
                context,
                ServiceType.IN_PROC,
                ModeType.CLIENT_SERVER,
                "0.0.0.0", 0,
                QualityOfService.LOW
        );

        msg("Configuring platform.");
        OcPlatform.Configure(platformConfig);

        String resourceUri = "/core/a/collection";
        String resourceTypeName = "a.collection";
        msg("Registering a collection resource.");
        try {
            mCollectionResourceHandle = OcPlatform.registerResource(
                    resourceUri,                //resource URI
                    resourceTypeName,           //resource type name
                    OcPlatform.BATCH_INTERFACE, //using batch interface
                    null,                       //use default entity handler
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to register a collection resource");
        }

        if (null != mCollectionResourceHandle) {
            try {
                OcPlatform.bindInterfaceToResource(
                        mCollectionResourceHandle,
                        OcPlatform.GROUP_INTERFACE);

                OcPlatform.bindInterfaceToResource(
                        mCollectionResourceHandle,
                        OcPlatform.DEFAULT_INTERFACE);
            } catch (OcException e) {
                e.printStackTrace();
            }

        }

        msg("Sending request to find all resources with \"core.light\" type name");
        try {
            OcPlatform.findResource(
                    "",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=core.light",
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    this);

        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to invoke find resource API");
        }

        OcResourceHandle localLightResourceHandle = null;
        msg("Registering a local light resource");
        try {
            localLightResourceHandle = OcPlatform.registerResource(
                    "/a/light/local",               //resource URI
                    "core.light",                   //resource type name
                    OcPlatform.DEFAULT_INTERFACE,   //using default interface
                    null,                           //use default entity handler
                    EnumSet.of(ResourceProperty.DISCOVERABLE)
            );
        } catch (OcException e) {
            Log.e(TAG, e.toString());
            msg("Failed to register a local ligh resource");
        }

        if (null != localLightResourceHandle) {
            msg("Binding a found resource proxy handle to the collection resource");
            try {
                OcPlatform.bindResource(mCollectionResourceHandle, localLightResourceHandle);
            } catch (OcException e) {
                Log.e(TAG, e.toString());
                msg("Failed to bind found resource proxy handle to the collection resource");
            }
            mProxyResourceHandleList.add(localLightResourceHandle);
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
        msg("Found resource with \"core.light\" type name\".");
        // Get the resource host address
        String hostAddress = foundResource.getHost();
        // Get the resource URI
        String resourceUri = foundResource.getUri();
        msg("\tHost address of the resource: " + hostAddress);
        msg("\tURI of the resource: " + resourceUri);
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

        //In this example we are only interested in the light resources
        if (resourceUri.equals("/a/light")) {
            msg("Registering a found resource as a local proxy resource");
            OcResourceHandle proxyResourceHandle = null;
            try {
                proxyResourceHandle = OcPlatform.registerResource(foundResource);
            } catch (OcException e) {
                Log.e(TAG, e.toString());
                msg("Failed to register a found resource as a local proxy resource");
            }

            if (null != proxyResourceHandle) {
                msg("Binding a found resource proxy handle to the collection resource");
                try {
                    OcPlatform.bindResource(mCollectionResourceHandle, proxyResourceHandle);
                } catch (OcException e) {
                    Log.e(TAG, e.toString());
                    msg("Failed to bind found resource proxy handle to the collection resource");
                }
                mProxyResourceHandleList.add(proxyResourceHandle);
            }
        }

        printLine();
        enableStartStopButton();
    }

    /**
     * A local method to reset group server
     */
    private synchronized void stopGroupServer() {
        msg("Unregistering resources");
        for (OcResourceHandle proxyResourceHandle : mProxyResourceHandleList) {
            try {
                OcPlatform.unbindResource(mCollectionResourceHandle, proxyResourceHandle);
            } catch (OcException e) {
                Log.e(TAG, e.toString());
                msg("Failed to unbind a proxy resource");
            }
            try {
                OcPlatform.unregisterResource(proxyResourceHandle);
            } catch (OcException e) {
                Log.e(TAG, e.toString());
                msg("Failed to unregister a proxy resource");
            }
        }
        mProxyResourceHandleList.clear();

        if (null != mCollectionResourceHandle) {
            try {
                OcPlatform.unregisterResource(mCollectionResourceHandle);
            } catch (OcException e) {
                Log.e(TAG, e.toString());
                msg("Failed to unregister a collection resource");
            }
        }
        msg("Group Server is reset.");

        printLine();
        enableStartStopButton();
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************

    private final static String TAG = GroupServer.class.getSimpleName();
    private TextView mConsoleTextView;
    private ScrollView mScrollView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_group_server);

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
                                startGroupServer();
                            }
                        }).start();
                    } else {
                        new Thread(new Runnable() {
                            public void run() {
                                stopGroupServer();
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