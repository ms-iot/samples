//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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

#ifndef RC_INTERNALTYPES_H_
#define RC_INTERNALTYPES_H_

#include "logger.h"

namespace OIC
{
    namespace Service
    {
        constexpr char CONTAINER_TAG[] = "RESOURCE_CONTAINER";

        constexpr char BUNDLE_TAG[] = "bundle";
        constexpr char BUNDLE_ID[] = "id";
        constexpr char BUNDLE_PATH[] = "path";
        constexpr char BUNDLE_VERSION[] = "version";
        constexpr char BUNDLE_ACTIVATOR[] = "activator";
        constexpr char BUNDLE_LIBRARY_PATH[] = "libraryPath";

        constexpr char INPUT_RESOURCE[] = "input";
        constexpr char INPUT_RESOURCE_URI[] = "resourceUri";
        constexpr char INPUT_RESOURCE_TYPE[] = "resourceType";
        constexpr char INPUT_RESOURCE_ATTRIBUTENAME[] = "name";

        constexpr char OUTPUT_RESOURCES_TAG[] = "resources";
        constexpr char OUTPUT_RESOURCE_INFO[] = "resourceInfo";
        constexpr char OUTPUT_RESOURCE_NAME[] = "name";
        constexpr char OUTPUT_RESOURCE_URI[] = "resourceUri";
        constexpr char OUTPUT_RESOURCE_TYPE[] = "resourceType";
        constexpr char OUTPUT_RESOURCE_ADDR[] = "address";
    }
}

#endif /* RC_INTERNALTYPES_H_ */
