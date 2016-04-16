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

package org.oic.simulator.clientcontroller;

/**
 * Types of automatic verification.
 */
public enum SimulatorVerificationType {
    RQ_TYPE_GET(0), RQ_TYPE_PUT(1), RQ_TYPE_POST(2), RQ_TYPE_DELETE(3);

    private int value;

    private SimulatorVerificationType(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }

    /**
     * Method to get the {@link SimulatorVerificationType} from an integer
     * value.
     *
     * @param value
     *            Integral value of {@link SimulatorVerificationType}.
     * @return {@link SimulatorVerificationType} corresponding to the given
     *         value.
     */
    public static SimulatorVerificationType getVerificationType(int value) {
        SimulatorVerificationType result = null;
        SimulatorVerificationType[] types = SimulatorVerificationType.values();
        for (SimulatorVerificationType type : types) {
            if (type.getValue() == value) {
                result = type;
                break;
            }
        }
        return result;
    }
}
