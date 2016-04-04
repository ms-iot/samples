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

import java.util.EnumSet;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import org.iotivity.base.ObserveType;
import org.iotivity.base.OcException;
import org.iotivity.base.OcHeaderOption;
import org.iotivity.base.OcPlatform;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResource;
import org.iotivity.base.ResourceProperty;
import org.iotivity.base.OcResource.OnObserveListener;
import org.iotivity.base.OcResourceHandle;
import org.iotivity.service.tm.Action;
import org.iotivity.service.tm.ActionSet;
import org.iotivity.service.tm.Capability;
import org.iotivity.service.tm.OCStackResult;
import org.iotivity.service.tm.GroupManager;
import org.iotivity.service.tm.GroupManager.*;
import org.iotivity.service.tm.Time.ActionSetType;

import android.os.Message;
import android.util.Log;

/**
 * For calling the group APIs as per user selection on UI and for updating the
 * UI
 */
public class GroupClient {

    private static final String        LOG_TAG             = "[TMSample] GroupClient";
    private static Message             msg;
    public String                      logMessage;

    private final String               groupResourceType   = "b.collection";
    private final String               groupResourceURI    = "/b/collection";

    private final GroupManager         groupManagerObj;
    private final ActionListener       actionListener;
    private final ObserveListener      observeListener;
    private OcResource                 groupResource;
    private OcResourceHandle           groupResourceHandle;
    private OcResourceHandle           foundLightHandle;
    private static GroupApiActivity    groupApiActivityObj = null;
    public static Vector<String>       lights              = new Vector<String>();
    public static Vector<String>       bookmarks           = new Vector<String>();
    public static boolean              groupFound          = false;

    /**
     * Listener for receiving observe notifications.
     */
    private class ObserveListener implements OnObserveListener {
        @Override
        public void onObserveCompleted(List<OcHeaderOption> headerOptionList,
                OcRepresentation ocRepresentation, int sequenceNumber) {
            Log.i(LOG_TAG, "onObserveCompleted invoked");
            if (0 == ocRepresentation.getValueInt("level")) {
                createActionSetBulbOn();
                executeActionSetBulbOn(0);
            } else if (5 == ocRepresentation.getValueInt("level")) {
                createActionSetBulbOff();
                executeActionSetBulbOff(0);
            }
        }

        @Override
        public void onObserveFailed(Throwable arg0) {
        }
    }

