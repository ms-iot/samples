/******************************************************************
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

#include "rhutil.h"

// Utility function to return the string equivalent of OCStackResult for the given int value
std::string getOCStackResultStringFromInt(int result)
{
    string ocresultstr;

    switch (result)
    {
        case 0 :
            ocresultstr = "OC_STACK_OK";
            break;
        case 1 :
            ocresultstr = "OC_STACK_RESOURCE_CREATED";
            break;
        case 2 :
            ocresultstr = "OC_STACK_RESOURCE_DELETED";
            break;
        case 3 :
            ocresultstr = "OC_STACK_CONTINUE";
            break;
        case 20 :
            ocresultstr = "OC_STACK_INVALID_URI";
            break;
        case 21 :
            ocresultstr = "OC_STACK_INVALID_QUERY";
            break;
        case 22 :
            ocresultstr = "OC_STACK_INVALID_QUERY";
            break;
        case 23 :
            ocresultstr = "OC_STACK_INVALID_PORT";
            break;
        case 24 :
            ocresultstr = "OC_STACK_INVALID_CALLBACK";
            break;
        case 25 :
            ocresultstr = "OC_STACK_INVALID_METHOD";
            break;
        case 26 :
            ocresultstr = "OC_STACK_INVALID_PARAM";
            break;
        case 27 :
            ocresultstr = "OC_STACK_INVALID_OBSERVE_PARAM";
            break;
        case 28 :
            ocresultstr = "OC_STACK_NO_MEMORY";
            break;
        case 29 :
            ocresultstr = "OC_STACK_COMM_ERROR";
            break;
        case 30 :
            ocresultstr = "OC_STACK_NOTIMPL";
            break;
        case 31 :
            ocresultstr = "OC_STACK_NO_RESOURCE";
            break;
        case 32 :
            ocresultstr = "OC_STACK_RESOURCE_ERROR";
            break;
        case 33 :
            ocresultstr = "OC_STACK_SLOW_RESOURCE";
            break;
        case 34 :
            ocresultstr = "OC_STACK_DUPLICATE_REQUEST";
            break;
        case 35 :
            ocresultstr = "OC_STACK_NO_OBSERVERS";
            break;
        case 36 :
            ocresultstr = "OC_STACK_OBSERVER_NOT_FOUND";
            break;
        case 37 :
            ocresultstr = "OC_STACK_VIRTUAL_DO_NOT_HANDLE";
            break;
        case 38 :
            ocresultstr = "OC_STACK_INVALID_OPTION";
            break;
        case 39 :
            ocresultstr = "OC_STACK_MALFORMED_RESPONSE";
            break;
        case 40 :
            ocresultstr = "OC_STACK_PERSISTENT_BUFFER_REQUIRED";
            break;
        case 41 :
            ocresultstr = "OC_STACK_INVALID_REQUEST_HANDLE";
            break;
        case 42 :
            ocresultstr = "OC_STACK_INVALID_DEVICE_INFO";
            break;
        case 43 :
            ocresultstr = "OC_STACK_INVALID_JSON";
            break;
        case 128 :
            ocresultstr = "OC_STACK_PRESENCE_STOPPED";
            break;
        case 129 :
            ocresultstr = "OC_STACK_PRESENCE_TIMEOUT";
            break;
        case 130 :
            ocresultstr = "OC_STACK_PRESENCE_DO_NOT_HANDLE";
            break;
        case 255 :
            ocresultstr = "OC_STACK_ERROR";
            break;
        default :
            ocresultstr = "OC_STACK_ERROR";
    }
    return ocresultstr;
}
