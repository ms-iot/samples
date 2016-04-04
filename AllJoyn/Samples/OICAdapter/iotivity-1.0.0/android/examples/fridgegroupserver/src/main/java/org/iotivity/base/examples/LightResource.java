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

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import org.iotivity.base.EntityHandlerResult;
import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResourceHandle;
import org.iotivity.base.OcResourceRequest;
import org.iotivity.base.OcResourceResponse;
import org.iotivity.base.RequestHandlerFlag;
import org.iotivity.base.ResourceProperty;

import java.util.EnumSet;


/**
 * LightResource
 * <p/>
 * LightResource is a sample OIC server resource created by the refrigerator.
 */
public class LightResource extends Resource implements OcPlatform.EntityHandler {
    LightResource(Context context) {
        mContext = context;

        registerLightResource();
    }

    private void registerLightResource() {
        try {
            logMessage(TAG + "RegisterLightResource " + LIGHT_URI + " : " + RESOURCE_TYPELIGHT);
            mResourceHandle = OcPlatform.registerResource(
                    LIGHT_URI,
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
     * sample implementation of eventHandler for lightResource - this can be implemented in many
     * different ways
     *
     * @param ocResourceRequest OcResourceRequest from the client
     * @return EntityHandlerResult indicates whether the request was handled successfully or not
     */
    @Override
    public synchronized EntityHandlerResult handleEntity(OcResourceRequest ocResourceRequest) {
        EntityHandlerResult result = EntityHandlerResult.ERROR;
        if (null != ocResourceRequest) {
            try {
                if (ocResourceRequest.getRequestHandlerFlagSet().contains(
                        RequestHandlerFlag.REQUEST)) {
                    OcResourceResponse response = new OcResourceResponse();
                    response.setRequestHandle(ocResourceRequest.getRequestHandle());
                    response.setResourceHandle(ocResourceRequest.getResourceHandle());
                    switch (ocResourceRequest.getRequestType()) {
                        case GET:
                            response.setErrorCode(Resource.SUCCESS);
                            updateRepresentationValues();
                            response.setResourceRepresentation(mRepresentation);
                            response.setResponseResult(EntityHandlerResult.OK);
                            OcPlatform.sendResponse(response);
                            result = EntityHandlerResult.OK;
                            break;
                        case PUT:
                            response.setErrorCode(Resource.SUCCESS);
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
                logMessage("Error in handleEntity");
                Log.e(TAG, e.getMessage());
                return EntityHandlerResult.ERROR;
            }
        }
        logMessage("-----------------------------------------------------");
        return result;
    }

    public OcResourceHandle getHandle() {
        return mResourceHandle;
    }

    /**
     * update the value of light (ON/ OFF) from the representation
     *
     * @param representation new state of light
     */
    private void put(OcRepresentation representation) {
        try {
            mIsLightOn = representation.getValue(LIGHT_STATUS_KEY);
        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    /**
     * helper function to update the current state of the light
     */
    private void updateRepresentationValues() {
        try {
            mRepresentation.setValue(LIGHT_STATUS_KEY, mIsLightOn);
        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************
    private Context mContext;
    private boolean mIsLightOn = false;

    private static String TAG = "LightResource: ";
    public static final String LIGHT_URI = "/light";
    public static final String RESOURCE_TYPELIGHT = "intel.fridge.light";
    public static final String LIGHT_STATUS_KEY = "light";

    private void logMessage(String msg) {
        Intent intent = new Intent(Resource.INTENT);
        intent.putExtra(Resource.MESSAGE, msg);
        mContext.sendBroadcast(intent);
    }
}
