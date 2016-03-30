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

#ifndef SIMULATOR_ERROR_CODES_H_
#define SIMULATOR_ERROR_CODES_H_

#include <iostream>

typedef enum
{
    /** STACK error codes - START */
    SIMULATOR_OK = 0,
    SIMULATOR_RESOURCE_CREATED,
    SIMULATOR_RESOURCE_DELETED,
    SIMULATOR_CONTINUE,
    SIMULATOR_INVALID_URI = 20,
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
#ifdef WITH_PRESENCE
    SIMULATOR_PRESENCE_STOPPED = 128,
    SIMULATOR_PRESENCE_TIMEOUT,
    SIMULATOR_PRESENCE_DO_NOT_HANDLE,
#endif
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
    SIMULATOR_BAD_OBJECT,
    /** Simulator specific error codes - END */

    SIMULATOR_ERROR = 255
} SimulatorResult;
#endif //SIMULATOR_ERROR_CODES_H_