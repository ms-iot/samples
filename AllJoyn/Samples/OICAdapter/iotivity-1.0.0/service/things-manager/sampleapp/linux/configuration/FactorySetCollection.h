//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
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

///
/// This sample shows how one could create a resource (collection) with children.
///

#include <functional>
#include <thread>

#include "OCPlatform.h"
#include "OCApi.h"
//#include "ThingsManager.h"
#include "ConfigurationCollection.h"

#pragma once

using namespace OC;

typedef std::function<
    OCEntityHandlerResult(std::shared_ptr< OCResourceRequest > request) > ResourceEntityHandler;

class FactorySetResource : public ConfigurationResource
{
public:
    /// Constructor
    FactorySetResource();

    ~FactorySetResource();

    /// This function internally calls registerResource API.
    void createResources(ResourceEntityHandler callback);
    void setFactorySetRepresentation(OCRepresentation& rep);
    OCRepresentation getFactorySetRepresentation();

    std::string getUri();
};

