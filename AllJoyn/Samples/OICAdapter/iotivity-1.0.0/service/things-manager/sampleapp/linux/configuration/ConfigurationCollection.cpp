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
#include <string.h>

#include "OCPlatform.h"
#include "OCApi.h"
#include "ConfigurationCollection.h"

using namespace OC;
using namespace std;

/// This function internally calls registerResource API.
void ConfigurationResource::createResources(ResourceEntityHandler callback)
{
    using namespace OC::OCPlatform;

    if (callback == NULL)
    {
        std::cout << "callback should be binded\t";
        return;
    }

    // This will internally create and register the resource.
    OCStackResult result = registerResource(m_configurationHandle, m_configurationUri,
            m_configurationTypes[0], m_configurationInterfaces[0], callback,
            OC_DISCOVERABLE | OC_OBSERVABLE);

    if (OC_STACK_OK != result)
    {
        std::cout << "Resource creation (configuration) was unsuccessful\n";
    }

    std::cout << "Configuration Resource is Created!(URI: " << m_configurationUri << ") \n";
}

void ConfigurationResource::setConfigurationRepresentation(OCRepresentation& rep)
{
    string value;

    if (rep.getValue("n", value))
    {
        m_deviceName = value;
        std::cout << "\t\t\t\t" << "m_deviceName: " << m_deviceName << std::endl;
    }

    if (rep.getValue("loc", value))
    {
        m_location = value;
        std::cout << "\t\t\t\t" << "m_location: " << m_location << std::endl;
    }

    if (rep.getValue("locn", value))
    {
        m_locationName = value;
        std::cout << "\t\t\t\t" << "m_locationName: " << m_locationName << std::endl;
    }

    if (rep.getValue("c", value))
    {
        m_currency = value;
        std::cout << "\t\t\t\t" << "m_currency: " << m_currency << std::endl;
    }

    if (rep.getValue("r", value))
    {
        m_region = value;
        std::cout << "\t\t\t\t" << "m_region: " << m_region << std::endl;
    }
}

OCRepresentation ConfigurationResource::getConfigurationRepresentation()
{
    m_configurationRep.setValue("n", m_deviceName);
    m_configurationRep.setValue("loc", m_location);
    m_configurationRep.setValue("locn", m_locationName);
    m_configurationRep.setValue("c", m_currency);
    m_configurationRep.setValue("r", m_region);

    return m_configurationRep;
}

std::string ConfigurationResource::getUri()
{
    return m_configurationUri;
}

void ConfigurationResource::factoryReset()
{
    m_deviceName = defaultDeviceName;
    m_location = defaultLocation;
    m_locationName = defaultLocationName;
    m_currency = defaultCurrency;
    m_region = defaultRegion;
}

