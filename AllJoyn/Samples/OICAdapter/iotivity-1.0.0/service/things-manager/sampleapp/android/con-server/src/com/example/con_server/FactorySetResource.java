/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/
package com.example.con_server;

import java.util.EnumSet;

import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.ResourceProperty;

import android.util.Log;

// For creating & deleting the FactorySet Resource
public class FactorySetResource extends ConfigurationResource {
    private final String LOG_TAG = "[CON-SERVER]"
                                         + this.getClass().getSimpleName();

    // constructor
    public FactorySetResource() {
        Log.i(LOG_TAG, "FactorySetCollection: enter");

        configurationUri = "/factoryset"; // uri of the resource
        configurationTypes.clear();
        configurationTypes.add("factoryset");
        configurationRep.setUri(configurationUri);
        configurationRep.setResourceTypes(configurationTypes);
    }

    // for creating FactoryReset Resource
    public void createResource(OcPlatform.EntityHandler listener)
            throws OcException {
        Log.i(LOG_TAG, "createResource(Factory Set): enter");
        EnumSet<ResourceProperty> propertySet = EnumSet.of(
                ResourceProperty.DISCOVERABLE, ResourceProperty.OBSERVABLE);
        if (null == listener) {
            Log.i(LOG_TAG, "CallBack Should be binded");
            return;
        }

        // Register factoryset resource
        configurationHandle = OcPlatform.registerResource(configurationUri,
                configurationTypes.elementAt(0),
                configurationInterfaces.elementAt(0), listener, propertySet);
        if (null == configurationHandle) {
            Log.e(LOG_TAG, "registerResource failed!");
            return;
        }
        Log.i(LOG_TAG, "createResource (Factory Set): exit");
    }

    // getters and Setters Methods for FacoryReset Resource
    public void setFactorySetRepresentation(OcRepresentation rep) {
        Log.i(LOG_TAG, "setFactorySetRepresentation: enter");

        String dName = rep.getValueString("n");
        String loc = rep.getValueString("loc");
        String locn = rep.getValueString("locn");
        String cur = rep.getValueString("c");
        String reg = rep.getValueString("r");

        if (!(dName.equalsIgnoreCase(""))) {
            deviceName = dName;
            Log.i(LOG_TAG,
                    "setConfigurationRepresentation: New value(deviceName): "
                            + deviceName);
        }
        if (!(loc.equalsIgnoreCase(""))) {
            location = loc;
            Log.i(LOG_TAG,
                    "setConfigurationRepresentation: New value(location): "
                            + location);
        }
        if (!(locn.equalsIgnoreCase(""))) {
            locationName = locn;
            Log.i(LOG_TAG,
                    "setConfigurationRepresentation: New value(locationName): "
                            + locationName);
        }
        if (!(cur.equalsIgnoreCase(""))) {
            currency = cur;
            Log.i(LOG_TAG,
                    "setConfigurationRepresentation: New value(currency): "
                            + currency);
        }
        if (!(reg.equalsIgnoreCase(""))) {
            region = reg;
            Log.i(LOG_TAG,
                    "setConfigurationRepresentation: New value(region): "
                            + region);
        }
        Log.i(LOG_TAG, "setFactorySetRepresentation: exit");
    }

    OcRepresentation getFactorySetRepresentation() {

        configurationRep.setValueString("n", deviceName);
        configurationRep.setValueString("loc", location);
        configurationRep.setValueString("locn", locationName);
        configurationRep.setValueString("c", currency);
        configurationRep.setValueString("r", region);
        return configurationRep;
    }

    public String getUri() {
        return configurationUri;
    }

    // For deletingFactorySet Resource
    public void deleteResource() {
        try {
            if (null != configurationHandle) {
                // Unregister the collection resource
                OcPlatform.unregisterResource(configurationHandle);
            }
        } catch (OcException e) {
            Log.e(LOG_TAG, "OcException occured! " + e.toString());
        }
    }
}
