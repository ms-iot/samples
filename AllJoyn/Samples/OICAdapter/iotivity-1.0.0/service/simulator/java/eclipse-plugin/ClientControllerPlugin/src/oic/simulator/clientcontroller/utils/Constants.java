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

package oic.simulator.clientcontroller.utils;

import org.oic.simulator.serviceprovider.AutomationType;

/**
 * This class maintains all constants which are used throughout the client
 * controller plug-in.
 */
public class Constants {
    public static final String         FIND_PAGE_TITLE               = "Find Resources";

    public static final String         FIND_PAGE_MESSAGE             = "Select the resource type of the resources to be discovered";

    public static final int            FIND_RESOURCES_TIMEOUT        = 10;

    public static final String         RESOURCE_URI                  = "Resource URI";
    public static final String         CONNECTIVITY_TYPE             = "Connectivity Type";
    public static final String         OBSERVABLE                    = "Observable";
    public static final String         RESOURCE_TYPES                = "Resource Types";
    public static final String         RESOURCE_INTERFACES           = "Resource Interfaces";

    public static final String[]       META_PROPERTIES               = {
            RESOURCE_URI, CONNECTIVITY_TYPE, OBSERVABLE, RESOURCE_TYPES,
            RESOURCE_INTERFACES                                     };

    public static final int            META_PROPERTY_COUNT           = META_PROPERTIES.length;

    public static final AutomationType DEFAULT_AUTOMATION_TYPE       = AutomationType.NORMAL;

    public static final int            DEFAULT_AUTOMATION_INTERVAL   = 500;

    public static final String         YES                           = "Yes";
    public static final String         NO                            = "No";

    public static final String         GET                           = "Get";
    public static final String         PUT                           = "Put";
    public static final String         POST                          = "Post";

    public static final String         ENABLE                        = "Enable";
    public static final String         DISABLE                       = "Disable";
    public static final String         ENABLED                       = "Enabled";
    public static final String         DISABLED                      = "Disabled";

    public static final String         NOT_AVAILABLE                 = "Not Available";

    public static final int            PROPER_LOG_TIME_TOKEN_LENGTH  = 3;

    public static final int            LOG_SIZE                      = 1000;

    public static final String         INFO_LOG                      = "info_log";
    public static final String         WARNING_LOG                   = "warning_log";
    public static final String         ERROR_LOG                     = "error_log";
    public static final String         DEBUG_LOG                     = "debug_log";
    public static final String         UNKNOWN_LOG                   = "unknown_log";

    public static final String         CHECKED                       = "Checked";
    public static final String         UNCHECKED                     = "Unchecked";

    public static final String         INFO                          = "Info";
    public static final String         WARNING                       = "Warning";
    public static final String         ERROR                         = "Error";
    public static final String         DEBUG                         = "Debug";
    public static final String         UNKNOWN                       = "Unknown";

    public static final String[]       BROWSE_RAML_FILTER_EXTENSIONS = new String[] {
            "*.raml", "*"                                           };
    public static final String[]       SAVE_LOG_FILTER_EXTENSIONS    = new String[] {
            "*.log", "*"                                            };

    public static final int            GET_AUTOMATION_INDEX          = 0;
    public static final int            PUT_AUTOMATION_INDEX          = 1;
    public static final int            POST_AUTOMATION_INDEX         = 2;
    public static final int            DELETE_AUTOMATION_INDEX       = 3;

    public static final String         OIC_R_LIGHT                   = "oic.r.light";
}
