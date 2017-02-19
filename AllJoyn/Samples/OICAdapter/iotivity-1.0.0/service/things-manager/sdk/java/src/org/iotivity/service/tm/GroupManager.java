/* *****************************************************************
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

/**
 * @file
 * This file contains class which provides functions
 * for group management.
 */

package org.iotivity.service.tm;

import java.util.Vector;

import org.iotivity.base.OcException;
import org.iotivity.base.OcHeaderOption;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResource;
import org.iotivity.base.OcResourceHandle;

import android.util.Log;

/**
 * This class provides a set of APIs relating to Group Management.
 */
public class GroupManager {

    private static IFindCandidateResourceListener s_resourceListener;
    private static ISubscribePresenceListener     s_presenceListener;
    private static IActionListener                actionListener;

    private final String                          LOG_TAG = this.getClass()
                                                                  .getSimpleName();

    // Native methods
    private static native int nativeFindCandidateResource(
            Vector<String> resourceTypes, int waitTime);

    private static native int nativeSubscribeCollectionPresence(
            OcResource resource);

    public static native OcResourceHandle nativeBindResourceToGroup(
            OcResource resource, OcResourceHandle collectionHandle);

    public static native int nativeAddActionSet(OcResource resource,
            ActionSet actionSet);

    public static native int nativeExecuteActionSet(OcResource resource,
            String actionsetName);

    public static native int nativeExecuteActionSetWithDelay(
            OcResource resource, String actionsetName, long delay);

    public static native int nativeCancelActionSet(OcResource resource,
            String actionsetName);

    public static native int nativeGetActionSet(OcResource resource,
            String actionsetName);

    public static native int nativeDeleteActionSet(OcResource resource,
            String actionsetName);

    static {
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("oc_logger");
        System.loadLibrary("connectivity_abstraction");
        System.loadLibrary("ca-interface");
        System.loadLibrary("octbstack");
        System.loadLibrary("oc");
        System.loadLibrary("TGMSDKLibrary");
        System.loadLibrary("ocstack-jni");
        System.loadLibrary("things-manager-jni");
    }

    /**
     * Provides interface for getting notification when resources are discovered
     * in network.
     */
    public interface IFindCandidateResourceListener {

        /**
         * This callback method will be called whenever resource is discovered
         * in the network.
         *
         * @param resources
         *            List of resources discovered in the network
         */
        public void onResourceFoundCallback(Vector<OcResource> resources);
    }

    /**
     * Provides interface for getting child resource presence status.
     */
    public interface ISubscribePresenceListener {

        /**
         * This callback method will be called for child resource presence
         * status.
         *
         * @param resource
         *            URI of resource.
         * @param result
         *            error code.
         */
        public void onPresenceCallback(String resource, int result);
    }

    /**
     * Provides interface for receiving the callback for the GET, PUT and POST
     * requested actions.
     */
    public interface IActionListener {

        /**
         * This callback method is called when a asynchronous response for the
         * getActionSet request is received.
         *
         * @param headerOptions
         *            It comprises of optionID and optionData as members.
         * @param rep
         *            Configuration parameters are carried as a pair of
         *            attribute key and value in a form of OCRepresentation
         *            instance.
         * @param errorValue
         *            error code.
         */
        public void onGetResponseCallback(Vector<OcHeaderOption> headerOptions,
                OcRepresentation rep, int errorValue);

        /**
         * This callback method is called when a asynchronous response for the
         * addActionSet request is received.
         *
         * @param headerOptions
         *            It comprises of optionID and optionData as members.
         * @param rep
         *            Configuration parameters are carried as a pair of
         *            attribute key and value in a form of OCRepresentation
         *            instance.
         * @param errorValue
         *            error code.
         */
        public void onPutResponseCallback(Vector<OcHeaderOption> headerOptions,
                OcRepresentation rep, int errorValue);

        /**
         * This callback method is called when a asynchronous response for the
         * executeActionSet or deleteActionSet request is received.
         *
         * @param headerOptions
         *            It comprises of optionID and optionData as members.
         * @param rep
         *            Configuration parameters are carried as a pair of
         *            attribute key and value in a form of OCRepresentation
         *            instance.
         * @param errorValue
         *            error code.
         */
        public void onPostResponseCallback(
                Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
                int errorValue);
    }

    /**
     * Set listener for receiving notification of resource discovery.
     *
     * @param listener
     *            IFindCandidateResourceListener to receive the discovered
     *            resources.
     */
    public void setFindCandidateResourceListener(
            IFindCandidateResourceListener listener) {
        s_resourceListener = listener;
    }

    /**
     * Set listener for receiving responses of Get, PUT and POST requests.
     *
     * @param listener
     *            IActionListener to receive Get, PUT and POST request
     *            responses.
     */
    public void setActionListener(IActionListener listener) {
        actionListener = listener;
    }

