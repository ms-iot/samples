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

/**
 * @file
 * This file contains class which provides functions to specify an attribute key and value
 * of the target resource.
 *
 */

package org.iotivity.service.tm;

import java.util.StringTokenizer;

import android.util.Log;

/**
 * This class needs to be created to specify an attribute key and value of the
 * target resource. The attribute key and value are written in capability and
 * status variables of Capability instance class, respectively. After filling
 * the Capability instance, store it to listOfCapability vector variable of
 * Action instance.
 */
public class Capability {
    /**
     * String Tag for Logging
     */
    private static final String LOG_TAG = "Capability";
    /**
     * Attribute Key of target resource
     */
    public String               capability;
    /**
     * Attribute Value of target resource
     */
    public String               status;

    /**
     * This function generates a Capability String.
     *
     * @return String for a specific Capability.
     */
    public String toString() {
        StringBuilder result = new StringBuilder();
        result.append(capability + "=" + status);
        return result.toString();
    }

    /**
     * This function parses the Capability String.
     *
     * @param capabilityString
     *            Capability in String format.
     *
     * @return Capability class.
     */
    public static Capability toCapability(String capabilityString) {
        StringTokenizer tokenizer = new StringTokenizer(capabilityString, "=");
        if (2 != tokenizer.countTokens()) {
            Log.e(LOG_TAG, "Invalid capability string = " + capabilityString);
            return null;
        }

        Capability result = new Capability();

        result.capability = tokenizer.nextToken();
        result.status = tokenizer.nextToken();
        return result;
    }
}