    /**
     * Listener for receiving group resource , Light resource and Observe resources discovered in
     * network.
     */
    private class FindCadidateResourceListener implements
            IFindCandidateResourceListener {

        @Override
        public void onResourceFoundCallback(Vector<OcResource> resources) {
            // TODO Auto-generated method stub
            Log.i(LOG_TAG, "onResourceCallback invoked");

            if (resources != null) {
                for (int i = 0; i < resources.size(); i++) {
                    Log.d(LOG_TAG, "Resource information");
                    OcResource ocResource = resources.get(i);
                    String resourceURI = ocResource.getUri();
                    String hostAddress = ocResource.getHost();

                    logMessage = "API RESULT : " + "OC_STACK_OK" + "\n";
                    logMessage += "URI: " + resourceURI + "\n";
                    logMessage += "Host:" + hostAddress;
                    GroupApiActivity.setMessageLog(logMessage);
                    msg = Message.obtain();
                    msg.what = 1;
                    groupApiActivityObj.getHandler().sendMessage(msg);
                    if (resourceURI.equals("/b/collection") == true) {

                        if(!groupFound)
                        {
                            Log.d(LOG_TAG, "Group Found URI : " + resourceURI);
                            Log.d(LOG_TAG, "Group Foundk HOST : " + hostAddress);

                            groupResource = ocResource;
                            groupFound = true;
                            Message msg = Message.obtain();
                            msg.what = 0;
                            groupApiActivityObj.getHandler().sendMessage(msg);

                            logMessage = "onGroupFind" + "\n";
                            logMessage += "URI : " + resourceURI + "\n";
                            logMessage += "Host :" + hostAddress;
                            GroupApiActivity.setMessageLog(logMessage);
                            msg = Message.obtain();
                            msg.what = 1;
                            groupApiActivityObj.getHandler().sendMessage(msg);
                        }else{
                            Log.d(LOG_TAG, "Group Already found ");
                        }

                    }else if (resourceURI.equals("/a/light") == true) {
                        if (lights.contains((hostAddress + resourceURI)) == false) {
                            lights.add((hostAddress + resourceURI));
                            if (groupApiActivityObj != null) {

                                logMessage = "API RESULT : " + "OC_STACK_OK"
                                        + "\n";
                                logMessage += "URI: " + resourceURI + "\n";
                                logMessage += "Host:" + hostAddress;
                                GroupApiActivity.setMessageLog(logMessage);
                                msg = Message.obtain();
                                msg.what = 1;
                                groupApiActivityObj.getHandler().sendMessage(
                                        msg);
                                try {
                                    foundLightHandle = OcPlatform.registerResource(ocResource);
                                    Log.d(LOG_TAG, "Platform registeration done");
                                } catch (OcException e) {
                                    // TODO Auto-generated catch block
                                    e.printStackTrace();
                                }

                                try {
                                    OcPlatform.bindResource(groupResourceHandle, foundLightHandle);
                                    Log.d(LOG_TAG, "Bind resource done");
                                } catch (OcException e) {
                                    // TODO Auto-generated catch block
                                    e.printStackTrace();
                                }
                            }
                        } else {
                            Log.i(LOG_TAG, "Resource is already registered!");
                        }
                    } else if (resourceURI.equalsIgnoreCase("/core/bookmark")) {
                        if (bookmarks.contains((hostAddress + resourceURI)) == false) {
                            bookmarks.add((hostAddress + resourceURI));
                            if (groupApiActivityObj != null) {
                                logMessage = "API RESULT : " + "OC_STACK_OK"
                                        + "\n";
                                logMessage += "URI: " + resourceURI + "\n";
                                logMessage += "Host:" + hostAddress;
                                GroupApiActivity.setMessageLog(logMessage);
                                msg = Message.obtain();
                                msg.what = 1;
                                groupApiActivityObj.getHandler().sendMessage(
                                        msg);

                            }
                            observe(ocResource);
                        }
                    }
                }
            }

        }
    };

    private final FindCadidateResourceListener findCandidateResourceListener;

    public GroupClient() {
        groupManagerObj = new GroupManager();
        actionListener = new ActionListener();
        observeListener = new ObserveListener();
        findCandidateResourceListener = new FindCadidateResourceListener();

        groupManagerObj
                .setFindCandidateResourceListener(findCandidateResourceListener);
        groupManagerObj.setActionListener(actionListener);

        groupApiActivityObj = GroupApiActivity.getGroupApiActivityObj();
    }

