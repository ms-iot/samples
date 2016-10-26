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
#ifndef MAINTENANCE_RESOURCE_H__
#define MAINTENANCE_RESOURCE_H__

#include <functional>
#include <thread>
#include <string>

#include "OCPlatform.h"
#include "OCApi.h"

#pragma once

#define DEFAULT_REBOOT "rb"
#define DEFAULT_FACTORYRESET "fr"
#define DEFAULT_STARTCOLLECTION "ssc"

using namespace OC;

typedef std::function<OCEntityHandlerResult(std::shared_ptr< OCResourceRequest > request)>
ResourceEntityHandler;

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

        MaintenanceResource();

        void createResource(ResourceEntityHandler callback);

        void setMaintenanceRepresentation(OCRepresentation &rep);

        OCRepresentation getMaintenanceRepresentation();

        std::string getUri();

        void maintenanceMonitor(int second);

        std::function< void() > factoryReset;

        void deleteResource();
};

#endif // MAINTENANCE_RESOURCE_H__