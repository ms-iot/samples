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
 * This file contains RCSBundleInfo class, which provides APIs related to Bundle information.
 */
package org.iotivity.service.resourcecontainer;

import org.iotivity.service.utils.RcsObject;

/**
 * This class provides APIs for getting and setting the Bundle Information
 */
public class RcsBundleInfo extends RcsObject {
    private native String nativeGetID();

    private native String nativeGetPath();

    private native String nativeGetActivatorName();

    private native String nativeGetLibraryPath();

    private native String nativeGetVersion();

    private RcsBundleInfo() {
    }

    /**
     * API for getting the Id of the bundle
     *
     * @return string - Id of the bundle
     *
     */
    public String getID() {
        return nativeGetID();
    }

    /**
     * API for getting the path of the bundle
     *
     * @return path - path of the bundle
     *
     */
    public String getPath() {
        return nativeGetPath();
    }

    /**
     * API for setting the Activator name for the bundle
     *
     * @return string - Name of the activator
     *
     */
    public String getActivatorName() {
        return nativeGetActivatorName();
    }

    /**
     * API for getting the library path for the bundle
     *
     * @return string - Library path in string form
     *
     */
    public String getLibraryPath() {
        return nativeGetLibraryPath();
    }

    /**
     * API for getting the version of the bundle
     *
     * @return string - version of the bundle
     *
     */
    public String getVersion() {
        return nativeGetVersion();
    }

}
