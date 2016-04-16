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

#pragma once

using namespace OC;

typedef std::function<
    OCEntityHandlerResult(std::shared_ptr< OCResourceRequest > request) > ResourceEntityHandler;

static std::string defaultMntURI = "/oic/mnt";
static std::string defaultMntResourceType = "oic.wk.mnt";

static std::string defaultFactoryReset = "false";
static std::string defaultReboot = "false";
static std::string defaultStartStatCollection = "false";

class MaintenanceResource
{
public:

    // Maintenance members
    std::string m_maintenanceUri;
    std::string m_factoryReset;
    std::string m_reboot;
    std::string m_startStatCollection;
    std::vector< std::string > m_maintenanceTypes;
    std::vector< std::string > m_maintenanceInterfaces;
    OCResourceHandle m_maintenanceHandle;
    OCRepresentation m_maintenanceRep;

public:
    /// Constructor
    MaintenanceResource() :
           m_factoryReset(defaultFactoryReset), m_reboot(defaultReboot),
            m_startStatCollection(defaultStartStatCollection)
    {
        m_maintenanceUri = defaultMntURI; // URI of the resource
        m_maintenanceTypes.push_back(defaultMntResourceType); // resource type name.
        m_maintenanceInterfaces.push_back(DEFAULT_INTERFACE); // resource interface.
        m_maintenanceRep.setValue("fr", m_factoryReset);
        m_maintenanceRep.setValue("rb", m_reboot);
        m_maintenanceRep.setValue("ssc", m_startStatCollection);
        m_maintenanceRep.setUri(m_maintenanceUri);
        m_maintenanceRep.setResourceTypes(m_maintenanceTypes);
        m_maintenanceRep.setResourceInterfaces(m_maintenanceInterfaces);
        m_maintenanceHandle = NULL;
    }
    ;

    /// This function internally calls registerResource API.
    void createResources(ResourceEntityHandler callback);

    void setMaintenanceRepresentation(OCRepresentation& rep);

    OCRepresentation getMaintenanceRepresentation();

    std::string getUri();

    void maintenanceMonitor(int second);

    std::function< void() > factoryReset;
};

