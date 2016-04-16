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

import java.util.Vector;

import org.iotivity.base.OcHeaderOption;
import org.iotivity.base.OcRepresentation;
import org.iotivity.service.tm.Action;
import org.iotivity.service.tm.ActionSet;
import org.iotivity.service.tm.Capability;
import org.iotivity.service.tm.GroupManager.*;

import android.os.Message;
import android.util.Log;

/*
 * For getting the put,get,observe and post Callback and updating
 *  the UI i.e. logMessage(TextBox) of GroupApiActivity.
 */
public class ActionListener implements IActionListener {

    private final String     LOG_TAG             = "[TMSample] "
                                                         + this.getClass()
                                                                 .getSimpleName();
    private static Message   msg;
    private String           logMessage;
    private GroupApiActivity groupApiActivityObj = null;

    @Override
    public void onPostResponseCallback(Vector<OcHeaderOption> headerOptions,
            OcRepresentation rep, int errorValue) {
        Log.i(LOG_TAG, "Got Callback : onPostResponseCallback");

        groupApiActivityObj = GroupApiActivity.getGroupApiActivityObj();
        logMessage = "API Result : SUCCESS" + "\n";
        logMessage += "Recieved Callback for called API (OnPostCallback)"
                + "\n";

        // sending message to handler of GroupApiActivity to Update the UI
        GroupApiActivity.setMessageLog(logMessage);
        msg = Message.obtain();
        msg.what = 1;
        groupApiActivityObj.getHandler().sendMessage(msg);
    }

    @Override
    public void onGetResponseCallback(Vector<OcHeaderOption> headerOptions,
            OcRepresentation rep, int errorValue) {
        Log.i(LOG_TAG, "Got Callback : onGetResponseCallback");

        String actionSetStr = rep.getValueString("ActionSet");
        groupApiActivityObj = GroupApiActivity.getGroupApiActivityObj();
        logMessage = "Recieved Callback for called API (onGetResponseCallback)"
                + "\n" + "ActionSet:" + actionSetStr;

        GroupApiActivity.setMessageLog(logMessage);
        // sending message to handler of GroupApiActivity to Update the UI
        msg = Message.obtain();
        msg.what = 1;
        groupApiActivityObj.getHandler().sendMessage(msg);

        if (actionSetStr != null) {
            ActionSet actionSet = ActionSet.toActionSet(actionSetStr);
            if (actionSet != null) {
                System.out.println("ActionSet Name : "
                        + actionSet.actionsetName);
                for (int i = 0; i < actionSet.listOfAction.size(); i++) {
                    Action action = actionSet.listOfAction.get(i);
                    System.out.println("Target : " + action.target);

                    Vector<Capability> listOfCapability = action.listOfCapability;
                    for (int j = 0; j < listOfCapability.size(); j++) {
                        Capability capability = listOfCapability.get(j);
                        System.out.println("Capability : "
                                + capability.capability);
                        System.out.println("Status : " + capability.status);
                    }
                }
            }
        }

        groupApiActivityObj = GroupApiActivity.getGroupApiActivityObj();
        GroupApiActivity.setMessageLog(logMessage);
        msg = Message.obtain();
        msg.what = 1;
        groupApiActivityObj.getHandler().sendMessage(msg);
    }

    @Override
    public void onPutResponseCallback(Vector<OcHeaderOption> headerOptions,
            OcRepresentation rep, int errorValue) {
        Log.i(LOG_TAG, "Got Callback : onPutResponseCallback");

        groupApiActivityObj = GroupApiActivity.getGroupApiActivityObj();
        logMessage = "API Result : SUCCESS" + "\n";
        logMessage += "Recieved Callback for called API (onPutResponseCallback)"
                + "\n";
        GroupApiActivity.setMessageLog(logMessage);
        // sending message to handler of GroupApiActivity to Update the UI
        msg = Message.obtain();
        msg.what = 1;
        groupApiActivityObj.getHandler().sendMessage(msg);
    }
}
