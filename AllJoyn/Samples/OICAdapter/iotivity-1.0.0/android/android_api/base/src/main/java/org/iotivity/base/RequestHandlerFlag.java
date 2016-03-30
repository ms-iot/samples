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

import java.util.EnumSet;

public enum RequestHandlerFlag {
    INIT(1 << 0),
    REQUEST(1 << 1),
    OBSERVER(1 << 2),;

    private int value;

    private RequestHandlerFlag(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }

    public static EnumSet<RequestHandlerFlag> convertToEnumSet(int value) {
        EnumSet<RequestHandlerFlag> flagSet = null;

        for (RequestHandlerFlag v : values()) {
            if (0 != (value & v.getValue())) {
                if (flagSet == null) {
                    flagSet = EnumSet.of(v);
                } else {
                    flagSet.add(v);
                }
            }
        }

        if (null == flagSet || flagSet.isEmpty()) {
            throw new IllegalArgumentException("Unexpected RequestHandlerFlag value");
        }

        return flagSet;
    }
}