    /**
     * Set listener for receiving child resource presence notifications.
     *
     * @param listener
     *            ISubscribePresenceListener to receive child resource presence
     *            state.
     */
    public void setSubscribePresenceListener(ISubscribePresenceListener listener) {
        s_presenceListener = listener;
    }

    /**
     * API for discovering candidate resources with waiting delay. This
     * operation returns all resources of given type on the network service.
     * This operation is sent via multicast to all services. However, the filter
     * limits the responders to just those that support the resource type in the
     * query. Currently only exact matches are supported.
     * <p>
     * Listener should be set using setFindCandidateResourceListener API
     * <p>
     * Listener IFindCandidateResourceListener::onResourceCallback will be
     * notified when resource is discovered in network.
     *
     * @param resourceTypes
     *            required resource types(called "candidate")
     * @param waitTime
     *            Delay time to add in seconds before starting to find the
     *            resources in network.
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     */
    public OCStackResult findCandidateResources(Vector<String> resourceTypes,
            int waitTime) {
        OCStackResult result;
        if (null == s_resourceListener) {
            result = OCStackResult.OC_STACK_LISTENER_NOT_SET;
        } else {
            int ordinal = nativeFindCandidateResource(resourceTypes, waitTime);
            result = OCStackResult.conversion(ordinal);
        }
        return result;
    }

    /**
     * API for subscribing child's state. Listener
     * <p>
     * ISubscribePresenceListener::onPresenceCallback will be notified for
     * resource presence status
     * </p>
     *
     * @param resource
     *            collection resource for subscribing presence of all child
     *            resources
     *
     * @return OCStackResult - return value of this API. It returns OC_STACK_OK
     *         if success.
     *
     * @throws OcException {@link OcException}
     */
    public OCStackResult subscribeCollectionPresence(OcResource resource)
            throws OcException {
        String LOG_TAG = this.getClass().getSimpleName();

        OCStackResult result;
        if (null == s_presenceListener) {
            result = OCStackResult.OC_STACK_LISTENER_NOT_SET;
        } else {

            int ordinal = nativeSubscribeCollectionPresence(resource);
            Log.i(LOG_TAG, "Ordinal value = : " + ordinal);
            result = OCStackResult.conversion(ordinal);
        }
        return result;
    }

    /**
     * API for register and bind resource to group.
     *
     * @param resource
     *            resource for register and bind to group. It has all data.
     * @param collectionHandle
     *            collection resource handle. It will be added child resource.
     *
     * @return OcResourceHandle - Child resource handle.
     *
     * @throws OcException {@link OcException}
     */
    public OcResourceHandle bindResourceToGroup(OcResource resource,
            OcResourceHandle collectionHandle) throws OcException {
        return nativeBindResourceToGroup(resource, collectionHandle);
    }

    /**
     * API for adding a new ActionSet onto a specific resource.
     *
     * <p>
     * Listener should be set using setActionListener API.
     * <p>
     * Listener IActionListener::onPutResponseCallback will be notified when the
     * response of PUT operation arrives.
     *
     * @param resource
     *            resource pointer of the group resource
     * @param actionSet
     *            pointer of ActionSet
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     *
     * @throws OcException {@link OcException}
     */
    public OCStackResult addActionSet(OcResource resource, ActionSet actionSet)
            throws OcException {
        if (null == actionListener) {
            Log.e(LOG_TAG, "addActionSet: listener not set!");
            return OCStackResult.OC_STACK_LISTENER_NOT_SET;
        }
        int ordinal = nativeAddActionSet(resource, actionSet);

        return OCStackResult.conversion(ordinal);
    }

    /**
     * API for executing a specific ActionSet belonging to a specific resource.
     *
     * <p>
     * Listener should be set using setActionListener API.
     * <p>
     * Listener IActionListener::onPostResponseCallback will be notified when
     * the response of POST operation arrives.
     *
     * @param resource
     *            resource pointer of the group resource
     * @param actionsetName
     *            ActionSet name for removing the ActionSet
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     *
     * @throws OcException {@link OcException}
     */
    public OCStackResult executeActionSet(OcResource resource,
            String actionsetName) throws OcException {
        if (null == actionListener) {
            Log.e(LOG_TAG, "executeActionSet: listener not set!");
            return OCStackResult.OC_STACK_LISTENER_NOT_SET;
        }

        int ordinal = nativeExecuteActionSet(resource, actionsetName);

        return OCStackResult.conversion(ordinal);
    }

