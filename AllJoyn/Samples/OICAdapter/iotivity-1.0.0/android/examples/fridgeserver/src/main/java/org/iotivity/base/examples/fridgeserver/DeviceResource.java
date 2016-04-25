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
import org.iotivity.base.OcHeaderOption;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcResourceRequest;
import org.iotivity.base.OcResourceResponse;
import org.iotivity.base.RequestHandlerFlag;
import org.iotivity.base.ResourceProperty;

import java.util.EnumSet;
import java.util.LinkedList;
import java.util.List;

/**
 * DeviceResource
 * <p/>
 * Creates a device resource and performs action based on client requests
 */
public class DeviceResource extends Resource implements OcPlatform.EntityHandler {
    public static final String DEVICE_URI = "/device";
    public static final String RESOURCE_TYPENAME = "intel.fridge";
    public static final String API_VERSION = "v.1.0";
    public static final String CLIENT_TOKEN = "21ae43gf";
    public static final String DEVICE_NAME = "device_name";
    private static String TAG = "DeviceResource: ";
    public static final int SUCCESS = 200;
    public static final int API_VERSION_KEY = 2048;
    public static final int CLIENT_VERSION_KEY = 3000;

    private Context mContext;

    /**
     * constructor
     *
     * @param context to enable sending of broadcast messages to be displayed on the user screen
     */
    DeviceResource(Context context) {
        mContext = context;
        registerDeviceResource();
    }

    private void registerDeviceResource() {
        try {
            logMessage("RegisterDeviceResource " + DEVICE_URI + " : " + RESOURCE_TYPENAME);
            mResourceHandle = OcPlatform.registerResource(
                    DEVICE_URI,
                    RESOURCE_TYPENAME,
                    OcPlatform.DEFAULT_INTERFACE,
                    this,
                    EnumSet.of(ResourceProperty.DISCOVERABLE));
        } catch (OcException e) {
            logMessage(TAG + "Failed to register DeviceResource");
            Log.e(TAG, e.getMessage());
        }
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
            List<OcHeaderOption> headerOptions = ocResourceRequest.getHeaderOptions();
            String clientAPIVersion = "";
            String clientToken = "";
            // search for header options map and look for API version and client token
            for (OcHeaderOption headerOption : headerOptions) {
                int optionId = headerOption.getOptionId();
                if (API_VERSION_KEY == optionId) {
                    clientAPIVersion = headerOption.getOptionData();
                    logMessage(TAG + " Client API Version: " + clientAPIVersion);
                } else if (CLIENT_VERSION_KEY == optionId) {
                    clientToken = headerOption.getOptionData();
                    logMessage(TAG + " Client Token: " + clientToken);
                }
            }
            if (clientAPIVersion.equals(API_VERSION) &&
                    clientToken.equals(CLIENT_TOKEN)) {
                List<OcHeaderOption> serverHeaderOptions = new LinkedList<>();
                OcHeaderOption apiVersion = new OcHeaderOption(API_VERSION_KEY,
                        API_VERSION);
                serverHeaderOptions.add(apiVersion);
                try {
                    if (ocResourceRequest.getRequestHandlerFlagSet().contains(RequestHandlerFlag.REQUEST)) {
                        OcResourceResponse response = new OcResourceResponse();
                        response.setRequestHandle(ocResourceRequest.getRequestHandle());
                        response.setResourceHandle(ocResourceRequest.getResourceHandle());
                        response.setHeaderOptions(serverHeaderOptions);

                        switch (ocResourceRequest.getRequestType()) {
                            case GET:
                                response.setErrorCode(SUCCESS);
                                response.setResponseResult(EntityHandlerResult.OK);
                                updateRepresentationValues();
                                response.setResourceRepresentation(mRepresentation);
                                OcPlatform.sendResponse(response);
                                break;
                        }
                        result = EntityHandlerResult.OK;
                    }
                } catch (OcException e) {
                    logMessage("Error in handleEntity of DeviceResource");
                    Log.e(TAG, e.getMessage());
                }
            }
        }
        logMessage("-----------------------------------------------------");
        return result;
    }

    /**
     * update state of device
     *
     * @return device representation
     */
    private void updateRepresentationValues() {
        try {
            mRepresentation.setValue(DEVICE_NAME,
                    "Intel Powered 3 door, 1 light refrigerator");
        } catch (OcException e) {
            Log.e(TAG, e.getMessage());
        }
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************
    public void logMessage(String msg) {
        Intent intent = new Intent(FridgeServer.INTENT);
        intent.putExtra("message", msg);
        mContext.sendBroadcast(intent);
    }
}
