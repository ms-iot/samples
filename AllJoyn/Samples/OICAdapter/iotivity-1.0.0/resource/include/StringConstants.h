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

#ifndef _STRING_CONSTANTS_H_
#define _STRING_CONSTANTS_H_

#include <string>

namespace OC
{
    namespace InitException
    {
        static const char NO_ERROR[]                   = "No Error";
        static const char INVALID_URI[]                = "Invalid URI";
        static const char INVALID_PORT[]               = "Invalid Port";
        static const char INVALID_IP[]                 = "Invalid IP";
        static const char INVALID_CB[]                 = "Invalid Callback";
        static const char INVALID_METHOD[]             = "Invalid Method";
        static const char GENERAL_FAULT[]              = "General Fault";
        static const char UNKNOWN_ERROR[]              = "Unknown Error";

        static const char STACK_INIT_ERROR[]           = "Error Initializing Stack";
        static const char NOT_CONFIGURED_AS_SERVER[]   =
                          "Cannot static construct a Server when configured as a client";
        static const char INVALID_PARAM[]              = "Invalid Param";
        static const char MISSING_URI[]                = "Missing URI;";
        static const char MISSING_TYPE[]               = "Missing Resource Type;";
        static const char MISSING_INTERFACE[]          = "Missing Interface;";
        static const char MISSING_CLIENT_WRAPPER[]     = "Missing ClientWrapper;";
    }

    namespace Exception // Not To Be Confused With 'InitException'
    {
        static const char SVCTYPE_OUTOFPROC[]          = "ServiceType::OutOfProc";
        static const char BIND_TYPE_FAILED[]           = "Bind Type to resource failed";
        static const char BIND_INTERFACE_FAILED[]      = "Bind Interface to resource failed";
        static const char START_PRESENCE_FAILED[]      = "startPresence failed";
        static const char END_PRESENCE_FAILED[]        = "stopPresence failed";
        static const char INVALID_ARRAY[]              = "Array type should have at least []";
        static const char STR_NULL_RESPONSE[]          = "Response is NULL";
        static const char STR_PAYLOAD_OVERFLOW[]       = "Payload overflow";
        static const char NIL_GUARD_NULL[]             = "nullptr at nil_guard()";
        static const char GENERAL_JSON_PARSE_FAILED[]  = "JSON Parser Error";
        static const char RESOURCE_UNREG_FAILED[]      = "Unregistering resource failed";
        static const char OPTION_ID_RANGE_INVALID[]    =
                            "Error: OptionID valid only from 2048 to 3000 inclusive.";
        static const char NO_ERROR[]                   = "No Error";
        static const char RESOURCE_CREATED[]           = "Resource Created";
        static const char RESOURCE_DELETED[]           = "Resource Deleted";
        static const char INVALID_URI[]                = "Invalid URI";
        static const char INVALID_IP[]                 = "Invalid IP";
        static const char INVALID_PORT[]               = "Invalid Port";
        static const char INVALID_CB[]                 = "Invalid Callback";
        static const char INVALID_METHOD[]             = "Invalid Method";
        static const char INVALID_QUERY[]              = "Invalid Query";
        static const char INVALID_PARAM[]              = "Invalid Param";
        static const char INVALID_OBESERVE[]           = "Invalid Observe Param";
        static const char NO_MEMORY[]                  = "No Memory";
        static const char COMM_ERROR[]                 = "Communication Error";
        static const char TIMEOUT[]                    = "Timeout";
        static const char ADAPTER_NOT_ENABLED[]        = "Adapter Not Enabled";
        static const char NOT_IMPL[]                   = "Not Implemented";
        static const char NOT_FOUND[]                  = "Resource Not Found";
        static const char RESOURCE_ERROR[]             = "Resource Error";
        static const char SLOW_RESOURCE[]              = "Slow Resource";
        static const char DUPLICATE_REQUEST[]          = "Duplicate Request";
        static const char NO_OBSERVERS[]               = "No Observers";
        static const char OBSV_NO_FOUND[]              = "Stack observer not found";
        static const char OBSV_NOT_ADDED[]             = "Stack observer not added";
        static const char OBSV_NOT_REMOVED[]           = "Stack observer not removed";
        static const char STACK_RESOURCE_DELETED[]     = "The specified resource has been deleted";
        static const char PRESENCE_STOPPED[]           = "Stack presence stopped";
        static const char PRESENCE_TIMEOUT[]           = "Stack presence timed out";
        static const char PRESENCE_NOT_HANDLED[]       = "Stack presence should not be handled";
        static const char INVALID_OPTION[]             = "Invalid option";
        static const char GENERAL_FAULT[]              = "General Fault";
        static const char MALFORMED_STACK_RESPONSE[]   = "Response from OC_STACK is malformed";
        static const char VIRTUAL_DO_NOT_HANDLE[]      = "Virtual Do Not Handle";
        static const char PERSISTENT_BUFFER_REQUIRED[] = "Persistent response buffer required";
        static const char STACK_CONTINUE[]             = "Stack continue";
        static const char INVALID_REQUEST_HANDLE[]     = "Invalid request handle";
        static const char UNKNOWN_ERROR[]              = "Unknown Error";
        static const char INVALID_REPRESENTATION[]     = "Invalid Payload JSON";
        static const char INVALID_JSON_TYPE[]          = "Unrecognized JSON Type ";
        static const char INVALID_JSON_NUMERIC[]       = "Unrecognized JSON Numeric ";
        static const char INVALID_JSON_ARRAY_DEPTH[]   = "Max JSON Array Depth exceeded";
        static const char INVALID_JSON_TYPE_TAG[]      = "Invalid JSON Type Tag";
        static const char INVALID_ATTRIBUTE[]          = "Invalid Attribute: ";
        static const char INVALID_DEVICE_INFO[]        = "Invalid Device Information";
        static const char UNAUTHORIZED_REQUEST[]       = "Unauthorized Request";
        static const char PDM_DB_NOT_INITIALIZED[]     = "Provisioning DB is not initialized";
        static const char DUPLICATE_UUID[]             = "Duplicate UUID in DB";
        static const char INCONSISTENT_DB[]            = "Data in provisioning DB is inconsistent";

    }

    namespace Error
    {
        static const char INVALID_IP[]                 = "Invalid IP";
    }

    namespace PlatformCommands
    {
        static const std::string GET                        = "GET";
        static const std::string PUT                        = "PUT";
        static const std::string POST                       = "POST";
        static const std::string DELETE                     = "DELETE";
    }

    namespace Key
    {
        static const std::string OCKEY                      = "oic";
        static const std::string URIKEY                     = "href";
        static const std::string POLICYKEY                  = "p";
        static const std::string BMKEY                      = "bm";
        static const std::string RESOURCETYPESKEY           = "rt";
        static const std::string INTERFACESKEY              = "if";
        static const std::string PROPERTYKEY                = "prop";
        static const std::string REPKEY                     = "rep";
        static const std::string SECUREKEY                  = "sec";
        static const std::string PORTKEY                    = "port";
        static const std::string DEVICEIDKEY                = "di";
        static const std::string LINKS                      = "links";

    }

}

#endif // _STRING_CONSTANTS_H_

