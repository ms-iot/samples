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

///
/// create a resource (collection) with children.
///

#include <functional>
#include <thread>

#include "OCPlatform.h"
#include "OCApi.h"

#pragma once

using namespace OC;

typedef std::function <
OCEntityHandlerResult(std::shared_ptr< OCResourceRequest > request) > ResourceEntityHandler;

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
        /// Constructor
        ConfigurationResource() :
            m_deviceName(defaultDeviceName), m_location(defaultLocation),
            m_locationName(defaultLocationName), m_currency(defaultCurrency),
            m_region(defaultRegion)
        {
            m_configurationUri = defaultConURI; // URI of the resource
            m_configurationTypes.push_back(defaultConResourceType); // resource type name.
            m_configurationInterfaces.push_back(DEFAULT_INTERFACE); // resource interface.
            m_configurationRep.setValue("st", m_deviceName);
            m_configurationRep.setValue("loc", m_location);
            m_configurationRep.setValue("locn", m_locationName);
            m_configurationRep.setValue("c", m_currency);
            m_configurationRep.setValue("r", m_region);
            m_configurationRep.setUri(m_configurationUri);
            m_configurationRep.setResourceTypes(m_configurationTypes);
            m_configurationRep.setResourceInterfaces(m_configurationInterfaces);
            m_configurationHandle = NULL;
        }
        ;

        ~ConfigurationResource()
        {

        }

        /// This function internally calls registerResource API.
        void createResources(ResourceEntityHandler callback);
        void setConfigurationRepresentation(OCRepresentation &rep);
        OCRepresentation getConfigurationRepresentation();
        std::string getUri();

        void factoryReset();

};
