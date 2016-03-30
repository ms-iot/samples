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
import android.os.Bundle;
import android.os.Message;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

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
import java.util.LinkedList;
import java.util.List;

/**
 * FridgeGroupClient
 * <p/>
 * FridgeGroupClient is a sample client app which should be started after the fridgeGroupServer is
 * started. It discovers a fridge resource and then creates the proxy resources for each one of its
 * children (light and door) and performs a GET on them.
 */
public class FridgeGroupClient extends Activity implements
        OcPlatform.OnResourceFoundListener,
        OcResource.OnGetListener {
    private static String TAG = "FridgeGroupClient: ";

    private final List<OcResource> childResourceList = new LinkedList<>();
    private OcResource fridgeResource;

    private void startFridgeClient() {
        PlatformConfig cfg = new PlatformConfig(
                this, // context
                ServiceType.IN_PROC,
                ModeType.CLIENT,
                "0.0.0.0", // bind to all available interfaces
                0,
                QualityOfService.LOW);

        logMessage("Configuring platform");
        OcPlatform.Configure(cfg);
        String requestUri = OcPlatform.WELL_KNOWN_QUERY + "?rt=intel.fridge.group";
        logMessage("Initiating fridge discovery");
        try {
            OcPlatform.findResource("",
                    requestUri,
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    this);
        } catch (OcException e) {
            logMessage("Failed to discover resource");
            Log.e(TAG, e.getMessage());
        }
        logMessage("-----------------------------------------------------");
    }

    /**
     * callback when a fridge resource is found.
     */
    @Override
    public synchronized void onResourceFound(OcResource ocResource) {
        if ((null != fridgeResource) && !fridgeResource.getUri().equals("/fridge/group")) {
            logMessage("Didn't find the correct fridge resource. Exiting");
            return;
        }
        fridgeResource = ocResource;
        logMessage("Discovered a fridge with \nHost: " + fridgeResource.getHost());
        logMessage("Trying to call GET api on fridgeResource");
        try {
            fridgeResource.get(new HashMap<String, String>(), this);
        } catch (OcException e) {
            logMessage("Failed to call GET api");
            Log.e(TAG, e.getMessage());
        }
        logMessage("-----------------------------------------------------");
    }

    /**
     * once the fridge resource is discovered, create proxy child resources of the fridge
     * and call GET on each of the child resource proxies.
     *
     * @param list
     * @param ocRepresentation parent resource
     */
    @Override
    public synchronized void onGetCompleted(List<OcHeaderOption> list,
                                            OcRepresentation ocRepresentation) {
        logMessage("Got a response from " + ocRepresentation.getUri());
        for (OcRepresentation child : ocRepresentation.getChildren()) {
            try {
                logMessage("Creating child resource proxy from fridgeResource with uri " +
                        child.getUri());
                OcResource childResource = OcPlatform.constructResourceObject(
                        fridgeResource.getHost(),
                        child.getUri(),
                        fridgeResource.getConnectivityTypeSet(),
                        false, // isObservable set to false
                        child.getResourceTypes(),
                        child.getResourceInterfaces());
                childResourceList.add(childResource);
            } catch (OcException e) {
                logMessage("Error in creating child resource proxy");
                Log.e(TAG, e.getMessage());
            }
            logMessage("-----------------------------------------------------");
        }

        OcResource.OnGetListener childOnGetListener = new OcResource.OnGetListener() {
            public static final String DOOR_STATE_KEY = "state";
            public static final String DOOR_SIDE_KEY = "side";
            public static final String LIGHT_STATUS_KEY = "light";

            @Override
            public synchronized void onGetCompleted(List<OcHeaderOption> list,
                                                    OcRepresentation ocRepresentation) {
                logMessage("Received a response from a child of the fridge with uri: " +
                        ocRepresentation.getUri());
                for (String resType : ocRepresentation.getResourceTypes()) {
                    if (resType.equals("intel.fridge.door")) {
                        try {
                            logMessage(ocRepresentation.getValue(DOOR_SIDE_KEY) +
                                    " door is " + ((ocRepresentation.getValue(DOOR_STATE_KEY)
                                    ) ? "open" : "close"));
                        } catch (OcException e) {
                            logMessage("Failed to get the door resource representation");
                            Log.e(TAG, e.getMessage());
                        }
                    } else if (resType.equals("intel.fridge.light")) {
                        try {
                            logMessage("Fridge light is " +
                                    ((ocRepresentation.getValue(LIGHT_STATUS_KEY)) ?
                                            "on" : "off"));
                        } catch (OcException e) {
                            logMessage("Failed to get the light resource representation");
                            Log.e(TAG, e.getMessage());
                        }
                    }
                }
                logMessage("-----------------------------------------------------");
            }

            @Override
            public synchronized void onGetFailed(Throwable throwable) {
                logMessage("OnGet failed for child of fridge");
                Log.e(TAG, throwable.getMessage());
            }
        };

        for (OcResource child : childResourceList) {
            try {
                logMessage("Trying to get a representation of " + child.getUri() +
                        " resource from server");
                child.get(new HashMap<String, String>(), childOnGetListener);
            } catch (OcException e) {
                logMessage(e.getMessage());
                Log.e(TAG, e.getMessage());
            }
        }

    }

    @Override
    public synchronized void onGetFailed(Throwable throwable) {
        logMessage("Failed to get representation of the fridge");
        Log.e(TAG, throwable.toString());
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************
    private TextView mConsoleTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_fridge_client);

        mConsoleTextView = (TextView) findViewById(R.id.consoleTextView);
        mConsoleTextView.setMovementMethod(new ScrollingMovementMethod());
        final Button button = (Button) findViewById(R.id.button);

        if (null == savedInstanceState) {
            button.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    button.setEnabled(false);
                    new Thread(new Runnable() {
                        public void run() {
                            startFridgeClient();
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

    private void logMessage(final String text) {
        runOnUiThread(new Runnable() {
            public void run() {
                final Message msg = new Message();
                msg.obj = text;
                mConsoleTextView.append("\n");
                mConsoleTextView.append(text);
            }
        });
        Log.i(TAG, text);
    }
}