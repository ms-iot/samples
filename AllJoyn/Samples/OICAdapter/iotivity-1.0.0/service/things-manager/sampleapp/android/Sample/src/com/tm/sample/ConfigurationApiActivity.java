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
package com.tm.sample;

import java.util.ArrayList;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import org.iotivity.base.OcException;
import org.iotivity.base.OcHeaderOption;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResource;
import org.iotivity.base.OcResourceHandle;
import org.iotivity.base.ResourceProperty;
import org.iotivity.service.tm.OCStackResult;
import org.iotivity.service.tm.GroupManager;
import org.iotivity.service.tm.ThingsMaintenance;
import org.iotivity.service.tm.GroupManager.*;
import org.iotivity.service.tm.ThingsConfiguration;
import org.iotivity.service.tm.ThingsConfiguration.*;
import org.iotivity.service.tm.ThingsMaintenance.*;

import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

/*
 * Activity for Handling all Configuration Apis as per user's selection on the UI.
 * and updating of UI
 */
public class ConfigurationApiActivity extends Activity {

    private class ResourceInformation {
        OcResource       resource       = null;
        OcResourceHandle resourceHandle = null;
    }

    private final String                     LOG_TAG                                = "[TMSample] "
                                                                                            + this.getClass()
                                                                                                    .getSimpleName();

    private final String                     CONFIGURATION_COLLECTION_RESOURCE_URI  = "/core/configuration/resourceset";
    private final String                     CONFIGURATION_COLLECTION_RESOURCE_TYPE = "core.configuration.resourceset";
    private final String                     CONFIGURATION_RESOURCE_TYPE            = "oic.wk.con";
    private final String                     MAINTENANCE_COLLECTION_RESOURCE_URI    = "/core/maintenance/resourceset";
    private final String                     MAINTENANCE_COLLECTION_RESOURCE_TYPE   = "core.maintenance.resourceset";
    private final String                     MAINTENANCE_RESOURCE_TYPE              = "oic.wk.mnt";
    private final String                     FACTORYSET_COLLECTION_RESOURCE_URI     = "/core/factoryset/resourceset";
    private final String                     FACTORYSET_COLLECTION_RESOURCE_TYPE    = "core.factoryset.resourceset";
    private final String                     FACTORYSET_RESOURCE_TYPE               = "factoryset";

    private final String                     CONFIGURATION_RESOURCE_URI             = "/oic/con";
    private final String                     MAINTENANCE_RESOURCE_URI               = "/oic/mnt";
    private final String                     FACTORYSET_RESOURCE_URI                = "/factoryset";

    private ListView                         list;
    private ArrayAdapter<String>             configurationApis;
    private ArrayList<String>                configurationApisList;

    private static int                       messageCount                           = 0;
    private static EditText                  logs;
    private static String                    logMessage                             = "";
    private static Handler                   mHandler;
    private static Message                   msg;

    private GroupManager                     groupManager                           = null;
    private ThingsConfiguration              thingsConfiguration                    = null;
    private ThingsMaintenance                thingsMaintenance                      = null;
    private Map<String, ResourceInformation> resourceList                           = null;
    private Map<String, ResourceInformation> collectionList                         = null;

    public boolean                           configurationResourceFlag              = false;
    public boolean                           factorysetResourceFlag                 = false;
    public boolean                           maintenanceResourceFlag                = false;

