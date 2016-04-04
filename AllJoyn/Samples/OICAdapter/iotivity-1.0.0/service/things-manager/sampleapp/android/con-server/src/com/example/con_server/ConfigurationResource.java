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
import java.util.Vector;

import org.iotivity.base.OcException;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResourceHandle;
import org.iotivity.base.ResourceProperty;

import android.util.Log;

//For creating/deleting the configuration Resource
public class ConfigurationResource {
    private final String       LOG_TAG                 = "[CON-SERVER]"
                                                               + this.getClass()
                                                                       .getSimpleName();
    // Configuration resource members
    protected String           configurationUri;
    protected String           deviceName;
    protected String           location;
    protected String           locationName;
    protected String           currency;
    protected String           region;
    protected Vector<String>   configurationTypes      = new Vector<String>();
    protected Vector<String>   configurationInterfaces = new Vector<String>();
    protected OcRepresentation configurationRep        = new OcRepresentation();
    protected OcResourceHandle configurationHandle;

    // constructors
    public ConfigurationResource() {
        Log.i(LOG_TAG, "ConfigurationResource: enter");

        deviceName = ConfigurationDefaultValues.defaultDeviceName;
        location = ConfigurationDefaultValues.defaultLocation;
        locationName = ConfigurationDefaultValues.defaultLocationName;
        currency = ConfigurationDefaultValues.defaultCurrency;
        region = ConfigurationDefaultValues.defaultRegion;

        configurationUri = ConfigurationDefaultValues.ConURIPrefix;
        configurationTypes
                .add(ConfigurationDefaultValues.ConResourceTypePrefix);
        configurationInterfaces.add(OcPlatform.DEFAULT_INTERFACE);

        configurationRep.setValueString("n", deviceName);
        configurationRep.setValueString("loc", location);
        configurationRep.setValueString("locn", locationName);
        configurationRep.setValueString("c", currency);
        configurationRep.setValueString("r", region);

        configurationRep.setUri(configurationUri);
        configurationRep.setResourceTypes(configurationTypes);
        configurationRep.setResourceInterfaces(configurationInterfaces);
    }

    // For creating Configuration Resource
    public void createResource(OcPlatform.EntityHandler listener)
            throws OcException {
        Log.i(LOG_TAG, "createResource(configuration): enter");
        EnumSet<ResourceProperty> propertySet = EnumSet.of(
                ResourceProperty.DISCOVERABLE, ResourceProperty.OBSERVABLE);

        // Register configuration resource
        configurationHandle = OcPlatform.registerResource(configurationUri,
                configurationTypes.elementAt(0),
                configurationInterfaces.elementAt(0), listener, propertySet);

        Log.i(LOG_TAG, "createResource(configuration): exit");
    }

    // Setters and Getters methods for Configuration resource
    public void setConfigurationRepresentation(OcRepresentation rep) {
        Log.i(LOG_TAG, "setConfigurationRepresentation: enter");

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

        Log.i(LOG_TAG, "setConfigurationRepresentation: exit");
    }

    public OcRepresentation getConfigurationRepresentation() {

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

    // For resetting the default values to configuration Resource
    public void factoryReset() {

        deviceName = ConfigurationDefaultValues.defaultDeviceName;
        location = ConfigurationDefaultValues.defaultLocation;
        locationName = ConfigurationDefaultValues.defaultLocationName;
        currency = ConfigurationDefaultValues.defaultCurrency;
        region = ConfigurationDefaultValues.defaultRegion;
        Log.i(LOG_TAG, "ConfiguartionResource: factoryReset done");
    }

    // Deleting all the resources, that we have created using createResources()
    public void deleteResource() {
        try {
            if (null != configurationHandle) {
                // Unregister the Configuration resource
                OcPlatform.unregisterResource(configurationHandle);
            }

        } catch (OcException e) {
            Log.e(LOG_TAG, "OcException occured! " + e.toString());
        }
    }
}
