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

package org.oic.simulator;

/**
 * This class contains remote device information and provides APIs to access it.
 */
public class DeviceInfo {
    private String mName;
    private String mID;
    private String mSpecVersion;
    private String mDMVVersion;

    /**
     * This method is for getting the device name.
     *
     * @return Device name.
     */
    public String getName() {
        return mName;
    }

    /**
     * This method is for getting the device id.
     *
     * @return Device id.
     */
    public String getID() {
        return mID;
    }

    /**
     * This method is for getting device specification version.
     *
     * @return Specification version.
     */
    public String getSpecVersion() {
        return mSpecVersion;
    }

    /**
     * This method is for getting device data model version.
     *
     * @return Device data model version.
     */
    public String getDataModelVersion() {
        return mDMVVersion;
    }
}
