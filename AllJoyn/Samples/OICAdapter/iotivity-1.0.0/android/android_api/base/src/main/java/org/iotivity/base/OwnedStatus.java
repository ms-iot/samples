/*
 * //******************************************************************
 * //
 * // Copyright Samsung Electronics All Rights Reserved.
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

public enum OwnedStatus {
    OWNED(0),
    UNOWNED(1),
    INVALID(-1);

    private int value;

    private OwnedStatus(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }

    public static OwnedStatus convertOwnedStatus(int value) {

        if (0 == value)
        {
            return OwnedStatus.UNOWNED;
        }
        else if (1 == value)
        {
            return OwnedStatus.OWNED;
        }
        else
        {
            return OwnedStatus.INVALID;
        }
    }
}
