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

#include "configurationresource.h"

#include <functional>
#include <dlog.h>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

// Constructor
ConfigurationResource::ConfigurationResource() :
    m_deviceName(defaultDeviceName), m_location(defaultLocation),
    m_locationName(defaultLocationName), m_currency(defaultCurrency),
    m_region(defaultRegion)
{
    m_configurationUri = defaultConURI; // URI of the resource
    m_configurationTypes.push_back(defaultConResourceType); // resource type name
    m_configurationInterfaces.push_back(DEFAULT_INTERFACE); // resource interface
    m_configurationRep.setValue(DEFAULT_DEVICENAME, m_deviceName);
    m_configurationRep.setValue(DEFAULT_LOCATION, m_location);
    m_configurationRep.setValue(DEFAULT_LOCATIONNAME, m_locationName);
    m_configurationRep.setValue(DEFAULT_CURRENCY, m_currency);
    m_configurationRep.setValue(DEFAULT_REGION, m_region);
    m_configurationRep.setUri(m_configurationUri);
    m_configurationRep.setResourceTypes(m_configurationTypes);
    m_configurationRep.setResourceInterfaces(m_configurationInterfaces);
    m_configurationHandle = nullptr;
}

// Creates a ConfigurationResource
void ConfigurationResource::createResource(ResourceEntityHandler callback)
{
    using namespace OC::OCPlatform;

    if (NULL == callback)
    {
        dlog_print(DLOG_INFO, "ConfigurationResource", "#### Callback should be binded");
        return;
    }

    // This will internally create and register the resource
    OCStackResult result = registerResource(m_configurationHandle, m_configurationUri,
                                            m_configurationTypes[0], m_configurationInterfaces[0],
                                            callback, OC_DISCOVERABLE | OC_OBSERVABLE);

    if (OC_STACK_OK != result)
    {
        dlog_print(DLOG_INFO, "ConfigurationResource", "#### Resource creation"
                   "(configuration) was unsuccessful");
        return;
    }

    dlog_print(DLOG_INFO, "ConfigurationResource", "#### Configuration Resource is Created");
}

void ConfigurationResource::setConfigurationRepresentation(OCRepresentation &rep)
{
    std::string value;

    if (rep.getValue(DEFAULT_DEVICENAME, value))
    {
        m_deviceName = value;
        dlog_print(DLOG_INFO, "ConfigurationResource", "#### m_deviceName: %s",
                   m_deviceName.c_str());
    }

    if (rep.getValue(DEFAULT_LOCATION, value))
    {
        m_location = value;
        dlog_print(DLOG_INFO, "ConfigurationResource", "#### m_location: %s",
                   m_location.c_str());
    }

    if (rep.getValue(DEFAULT_LOCATIONNAME, value))
    {
        m_locationName = value;
        dlog_print(DLOG_INFO, "ConfigurationResource", "#### m_locationName: %s",
                   m_locationName.c_str());
    }

    if (rep.getValue(DEFAULT_CURRENCY, value))
    {
        m_currency = value;
        dlog_print(DLOG_INFO, "ConfigurationResource", "#### m_currency: %s",
                   m_currency.c_str());
    }

    if (rep.getValue(DEFAULT_REGION, value))
    {
        m_region = value;
        dlog_print(DLOG_INFO, "ConfigurationResource", "#### m_region: %s",
                   m_region.c_str());
    }
}

OCRepresentation ConfigurationResource::getConfigurationRepresentation()
{
    m_configurationRep.setValue(DEFAULT_DEVICENAME, m_deviceName);
    m_configurationRep.setValue(DEFAULT_LOCATION, m_location);
    m_configurationRep.setValue(DEFAULT_LOCATIONNAME, m_locationName);
    m_configurationRep.setValue(DEFAULT_CURRENCY, m_currency);
    m_configurationRep.setValue(DEFAULT_REGION, m_region);

    return m_configurationRep;
}

std::string ConfigurationResource::getUri()
{
    return m_configurationUri;
}

// Assigns default values to all the attributes of the configuration resource
void ConfigurationResource::factoryReset()
{
    m_deviceName = defaultDeviceName;
    m_location = defaultLocation;
    m_locationName = defaultLocationName;
    m_currency = defaultCurrency;
    m_region = defaultRegion;
}

// Deletes the configuration resource which has been created using createResource()
void ConfigurationResource::deleteResource()
{
    // Unregister the resource
    if (nullptr != m_configurationHandle)
    {
        OCPlatform::unregisterResource(m_configurationHandle);
    }
}
