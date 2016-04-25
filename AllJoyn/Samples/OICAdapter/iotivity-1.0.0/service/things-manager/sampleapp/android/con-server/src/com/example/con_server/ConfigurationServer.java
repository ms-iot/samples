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

import org.iotivity.base.EntityHandlerResult;
import org.iotivity.base.OcException;
import org.iotivity.base.OcHeaderOption;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResourceRequest;
import org.iotivity.base.OcResourceResponse;
import org.iotivity.base.RequestHandlerFlag;
import org.iotivity.base.RequestType;
import org.iotivity.service.tm.OCStackResult;
import org.iotivity.service.tm.GroupManager;
import org.iotivity.service.tm.ThingsConfiguration;
import org.iotivity.service.tm.ThingsConfiguration.*;
import org.iotivity.service.tm.ThingsMaintenance;
import org.iotivity.service.tm.ThingsMaintenance.*;

import android.os.Message;
import android.util.Log;

/*
 * For Creating the Resources [configuration, Maintenance & FactoryRest]  &
 * for Handling of the Client's Request
 */
public class ConfigurationServer {
    private final String                      LOG_TAG            = "[CON-SERVER]"
                                                                         + this.getClass()
                                                                                 .getSimpleName();
    private GroupManager                      groupmanager       = null;
    private ThingsConfiguration               thingsconfig       = null;
    private ThingsMaintenance                 thingsmnt          = null;
    private ConfigurationResource             conResource        = null;
    private MaintenanceResource               mntResource        = null;
    private FactorySetResource                factorysetResource = null;

    private final ThingsConfigurationListener thingConfigurationListener;
    private final ThingsMaintenanceListener   thingsMaintenanceListener;
    private final RequestHandler              requestHandler;

    // constructor
    public ConfigurationServer() {
        groupmanager = new GroupManager();
        thingsconfig = ThingsConfiguration.getInstance();
        thingsmnt = ThingsMaintenance.getInstance();
        thingConfigurationListener = new ThingsConfigurationListener();
        thingsMaintenanceListener = new ThingsMaintenanceListener();
        requestHandler = new RequestHandler();

        thingsconfig.setConfigurationListener(thingConfigurationListener);
        thingsmnt.setThingsMaintenanceListener(thingsMaintenanceListener);

    }

    // Creating resources : configuration, maintenance, factoryReset
    public void CreateConfigurationResource() {
        Log.i(LOG_TAG, "CreateConfigurationResource: enter");

        try {
            conResource = new ConfigurationResource();
            conResource.createResource(requestHandler);

            mntResource = new MaintenanceResource();
            mntResource.createResource(requestHandler);

            factorysetResource = new FactorySetResource();
            factorysetResource.createResource(requestHandler);
        } catch (OcException e) {
            Log.e(LOG_TAG, "OcException occured: " + e.toString());
        }

        Log.i(LOG_TAG, "CreateConfigurationResource: exit");

        String message = "Resources Created Successfully(Server is Ready)";

        Message msg = Message.obtain();
        msg.what = 0;
        MainActivity mainActivityObj = MainActivity.getMainActivityObject();
        MainActivity.setmessage(message);
        mainActivityObj.getmHandler().sendMessage(msg);
    }

    public void DoBootStrap() {
        Log.i(LOG_TAG, "DoBootStrap: enter");

        OCStackResult result = thingsconfig.doBootstrap();
        if (OCStackResult.OC_STACK_ERROR == result) {
            Log.e(LOG_TAG, "doBootStrap returned error: "
                    + OCStackResult.OC_STACK_ERROR.name());
        }
        Log.i(LOG_TAG, "DoBootStrap: exit");
    }

    private class ThingsConfigurationListener implements IConfigurationListener {

        @Override
        public void onBootStrapCallback(Vector<OcHeaderOption> headerOptions,
                OcRepresentation rep, int errorValue) {

            String message;
            Log.i(LOG_TAG, "onBootStrapCallback");

            // setting the default values received from bootstrap Server
            ConfigurationDefaultValues.defaultDeviceName = rep
                    .getValueString("n");
            ConfigurationDefaultValues.defaultLocation = rep
                    .getValueString("loc");
            ConfigurationDefaultValues.defaultLocationName = rep
                    .getValueString("locn");
            ConfigurationDefaultValues.defaultCurrency = rep
                    .getValueString("c");
            ConfigurationDefaultValues.defaultRegion = rep.getValueString("r");

            // forming the message to display on UI
            message = "URI : " + rep.getUri() + "\n";
            message += "Device Name : "
                    + ConfigurationDefaultValues.defaultDeviceName + "\n";
            message += "Location : "
                    + ConfigurationDefaultValues.defaultLocation + "\n";
            message += "Location Name : "
                    + ConfigurationDefaultValues.defaultLocationName + "\n";
            message += "Currency : "
                    + ConfigurationDefaultValues.defaultCurrency + "\n";
            message += "Region : " + ConfigurationDefaultValues.defaultRegion
                    + "\n";

            Log.i(LOG_TAG, "Resource URI: " + rep.getUri());
            Log.i(LOG_TAG, "Region: "
                    + ConfigurationDefaultValues.defaultRegion);
            Log.i(LOG_TAG, "Device Name: "
                    + ConfigurationDefaultValues.defaultDeviceName);
            Log.i(LOG_TAG, "Location: "
                    + ConfigurationDefaultValues.defaultLocation);
            Log.i(LOG_TAG, "Location Name: "
                    + ConfigurationDefaultValues.defaultLocationName);
            Log.i(LOG_TAG, "Currency: "
                    + ConfigurationDefaultValues.defaultCurrency);

            // showing the formed message on the UI
            Message msg = Message.obtain();
            msg.what = 0;
            MainActivity mainActivityObj = MainActivity.getMainActivityObject();
            MainActivity.setmessage(message);
            mainActivityObj.getmHandler().sendMessage(msg);
            // TODO Auto-generated method stub
        }

