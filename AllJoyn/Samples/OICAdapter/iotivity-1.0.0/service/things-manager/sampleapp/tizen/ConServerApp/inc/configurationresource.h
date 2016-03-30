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
#ifndef CONFIGURATION_RESOURCE_H__
#define CONFIGURATION_RESOURCE_H__

#include <functional>

#include "OCPlatform.h"
#include "OCApi.h"

#pragma once

#define DEFAULT_DEVICENAME "n"
#define DEFAULT_LOCATION "loc"
#define DEFAULT_LOCATIONNAME "locn"
#define DEFAULT_CURRENCY "c"
#define DEFAULT_REGION "r"

using namespace OC;

typedef std::function<OCEntityHandlerResult(std::shared_ptr< OCResourceRequest > request)>
ResourceEntityHandler;

static std::string defaultConURI = "/oic/con";
static std::string defaultConResourceType = "oic.wk.con";

extern std::string defaultDeviceName;
extern std::string defaultLocation;
extern std::string defaultLocationName;
extern std::string defaultCurrency;
extern std::string defaultRegion;

class ConfigurationResource
{
    public:
        // Configuration members
        std::string m_configurationUri;
        std::string m_deviceName;
        std::string m_location;
        std::string m_locationName;
        std::string m_currency;
        std::string m_region;
        std::vector< std::string > m_configurationTypes;
        std::vector< std::string > m_configurationInterfaces;
        OCResourceHandle m_configurationHandle;
        OCRepresentation m_configurationRep;

    public:

        ConfigurationResource();

        void createResource(ResourceEntityHandler callback);

        void setConfigurationRepresentation(OCRepresentation &rep);

        OCRepresentation getConfigurationRepresentation();

        std::string getUri();

        void factoryReset();

        void deleteResource();
};

#endif // CONFIGURATION_RESOURCE_H__