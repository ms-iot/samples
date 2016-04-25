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
 * This file contains class which provides functions to retrieve ActionSet from plain text.
 */
package org.iotivity.service.tm;

import java.util.StringTokenizer;
import java.util.Vector;

import android.util.Log;

/**
 * This class provides functions to retrieve ActionSet from plain text. An
 * ActionSet is a set of descriptions of actions needed by remote devices as
 * members of a specific group. To create an ActionSet, one needs to know the
 * Delimeter serialization.
 */
public class ActionSet extends Time {
    /**
     * String Tag for Logging
     */
    private static final String LOG_TAG      = "ActionSet";
    /**
     * A first segment before the first asterisk(*) in ActionSet string is an
     * actionset name
     */
    public String               actionsetName;
    /**
     * Action instance stored in listOfAction vector variable.
     */
    public Vector<Action>       listOfAction = new Vector<Action>();

    /**
     * This function generates an ActionSet String which contains ActionSet
     * name, delay in seconds, ActionSetType which can be 0(NONE), 1(SCHEDULED)
     * or 2(RECURSIVE) and a list of actions. (Example: movieTime*10
     * 1*uri=coap://10.251.44.228:49858/a/light|power=
     * OFF*uri=coap://10.251.44.228:49858). The first segment before the first
     * asterisk(*) is an ActionSet name. The second segment goes before the next
     * asterisk. In the above example, 10 is the second segment which signifies
     * time delay in seconds followed by a space and ActionSetType which
     * signifies whether an ActionSet has to be triggered Recursively(2) or
     * Scheduled manner(1) or immediately(0).
     * "uri=coap://10.251.44.228:49858/a/light|power=10" is the third segment.
     * This can be also divided into two sub segments by a vertical bar(|): URI
     * and a pair of attribute key and value.
     *
     * @return String for a specific action.
     */
    @Override
    public String toString() {
        StringBuilder result = new StringBuilder();

        // Append action name
        result.append(actionsetName + "*");

        // Append delay and type
        result.append(getDelay() + " " + getType().ordinal() + "*");

        // Append list of actions
        for (int i = 0; i < listOfAction.size(); i++) {
            if (i != 0)
                result.append('*');
            result.append(listOfAction.elementAt(i).toString());
        }

        return result.toString();
    }

    /**
     * This function extracts a list of Actions from plain text and constructs
     * an ActionSet.
     *
     * @param actionsetString
     *            ActionSet in String format. (Example: movieTime*10
     *            1*uri=coap://10.251.44.228:49858/a/light|power=
     *            OFF*uri=coap://10.251.44.228:49858). The first segment before
     *            the first asterisk(*) is an ActionSet name. The second segment
     *            goes before the next asterisk. In the above example, 10 is the
     *            second segment which signifies time delay in seconds followed
     *            by a space and ActionSetType which signifies whether an
     *            ActionSet has to be triggered Recursively(2) or Scheduled
     *            manner(1) or immediately(0).
     *            "uri=coap://10.251.44.228:49858/a/light|power=10" is the third
     *            segment. This can be also divided into two sub segments by a
     *            vertical bar(|): URI and a pair of attribute key and value).
     *
     * @return ActionSet which is a set of descriptions of actions needed by
     *         remote devices as members of a specific group.
     */
    public static ActionSet toActionSet(String actionsetString) {
        if (0 == actionsetString.length()) {
            return null;
        }

        ActionSet result = new ActionSet();
        StringTokenizer tokenizer = new StringTokenizer(actionsetString, "*");
        boolean actionNameFlag = false;
        while (tokenizer.hasMoreTokens()) {
            String segment = tokenizer.nextToken();
            if (false == actionNameFlag) {
                if (true == segment.contains("|")
                        || true == segment.contains("=")) {
                    Log.e(LOG_TAG, "Invalid actionset name string!");
                    return null;
                }

                result.actionsetName = segment;
                actionNameFlag = true;
            } else {
                Action action = Action.toAction(segment);
                if (null == action) {
                    Log.e(LOG_TAG, "Failed to convert string to Action class!");
                    return null;
                }

                result.listOfAction.add(action);
            }
        }
        return result;
    }
}
