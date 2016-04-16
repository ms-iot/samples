/* *****************************************************************
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
 * This file provides a Enum which contains Status codes
 * for Success and Errors
 */

package org.iotivity.service.tm;

/**
 * This Enum contains Status codes for Success and Errors
 */
public enum OCStackResult {

    OC_STACK_OK,
    OC_STACK_RESOURCE_CREATED,
    OC_STACK_RESOURCE_DELETED,
    OC_STACK_CONTINUE,
    OC_STACK_INVALID_URI,
    OC_STACK_INVALID_QUERY,
    OC_STACK_INVALID_IP,
    OC_STACK_INVALID_PORT,
    OC_STACK_INVALID_CALLBACK,
    OC_STACK_INVALID_METHOD,
    OC_STACK_INVALID_PARAM,
    OC_STACK_INVALID_OBSERVE_PARAM,
    OC_STACK_NO_MEMORY,
    OC_STACK_COMM_ERROR,
    OC_STACK_NOTIMPL,
    OC_STACK_NO_RESOURCE,
    OC_STACK_RESOURCE_ERROR,
    OC_STACK_SLOW_RESOURCE,
    OC_STACK_REPEATED_REQUEST,
    OC_STACK_NO_OBSERVERS,
    OC_STACK_OBSERVER_NOT_FOUND,
    OC_STACK_VIRTUAL_DO_NOT_HANDLE,
    OC_STACK_INVALID_OPTION,
    OC_STACK_MALFORMED_RESPONSE,
    OC_STACK_PERSISTENT_BUFFER_REQUIRED,
    OC_STACK_INVALID_REQUEST_HANDLE,
    OC_STACK_INVALID_DEVICE_INFO,
    OC_STACK_INVALID_JSON,
    OC_STACK_PRESENCE_STOPPED,
    OC_STACK_PRESENCE_TIMEOUT,
    OC_STACK_PRESENCE_DO_NOT_HANDLE,
    OC_STACK_ERROR,
    OC_STACK_LISTENER_NOT_SET;

    public static OCStackResult conversion(int ordinal) {

        OCStackResult result = OCStackResult.values()[31];

        if (ordinal == 0)
            result = OCStackResult.values()[0];
        else if (ordinal == 1)
            result = OCStackResult.values()[1];
        else if (ordinal == 2)
            result = OCStackResult.values()[2];
        else if (ordinal == 3)
            result = OCStackResult.values()[3];

        else if (ordinal == 20)
            result = OCStackResult.values()[4];
        else if (ordinal == 21)
            result = OCStackResult.values()[5];
        else if (ordinal == 22)
            result = OCStackResult.values()[6];
        else if (ordinal == 23)
            result = OCStackResult.values()[7];
        else if (ordinal == 24)
            result = OCStackResult.values()[8];
        else if (ordinal == 25)
            result = OCStackResult.values()[9];
        else if (ordinal == 26)
            result = OCStackResult.values()[10];
        else if (ordinal == 27)
            result = OCStackResult.values()[11];
        else if (ordinal == 28)
            result = OCStackResult.values()[12];
        else if (ordinal == 29)
            result = OCStackResult.values()[13];
        else if (ordinal == 30)
            result = OCStackResult.values()[14];
        else if (ordinal == 31)
            result = OCStackResult.values()[15];
        else if (ordinal == 32)
            result = OCStackResult.values()[16];
        else if (ordinal == 33)
            result = OCStackResult.values()[17];
        else if (ordinal == 34)
            result = OCStackResult.values()[18];
        else if (ordinal == 35)
            result = OCStackResult.values()[19];
        else if (ordinal == 36)
            result = OCStackResult.values()[20];
        else if (ordinal == 37)
            result = OCStackResult.values()[21];
        else if (ordinal == 38)
            result = OCStackResult.values()[22];
        else if (ordinal == 39)
            result = OCStackResult.values()[23];
        else if (ordinal == 40)
            result = OCStackResult.values()[24];
        else if (ordinal == 41)
            result = OCStackResult.values()[25];
        else if (ordinal == 42)
            result = OCStackResult.values()[26];
        else if (ordinal == 43)
            result = OCStackResult.values()[27];

        else if (ordinal == 128)
            result = OCStackResult.values()[28];
        else if (ordinal == 129)
            result = OCStackResult.values()[29];
        else if (ordinal == 130)
            result = OCStackResult.values()[30];

        return result;
    }
}