        @Override
        public void onUpdateConfigurationsCallback(
                Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
                int errorValue) {
            Log.i(LOG_TAG, "onUpdateConfigurationsCallback");
        }

        @Override
        public void onGetConfigurationsCallback(
                Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
                int errorValue) {
            Log.i(LOG_TAG, "onGetConfigurationsCallback");
        }
    }

    private class ThingsMaintenanceListener implements
            IThingsMaintenanceListener {

        // Callback Function for Reboot
        @Override
        public void onRebootCallback(Vector<OcHeaderOption> headerOptions,
                OcRepresentation rep, int errorValue) {
            Log.i(LOG_TAG, "onRebootCallback");
        }

        // Callback Function for FactoryReset
        @Override
        public void onFactoryResetCallback(
                Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
                int errorValue) {
            Log.i(LOG_TAG, "onFactoryResetCallback");
        }
    }

    // For deleting all the resources
    public void deleteResources() {
        if (null != conResource)
            conResource.deleteResource();
        if (null != mntResource)
            mntResource.deleteResource();
        if (null != factorysetResource)
            factorysetResource.deleteResource();
    }

    private class RequestHandler implements OcPlatform.EntityHandler {

        @Override
        public EntityHandlerResult handleEntity(OcResourceRequest request) {
            Log.i(LOG_TAG, "handleEntity: enter");

            EntityHandlerResult result = EntityHandlerResult.ERROR;
            if (null == request) {
                Log.e(LOG_TAG, "handleEntity: Invalid OcResourceRequest!");
                return result;
            }

            RequestType requestType = request.getRequestType();
            EnumSet<RequestHandlerFlag> requestHandlerFlag = request
                    .getRequestHandlerFlagSet();
            Log.i(LOG_TAG, "prepareResponseForResource: request type: "
                    + requestType.name());
            Log.i(LOG_TAG, "prepareResponseForResource: request for resource: "
                    + request.getResourceUri());

            if (requestHandlerFlag.contains(RequestHandlerFlag.REQUEST)) {
                if (RequestType.GET == requestType) {
                    sendResponse(request);
                } else if (RequestType.PUT == requestType) {
                    OcRepresentation rep = request.getResourceRepresentation();
                    if (null == rep) {
                        Log.e(LOG_TAG,
                                "handleEntity: Invalid resource representation!");
                        return result;
                    }

                    if (request.getResourceUri().equalsIgnoreCase(
                            conResource.getUri())) {
                        conResource.setConfigurationRepresentation(rep);
                    } else if (request.getResourceUri().equalsIgnoreCase(
                            mntResource.getUri())) {

                        String factorysetAtt = rep.getValueString("fr");
                        if (factorysetAtt.equalsIgnoreCase("true")) {
                            conResource.factoryReset();
                        }
                        mntResource.setDiagnosticsRepresentation(rep);
                    }
                    sendResponse(request);
                }
            }

            Log.i(LOG_TAG, "handleEntity: exit");
            return result;
        }
    }

    // For sending response to the client
    private void sendResponse(OcResourceRequest request) {
        Log.i(LOG_TAG, "sendResponse: enter");

        OcResourceResponse response = new OcResourceResponse();
        OcRepresentation rep = null;

        response.setRequestHandle(request.getRequestHandle());
        response.setResourceHandle(request.getResourceHandle());

        if (request.getResourceUri().equalsIgnoreCase(conResource.getUri())) {
            rep = conResource.getConfigurationRepresentation();
        } else if (request.getResourceUri().equalsIgnoreCase(
                mntResource.getUri())) {
            rep = mntResource.getDiagnosticsRepresentation();
        }
        response.setResourceRepresentation(rep, OcPlatform.DEFAULT_INTERFACE);
        response.setErrorCode(200);

        try {
            OcPlatform.sendResponse(response);
        } catch (OcException e) {
            Log.e(LOG_TAG, "sendResponse: OcException occured: " + e.toString());
        }
        Log.i(LOG_TAG, "sendResponse: exit");
    }
}

// Default values for Resources
class ConfigurationDefaultValues {

    // configuration Resource default values

    public static String defaultDeviceName      = new String();
    public static String defaultLocation        = new String();
    public static String defaultLocationName    = new String();
    public static String defaultCurrency        = new String();
    public static String defaultRegion          = new String();

    public static String ConURIPrefix           = "/oic/con";
    public static String ConResourceTypePrefix  = "oic.wk.con";

    // Diagnostics Resource default values
    public static String diagURIPrefix          = "/oic/mnt";
    public static String diagResourceTypePrefix = "oic.wk.mnt";
    public static String diagnosticsValue       = "false";
    public static String defaultFactoryReset    = "false";
    public static String defaultReboot          = "false";
    public static String defaultStartCollection = "false";
}
