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
 * for things maintenance.
 */

package org.iotivity.service.tm;

import java.util.Vector;

import org.iotivity.base.OcException;
import org.iotivity.base.OcHeaderOption;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResource;

import android.util.Log;

/**
 * This class provides a set of APIs relating to Things Maintenance.
 */
public class ThingsMaintenance {

    private IThingsMaintenanceListener thingsMaintenanceListener;
    private static ThingsMaintenance   s_thingsMaintenanceInstance;
    private final String               LOG_TAG = this.getClass()
                                                       .getSimpleName();

    // Native methods
    private static native int nativeReboot(OcResource resource);

    private static native int nativeFactoryReset(OcResource resource);

    private static native String nativeGetListOfSupportedMaintenanceUnits();

    /**
     * Function for Getting instance of ThingsMaintenance object.
     *
     * @return ThingsMaintenance instance.
     */
    public static ThingsMaintenance getInstance() {
        if (null == s_thingsMaintenanceInstance) {
            s_thingsMaintenanceInstance = new ThingsMaintenance();
        }
        return s_thingsMaintenanceInstance;
    }

    /**
     * Provides interface for receiving asynchronous response for Things
     * Maintenance feature APIs.
     */
    public interface IThingsMaintenanceListener {
        /**
         * Asynchronous response callback for reboot API.
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
        public void onRebootCallback(Vector<OcHeaderOption> headerOptions,
                OcRepresentation rep, int errorValue);

        /**
         * Asynchronous response callback for factoryReset API.
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
        public void onFactoryResetCallback(
                Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
                int errorValue);
    }

    /**
     * Set listener for receiving asynchronous response for Things Maintenance
     * APIs.
     *
     * @param listener
     *            IThingsMaintenanceListener to receive asynchronous response
     *            for diagnostic feature APIs.
     */
    public void setThingsMaintenanceListener(IThingsMaintenanceListener listener) {
        thingsMaintenanceListener = listener;
    }

    /**
     * API to is used to send a request to a server(thing or device) to be
     * rebooted. On receiving the request, the server attempts to reboot itself
     * in a deterministic time. The target thing could be a group of multiple
     * things or a single thing.
     * <p>
     * Listener should be set using setDiagnosticsListener API.
     * <p>
     * Listener IThingsMaintenanceListener::onRebootCallback will be notified
     * when the response arrives.
     *
     * @param resource
     *            resource pointer representing the target group
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     *
     * @throws OcException {@link OcException}
     */
    public OCStackResult reboot(OcResource resource) throws OcException {

        OCStackResult result;
        if (null == thingsMaintenanceListener) {
            result = OCStackResult.OC_STACK_LISTENER_NOT_SET;
        } else {
            int ordinal = nativeReboot(resource);
            result = OCStackResult.conversion(ordinal);
        }
        return result;
    }

    /**
     * API to restore all configuration parameters to default one on
     * thing(device). All configuration parameters refers Configuration
     * resource, which they could have been modified for various reasons (e.g.,
     * for a request to update a value). If developers on the client want to
     * restore the parameters, just use the factoryReset function.The target
     * thing could be a group of multiple things or a single thing.
     *
     * <p>
     * Listener should be set using setDiagnosticsListener API.
     * <p>
     * Listener IThingsMaintenanceListener::onFactoryResetCallback will be
     * notified when the response arrives.
     *
     * @param resource
     *            resource pointer representing the target group
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     *
     * @throws OcException {@link OcException}
     */
    public OCStackResult factoryReset(OcResource resource) throws OcException {

        OCStackResult result;
        if (null == thingsMaintenanceListener) {
            result = OCStackResult.OC_STACK_LISTENER_NOT_SET;
        } else {
            int ordinal = nativeFactoryReset(resource);
            result = OCStackResult.conversion(ordinal);
        }
        return result;
    }

    /**
     * This callback method is called when a response for the reboot request
     * just arrives.
     *
     * @param headerOptions
     *            It comprises of optionID and optionData as members.
     * @param rep
     *            Configuration parameters are carried as a pair of attribute
     *            key and value in a form of OCRepresentation instance.
     * @param errorValue
     *            error code.
     */
    public void onRebootCallback(Vector<OcHeaderOption> headerOptions,
            OcRepresentation rep, int errorValue) {
        if (null != thingsMaintenanceListener) {
            Log.i("ThingsManagerCallback : onRebootCallback",
                    "Received Callback from JNI");
            thingsMaintenanceListener.onRebootCallback(headerOptions, rep,
                    errorValue);
        }
    }

    /**
     * This callback method is called when a response for the factoryReset
     * request just arrives.
     *
     * @param headerOptions
     *            It comprises of optionID and optionData as members.
     * @param rep
     *            Configuration parameters are carried as a pair of attribute
     *            key and value in a form of OCRepresentation instance.
     * @param errorValue
     *            error code.
     */
    public void onFactoryResetCallback(Vector<OcHeaderOption> headerOptions,
            OcRepresentation rep, int errorValue) {
        if (null != thingsMaintenanceListener) {
            Log.i("ThingsManagerCallback : onFactoryResetCallback",
                    "Received Callback from JNI");
            thingsMaintenanceListener.onFactoryResetCallback(headerOptions,
                    rep, errorValue);
        }
    }

    /**
     * API for getting the list of supported Maintenance units. It shows which
     * Maintenance Names are supported and their brief descriptions. This
     * information is provided in JSON format.
     *
     * @return Returns the configuration list in JSON format.
     */
    public String getListOfSupportedMaintenanceUnits() {
        return nativeGetListOfSupportedMaintenanceUnits();
    }
}