    /**
     * API for executing a specific ActionSet belonging to a specific resource.
     *
     * <p>
     * Listener should be set using setActionListener API.
     * <p>
     * Listener IActionListener::onPostResponseCallback will be notified when
     * the response of POST operation arrives.
     *
     * @param resource
     *            resource pointer of the group resource
     * @param actionsetName
     *            ActionSet name for removing the ActionSet
     * @param delay
     *            Wait time for the ActionSet execution
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     *
     * @throws OcException {@link OcException}
     */
    public OCStackResult executeActionSet(OcResource resource,
            String actionsetName, long delay) throws OcException {
        if (null == actionListener) {
            Log.e(LOG_TAG, "executeActionSet: listener not set!");
            return OCStackResult.OC_STACK_LISTENER_NOT_SET;
        }

        OCStackResult result;
        int ordinal = nativeExecuteActionSetWithDelay(resource, actionsetName,
                delay);
        result = OCStackResult.conversion(ordinal);

        return result;
    }

    /**
     * API to cancel the existing ActionSet.
     *
     * <p>
     * Listener should be set using setActionListener API.
     * <p>
     * Listener IActionListener::onPostResponseCallback will be notified when
     * the response of POST operation arrives.
     *
     * @param resource
     *            resource pointer of the group resource.
     * @param actionsetName
     *            ActionSet name that has to be cancelled.
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     *
     * @throws OcException {@link OcException}
     */

    public OCStackResult cancelActionSet(OcResource resource,
            String actionsetName) throws OcException {
        if (null == actionListener) {
            Log.e(LOG_TAG, "cancelActionSet: listener not set!");
            return OCStackResult.OC_STACK_LISTENER_NOT_SET;
        }

        OCStackResult result;
        int ordinal = nativeCancelActionSet(resource, actionsetName);
        result = OCStackResult.conversion(ordinal);

        return result;
    }

    /**
     * API to to get an existing ActionSet belonging to a specific resource.
     *
     * <p>
     * Listener should be set using setActionListener API.
     * <p>
     * Listener IActionListener::onPostResponseCallback will be notified when
     * the response of POST operation arrives.
     *
     * @param resource
     *            resource pointer of the group resource
     * @param actionsetName
     *            ActionSet name for removing the ActionSet
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     *
     * @throws OcException {@link OcException}
     */
    public OCStackResult getActionSet(OcResource resource, String actionsetName)
            throws OcException {
        if (null == actionListener) {
            Log.e(LOG_TAG, "getActionSet: listener not set!");
            return OCStackResult.OC_STACK_LISTENER_NOT_SET;
        }

        OCStackResult result;
        int ordinal = nativeGetActionSet(resource, actionsetName);
        result = OCStackResult.conversion(ordinal);

        return result;
    }

    /**
     * API to delete an existing ActionSet belonging to a specific resource.
     *
     * <p>
     * Listener should be set using setActionListener API.
     * <p>
     * Listener IActionListener::onPutResponseCallback will be notified when the
     * response of PUT operation arrives.
     *
     * @param resource
     *            resource pointer of the group resource
     * @param actionsetName
     *            ActionSet name for removing the ActionSet
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     *
     * @throws OcException {@link OcException}
     */
    public OCStackResult deleteActionSet(OcResource resource,
            String actionsetName) throws OcException {
        if (null == actionListener) {
            Log.e(LOG_TAG, "deleteActionSet: listener not set!");
            return OCStackResult.OC_STACK_LISTENER_NOT_SET;
        }

        OCStackResult result;
        int ordinal = nativeDeleteActionSet(resource, actionsetName);
        result = OCStackResult.conversion(ordinal);

        return result;
    }

    // ******** JNI UTILITY FUNCTIONS **********//

    /*
     * This callback method will be called from JNI whenever resource is
     * discovered in the network.
     */
    private static void onResourceFoundCallback(Vector<OcResource> resources) {
        if (null != s_resourceListener)
            s_resourceListener.onResourceFoundCallback(resources);
    }

    /*
     * This callback method is called from JNI for child resource presence
     * status.
     */
    private static void onPresenceCallback(String resource, int errorValue) {
        if (null != s_presenceListener) {
            Log.i("ThingsManagerCallback : onPresenceCallback",
                    "Received Callback from JNI");
            s_presenceListener.onPresenceCallback(resource, errorValue);
        }
    }

    /*
     * This callback method is called from JNI when a response for the
     * executeActionSet or deleteActionSet request just arrives.
     */
    private static void onPostResponseCallback(
            Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
            int errorValue) {
        if (null != actionListener)
            actionListener.onPostResponseCallback(headerOptions, rep,
                    errorValue);
    }

    /*
     * This callback method is called from JNI when a response for the
     * addActionSet request just arrives.
     */
    private static void onPutResponseCallback(
            Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
            int errorValue) {
        if (null != actionListener)
            actionListener
                    .onPutResponseCallback(headerOptions, rep, errorValue);
    }

    /*
     * This callback method is called from JNI when a response for the
     * getActionSet request just arrives.
     */
    private static void onGetResponseCallback(
            Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
            int errorValue) {
        if (null != actionListener)
            actionListener
                    .onGetResponseCallback(headerOptions, rep, errorValue);
    }
}
