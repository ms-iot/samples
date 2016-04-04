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

package org.oic.simulator.serviceprovider;

/**
 * This enum contains the different levels of server side automation which are
 * supported by the simulator.
 */
public enum AutomationType {
    NORMAL(0), RECURRENT(1);

    private int value;

    private AutomationType(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }
}
