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
 * FridgeClient
 * <p/>
 * FridgeClient is a sample client app which should be started after the fridgeServer is started.
 * It creates DeviceResource, DoorResources, LightResource and performs a GET operation on them.
 */
public class FridgeClient extends Activity implements
        OcPlatform.OnResourceFoundListener,
        OcResource.OnGetListener {
    public static final String DEVICE_URI = "/device";
    public static final String LIGHT = "/light";
    public static final String LEFT_DOOR = "/door/left";
    public static final String RIGHT_DOOR = "/door/right";
    public static final String RANDOM_DOOR = "/door/random";
    public static final String API_VERSION = "v.1.0";
    public static final String CLIENT_TOKEN = "21ae43gf";
    public static final int API_VERSION_KEY = 2048;
    public static final int CLIENT_TOKEN_KEY = 3000;

    private final List<OcResource> mResourceList = new LinkedList<OcResource>();
    private OcResource mFridgeResource;

    /**
     * configure OIC platform and call findResource
     */
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
        logMessage("Initiating fridge discovery");
        try {
            OcPlatform.findResource("",
                    OcPlatform.WELL_KNOWN_QUERY + "?rt=" + "intel.fridge",
                    EnumSet.of(OcConnectivityType.CT_DEFAULT),
                    this);
        } catch (OcException e) {
            logMessage(" Failed to discover resource");
            Log.e(TAG, e.getMessage());
        }
        logMessage("-----------------------------------------------------");
    }

    /**
     * An event handler to be executed whenever a "findResource" request completes successfully
     *
     * @param ocResource found resource
     */
    @Override
    public synchronized void onResourceFound(OcResource ocResource) {
        if (null != mFridgeResource || !ocResource.getUri().equals(DEVICE_URI)) {
            logMessage("Didn't find the correct fridge resource. Exiting");
            return;
        }
        mFridgeResource = ocResource;
        logMessage("Discovered a fridge with \nHost: " + mFridgeResource.getHost());

        List<String> lightTypes = new LinkedList<>();
        lightTypes.add("intel.fridge.light");
        List<String> doorTypes = new LinkedList<>();
        doorTypes.add("intel.fridge.door");
        List<String> resourceInterfaces = new LinkedList<>();
        resourceInterfaces.add(OcPlatform.DEFAULT_INTERFACE);
        logMessage("Creating child resource proxies for the previously known fridge components");
        OcResource light = null;
        OcResource leftDoor = null;
        OcResource rightDoor = null;
        OcResource randomDoor = null;
        try {
            light = OcPlatform.constructResourceObject(mFridgeResource.getHost(),
                    LIGHT,
                    mFridgeResource.getConnectivityTypeSet(),
                    false, //isObservable
                    lightTypes,
                    resourceInterfaces);
            mResourceList.add(light);

            leftDoor = OcPlatform.constructResourceObject(mFridgeResource.getHost(),
                    LEFT_DOOR,
                    mFridgeResource.getConnectivityTypeSet(),
                    false, //isObservable
                    doorTypes,
                    resourceInterfaces);
            mResourceList.add(leftDoor);

            rightDoor = OcPlatform.constructResourceObject(mFridgeResource.getHost(),
                    RIGHT_DOOR,
                    mFridgeResource.getConnectivityTypeSet(),
                    false, //isObservable
                    doorTypes,
                    resourceInterfaces);
            mResourceList.add(rightDoor);

            randomDoor = OcPlatform.constructResourceObject(mFridgeResource.getHost(),
                    RANDOM_DOOR,
                    mFridgeResource.getConnectivityTypeSet(),
                    false, //isObservable
                    doorTypes,
                    resourceInterfaces);
            mResourceList.add(randomDoor);
        } catch (OcException e) {
            logMessage("Error in constructResourceObject");
            Log.e(TAG, e.getMessage());
        }

        List<OcHeaderOption> headerOptions = new LinkedList<>();
        OcHeaderOption apiVersion = new OcHeaderOption(API_VERSION_KEY, API_VERSION);
        OcHeaderOption clientToken = new OcHeaderOption(CLIENT_TOKEN_KEY, CLIENT_TOKEN);
        headerOptions.add(apiVersion);
        headerOptions.add(clientToken);
        mFridgeResource.setHeaderOptions(headerOptions);

        logMessage("Calling GET api on mFridgeResource and other component resources");
        try {
            mFridgeResource.get(new HashMap<String, String>(), this);
            if (null != light) light.get(new HashMap<String, String>(), this);
            if (null != leftDoor) leftDoor.get(new HashMap<String, String>(), this);
            if (null != rightDoor) rightDoor.get(new HashMap<String, String>(), this);
            if (null != randomDoor) randomDoor.get(new HashMap<String, String>(), this);
        } catch (OcException e) {
            logMessage("Error in GET calls");
            Log.e(TAG, e.getMessage());
        }
    }

    /**
     * An event handler to be executed whenever a "get" request completes successfully
     *
     * @param headerOptionList list of the header options
     * @param ocRepresentation representation of a resource
     */
    @Override
    public synchronized void onGetCompleted(List<OcHeaderOption> headerOptionList,
                                            OcRepresentation ocRepresentation) {
        logMessage("Got a response from " + ocRepresentation.getUri());
    }

    /**
     * An event handler to be executed whenever a "get" request fails
     *
     * @param throwable exception
     */
    @Override
    public synchronized void onGetFailed(Throwable throwable) {
        logMessage("GET request has failed");
        Log.e(TAG, throwable.toString());
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************
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

    private static String TAG = "FridgeClient: ";
    private TextView mConsoleTextView;
}