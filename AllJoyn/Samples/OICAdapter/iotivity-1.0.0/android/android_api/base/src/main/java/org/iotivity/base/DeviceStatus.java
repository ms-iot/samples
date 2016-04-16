/*
 * //******************************************************************
 * //
 * // Copyright 2015 Samsung Electronics All Rights Reserved.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //      http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

package org.iotivity.base;

public enum DeviceStatus {
    ON(1),
    OFF(2),
    INVALID(-1);

    private int value;

    private DeviceStatus(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }

    public static DeviceStatus convertDeviceStatus(int value) {

        if (1 == value)
        {
            return DeviceStatus.ON;
        }
        else if (2 == value)
        {
            return DeviceStatus.OFF;
        }
        else
        {
            return DeviceStatus.INVALID;
        }
    }
}
