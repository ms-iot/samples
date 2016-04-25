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

#include "maintenanceresource.h"

#include <functional>
#include <dlog.h>
#include <thread>
#include <string>

#include "conserverapp.h"
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

extern std::string logMessage;

extern void *updateLog(void *);

static std::string defaultMntURI = "/oic/mnt";
static std::string defaultMntResourceType = "oic.wk.mnt";

// Constructor
MaintenanceResource::MaintenanceResource() :
    m_factoryReset(defaultFactoryReset), m_reboot(defaultReboot),
    m_startStatCollection(defaultStartStatCollection)
{
    m_maintenanceUri = defaultMntURI; // URI of the resource
    m_maintenanceTypes.push_back(defaultMntResourceType); // resource type name
    m_maintenanceInterfaces.push_back(DEFAULT_INTERFACE); // resource interface
    m_maintenanceRep.setValue(DEFAULT_FACTORYRESET, m_factoryReset);
    m_maintenanceRep.setValue(DEFAULT_REBOOT, m_reboot);
    m_maintenanceRep.setValue(DEFAULT_STARTCOLLECTION, m_startStatCollection);
    m_maintenanceRep.setUri(m_maintenanceUri);
    m_maintenanceRep.setResourceTypes(m_maintenanceTypes);
    m_maintenanceRep.setResourceInterfaces(m_maintenanceInterfaces);
    m_maintenanceHandle = NULL;
}

// Creates a DiagnosticResource
void MaintenanceResource::createResource(ResourceEntityHandler callback)
{
    using namespace OC::OCPlatform;

    if (NULL == callback)
    {
        dlog_print(DLOG_INFO, "MaintenanceResource", "#### Callback should be binded");
        return;
    }

    // This will internally create and register the resource
    OCStackResult result = registerResource(m_maintenanceHandle, m_maintenanceUri,
                                            m_maintenanceTypes[0], m_maintenanceInterfaces[0],
                                            callback, OC_DISCOVERABLE | OC_OBSERVABLE);

    if (OC_STACK_OK != result)
    {
        dlog_print(DLOG_INFO, "MaintenanceResource", "#### Resource creation"
                   "(maintenance) was unsuccessful");
        return;
    }

    std::thread exec(
        std::function< void(int second) >(
            std::bind(&MaintenanceResource::maintenanceMonitor, this,
                      std::placeholders::_1)), 1);
    exec.detach();

    dlog_print(DLOG_INFO, "MaintenanceResource", "#### maintenance Resource is Created");
}

void MaintenanceResource::setMaintenanceRepresentation(OCRepresentation &rep)
{
    std::string value;

    if (rep.getValue(DEFAULT_FACTORYRESET, value))
    {
        m_factoryReset = value;
        dlog_print(DLOG_INFO, "MaintenanceResource", "#### m_factoryReset: %s",
                   m_factoryReset.c_str());
    }

    if (rep.getValue(DEFAULT_REBOOT, value))
    {
        m_reboot = value;
        dlog_print(DLOG_INFO, "MaintenanceResource", "#### m_reboot: %s", m_reboot.c_str());
    }

    if (rep.getValue(DEFAULT_STARTCOLLECTION, value))
    {
        m_startStatCollection = value;
        dlog_print(DLOG_INFO, "MaintenanceResource", "#### m_startStatCollection: %s",
                   m_startStatCollection.c_str());
    }
}

OCRepresentation MaintenanceResource::getMaintenanceRepresentation()
{
    m_maintenanceRep.setValue(DEFAULT_FACTORYRESET, m_factoryReset);
    m_maintenanceRep.setValue(DEFAULT_REBOOT, m_reboot);
    m_maintenanceRep.setValue(DEFAULT_STARTCOLLECTION, m_startStatCollection);

    return m_maintenanceRep;
}

std::string MaintenanceResource::getUri()
{
    return m_maintenanceUri;
}

// Handles the Reboot and FactoryReset request
void MaintenanceResource::maintenanceMonitor(int second)
{
    while (1)
    {
        sleep(second);

        if (m_reboot == "true")
        {
            int res;
            dlog_print(DLOG_INFO, "MaintenanceResource", "#### Reboot will be soon...");
            m_reboot = defaultReboot;
            res = system("sudo reboot"); // System reboot

            dlog_print(DLOG_INFO, "MaintenanceResource", "#### return: %d", res);

            logMessage = "----------------------------<br>";
            logMessage += "*** System Reboot Success ***<br>";

            dlog_print(DLOG_INFO, LOG_TAG, "  %s", logMessage.c_str());
            //Show the log
            ecore_main_loop_thread_safe_call_sync(updateLog, &logMessage);

        }
        else if (m_factoryReset == "true")
        {
            dlog_print(DLOG_INFO, "MaintenanceResource", "#### Factory Reset will be soon...");
            m_factoryReset = defaultFactoryReset;
            factoryReset();
        }
    }
}

// Deletes the diagnostic resource which has been created using createResource()
void MaintenanceResource::deleteResource()
{
    // Unregister the resource
    if (NULL != m_maintenanceHandle)
    {
        OCPlatform::unregisterResource(m_maintenanceHandle);
    }
}
