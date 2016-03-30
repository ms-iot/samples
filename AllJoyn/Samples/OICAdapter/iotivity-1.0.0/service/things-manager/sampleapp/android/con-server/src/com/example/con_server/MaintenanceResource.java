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

//For creating/deleting the Diagnostics Resource
public class MaintenanceResource {
    private final String     LOG_TAG                 = "[CON-SERVER]"
                                                             + this.getClass()
                                                                     .getSimpleName();
    // maintenance members
    private String           m_maintenanceUri;
    private String           m_factoryReset;
    private String           m_reboot;
    private String           m_startCollection;
    private Vector<String>   m_maintenanceTypes      = new Vector<String>();
    private Vector<String>   m_maintenanceInterfaces = new Vector<String>();
    private OcResourceHandle m_maintenanceHandle     = null;
    private OcRepresentation m_maintenanceRep        = new OcRepresentation();

    // constructor
    public MaintenanceResource() {
        Log.i(LOG_TAG, "MaintenanceResource: enter");

        m_factoryReset = ConfigurationDefaultValues.defaultFactoryReset;
        m_reboot = ConfigurationDefaultValues.defaultReboot;
        m_startCollection = ConfigurationDefaultValues.defaultStartCollection;

        m_maintenanceUri = ConfigurationDefaultValues.diagURIPrefix;
        m_maintenanceTypes
                .add(ConfigurationDefaultValues.diagResourceTypePrefix);
        m_maintenanceInterfaces.add(OcPlatform.DEFAULT_INTERFACE);
        m_maintenanceRep.setValueString("fr", m_factoryReset);
        m_maintenanceRep.setValueString("rb", m_reboot);
        m_maintenanceRep.setValueString("ssc", m_startCollection);
        m_maintenanceRep.setUri(m_maintenanceUri);
        m_maintenanceRep.setResourceTypes(m_maintenanceTypes);
        m_maintenanceRep.setResourceInterfaces(m_maintenanceInterfaces);
    }

    // for creating Diagnostic Resource
    public void createResource(OcPlatform.EntityHandler listener)
            throws OcException {
        Log.i(LOG_TAG, "createResource(Maintenance): enter");
        EnumSet<ResourceProperty> propertySet = EnumSet.of(
                ResourceProperty.DISCOVERABLE, ResourceProperty.OBSERVABLE);
        if (null == listener) {
            Log.i(LOG_TAG, "CallBack Should be binded");
            return;
        }

        // Register diagnostic resource
        m_maintenanceHandle = OcPlatform.registerResource(m_maintenanceUri,
                m_maintenanceTypes.get(0), m_maintenanceInterfaces.get(0),
                listener, propertySet);
        if (null == m_maintenanceHandle) {
            Log.e(LOG_TAG, "registerResource failed!");
            return;
        }

        Thread thread = new Thread() {
            @Override
            public void run() {
                try {
                    while (true) {
                        // Put this thread for sleep for 1 sec.
                        // Sleep value can be changed as per the developer
                        // convenience.
                        Thread.sleep(1000);
                        if (m_reboot.equalsIgnoreCase("true")) {
                            Log.i(LOG_TAG, "Reboot will be soon...");
                            MainActivity mainActivityObj = MainActivity
                                    .getMainActivityObject();
                            if (null == mainActivityObj) {
                                Log.e(LOG_TAG,
                                        "Mainactivity object is invalid!");
                                return;
                            }
                            try {
                                mainActivityObj.runOnUiThread(new Runnable() {
                                    @Override
                                    public void run() {
                                        try {
                                            MainActivity.reboot();
                                        } catch (InterruptedException e) {
                                            e.printStackTrace();
                                        }
                                    }
                                });
                            } catch (Exception e) {
                                Log.e(LOG_TAG, "InterruptedException occured: "
                                        + e.toString());
                                continue;
                            }
                            m_reboot = ConfigurationDefaultValues.defaultReboot;
                        }
                        if (m_factoryReset.equalsIgnoreCase("true")) {
                            m_factoryReset = ConfigurationDefaultValues.defaultFactoryReset;
                            factoryReset();
                        }
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        };
        thread.start();
        Log.i(LOG_TAG, "createResource(Diagnostics): exit");
    }

    // getters and Setters Methods for diagnostics Resource
    public void setDiagnosticsRepresentation(OcRepresentation rep) {
        Log.i(LOG_TAG, "setDiagnosticsRepresentation: enter");

        String fr = rep.getValueString("fr");
        String rb = rep.getValueString("rb");
        String ssc = rep.getValueString("ssc");

        if (!(fr.equalsIgnoreCase(""))) {
            m_factoryReset = fr;
            Log.i(LOG_TAG,
                    "setConfigurationRepresentation: New value(FactoryReset): "
                            + fr);
        }
        if (!(rb.equalsIgnoreCase(""))) {
            m_reboot = rb;
            Log.i(LOG_TAG, "setDiagnosticsRepresentation: new value:(reboot) "
                    + rb);
        }

        if (!(ssc.equalsIgnoreCase(""))) {
            m_startCollection = ssc;
            Log.i(LOG_TAG,
                    "setDiagnosticsRepresentation: new value:(startcollection) "
                            + ssc);
        }
        Log.i(LOG_TAG, "setDiagnosticsRepresentation: exit");
    }

    OcRepresentation getDiagnosticsRepresentation() {
        m_maintenanceRep.setValueString("fr", m_factoryReset);
        m_maintenanceRep.setValueString("rb", m_reboot);
        m_maintenanceRep.setValueString("ssc", m_startCollection);
        return m_maintenanceRep;
    }

    public String getUri() {
        return m_maintenanceUri;
    }

    // For Resetting diagnostics Resource attributes to their default values
    public void factoryReset() {
        m_factoryReset = ConfigurationDefaultValues.defaultFactoryReset;
        m_reboot = ConfigurationDefaultValues.defaultReboot;
        m_startCollection = ConfigurationDefaultValues.defaultStartCollection;
    }

    // For Deleting diagnostic resource
    public void deleteResource() {
        try {
            if (null != m_maintenanceHandle) {
                // Unregister the collection resource
                OcPlatform.unregisterResource(m_maintenanceHandle);
            }
        } catch (OcException e) {
            Log.e(LOG_TAG, "OcException occured! " + e.toString());
        }
    }
}