    /**
     * This method creates group of the type "b.collection" handling light
     * resources.
     */
    public void createGroup() {
        groupFound = false;
        try {
            groupResourceHandle = OcPlatform.registerResource(
                    groupResourceURI,
                    groupResourceType,
                    OcPlatform.BATCH_INTERFACE, null, EnumSet.of(
                            ResourceProperty.DISCOVERABLE));
        } catch (OcException e) {
            Log.e(LOG_TAG, "go exception");
            Log.e(LOG_TAG, "RegisterResource error. " + e.getMessage());
        }

        try {
            OcPlatform.bindInterfaceToResource(groupResourceHandle, OcPlatform.GROUP_INTERFACE);
        } catch (OcException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        try {
            OcPlatform.bindInterfaceToResource(groupResourceHandle, OcPlatform.DEFAULT_INTERFACE);
        } catch (OcException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    /**
     * This method finds the group of type "b.collection".
     */
    public void findGroup() {
        Log.d(LOG_TAG, "finding group");

        Vector<String> types = new Vector<String>();
        types.add(groupResourceType);
        OCStackResult result = groupManagerObj.findCandidateResources(types, 5);
        if (OCStackResult.OC_STACK_OK != result) {
            Log.e(LOG_TAG,
                    "findCandidateResources returned error: " + result.name());
        }

        logMessage = "API RESULT : " + result.toString();
        GroupApiActivity.setMessageLog(logMessage);
        logMessage = "";
        if (groupApiActivityObj != null) {
            msg = Message.obtain();
            msg.what = 1;
            groupApiActivityObj.getHandler().sendMessage(msg);
        }
    }

    /**
     * This method finds the light resources of type "core.light".
     */
    public void findLightResources() {
        Log.d(LOG_TAG, "finding light resources");

        Vector<String> types = new Vector<String>();
        types.add("core.light");
        OCStackResult result = groupManagerObj.findCandidateResources(types, 5);
        if (OCStackResult.OC_STACK_OK != result) {
            Log.e(LOG_TAG,
                    "findCandidateResources returned error: " + result.name());
        }
    }

    /**
     * This method finds the bookmark resources of type "core.bookmark".
     */
    public void findBookMarkResources() {
        Log.d(LOG_TAG, "finding bookmark resources");

        Vector<String> types = new Vector<String>();
        types.add("core.bookmark");
        OCStackResult result = groupManagerObj.findCandidateResources(types, 5);
        if (OCStackResult.OC_STACK_OK != result) {
            Log.e(LOG_TAG,
                    "findCandidateResources returned error: " + result.name());
        }
    }

    /**
     * This method creates the action set for bulb on action.
     */
    public void createActionSetBulbOn() {
        Log.i(LOG_TAG, "creating action set for bulb on action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        ActionSet actionSet = new ActionSet();
        actionSet.actionsetName = "AllBulbOn";

        // Create actions list
        for (int i = 0; i < lights.size(); i++) {
            Action action = new Action();
            action.target = lights.get(i);

            Capability capability = new Capability();
            capability.capability = "power";
            capability.status = "on";

            action.listOfCapability.add(capability);
            actionSet.listOfAction.add(action);
        }

        try {
            OCStackResult result = groupManagerObj.addActionSet(groupResource,
                    actionSet);
            if (OCStackResult.OC_STACK_OK != result) {
                Log.e(LOG_TAG, "addActionSet returned error: " + result.name());
                return;
            }
        } catch (OcException e) {
            e.printStackTrace();
            return;
        }
    }

    /**
     * This method creates the action set for bulb off action.
     */
    public void createActionSetBulbOff() {
        Log.i(LOG_TAG, "creating action set for bulb off action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        ActionSet actionSet = new ActionSet();
        actionSet.actionsetName = "AllBulbOff";

        // Create actions list
        for (int i = 0; i < lights.size(); i++) {
            Action action = new Action();
            action.target = lights.get(i);

            Capability capability = new Capability();
            capability.capability = "power";
            capability.status = "off";

            action.listOfCapability.add(capability);
            actionSet.listOfAction.add(action);
        }

        try {
            OCStackResult result = groupManagerObj.addActionSet(groupResource,
                    actionSet);
            if (OCStackResult.OC_STACK_OK != result) {
                Log.e(LOG_TAG, "addActionSet returned error: " + result.name());
                return;
            }
        } catch (OcException e) {
            e.printStackTrace();
            return;
        }
    }

    /**
     * This method creates the recursive action set for bulb on action.
     */
    public void createRecursiveActionSetBulbOn() {
        Log.i(LOG_TAG, "creating recursive action set for bulb on action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        ActionSet actionSet = new ActionSet();
        actionSet.actionsetName = "AllBulbOnRecursive";
        actionSet.setType(ActionSetType.RECURSIVE);
        actionSet.mYear = 0;
        actionSet.mMonth = 0;
        actionSet.mDay = 0;
        actionSet.mHour = 0;
        actionSet.mMin = 0;
        actionSet.mSec = 5;
        actionSet.setDelay(actionSet.getSecAbsTime());

        // Create actions list
        for (int i = 0; i < lights.size(); i++) {
            Action action = new Action();
            action.target = lights.get(i);

            Capability capability = new Capability();
            capability.capability = "power";
            capability.status = "on";

            action.listOfCapability.add(capability);
            actionSet.listOfAction.add(action);
        }

        try {
            OCStackResult result = groupManagerObj.addActionSet(groupResource,
                    actionSet);
            if (OCStackResult.OC_STACK_OK != result) {
                Log.e(LOG_TAG, "addActionSet returned error: " + result.name());
                return;
            }
        } catch (OcException e) {
            e.printStackTrace();
            return;
        }
    }

    /**
     * This method creates the scheduled action set for bulb off action.
     */
    public void createScheduledActionSetBulbOff(long delay) {
        Log.i(LOG_TAG, "creating scheduled action set for bulb off action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        ActionSet actionSet = new ActionSet();
        actionSet.actionsetName = "AllBulbOffScheduled";
        actionSet.setType(ActionSetType.SCHEDULED);
        actionSet.setDelay(delay);
        Log.i(LOG_TAG, "Set the delay of " + delay + " seconds");

        // Create actions list
        for (int i = 0; i < lights.size(); i++) {
            Action action = new Action();
            action.target = lights.get(i);

            Capability capability = new Capability();
            capability.capability = "power";
            capability.status = "off";

            action.listOfCapability.add(capability);
            actionSet.listOfAction.add(action);
        }

        try {
            OCStackResult result = groupManagerObj.addActionSet(groupResource,
                    actionSet);
            if (OCStackResult.OC_STACK_OK != result) {
                Log.e(LOG_TAG, "addActionSet returned error: " + result.name());
                return;
            }
        } catch (OcException e) {
            e.printStackTrace();
            return;
        }
    }

    /**
     * This method is for executing the action Set "AllBulbOn".
     */
    public void executeActionSetBulbOn(long delay) {
        Log.i(LOG_TAG, "executing the action set of bulb on action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        executeActionSet("AllBulbOn", delay);
    }

    /**
     * This method is for executing the action Set "AllBulbOff".
     */
    public void executeActionSetBulbOff(long delay) {
        Log.i(LOG_TAG, "executing the action set of bulb off action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        executeActionSet("AllBulbOff", delay);
    }

    /**
     * This method is for executing the recursive action Set
     * "AllBulbOnRecursive".
     */
    public void executeRecursiveActionSetBulbOn(long delay) {
        Log.i(LOG_TAG, "executing the recursive action set of bulb on action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        executeActionSet("AllBulbOnRecursive", delay);
    }

    /**
     * This method is for executing the schedule action Set
     * "AllBulbOffScheduled".
     */
    public void executeScheduledActionSetBulbOff(long delay) {
        Log.i(LOG_TAG, "executing the schedule action set of bulb off action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        executeActionSet("AllBulbOffScheduled", delay);
    }

    /**
     * This method is for canceling the action Set "AllBulbOn".
     */
    public void cancelActionSetBulbOn() {
        Log.i(LOG_TAG, "cancelling the action set of bulb on action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        cancelActionSet("AllBulbOn");
    }

    /**
     * This method is for canceling the action Set "AllBulbOff".
     */
    public void cancelActionSetBulbOff() {
        Log.i(LOG_TAG, "cancelling the action set of bulb off action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        cancelActionSet("AllBulbOff");
    }

    /**
     * This method is for canceling the recursive action Set
     * "AllBulbOnRecursive".
     */
    public void cancelRecursiveActionSetBulbOn() {
        Log.i(LOG_TAG, "cancelling the recursive action set of bulb on action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        cancelActionSet("AllBulbOnRecursive");
    }

    /**
     * This method is for canceling the scheduled action Set
     * "AllBulbOffScheduled".
     */
    public void cancelScheduledActionSetBulbOff() {
        Log.i(LOG_TAG, "cancelling the scheduled action set of bulb off action");

        if (lights.size() == 0) {
            groupApiActivityObj
                    .displayToastMessage("No Light server found in network!");
            return;
        }

        cancelActionSet("AllBulbOffScheduled");
    }

    /**
     * This method is for getting the action Set "AllBulbOn".
     */
    public void getActionSetBulbOn() {
        Log.i(LOG_TAG, "getting the action set of bulb on action");

        try {
            OCStackResult result = groupManagerObj.getActionSet(groupResource,
                    "AllBulbOn");
            if (OCStackResult.OC_STACK_OK != result) {
                Log.e(LOG_TAG,
                        "getActionSetOn returned error: " + result.name());
                return;
            }
        } catch (OcException e) {
            e.printStackTrace();
        }
    }

    /**
     * This method is for getting the action Set "AllBulbOff".
     */
    public void getActionSetBulbOff() {
        Log.i(LOG_TAG, "getting the action set of bulb off action");

        try {
            OCStackResult result = groupManagerObj.getActionSet(groupResource,
                    "AllBulbOff");
            if (OCStackResult.OC_STACK_OK != result) {
                Log.e(LOG_TAG,
                        "getActionSetOn returned error: " + result.name());
                return;
            }
        } catch (OcException e) {
            e.printStackTrace();
        }
    }

    /**
     * This method is for deleting the action Set "AllBulbOn".
     */
    public void deleteActionSetBulbOn() {
        Log.i(LOG_TAG, "deleting the action set of bulb on action");

        try {
            OCStackResult result = groupManagerObj.deleteActionSet(
                    groupResource, "AllBulbOn");
            if (OCStackResult.OC_STACK_OK != result) {
                Log.e(LOG_TAG,
                        "deleteActionSet returned error : " + result.name());
                return;
            }
        } catch (OcException e) {
            e.printStackTrace();
        }
    }

    /**
     * This method is for deleting the action Set "AllBulbOff".
     */
    public void deleteActionSetBulbOff() {
        Log.i(LOG_TAG, "deleting the action set of bulb off action");

        try {
            OCStackResult result = groupManagerObj.deleteActionSet(
                    groupResource, "AllBulbOff");
            if (OCStackResult.OC_STACK_OK != result) {
                Log.e(LOG_TAG,
                        "deleteActionSet returned error : " + result.name());
                return;
            }
        } catch (OcException e) {
            e.printStackTrace();
        }
    }

    /**
     * This method is for observing the bulb on/off status.
     */
    public void observe(OcResource resource) {
        Log.i(LOG_TAG, "Registering observer for bookmark resources status");

        Map<String, String> queryMap = new HashMap<String, String>();
        try {
            resource.observe(ObserveType.OBSERVE, queryMap, observeListener);
        } catch (OcException e) {
            e.printStackTrace();
        }
    }

    private void executeActionSet(String actonSetName, long delay) {
        try {
            OCStackResult result = groupManagerObj.executeActionSet(
                    groupResource, actonSetName, delay);
            if (OCStackResult.OC_STACK_OK != result) {
                Log.e(LOG_TAG,
                        "executeActionSet retuned error : " + result.name());
                return;
            }
        } catch (OcException e) {
            e.printStackTrace();
        }
    }

    private void cancelActionSet(String actionSetName) {
        try {
            OCStackResult result = groupManagerObj.cancelActionSet(
                    groupResource, actionSetName);
            if (OCStackResult.OC_STACK_OK != result) {
                Log.e(LOG_TAG,
                        "cancelActionSet returned error : " + result.name());
                return;
            }
        } catch (OcException e) {
            e.printStackTrace();
        }
    }
}