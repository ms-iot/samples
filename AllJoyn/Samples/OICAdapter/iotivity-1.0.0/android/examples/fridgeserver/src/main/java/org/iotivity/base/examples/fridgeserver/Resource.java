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

import android.util.Log;

import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResourceHandle;

/**
 * Resource
 * <p/>
 * Each of the other resource classes (DeviceResource, DoorResource and LightResource extend Resource
 */
public class Resource {
    protected OcResourceHandle mResourceHandle;
    protected OcRepresentation mRepresentation;

    Resource() {
        mResourceHandle = null;
        mRepresentation = new OcRepresentation();
    }

    public void bindTo(OcResourceHandle collectionResourceHandle) {
        try {
            if (null != mResourceHandle && null != collectionResourceHandle) {
                OcPlatform.bindResource(collectionResourceHandle, mResourceHandle);
            }
        } catch (OcException e) {
            Log.e("Resource", e.getMessage());
        }
    }

    public OcResourceHandle getHandle() {
        return mResourceHandle;
    }

    public static final int SUCCESS = 200;
}