    public static Context                    mcontext;
    public String                            region                                 = "";
    public boolean                           findGroupPressedFlag                   = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.configapis);

        mcontext = this;
        groupManager = new GroupManager();
        thingsConfiguration = ThingsConfiguration.getInstance();
        thingsMaintenance = ThingsMaintenance.getInstance();

        // set the listeners
        setResourceListener();
        setConfigurationListener();
        setMaintenanceListener();

        // Create API menu list
        configurationApisList = new ArrayList<String>();

        logs = (EditText) findViewById(R.id.EditText);

        list = (ListView) findViewById(R.id.configApisList);
        configurationApisList.add("Find All Groups");
        configurationApisList.add("Find All Resources");
        configurationApisList.add("Get a Configuration Resource");
        configurationApisList.add("Update Attribute (Region)");
        configurationApisList.add("Factory Reset");
        configurationApisList.add("Reboot");
        configurationApisList.add("Get Supported Configuration Units");
        configurationApis = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, configurationApisList);
        list.setAdapter(configurationApis);

        // setting the Listener for calling the APIs as per User selection
        list.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> parent, View view,
                    int position, long id) {

                // Find All Groups
                if (position == 0) {
                    Vector<String> resourceTypes = new Vector<String>();
                    resourceTypes.add(CONFIGURATION_COLLECTION_RESOURCE_TYPE);
                    findCandidateResources(resourceTypes);

                    logMessage = "";
                    messageCount = 0;
                    messageCount++;

                    resourceTypes.clear();
                    resourceTypes.add(MAINTENANCE_COLLECTION_RESOURCE_TYPE);
                    findCandidateResources(resourceTypes);

                    messageCount++;

                    resourceTypes.clear();
                    resourceTypes.add(FACTORYSET_COLLECTION_RESOURCE_TYPE);
                    findCandidateResources(resourceTypes);

                    messageCount++;

                } else if (position == 1) { // Find All Resources
                    if (false == findGroupPressedFlag) {
                        displayToastMessage("Configuration collection resource does not exist!");
                    } else {
                        Vector<String> resourceTypes = new Vector<String>();
                        resourceTypes.add(CONFIGURATION_RESOURCE_TYPE);
                        findCandidateResources(resourceTypes);

                        logMessage = "";
                        messageCount = 0;
                        messageCount++;

                        resourceTypes.clear();
                        resourceTypes.add(MAINTENANCE_RESOURCE_TYPE);
                        findCandidateResources(resourceTypes);

                        messageCount++;

                        resourceTypes.clear();
                        resourceTypes.add(FACTORYSET_RESOURCE_TYPE);
                        findCandidateResources(resourceTypes);

                        messageCount++;
                    }
                } else if (position == 2) { // Get Configuration
                    getConfiguration();
                } else if (position == 3) { // Update Attribute (Region)
                    userInputDialog();
                } else if (position == 4) { // Factory Reset
                    factoryReset();
                } else if (position == 5) { // Reboot
                    reboot();
                } else if (position == 6) { // Get Supported Configuration Units
                    String configurationUnits;
                    configurationUnits = getListOfSupportedConfigurationUnits();
                    Log.i(LOG_TAG, "Configuration Units:" + configurationUnits);
                    logMessage = configurationUnits;
                    logs.setText("");
                    logs.setText(logMessage);

                }
            }
        });

        resourceList = new HashMap<String, ResourceInformation>();
        collectionList = new HashMap<String, ResourceInformation>();

        try {
            createResourceCollection(CONFIGURATION_COLLECTION_RESOURCE_URI,
                    CONFIGURATION_COLLECTION_RESOURCE_TYPE);
            createResourceCollection(MAINTENANCE_COLLECTION_RESOURCE_URI,
                    MAINTENANCE_COLLECTION_RESOURCE_TYPE);
            createResourceCollection(FACTORYSET_COLLECTION_RESOURCE_URI,
                    FACTORYSET_COLLECTION_RESOURCE_TYPE);
        } catch (OcException e) {
            Log.e(LOG_TAG, "OcException occured! " + e.toString());
        }

        // handler for updating the UI i.e. LogMessage TextBox
        mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case 0:
                        logs.setText("");
                        logs.setText(logMessage);
                        Log.i(LOG_TAG, logMessage);
                }
            }
        };

    }

    private void userInputDialog() {

        ResourceInformation configurationCollection = collectionList
                .get(CONFIGURATION_COLLECTION_RESOURCE_URI);
        if (null == configurationCollection
                || null == configurationCollection.resource) {
            displayToastMessage("Configuration collection resource does not exist!");
        }
        if (false == configurationResourceFlag) {
            displayToastMessage("Configuration resource does not exist!");
        } else {
            final Dialog dialog = new Dialog(mcontext);
            dialog.setContentView(R.layout.userinputforregionvalue);
            dialog.setTitle("Enter the Region Value");

            dialog.setCancelable(false);
            dialog.show();
            Button ok = (Button) dialog.findViewById(R.id.ok);
            Button cancel = (Button) dialog.findViewById(R.id.cancel);

            ok.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {

                    EditText regionValue = (EditText) dialog
                            .findViewById(R.id.region);
                    region = regionValue.getText().toString();
                    if (region.equalsIgnoreCase("")) {
                        String toastmessage = "Please enter the Region Value";
                        displayToastMessage(toastmessage);
                    } else {
                        dialog.dismiss();
                        updateConfiguration(region);
                    }
                }
            });
            cancel.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    dialog.dismiss();
                }
            });
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();

        // unregistering resource from the platform
        deleteResources();
        resourceList.clear();
        resourceList = null;
        collectionList.clear();
        collectionList = null;
    }

    private void setResourceListener() {
        groupManager
                .setFindCandidateResourceListener(new IFindCandidateResourceListener() {

                    @Override
                    public void onResourceFoundCallback(
                            Vector<OcResource> resources) {
                        // TODO Auto-generated method stub
                        Log.i(LOG_TAG, "onResourceCallback: enter");
                        for (int i = 0; i < resources.size(); i++) {
                            OcResource resource = resources.get(i);
                            String uri = resource.getUri();
                            String host = resource.getHost();

                            // Display resource information
                            Log.i(LOG_TAG, "Resource URI:" + uri);
                            Log.i(LOG_TAG, "Resource HOST: " + host);
                            Log.i(LOG_TAG, "Resource types: ");
                            logMessage += "Resource URI : " + uri + "\n";
                            logMessage += "Resource Host : " + host + "\n";

                            List<String> resourcetypes = resource
                                    .getResourceTypes();
                            for (int j = 0; j < resourcetypes.size(); j++) {
                                Log.i(LOG_TAG, resourcetypes.get(j));
                                logMessage += "ResourceType " + (j + 1) + " : "
                                        + resourcetypes.get(j) + "\n";
                            }

                            Log.i(LOG_TAG, "Interface types: ");
                            List<String> interfacetypes = resource
                                    .getResourceInterfaces();
                            for (int j = 0; j < interfacetypes.size(); j++) {
                                Log.i(LOG_TAG, interfacetypes.get(j));
                                logMessage += "interfacetype " + (j + 1)
                                        + " : " + interfacetypes.get(j) + "\n";
                            }

                            try {
                                if (true == uri.contains("/resourceset")) {
                                    collectionFound(resource);
                                } else {
                                    resourceFound(resource);
                                }
                            } catch (OcException e) {
                                Log.e(LOG_TAG,
                                        "OcExcepion occured! " + e.toString());
                            }
                        }
                        if (messageCount == 3) {
                            msg = Message.obtain();
                            msg.what = 0;
                            mHandler.sendMessage(msg);
                        }
                    }
                });
    }

    private void setConfigurationListener() {
        thingsConfiguration
                .setConfigurationListener(new IConfigurationListener() {
                    @Override
                    public void onBootStrapCallback(
                            Vector<OcHeaderOption> headerOptions,
                            OcRepresentation rep, int errorValue) {
                        Log.i(LOG_TAG, "onBootStrapCallback: enter");
                    }

                    @Override
                    public void onUpdateConfigurationsCallback(
                            Vector<OcHeaderOption> headerOptions,
                            OcRepresentation rep, int errorValue) {
                        Log.i(LOG_TAG, "onUpdateConfigurationsCallback: enter");
                        Log.i(LOG_TAG, "Resource URI: " + rep.getUri());
                        if (rep.hasAttribute("n")) {
                            logMessage += "Device Name : "
                                    + rep.getValueString("n") + "\n";
                        }
                        if (rep.hasAttribute("loc")) {
                            logMessage += "Location : "
                                    + rep.getValueString("loc") + "\n";
                        }
                        if (rep.hasAttribute("locn")) {
                            logMessage += "Location Name : "
                                    + rep.getValueString("locn") + "\n";
                        }
                        if (rep.hasAttribute("r")) {
                            logMessage += "Region : " + rep.getValueString("r")
                                    + "\n";
                        }
                        if (rep.hasAttribute("c")) {
                            logMessage += "Currency : "
                                    + rep.getValueString("c") + "\n";
                        }
                    }

                    @Override
                    public void onGetConfigurationsCallback(
                            Vector<OcHeaderOption> headerOptions,
                            OcRepresentation rep, int errorValue) {
                        Log.i(LOG_TAG, "onGetConfigurationsCallback: enter");
                        Log.i(LOG_TAG, "Resource URI: " + rep.getUri());
                        logMessage += "Resource URI : " + rep.getUri() + "\n";

                        if (rep.hasAttribute("n")) {
                            logMessage += "Device Name : "
                                    + rep.getValueString("n") + "\n";
                        }
                        if (rep.hasAttribute("loc")) {
                            logMessage += "Location : "
                                    + rep.getValueString("loc") + "\n";
                        }
                        if (rep.hasAttribute("locn")) {
                            logMessage += "Location Name : "
                                    + rep.getValueString("locn") + "\n";
                        }
                        if (rep.hasAttribute("r")) {
                            logMessage += "Region : " + rep.getValueString("r")
                                    + "\n";
                        }
                        if (rep.hasAttribute("c")) {
                            logMessage += "Currency : "
                                    + rep.getValueString("c") + "\n";
                        }

                        msg = Message.obtain();
                        msg.what = 0;
                        mHandler.sendMessage(msg);
                    }
                });
    }

    private void setMaintenanceListener() {
        thingsMaintenance
                .setThingsMaintenanceListener(new IThingsMaintenanceListener() {

                    @Override
                    public void onRebootCallback(
                            Vector<OcHeaderOption> headerOptions,
                            OcRepresentation rep, int errorValue) {
                        // TODO Auto-generated method stu
                    }

                    @Override
                    public void onFactoryResetCallback(
                            Vector<OcHeaderOption> headerOptions,
                            OcRepresentation rep, int errorValue) {
                        // TODO Auto-generated method stub

                    }
                });
    }

    /**
     * This method find the resources available in network.
     */
    private void findCandidateResources(Vector<String> resourceTypes) {
        OCStackResult result = groupManager.findCandidateResources(
                resourceTypes, 5);
        if (OCStackResult.OC_STACK_OK != result) {
            Log.e(LOG_TAG, "Error while calling findCandidateResources");
            String toastmessage = "findCandidateResources API returned error! ["
                    + result + "]";
            displayToastMessage(toastmessage);
        }
        if (messageCount == 1)
            logMessage += "API RESULT : " + result.toString() + "\n";
    }

    /**
     * This method gets the configuration data from con-server.
     */
    private void getConfiguration() {
        Log.i(LOG_TAG, "There are " + resourceList.size()
                + " servers present in network");
        ResourceInformation configurationCollection = collectionList
                .get(CONFIGURATION_COLLECTION_RESOURCE_URI);
        if (null == configurationCollection
                || null == configurationCollection.resource) {
            displayToastMessage("Configuration collection resource does not exist!");
            return;
        }
        if (false == configurationResourceFlag) {

            displayToastMessage("Configuration resource does not exist!");
            return;
        }

        String name = "all";
        Vector<String> configs = new Vector<String>();
        configs.add(name);

        OCStackResult result = OCStackResult.OC_STACK_ERROR;
        try {
            result = thingsConfiguration.getConfigurations(
                    configurationCollection.resource, configs);
        } catch (OcException e) {
            e.printStackTrace();
        }

        if (OCStackResult.OC_STACK_OK != result) {
            Log.e(LOG_TAG, "getConfigurations failed!");
            String toastmessage = "getConfigurations API returned error! ["
                    + result + "]";
            displayToastMessage(toastmessage);
        }
        logMessage = "API RESULT : " + result.toString() + "\n";
        msg = Message.obtain();
        msg.what = 0;
        mHandler.sendMessage(msg);
    }

    /**
     * This method update the configuration resource region attribute to given
     * value.
     *
     * @param region
     */
    private void updateConfiguration(String region) {
        ResourceInformation configurationCollection = collectionList
                .get(CONFIGURATION_COLLECTION_RESOURCE_URI);
        String name = "r";
        Map<String, String> configurations = new HashMap<String, String>();

        try {
            configurations.put(name, region);
        } catch (Exception e) {
            Log.e(LOG_TAG, "Exception occured! " + e.toString());
        }

        OCStackResult result = OCStackResult.OC_STACK_ERROR;
        try {
            result = thingsConfiguration.updateConfigurations(
                    configurationCollection.resource, configurations);
        } catch (OcException e) {
            e.printStackTrace();
        }
        if (OCStackResult.OC_STACK_OK != result) {
            Log.e(LOG_TAG, "updateConfigurations failed!");
            String toastmessage = "updateConfigurations API returned error! ["
                    + result + "]";
            displayToastMessage(toastmessage);
        }
        logMessage = "API RESULT : " + result.toString() + "\n";
        logMessage += "Updating region to " + region;
        msg = Message.obtain();
        msg.what = 0;
        mHandler.sendMessage(msg);
    }

    /**
     * This method send request to reset all the configuration attribute values
     * to default.
     */
    private void factoryReset() {
        ResourceInformation MaintenanceCollection = collectionList
                .get(MAINTENANCE_COLLECTION_RESOURCE_URI);
        if (null == MaintenanceCollection
                || null == MaintenanceCollection.resource) {
            displayToastMessage("Maintenance collection does not exist!");
            return;
        }

        if (false == maintenanceResourceFlag) {
            displayToastMessage("Maintenance resource does not exist!");
            return;
        }

        OCStackResult result = OCStackResult.values()[30];

        try {
            result = thingsMaintenance
                    .factoryReset(MaintenanceCollection.resource);
        } catch (OcException e) {
            e.printStackTrace();
        }

        if (OCStackResult.OC_STACK_OK != result) {
            Log.e(LOG_TAG, "factoryReset failed!");
            String toastmessage = "factoryReset API returned error! [" + result
                    + "]";
            displayToastMessage(toastmessage);

        }
        logMessage = "API RESULT : " + result.toString() + "\n";
        msg = Message.obtain();
        msg.what = 0;
        mHandler.sendMessage(msg);
    }

    /**
     * This method send request to reboot server.
     */
    private void reboot() {
        ResourceInformation MaintenanceCollection = collectionList
                .get(MAINTENANCE_COLLECTION_RESOURCE_URI);
        if (null == MaintenanceCollection
                || null == MaintenanceCollection.resource) {
            displayToastMessage("Maintenance collection does not exist!");
            return;
        }
        if (false == maintenanceResourceFlag) {
            displayToastMessage("Maintenance resource does not exist!");
            return;
        }

        OCStackResult result = OCStackResult.OC_STACK_ERROR;
        try {
            result = thingsMaintenance.reboot(MaintenanceCollection.resource);
        } catch (OcException e) {
            e.printStackTrace();
        }

        if (OCStackResult.OC_STACK_OK != result) {
            Log.e(LOG_TAG, "reboot failed!");
            String toastmessage = "reboot API returned error! [" + result + "]";
            displayToastMessage(toastmessage);
        }
        logMessage = "API RESULT : " + result.toString() + "\n";
        msg = Message.obtain();
        msg.what = 0;
        mHandler.sendMessage(msg);
    }

    /**
     * This method is for getting list of all supported configuration values.
     * Response will be in JSON format (key-value pair).
     */
    private String getListOfSupportedConfigurationUnits() {
        return thingsConfiguration.getListOfSupportedConfigurationUnits();
    }

    private void displayToastMessage(String message) {
        Toast toast = Toast.makeText(this, message, Toast.LENGTH_SHORT);
        toast.show();
        Log.i(LOG_TAG, message);
    }

    private void collectionFound(OcResource resource) {
        String uri = resource.getUri();
        ResourceInformation resourceInfo = collectionList.get(uri);
        if (null == resourceInfo) {
            Log.e(LOG_TAG, "Collection is not created!");
            return;
        }

        if (null != resourceInfo.resource) {
            Log.e(LOG_TAG, "Collection resource already updated!");
            return;
        }

        resourceInfo.resource = resource;
        if (3 == messageCount) {
            findGroupPressedFlag = true;
        }
    }

    /**
     * This callback will be invoked when the interested resource discovered in
     * network.
     */
    private void resourceFound(OcResource resource) throws OcException {
        String uri = resource.getUri();
        String host = resource.getHost();

        // Check if the resource already exist in the map table
        ResourceInformation resourceInfo = resourceList.get(uri + host);
        if (null != resourceInfo) {
            Log.e(LOG_TAG, "Resource already exists!");
            return;
        }

        // Create resource
        resourceInfo = new ResourceInformation();
        resourceInfo.resource = resource;
        if (uri.equalsIgnoreCase("/oic/con")) {
            ResourceInformation collectionResource = collectionList
                    .get(CONFIGURATION_COLLECTION_RESOURCE_URI);
            if (null == collectionResource
                    || null == collectionResource.resourceHandle) {
                Log.e(LOG_TAG, "Invalid Configuration collection!");
                return;
            }

            ResourceInformation resourceInfoCollection;
            resourceInfoCollection = collectionList
                    .get(CONFIGURATION_COLLECTION_RESOURCE_URI);
            OcResourceHandle handle;
            handle = resourceInfoCollection.resourceHandle;
            resourceInfo.resourceHandle = handle;
            resourceInfo.resourceHandle = groupManager.bindResourceToGroup(
                    resource, handle);

            resourceList.put(uri + host, resourceInfo);
            configurationResourceFlag = true;
        } else if (uri.equalsIgnoreCase("/oic/mnt")) {
            ResourceInformation maintenanceResource = collectionList
                    .get(MAINTENANCE_COLLECTION_RESOURCE_URI);
            if (null == maintenanceResource
                    || null == maintenanceResource.resourceHandle) {
                Log.e(LOG_TAG, "Invalid Configuration collection!");
                return;
            }

            ResourceInformation resourceInfoCollection;
            resourceInfoCollection = collectionList
                    .get(MAINTENANCE_COLLECTION_RESOURCE_URI);
            OcResourceHandle handle;
            handle = resourceInfoCollection.resourceHandle;
            resourceInfo.resourceHandle = handle;
            resourceInfo.resourceHandle = groupManager.bindResourceToGroup(
                    resource, handle);

            resourceList.put(uri + host, resourceInfo);
            maintenanceResourceFlag = true;
        } else if (uri.equalsIgnoreCase("/factoryset")) {
            ResourceInformation factorysetResource = collectionList
                    .get(FACTORYSET_COLLECTION_RESOURCE_URI);
            if (null == factorysetResource
                    || null == factorysetResource.resourceHandle) {
                Log.e(LOG_TAG, "Invalid Configuration collection!");
                return;
            }

            ResourceInformation resourceInfoCollection;
            resourceInfoCollection = collectionList
                    .get(FACTORYSET_COLLECTION_RESOURCE_URI);
            OcResourceHandle handle;
            handle = resourceInfoCollection.resourceHandle;
            resourceInfo.resourceHandle = handle;
            resourceInfo.resourceHandle = groupManager.bindResourceToGroup(
                    resource, handle);

            resourceList.put(uri + host, resourceInfo);
            factorysetResourceFlag = true;
        } else {
            Log.e(LOG_TAG, "Resource is of different type: " + uri);
            return;
        }
    }

    private void createResourceCollection(String uri, String typename)
            throws OcException {

        Map<String, OcResourceHandle> groupList = new HashMap<String, OcResourceHandle>();

        Log.i(LOG_TAG, "createGroup: " + typename);
        // Check if the resource collection is already created
        if (null != collectionList.get(uri)) {
            Log.e(LOG_TAG, "Collection is already exist!");
            return;
        }

        OcResourceHandle resourceHandle = null;

        try {
            resourceHandle = OcPlatform.registerResource(uri, typename,
                    OcPlatform.BATCH_INTERFACE, null,
                    EnumSet.of(ResourceProperty.DISCOVERABLE));
        } catch (OcException e) {
            Log.e(LOG_TAG, "go exception");
            Log.e(LOG_TAG, "RegisterResource error. " + e.getMessage());
        }
        try {
            OcPlatform.bindInterfaceToResource(resourceHandle,
                    OcPlatform.GROUP_INTERFACE);
        } catch (OcException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        try {
            OcPlatform.bindInterfaceToResource(resourceHandle,
                    OcPlatform.DEFAULT_INTERFACE);
        } catch (OcException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        if (null == resourceHandle) {
            Log.e(LOG_TAG, " createResourceCollection : resourceHandle is NULL");

        }
        // Add the collection resource to map table
        ResourceInformation resourceInfo = new ResourceInformation();
        resourceInfo.resourceHandle = resourceHandle;
        collectionList.put(uri, resourceInfo);
        Log.i(LOG_TAG, "size of collectionList : " + collectionList.size());
    }

    private void deleteResources() {
        Log.i(LOG_TAG, "deleteResources: enter");
        try {
            // unregister all resources
            for (ResourceInformation resource : resourceList.values()) {
                if (null != resource.resourceHandle) {
                    if (resource.resource.getUri().equalsIgnoreCase(
                            CONFIGURATION_RESOURCE_URI)) {
                        ResourceInformation collectionResource = collectionList
                                .get(CONFIGURATION_COLLECTION_RESOURCE_URI);
                        if (null != collectionResource
                                && null != collectionResource.resourceHandle) {
                            OcPlatform
                                    .unregisterResource(resource.resourceHandle);
                            Log.i(LOG_TAG, "unregistered resource"
                                    + CONFIGURATION_COLLECTION_RESOURCE_URI);

                        }
                    } else if (resource.resource.getUri().equalsIgnoreCase(
                            MAINTENANCE_RESOURCE_URI)) {
                        ResourceInformation maintenanceResource = collectionList
                                .get(MAINTENANCE_COLLECTION_RESOURCE_URI);
                        if (null != maintenanceResource
                                && null != maintenanceResource.resourceHandle) {
                            OcPlatform
                                    .unregisterResource(resource.resourceHandle);
                            Log.i(LOG_TAG, "unregistered resource"
                                    + CONFIGURATION_COLLECTION_RESOURCE_URI);
                        }
                    } else if (resource.resource.getUri().equalsIgnoreCase(
                            FACTORYSET_RESOURCE_URI)) {
                        ResourceInformation factorysetResource = collectionList
                                .get(FACTORYSET_COLLECTION_RESOURCE_URI);
                        if (null != factorysetResource
                                && null != factorysetResource.resourceHandle) {
                            OcPlatform
                                    .unregisterResource(resource.resourceHandle);
                            Log.i(LOG_TAG, "unregistered resource"
                                    + CONFIGURATION_COLLECTION_RESOURCE_URI);
                        }
                    }
                }
            }

        } catch (OcException e) {
            Log.e(LOG_TAG, "OcException occured! " + e.toString());
        }
    }
}
