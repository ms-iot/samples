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

package org.iotivity.base.examples.fridgeserver;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import org.iotivity.base.EntityHandlerResult;
import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResourceRequest;
import org.iotivity.base.OcResourceResponse;
import org.iotivity.base.RequestHandlerFlag;
import org.iotivity.base.ResourceProperty;

import java.util.EnumSet;

/**
 * LightResource
 * <p/>
 * Creates a light resource and performs actions based on the client requests
 */
public class LightResource extends Resource implements OcPlatform.EntityHandler {
    public static final String LIGHT_STATUS_KEY = "light";
    public static final String LIGHT_URI = "/light";
    public static final String RESOURCE_TYPELIGHT = "intel.fridge.light";
    private boolean mIsLightOn = false;

    /**
     * constructor
     *
     * @param context to enable sending of broadcast messages to be displayed on the user screen
     */
    LightResource(Context context) {
        mContext = context;
        registerLightResource();
    }

    private void registerLightResource() {
        try {
            logMessage(TAG + "RegisterLightResource " + LIGHT_URI + " : " + RESOURCE_TYPELIGHT);
            mResourceHandle = OcPlatform.registerResource(LIGHT_URI,
                    RESOURCE_TYPELIGHT,
                    OcPlatform.DEFAULT_INTERFACE,
                    this,
                    EnumSet.of(ResourceProperty.DISCOVERABLE));
        } catch (OcException e) {
            logMessage(TAG + "Failed to register LightResource");
            Log.e(TAG, e.getMessage());
        }
        logMessage("-----------------------------------------------------");
    }

    /**
     * this is the main method which handles different incoming requests appropriately.
     *
     * @param ocResourceRequest OcResourceRequest from the client
     * @return EntityHandlerResult indicates whether the request was handled successfully or not
     */
    @Override
    public synchronized EntityHandlerResult handleEntity(OcResourceRequest ocResourceRequest) {
        EntityHandlerResult result = EntityHandlerResult.ERROR;
        if (null != ocResourceRequest) {
            try {
                if (ocResourceRequest.getRequestHandlerFlagSet().contains(RequestHandlerFlag.REQUEST)) {
                    OcResourceResponse response = new OcResourceResponse();
                    response.setRequestHandle(ocResourceRequest.getRequestHandle());
                    response.setResourceHandle(ocResourceRequest.getResourceHandle());

                    switch (ocResourceRequest.getRequestType()) {
                        case GET:
                            response.setErrorCode(SUCCESS);
                            updateRepresentationValues();
                            response.setResourceRepresentation(mRepresentation);
                            response.setResponseResult(EntityHandlerResult.OK);
                            OcPlatform.sendResponse(response);
                            result = EntityHandlerResult.OK;
                            break;
                        case PUT:
                            response.setErrorCode(SUCCESS);
                            put(ocResourceRequest.getResourceRepresentation());
                            updateRepresentationValues();
                            response.setResourceRepresentation(mRepresentation);
                            response.setResponseResult(EntityHandlerResult.OK);
                            OcPlatform.sendResponse(response);
                            result = EntityHandlerResult.OK;
                            break;
                    }
                }
            } catch (OcException e) {
                logMessage("Error in handleEntity of LightResource");
                Log.e(TAG, e.getMessage());
                return EntityHandlerResult.ERROR;
            }
        }
        logMessage("-----------------------------------------------------");
        return result;
    }

    /**
     * updates the current state of the light (on/ off)
     *
     * @return light is on or off
     */
    private void updateRepresentationValues() {
        try {
            mRepresentation.setValue(LIGHT_STATUS_KEY, mIsLightOn);
        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    /**
     * update the value of mIsOn from the representation
     *
     * @param representation get current state of light
     */
    private void put(OcRepresentation representation) {
        try {
            mIsLightOn = representation.getValue(LIGHT_STATUS_KEY);
        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************
    private Context mContext;
    private static String TAG = "LightResource: ";

    public void logMessage(String msg) {
        Intent intent = new Intent(FridgeServer.INTENT);
        intent.putExtra(FridgeServer.MESSAGE, msg);
        mContext.sendBroadcast(intent);
    }
}
