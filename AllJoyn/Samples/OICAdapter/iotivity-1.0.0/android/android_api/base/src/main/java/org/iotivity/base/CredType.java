/*
 * //******************************************************************
 * //
 * // Copyright 2015  Samsung Electronics All Rights Reserved.
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
import java.util.EnumSet;

public enum CredType {

    NO_SECURITY_MODE                (0),

    SYMMETRIC_PAIR_WISE_KEY         (1 << 0),

    SYMMETRIC_GROUP_KEY             (1 << 1),

    ASYMMETRIC_KEY                  (1 << 2),

    SIGNED_ASYMMETRIC_KEY           (1 << 3),

    PIN_PASSWORD                    (1 << 4),

    ASYMMETRIC_ENCRYPTION_KEY       (1 << 5),

    ;
    private int value;

    private CredType(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }

    public static EnumSet<CredType> convertToEnumSet(int value) {
        EnumSet<CredType> typeSet = null;

        for (CredType v : values()) {
            if (0 != (value & v.getValue())) {
                if (null == typeSet) {
                    typeSet = EnumSet.of(v);
                } else {
                    typeSet.add(v);
                }
            }
        }

        if (null == typeSet || typeSet.isEmpty()) {
            throw new InvalidParameterException("Unexpected CredType value:" + value);
        }

        return typeSet;
    }
}
