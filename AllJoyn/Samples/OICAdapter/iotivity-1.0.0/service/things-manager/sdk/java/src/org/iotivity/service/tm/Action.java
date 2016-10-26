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
 * This file contains class which provides functions to retrieve the Action
 * details from a Segment.
 */

package org.iotivity.service.tm;

import java.util.StringTokenizer;
import java.util.Vector;

import android.util.Log;

/**
 * This class provides functions to retrieve the Action details from a Segment.
 */
public class Action {
    private static final String LOG_TAG          = "Action";
    /**
     * Sub Segment Value
     */
    public String               target;

    /**
     * Capability instance stored in listOfCapability vector variable.
     */
    public Vector<Capability>   listOfCapability = new Vector<Capability>();

    /**
     * This function generates an Action String (Example:
     * uri=coap://10.251.44.228:49858/a/light|power=10)
     *
     * @return String for a specific action.
     */
    public String toString() {
        StringBuilder result = new StringBuilder();

        result.append("uri=" + target + "|");
        for (int i = 0; i < listOfCapability.size(); i++) {
            if (i != 0)
                result.append('|');
            result.append(listOfCapability.elementAt(i).toString());
        }

        return result.toString();
    }

    /**
     * This function parses the Segment value to retrieve sub segments separated
     * by a vertical bar(|): URI and a pair of attribute key and value.
     *
     * @param actionString
     *            Segment String
     *
     * @return Action needed by remote devices as members of a specific group
     */
    public static Action toAction(String actionString) {
        Action result = new Action();
        StringTokenizer tokenizer = new StringTokenizer(actionString, "|");

        boolean actionFlag = false;
        while (tokenizer.hasMoreTokens()) {
            String segment = tokenizer.nextToken();
            if (false == actionFlag) {
                // Parse the action string
                StringTokenizer targetTokenizer = new StringTokenizer(segment,
                        "=");
                if (2 != targetTokenizer.countTokens()
                        || false == targetTokenizer.nextToken()
                                .equalsIgnoreCase("uri")) {
                    Log.e(LOG_TAG, "Invalid action segment = " + segment);
                    return null;
                }

                result.target = targetTokenizer.nextToken();
                actionFlag = true;
            } else {
                // Parse the capability string
                Capability capability = Capability.toCapability(segment);
                if (null == capability) {
                    Log.e(LOG_TAG,
                            "Failed to convert string to Capability class!");
                    return null;
                }

                // Add the parsed capability to list
                result.listOfCapability.add(capability);
            }
        }
        return result;
    }
}
