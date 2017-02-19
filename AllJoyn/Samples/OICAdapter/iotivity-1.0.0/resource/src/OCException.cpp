//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "OCException.h"
#include "StringConstants.h"

std::string OC::OCException::reason(const OCStackResult sr)
{
    switch(sr)
    {
        case OC_STACK_OK:
             return OC::Exception::NO_ERROR;
        case OC_STACK_RESOURCE_CREATED:
             return OC::Exception::RESOURCE_CREATED;
        case OC_STACK_RESOURCE_DELETED:
             return OC::Exception::RESOURCE_DELETED;
        case OC_STACK_INVALID_URI:
            return OC::Exception::INVALID_URI;
        case OC_STACK_INVALID_IP:
            return OC::Exception::INVALID_IP;
        case OC_STACK_INVALID_PORT:
            return OC::Exception::INVALID_PORT;
        case OC_STACK_INVALID_CALLBACK:
            return OC::Exception::INVALID_CB;
        case OC_STACK_INVALID_METHOD:
            return OC::Exception::INVALID_METHOD;
        case OC_STACK_INVALID_QUERY:
            return OC::Exception::INVALID_QUERY;
        case OC_STACK_INVALID_PARAM:
            return OC::Exception::INVALID_PARAM;
        case OC_STACK_INVALID_OBSERVE_PARAM:
            return OC::Exception::INVALID_OBESERVE;
        case OC_STACK_NO_MEMORY:
            return OC::Exception::NO_MEMORY;
        case OC_STACK_COMM_ERROR:
            return OC::Exception::COMM_ERROR;
        case OC_STACK_TIMEOUT:
            return OC::Exception::TIMEOUT;
        case OC_STACK_ADAPTER_NOT_ENABLED:
            return OC::Exception::ADAPTER_NOT_ENABLED;
        case OC_STACK_NOTIMPL:
            return OC::Exception::NOT_IMPL;
        case OC_STACK_NO_RESOURCE:
            return OC::Exception::NOT_FOUND;
        case OC_STACK_RESOURCE_ERROR:
            return OC::Exception::RESOURCE_ERROR;
        case OC_STACK_SLOW_RESOURCE:
            return OC::Exception::SLOW_RESOURCE;
        case OC_STACK_DUPLICATE_REQUEST:
            return OC::Exception::DUPLICATE_REQUEST;
        case OC_STACK_NO_OBSERVERS:
            return OC::Exception::NO_OBSERVERS;
        case OC_STACK_OBSERVER_NOT_FOUND:
            return OC::Exception::OBSV_NO_FOUND;
#ifdef WITH_PRESENCE
        case OC_STACK_PRESENCE_STOPPED:
            return OC::Exception::PRESENCE_STOPPED;
        case OC_STACK_PRESENCE_TIMEOUT:
            return OC::Exception::PRESENCE_TIMEOUT;
        case OC_STACK_PRESENCE_DO_NOT_HANDLE:
            return OC::Exception::PRESENCE_NOT_HANDLED;
#endif
        case OC_STACK_VIRTUAL_DO_NOT_HANDLE:
            return OC::Exception::VIRTUAL_DO_NOT_HANDLE;
        case OC_STACK_INVALID_OPTION:
            return OC::Exception::INVALID_OPTION;
        case OC_STACK_MALFORMED_RESPONSE:
            return OC::Exception::MALFORMED_STACK_RESPONSE;
        case OC_STACK_PERSISTENT_BUFFER_REQUIRED:
            return OC::Exception::PERSISTENT_BUFFER_REQUIRED;
        case OC_STACK_CONTINUE:
            return OC::Exception::STACK_CONTINUE;
        case OC_STACK_INVALID_REQUEST_HANDLE:
            return OC::Exception::INVALID_REQUEST_HANDLE;
        case OC_STACK_ERROR:
            return OC::Exception::GENERAL_FAULT;
        case OC_STACK_INVALID_DEVICE_INFO:
            return OC::Exception::INVALID_DEVICE_INFO;
        case OC_STACK_INVALID_JSON:
            return OC::Exception::INVALID_REPRESENTATION;
        case OC_STACK_UNAUTHORIZED_REQ:
            return OC::Exception::UNAUTHORIZED_REQUEST;
        case OC_STACK_PDM_IS_NOT_INITIALIZED:
            return OC::Exception::PDM_DB_NOT_INITIALIZED;
        case OC_STACK_DUPLICATE_UUID:
            return OC::Exception::DUPLICATE_UUID;
        case OC_STACK_INCONSISTENT_DB:
            return OC::Exception::INCONSISTENT_DB;
    }

    return OC::Exception::UNKNOWN_ERROR;
}


