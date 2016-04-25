/*
 * //******************************************************************
 * //
 * // Copyright 2015 Intel Corporation.
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

import java.security.InvalidParameterException;

public enum OcPresenceStatus {
    OK("OK"),
    STOPPED("PRESENCE_STOPPED"),
    TIMEOUT("PRESENCE_TIMEOUT"),
    DO_NOT_HANDLE("PRESENCE_DO_NOT_HANDLE");

    private String value;

    private OcPresenceStatus(String value) {
        this.value = value;
    }

    public static OcPresenceStatus get(String val) {
        for (OcPresenceStatus v : OcPresenceStatus.values()) {
            if (v.getValue().equals(val))
                return v;
        }
        throw new InvalidParameterException("Unexpected OcPresenceStatus value");
    }

    public String getValue() {
        return this.value;
    }
}
