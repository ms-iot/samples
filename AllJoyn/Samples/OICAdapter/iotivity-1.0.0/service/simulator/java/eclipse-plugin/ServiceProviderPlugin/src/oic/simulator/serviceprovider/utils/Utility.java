/*
 * Copyright 2015 Samsung Electronics All Rights Reserved.
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
 */

package oic.simulator.serviceprovider.utils;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

/**
 * This class has common utility methods.
 */
public class Utility {

    public static String uriToDisplayName(String uri) {
        String result = null;
        if (null != uri) {
            String tokens[] = uri.split(Constants.FORWARD_SLASH);
            if (null != tokens && tokens.length > 2) {
                result = tokens[tokens.length - 3] + Constants.UNDERSCORE
                        + tokens[tokens.length - 1];
            }
        }
        return result;
    }

    public static String fileNameToDisplay(String fileName) {
        if (null == fileName || fileName.length() < 1) {
            return null;
        }
        // Remove the RAML file standard prefix
        int len = Constants.RAML_FILE_PREFIX.length();
        if (len > 0) {
            if (fileName.startsWith(Constants.RAML_FILE_PREFIX)) {
                fileName = fileName.substring(len);
            }
        }

        // Removing the file extension
        int index = fileName.lastIndexOf('.');
        fileName = fileName.substring(0, index);
        return fileName;
    }

    public static String displayToFileName(String displayName) {
        if (null == displayName || displayName.length() < 1) {
            return null;
        }
        String fileName;
        // Adding the prefix
        fileName = Constants.RAML_FILE_PREFIX + displayName;

        // Adding the file extension
        fileName = fileName + Constants.RAML_FILE_EXTENSION;

        return fileName;
    }

    public static String getAutomationStatus(boolean status) {
        if (status) {
            return Constants.ENABLED;
        } else {
            return Constants.DISABLED;
        }
    }

    public static String getAutomationString(boolean status) {
        if (status) {
            return Constants.ENABLE;
        } else {
            return Constants.DISABLE;
        }
    }

    public static boolean getAutomationBoolean(String status) {
        if (null != status) {
            if (status.equals(Constants.ENABLE)) {
                return true;
            }
        }
        return false;
    }

    public static int getUpdateIntervalFromString(String value) {
        int result = Constants.DEFAULT_AUTOMATION_INTERVAL;
        if (null != value) {
            try {
                result = Integer.parseInt(value);
            } catch (NumberFormatException nfe) {
                System.out
                        .println("Getting UpdateInterval from string failed!");
            }
        }
        return result;
    }

    public static List<String> convertSetToList(Set<String> typeSet) {
        if (null == typeSet) {
            return null;
        }
        List<String> list = new ArrayList<String>();
        Iterator<String> typeItr = typeSet.iterator();
        while (typeItr.hasNext()) {
            list.add(typeItr.next());
        }
        return list;
    }
}