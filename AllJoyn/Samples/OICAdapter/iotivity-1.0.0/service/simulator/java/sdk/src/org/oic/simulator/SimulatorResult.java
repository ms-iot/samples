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
 * This Enum contains Status codes for Success and Errors.
 */
public enum SimulatorResult {
    /** STACK error codes - START */
    SIMULATOR_OK,
    SIMULATOR_RESOURCE_CREATED,
    SIMULATOR_RESOURCE_DELETED,
    SIMULATOR_CONTINUE,
    SIMULATOR_INVALID_URI,
    SIMULATOR_INVALID_QUERY,
    SIMULATOR_INVALID_IP,
    SIMULATOR_INVALID_PORT,
    SIMULATOR_INVALID_CALLBACK,
    SIMULATOR_INVALID_METHOD,
    SIMULATOR_INVALID_PARAM,
    SIMULATOR_INVALID_OBSERVE_PARAM,
    SIMULATOR_NO_MEMORY,
    SIMULATOR_COMM_ERROR,
    SIMULATOR_TIMEOUT,
    SIMULATOR_ADAPTER_NOT_ENABLED,
    SIMULATOR_NOTIMPL,
    SIMULATOR_NO_RESOURCE,
    SIMULATOR_RESOURCE_ERROR,
    SIMULATOR_SLOW_RESOURCE,
    SIMULATOR_DUPLICATE_REQUEST,
    SIMULATOR_NO_OBSERVERS,
    SIMULATOR_OBSERVER_NOT_FOUND,
    SIMULATOR_VIRTUAL_DO_NOT_HANDLE,
    SIMULATOR_INVALID_OPTION,
    SIMULATOR_MALFORMED_RESPONSE,
    SIMULATOR_PERSISTENT_BUFFER_REQUIRED,
    SIMULATOR_INVALID_REQUEST_HANDLE,
    SIMULATOR_INVALID_DEVICE_INFO,
    SIMULATOR_INVALID_JSON,
    SIMULATOR_UNAUTHORIZED_REQ,

    SIMULATOR_PRESENCE_STOPPED,
    SIMULATOR_PRESENCE_TIMEOUT,
    SIMULATOR_PRESENCE_DO_NOT_HANDLE,
    /** STACK error codes - END */

    /** Simulator specific error codes - START */
    SIMULATOR_INVALID_TYPE,
    SIMULATOR_NOT_SUPPORTED,
    SIMULATOR_OPERATION_NOT_ALLOWED,
    SIMULATOR_OPERATION_IN_PROGRESS,

    SIMULATOR_INVALID_RESPONSE_CODE,
    SIMULATOR_UKNOWN_PROPERTY,
    SIMULATOR_TYPE_MISMATCH,
    SIMULATOR_BAD_VALUE,
    /** Simulator specific error codes - END */

    SIMULATOR_ERROR;

    public static SimulatorResult get(int ordinal) {

        SimulatorResult result;

        if (ordinal == 0)
            result = SimulatorResult.values()[0];
        else if (ordinal == 1)
            result = SimulatorResult.values()[1];
        else if (ordinal == 2)
            result = SimulatorResult.values()[2];
        else if (ordinal == 3)
            result = SimulatorResult.values()[3];

        else if (ordinal == 20)
            result = SimulatorResult.values()[4];
        else if (ordinal == 21)
            result = SimulatorResult.values()[5];
        else if (ordinal == 22)
            result = SimulatorResult.values()[6];
        else if (ordinal == 23)
            result = SimulatorResult.values()[7];
        else if (ordinal == 24)
            result = SimulatorResult.values()[8];
        else if (ordinal == 25)
            result = SimulatorResult.values()[9];
        else if (ordinal == 26)
            result = SimulatorResult.values()[10];
        else if (ordinal == 27)
            result = SimulatorResult.values()[11];
        else if (ordinal == 28)
            result = SimulatorResult.values()[12];
        else if (ordinal == 29)
            result = SimulatorResult.values()[13];
        else if (ordinal == 30)
            result = SimulatorResult.values()[14];
        else if (ordinal == 31)
            result = SimulatorResult.values()[15];
        else if (ordinal == 32)
            result = SimulatorResult.values()[16];
        else if (ordinal == 33)
            result = SimulatorResult.values()[17];
        else if (ordinal == 34)
            result = SimulatorResult.values()[18];
        else if (ordinal == 35)
            result = SimulatorResult.values()[19];
        else if (ordinal == 36)
            result = SimulatorResult.values()[20];
        else if (ordinal == 37)
            result = SimulatorResult.values()[21];
        else if (ordinal == 38)
            result = SimulatorResult.values()[22];
        else if (ordinal == 39)
            result = SimulatorResult.values()[23];
        else if (ordinal == 40)
            result = SimulatorResult.values()[24];
        else if (ordinal == 41)
            result = SimulatorResult.values()[25];
        else if (ordinal == 42)
            result = SimulatorResult.values()[26];
        else if (ordinal == 43)
            result = SimulatorResult.values()[27];
        else if (ordinal == 44)
            result = SimulatorResult.values()[28];
        else if (ordinal == 45)
            result = SimulatorResult.values()[29];
        else if (ordinal == 46)
            result = SimulatorResult.values()[30];

        else if (ordinal == 128)
            result = SimulatorResult.values()[31];
        else if (ordinal == 129)
            result = SimulatorResult.values()[32];
        else if (ordinal == 130)
            result = SimulatorResult.values()[33];

        else if (ordinal == 131 || ordinal == 47)
            result = SimulatorResult.values()[34];
        else if (ordinal == 132 || ordinal == 48)
            result = SimulatorResult.values()[35];
        else if (ordinal == 133 || ordinal == 49)
            result = SimulatorResult.values()[36];
        else if (ordinal == 134 || ordinal == 50)
            result = SimulatorResult.values()[37];

        else if (ordinal == 135 || ordinal == 51)
            result = SimulatorResult.values()[38];
        else if (ordinal == 136 || ordinal == 52)
            result = SimulatorResult.values()[39];
        else if (ordinal == 137 || ordinal == 53)
            result = SimulatorResult.values()[40];
        else if (ordinal == 138 || ordinal == 54)
            result = SimulatorResult.values()[41];

        else
            result = SimulatorResult.values()[42];
        return result;
    }
}