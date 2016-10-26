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
 * This file contains class which provides functions for
 * things configuration.
 */

package org.iotivity.service.tm;

import java.util.Map;
import java.util.Vector;

import org.iotivity.base.OcException;
import org.iotivity.base.OcHeaderOption;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResource;

import android.util.Log;

/**
 * This class provides a set of APIs relating to Things Configuration.
 */
public class ThingsConfiguration {

    private IConfigurationListener     configurationListener;
    private static ThingsConfiguration s_thingsConfigurationInstance;
    private final String               LOG_TAG = this.getClass()
                                                       .getSimpleName();

    // Native Methods
    private static native int nativeUpdateConfigurations(OcResource resource,
            Map<String, String> configurations);

    private static native int nativeGetConfigurations(OcResource resource,
            Vector<String> configurations);

    private static native String nativeGetListOfSupportedConfigurationUnits();

    private static native int nativeDoBootstrap();

    /**
     * Function for Getting instance of ThingsConfiguration object.
     *
     * @return ThingsConfiguration instance.
     *
     */
    public static ThingsConfiguration getInstance() {
        if (null == s_thingsConfigurationInstance) {
            s_thingsConfigurationInstance = new ThingsConfiguration();
        }
        return s_thingsConfigurationInstance;
    }

    /**
     * Provides interface for receiving asynchronous response for configuration
     * APIs.
     */
    public interface IConfigurationListener {
        /**
         * Asynchronous response callback for doBootstrap API.
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
        public void onBootStrapCallback(Vector<OcHeaderOption> headerOptions,
                OcRepresentation rep, int errorValue);

        /**
         * Asynchronous response callback for updateConfigurations API.
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
        public void onUpdateConfigurationsCallback(
                Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
                int errorValue);

        /**
         * Asynchronous response callback for getConfigurations API.
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
        public void onGetConfigurationsCallback(
                Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
                int errorValue);
    }

    /**
     * Set listener for receiving asynchronous response for configuration APIs.
     *
     * @param listener
     *            IConfigurationListener to receive asynchronous response for
     *            configuration APIs.
     */
    public void setConfigurationListener(IConfigurationListener listener) {
        configurationListener = listener;
    }

    /**
     * API for updating configuration value of multiple things of a target group
     * or a single thing to a resource server(s).
     *
     * <p>
     * Listener should be set using setConfigurationListener API.
     * <p>
     * Listener IConfigurationListener::onUpdateConfigurationsCallback will be
     * notified when the response of update configuration arrives.
     *
     * @param resource
     *            resource representing the target group or the single thing. It
     *            is a pointer of resource instance of Configuration resource.
     *            The resource pointer can be acquired by performing
     *            findResource() function with a dedicated resource type,
     *            "oic.con". Note that, the resource pointer represents not only
     *            a single simple resource but also a collection resource
     *            composing multiple simple resources. In other words, using
     *            these APIs, developers can send a series of requests to
     *            multiple things by calling the corresponding function at once.
     * @param configurations
     *            ConfigurationUnit: a nickname of attribute of target resource
     *            (e.g., installed location, currency, (IP)address) Value : a
     *            value to be updated. It represents an indicator of which
     *            resource developers want to access and which value developers
     *            want to update. Basically, developers could use a resource URI
     *            to access a specific resource but a resource URI might be hard
     *            for all developers to memorize lots of URIs, especially in the
     *            case of long URIs. To relieve the problem, Things
     *            Configuration introduces a easy-memorizing name, called
     *            ConfigurationName, instead of URI. And ConfigurationValue used
     *            in updateConfiguration() function indicates a value to be
     *            updated. Note that, only one configuration parameter is
     *            supported in this release. Multiple configuration parameters
     *            will be supported in future release.
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     *
     * @throws OcException {@link OcException}
     */
    public OCStackResult updateConfigurations(OcResource resource,
            Map<String, String> configurations) throws OcException {
        OCStackResult result;
        if (null == configurationListener) {
            result = OCStackResult.OC_STACK_LISTENER_NOT_SET;
        } else {
            int ordinal = nativeUpdateConfigurations(resource, configurations);
            result = OCStackResult.conversion(ordinal);
        }
        return result;
    }

    /**
     * API for getting configuration value of multiple things of a target group
     * or a single thing.To get a value, users need to know a Configuration Name
     * indicating the target resource. In this release, the Configuration Name
     * is "configuration".An update value is not needed. After that, users store
     * them in form of a Vector and then use a getConfigurations() function.
     *
     * @param resource
     *            resource representing the target group or the single thing.
     * @param configurations
     *            ConfigurationUnit: a nickname of attribute of target resource.
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     *
     * @throws OcException {@link OcException}
     */
    public OCStackResult getConfigurations(OcResource resource,
            Vector<String> configurations) throws OcException {
        OCStackResult result;
        if (null == configurationListener) {
            result = OCStackResult.OC_STACK_LISTENER_NOT_SET;
        } else {
            int ordinal = nativeGetConfigurations(resource, configurations);
            result = OCStackResult.conversion(ordinal);
        }
        return result;
    }

    /**
     * API for getting the list of supported configuration units (configurable
     * parameters). It shows which Configuration Names are supported and their
     * brief descriptions. This information is provided in JSON format.
     *
     * @return Returns the configuration list in JSON format.
     */
    public String getListOfSupportedConfigurationUnits() {
        return nativeGetListOfSupportedConfigurationUnits();
    }

    /**
     * API for bootstrapping system configuration parameters from a bootstrap
     * server.
     * <p>
     * Listener should be set using setConfigurationListener API.
     * <p>
     * Listener IConfigurationListener::onBootStrapCallback will be notified
     * when the response arrives.
     *
     * @return OCStackResult - OC_STACK_OK on success, otherwise a failure error
     *         code.
     */
    public OCStackResult doBootstrap() {
        if (null == configurationListener) {
            Log.e(LOG_TAG, "doBootstrap: listener not set!");
            return OCStackResult.OC_STACK_LISTENER_NOT_SET;
        }

        OCStackResult result;
        int ordinal = nativeDoBootstrap();
        result = OCStackResult.conversion(ordinal);
        return result;
    }

    // ******** JNI UTILITY FUNCTIONS **********//

    /*
     * This callback method is called from JNI when a response for the
     * updateConfigurations request just arrives.
     */
    public void onUpdateConfigurationsCallback(
            Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
            int errorValue) {
        if (null != configurationListener)
            configurationListener.onUpdateConfigurationsCallback(headerOptions,
                    rep, errorValue);
    }

    /*
     * This callback method is called from JNI when a response for the
     * getConfigurations request just arrives.
     */
    public void onGetConfigurationsCallback(
            Vector<OcHeaderOption> headerOptions, OcRepresentation rep,
            int errorValue) {
        if (null != configurationListener)
            configurationListener.onGetConfigurationsCallback(headerOptions,
                    rep, errorValue);
    }

    /*
     * This callback method is called from JNI when a response for the
     * doBootstrap request just arrives.
     */
    public void onBootStrapCallback(Vector<OcHeaderOption> headerOptions,
            OcRepresentation rep, int errorValue) {
        if (null != configurationListener) {
            Log.i("ThingsManagerCallback : onBootStrapCallback",
                    "Received Callback from JNI");
            configurationListener.onBootStrapCallback(headerOptions, rep,
                    errorValue);
        }
    }
}
