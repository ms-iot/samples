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

import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcResourceHandle;
import org.iotivity.base.ResourceProperty;

import java.util.EnumSet;


/**
 * FridgeResource
 * <p/>
 * FridgeResource is a sample OIC server resource created by the refrigerator.
 */
public class FridgeResource extends Resource {
    FridgeResource(Context context) {
        mContext = context;
        registerFridgeResource();
    }

    private void registerFridgeResource() {
        try {
            logMessage(TAG + "RegisterFridgeResource " + FRIDGE_URI +
                    " : " + FRIDGE_TYPENAME);
            mResourceHandle = OcPlatform.registerResource(
                    FRIDGE_URI,
                    FRIDGE_TYPENAME,
                    OcPlatform.GROUP_INTERFACE,
                    null,
                    EnumSet.of(ResourceProperty.DISCOVERABLE));
        } catch (OcException e) {
            logMessage(TAG + "FridgeResource register error: " + e.getMessage());
            Log.e(TAG, e.getMessage());
        }
        logMessage("-----------------------------------------------------");
    }

    public OcResourceHandle getHandle() {
        return mResourceHandle;
    }

    //******************************************************************************
    // End of the OIC specific code
    //******************************************************************************
    private Context mContext;
    public static final String FRIDGE_URI = "/fridge/group";
    public static final String FRIDGE_TYPENAME = "intel.fridge.group";
    private static String TAG = "FridgeResource: ";

    private void logMessage(String msg) {
        Intent intent = new Intent(Resource.INTENT);
        intent.putExtra("message", msg);
        mContext.sendBroadcast(intent);
    }
}
