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
 * DoorResource
 * <p/>
 * DoorResource is a sample OIC server resource created by the refrigerator.
 */
public class DoorResource extends Resource implements OcPlatform.EntityHandler {
    DoorResource(String side, Context context) {
        mContext = context;
        mSide = side;

        registerDoorResource();
    }

    private void registerDoorResource() {
        String resourceURI = DOOR_URI + mSide;
        logMessage(TAG + "RegisterDoorResource " + resourceURI + " : " + RESOURCE_TYPEDOOR);
        try {
            mResourceHandle = OcPlatform.registerResource(
                    resourceURI,
                    RESOURCE_TYPEDOOR,
                    OcPlatform.DEFAULT_INTERFACE,
                    this,
                    EnumSet.of(ResourceProperty.DISCOVERABLE));
        } catch (OcException e) {
            logMessage(TAG + "Failed to register DoorResource");
            Log.e(TAG, e.getMessage());
        }
        logMessage("-----------------------------------------------------");
    }

    /**
     * sample implementation of eventHandler for doorResource - this can be implemented in many
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
                            break;
                        case PUT:
                            response.setErrorCode(Resource.SUCCESS);
                            put(ocResourceRequest.getResourceRepresentation());
                            updateRepresentationValues();
                            response.setResourceRepresentation(mRepresentation);
                            response.setResponseResult(EntityHandlerResult.OK);
                            OcPlatform.sendResponse(response);
                            break;
                        case DELETE:
                            response.setResponseResult(EntityHandlerResult.RESOURCE_DELETED);
                            response.setErrorCode(204);
                            OcPlatform.sendResponse(response);
                            break;
                    }
                    result = EntityHandlerResult.OK;
                }
            } catch (OcException e) {
                logMessage("Error in handleEntity of DoorResource");
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
     * helper function to update the current value of the door resource
     */
    private void updateRepresentationValues() {
        try {
            mRepresentation.setValue(DOOR_STATE_KEY, mDoorState);
            mRepresentation.setValue(DOOR_SIDE_KEY, mSide);
            logMessage(TAG + "door state is  " + ((mDoorState == true) ? "open" : "close") +
                    " and door side is " + mSide);
        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    /**
     * update the value of doorResource, depending on if door is open/ closed
     *
     * @param representation new state of a door
     */
    private void put(OcRepresentation representation) {
        try {
            mDoorState = representation.getValue(DOOR_STATE_KEY);
        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
        }
        // Note, we won't let the user change the door side!
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************
    private Context mContext;
    private String mSide;
    private boolean mDoorState;
    public static final String DOOR_URI = "/door/";
    public static final String RESOURCE_TYPEDOOR = "intel.fridge.door";
    private static String TAG = "DoorResource: ";
    public static final String DOOR_STATE_KEY = "state";
    public static final String DOOR_SIDE_KEY = "side";

    private void logMessage(String msg) {
        Intent intent = new Intent(Resource.INTENT);
        intent.putExtra(Resource.MESSAGE, msg);
        mContext.sendBroadcast(intent);
    }
}