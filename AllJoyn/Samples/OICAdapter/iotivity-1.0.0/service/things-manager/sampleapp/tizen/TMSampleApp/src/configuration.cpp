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

#include "tmsampleapp.h"
#include "tmutil.h"

using namespace std;
using namespace OC;
using namespace OIC;

int isWaiting = 0; // 0: none to wait, 1: wait for the response of "getConfigurationValue"
const int SUCCESS_RESPONSE = 0;

OCResourceHandle configurationCollectionHandle = NULL;
OCResourceHandle configurationFoundHandle = NULL;
std::shared_ptr< OCResource > g_configurationCollection; // For a group of multiple resources
std::shared_ptr< OCResource > g_configurationResource; // For a single resource

OCResourceHandle maintenanceCollectionHandle = NULL;
OCResourceHandle maintenanceFoundHandle = NULL;
std::shared_ptr< OCResource > g_maintenanceCollection; // For a group of multiple resources
std::shared_ptr< OCResource > g_maintenanceResource; // For a single resource

OCResourceHandle setCollectionHandle = NULL;
OCResourceHandle setFoundHandle = NULL;
std::shared_ptr< OCResource > g_setCollection; // For a group of multiple resources
std::shared_ptr< OCResource > g_setResource; // For a single resource

std::map< std::string, std::shared_ptr< OCResource > > resourceTable;
std::vector< OCResourceHandle > configResourceHandleVector;

typedef std::string ConfigurationName;
typedef std::string ConfigurationValue;

static Evas_Object *log_entry = NULL;

string CONFIGURATION_COLLECTION_RESOURCE_URI  = "/core/a/configuration/resourceset";
string CONFIGURATION_COLLECTION_RESOURCE_TYPE = "core.configuration.resourceset";
string MAINTENANCE_COLLECTION_RESOURCE_URI     = "/core/a/maintenance/resourceset";
string MAINTENANCE_COLLECTION_RESOURCE_TYPE    = "core.maintenance.resourceset";
string FACTORYSET_COLLECTION_RESOURCE_URI     = "/core/a/factoryset/resourceset";
string FACTORYSET_COLLECTION_RESOURCE_TYPE    = "core.factoryset.resourceset";

string CONFIGURATION_RESOURCE_URI             = "/oic/con";
string MAINTENANCE_RESOURCE_URI                = "/oic/mnt";
string FACTORYSET_RESOURCE_URI                = "/factoryset";

GroupManager *g_groupManager = nullptr;
ThingsConfiguration *g_thingsConfig = nullptr;
ThingsMaintenance *g_thingsMnt = nullptr;


typedef struct region_popup
{
    Evas_Object *popup;
    Evas_Object *entry;
} region_popup_fields;

void *updateConfigLog(void *data)
{
    string *log = (string *)data;
    // Show the log
    elm_entry_entry_append(log_entry, (*log).c_str());
    elm_entry_cursor_end_set(log_entry);
    return NULL;
}

// Callback to found collection resource
void onFoundCollectionResource(std::vector< std::shared_ptr< OCResource > > resources)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onFoundCollectionResource ENTRY!!!!");
    try
    {
        // Do some operations with resource object.
        for (unsigned int i = 0; i < resources.size(); ++i)
        {
            string logMessage;
            std::shared_ptr< OCResource > resource = resources.at(i);
            if (resource)
            {
                string resourceURI = resource->host();
                string hostAddress = resource->uri();
                logMessage = "FoundHost: " + resource->host() + "<br>";
                logMessage += "FoundUri : " + resource->uri() + "<br>";
                logMessage += "----------------------<br>";
                dlog_print(DLOG_INFO, LOG_TAG, "FoundHost: %s", resourceURI.c_str());
                dlog_print(DLOG_INFO, LOG_TAG, "FoundUri : %s", hostAddress.c_str());
                dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
                ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog,
                                                      &logMessage);

                if (resource->uri() == CONFIGURATION_COLLECTION_RESOURCE_URI)
                    g_configurationCollection = resource;
                else if (resource->uri() == MAINTENANCE_COLLECTION_RESOURCE_URI)
                    g_maintenanceCollection = resource;
                else if (resource->uri() == FACTORYSET_COLLECTION_RESOURCE_URI)
                    g_setCollection = resource;
                else
                {
                    isWaiting = 0;
                    return;
                }
            }
            else
            {
                // Resource is invalid

                logMessage = "Found Resource invalid!";
                dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
                ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog,
                                                      &logMessage);
            }
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }
    isWaiting = 0;
    dlog_print(DLOG_INFO, LOG_TAG, "#### onFoundCollectionResource EXIT!!!!");
}

// Callback to found candidate resources
void onFoundCandidateResource(std::vector< std::shared_ptr< OCResource > > resources)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onFoundCandidateCollection ENTRY!!!!");
    try
    {
        // Do some operations with resource object.
        for (unsigned int i = 0; i < resources.size(); ++i)
        {
            string logMessage;
            std::shared_ptr< OCResource > resource = resources.at(i);
            if (resource)
            {
                // Check if the resource is new one. If so, store it.
                std::map< std::string, std::shared_ptr< OCResource > >::iterator iter =
                    resourceTable.find(resource->host() + resource->uri());

                if (iter == resourceTable.end()) // new one
                {
                    resourceTable[resource->host() + resource->uri()] = resource;

                    OCResourceHandle foundResourceHandle = NULL;
                    OCStackResult result = OCPlatform::registerResource(foundResourceHandle,
                                           resource);
                    // TODO: null check for foundResourceHandle
                    dlog_print(DLOG_INFO, LOG_TAG, "#### (%s) REGISTERED",
                               resource->host().c_str());
                    if (OC_STACK_OK == result)
                    {
                        string resourceURI = resource->host();
                        string hostAddress = resource->uri();
                        logMessage = "FoundHost: " + resource->host() + "<br>";
                        logMessage += "FoundUri: " + resource->uri()
                                      + " Registered <br>";
                        logMessage += "----------------------<br>";
                        dlog_print(DLOG_INFO, LOG_TAG, "Host: %s", resourceURI.c_str());
                        dlog_print(DLOG_INFO, LOG_TAG, "Uri : %s", hostAddress.c_str());
                        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
                        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog,
                                                              &logMessage);

                        if (resource->uri() == CONFIGURATION_RESOURCE_URI &&
                            NULL != configurationCollectionHandle)
                        {
                            OCPlatform::bindResource(configurationCollectionHandle,
                                                     foundResourceHandle);
                            configurationFoundHandle = foundResourceHandle;
                            if (NULL == g_configurationResource)
                            {
                                dlog_print(DLOG_INFO, LOG_TAG, "g_configurationResource updated");
                                g_configurationResource = resource;
                            }
                            else
                            {
                                dlog_print(DLOG_INFO, LOG_TAG,
                                           "g_configurationResource is not null");
                            }
                        }
                        else if (resource->uri() == MAINTENANCE_RESOURCE_URI &&
                                 NULL != maintenanceCollectionHandle)
                        {
                            OCPlatform::bindResource(maintenanceCollectionHandle,
                                                     foundResourceHandle);
                            maintenanceFoundHandle = foundResourceHandle;
                            if (NULL == g_maintenanceResource)
                            {
                                dlog_print(DLOG_INFO, LOG_TAG,
                                           "g_maintenanceResource updated");
                                g_maintenanceResource = resource;
                            }
                            else
                            {
                                dlog_print(DLOG_INFO, LOG_TAG,
                                           "g_maintenanceResource is not null");
                            }
                        }
                        else if (resource->uri() == FACTORYSET_RESOURCE_URI &&
                                 NULL != setCollectionHandle)
                        {
                            OCPlatform::bindResource(setCollectionHandle, foundResourceHandle);
                            setFoundHandle = foundResourceHandle;
                            if (NULL == g_setResource)
                            {
                                dlog_print(DLOG_INFO, LOG_TAG, "g_setResource updated");
                                g_setResource = resource;
                            }
                            else
                            {
                                dlog_print(DLOG_INFO, LOG_TAG, "g_setResource is not null");
                            }
                        }
                        configResourceHandleVector.push_back(foundResourceHandle);
                    }
                    else
                    {
                        logMessage = "Resource Error!";
                        dlog_print(DLOG_INFO, LOG_TAG, "Resource Error!");
                        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog,
                                                              &logMessage);
                    }
                }
            }
            else
            {
                // Resource is invalid
                logMessage = "Resource is invalid!";
                dlog_print(DLOG_INFO, LOG_TAG, "Resource is invalid!");
                ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog,
                                                      &logMessage);
            }
        }
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception occured! (%s)", e.what());
    }

    isWaiting = 0;
    dlog_print(DLOG_INFO, LOG_TAG, "#### onFoundCandidateCollection EXIT!!!!");
}

// Callback to updateConfiguration
static void onUpdateConfigurationsCallback(const HeaderOptions &headerOptions,
        const OCRepresentation &rep,
        const int eCode)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onUpdateConfigurationsCallback: ENTRY!!!!");
    isWaiting = 0;

    if (SUCCESS_RESPONSE != eCode)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "#### onUpdateConfigurationsCallback: "
                   "ERROR RESPONSE!!!!");
        return;
    }

    string logMessage = "Resource URI: " + rep.getUri() + "<br>";
    dlog_print(DLOG_INFO, LOG_TAG, "#### Resource URI: %s", rep.getUri().c_str());

    if (rep.hasAttribute(DEFAULT_DEVICENAME))
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Device Name : %s",
                   rep.getValue< std::string >(DEFAULT_DEVICENAME).c_str());
        logMessage = logMessage + "Device Name : " +
                     rep.getValue< std::string >(DEFAULT_DEVICENAME) + "<br>";
    }
    if (rep.hasAttribute(DEFAULT_LOCATION))
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Location : %s",
                   rep.getValue< std::string >(DEFAULT_LOCATION).c_str());
        logMessage = logMessage + "Location : " +
                     rep.getValue< std::string >(DEFAULT_LOCATION) + "<br>";
    }
    if (rep.hasAttribute(DEFAULT_LOCATIONNAME))
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Location Name : %s",
                   rep.getValue< std::string >(DEFAULT_LOCATIONNAME).c_str());
        logMessage = logMessage + "Location Name: " +
                     rep.getValue< std::string >(DEFAULT_LOCATIONNAME) + "<br>";
    }
    if (rep.hasAttribute(DEFAULT_CURRENCY))
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Currency : %s",
                   rep.getValue< std::string >(DEFAULT_CURRENCY).c_str());
        logMessage = logMessage + "Currency : " +
                     rep.getValue< std::string >(DEFAULT_CURRENCY) + "<br>";
    }
    if (rep.hasAttribute(DEFAULT_REGION))
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Region : %s",
                   rep.getValue< std::string >(DEFAULT_REGION).c_str());
        logMessage = logMessage + "Region : " +
                     rep.getValue< std::string >(DEFAULT_REGION) + "<br>";
    }
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog,
                                          &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### onUpdateConfigurationsCallback: EXIT!!!!");
}

// Callback to getConfiguration
static void onGetConfigurationsCallback(const HeaderOptions &headerOptions,
                                        const OCRepresentation &rep,
                                        const int eCode)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onGetConfigurationsCallback: enter!!!!");
    isWaiting = 0;

    if (SUCCESS_RESPONSE != eCode)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "#### onGetConfigurationsCallback: "
                   "ERROR RESPONSE!!!!");
        return ;
    }

    string logMessage = "Resource URI: " + rep.getUri() + "<br>";
    dlog_print(DLOG_INFO, LOG_TAG, "#### Resource URI: %s", rep.getUri().c_str());

    if (rep.hasAttribute(DEFAULT_DEVICENAME))
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Device Name : %s",
                   rep.getValue< std::string >(DEFAULT_DEVICENAME).c_str());
        logMessage = logMessage + "Device Name : "
                     + rep.getValue< std::string >(DEFAULT_DEVICENAME) + "<br>";
    }
    if (rep.hasAttribute(DEFAULT_LOCATION))
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Location : %s",
                   rep.getValue< std::string >(DEFAULT_LOCATION).c_str());
        logMessage = logMessage + "Location : "
                     + rep.getValue< std::string >(DEFAULT_LOCATION) + "<br>";
    }
    if (rep.hasAttribute(DEFAULT_LOCATIONNAME))
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Location Name : %s",
                   rep.getValue< std::string >(DEFAULT_LOCATIONNAME).c_str());
        logMessage = logMessage + "Location Name : "
                     + rep.getValue< std::string >(DEFAULT_LOCATIONNAME) + "<br>";
    }
    if (rep.hasAttribute(DEFAULT_CURRENCY))
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Currency : %s",
                   rep.getValue< std::string >(DEFAULT_CURRENCY).c_str());
        logMessage = logMessage + "Currency : "
                     + rep.getValue< std::string >(DEFAULT_CURRENCY) + "<br>";
    }
    if (rep.hasAttribute(DEFAULT_REGION))
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Region : %s",
                   rep.getValue< std::string >(DEFAULT_REGION).c_str());
        logMessage = logMessage + "Region : "
                     + rep.getValue< std::string >(DEFAULT_REGION) + "<br>";
    }

    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### onGetConfigurationsCallback: exit!!!!");
}

static void onFactoryReset(const HeaderOptions &headerOptions, const OCRepresentation &rep,
                           const int eCode)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onFactoryReset: enter!!!!");
    isWaiting = 0;

    if (SUCCESS_RESPONSE != eCode)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "#### onFactoryReset: ERROR RESPONSE!!!!");
        return ;
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### Resource URI: %s", rep.getUri().c_str());
    dlog_print(DLOG_INFO, LOG_TAG, "#### FactoryReset : %s",
               rep.getValue< std::string >("value").c_str());
    string logMessage = "Resource URI : " + rep.getUri() + "<br>";
    logMessage = logMessage + "FactoryReset : " +
                 rep.getValue< std::string >("value") + "<br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### onFactoryReset: exit!!!!");
}

static void onReboot(const HeaderOptions &headerOptions, const OCRepresentation &rep,
                     const int eCode)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### onReboot: enter!!!!");
    isWaiting = 0;

    if (SUCCESS_RESPONSE != eCode)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "#### onReboot: ERROR RESPONSE!!!!");
        return ;
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### Resource URI: %s", rep.getUri().c_str());
    dlog_print(DLOG_INFO, LOG_TAG, "#### Reboot : %s",
               rep.getValue< std::string >("value").c_str());
    string logMessage = "Resource URI : " + rep.getUri() + "<br>";
    logMessage = logMessage + "Reboot : " +
                 rep.getValue< std::string >("value") + "<br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### onReboot: exit!!!!");
}

static void createResourceCollection(string uri, string typeName, OCResourceHandle &tempHandle)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### createResourceCollection: enter!!!!");
    if (NULL != tempHandle)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Resource already exists!!!!");
        return;
    }

    // Create resource collection
    OCPlatform::registerResource(tempHandle, uri, typeName, BATCH_INTERFACE, NULL,
                                 OC_DISCOVERABLE);
    OCPlatform::bindInterfaceToResource(tempHandle, GROUP_INTERFACE);
    OCPlatform::bindInterfaceToResource(tempHandle, DEFAULT_INTERFACE);

    dlog_print(DLOG_INFO, LOG_TAG, "#### Resource created : %s", typeName.c_str());
    string logMessage;
    logMessage = "Resource created : <br>" + typeName + "<br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### createResourceCollection: exit!!!!");
}

// Unbinds and unregisters all resources
static void deleteResource(OCResourceHandle &tempCollectionHandle,
                           OCResourceHandle &tempResourceHandle)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### calling deleteResources ENTRY!!!!");

    if (NULL == tempCollectionHandle || NULL == tempResourceHandle)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Collection or resource does not exists");
        return;
    }

    try
    {
        OCPlatform::unbindResource(tempCollectionHandle, tempResourceHandle);
        dlog_print(DLOG_INFO, LOG_TAG, "#### unbindResource DONE!!!!");
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception Occured! (%s)", e.what());
    }

    try
    {
        OCPlatform::unregisterResource(tempResourceHandle);
        dlog_print(DLOG_INFO, LOG_TAG, "#### unregisterResource DONE!!!!");
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception Occured! (%s)", e.what());
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### calling deleteResources EXIT!!!!");
}

static void findAllGroups(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### calling findCandidateResources ENTRY!!!!");
    std::vector<string> resourceTypes;
    resourceTypes.push_back(CONFIGURATION_COLLECTION_RESOURCE_TYPE);

    if (NULL != g_groupManager)
    {
        g_groupManager->findCandidateResources(resourceTypes, &onFoundCollectionResource,
                                               FINDGROUP_TIMEOUT);
    }

    resourceTypes.clear();
    resourceTypes.push_back(MAINTENANCE_COLLECTION_RESOURCE_TYPE);
    if (NULL != g_groupManager)
    {
        g_groupManager->findCandidateResources(resourceTypes, &onFoundCollectionResource,
                                               FINDGROUP_TIMEOUT);
    }

    resourceTypes.clear();
    resourceTypes.push_back(FACTORYSET_COLLECTION_RESOURCE_TYPE);
    if (NULL != g_groupManager)
    {
        g_groupManager->findCandidateResources(resourceTypes, &onFoundCollectionResource,
                                               FINDGROUP_TIMEOUT);
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### calling findCandidateResources EXIT!!!!");
}

static void findAllResources(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### calling findCandidateResources ENTRY!!!!");
    std::vector<string> resourceTypes;
    resourceTypes.push_back("oic.wk.con");

    if (NULL != g_groupManager)
    {
        g_groupManager->findCandidateResources(resourceTypes, &onFoundCandidateResource,
                                               FINDRESOURCE_TIMEOUT);
    }

    resourceTypes.clear();
    resourceTypes.push_back("oic.wk.mnt");
    if (NULL != g_groupManager)
    {
        g_groupManager->findCandidateResources(resourceTypes, &onFoundCandidateResource,
                                               FINDRESOURCE_TIMEOUT);
    }

    resourceTypes.clear();
    resourceTypes.push_back("factoryset");
    if (NULL != g_groupManager)
    {
        g_groupManager->findCandidateResources(resourceTypes, &onFoundCandidateResource,
                                               FINDRESOURCE_TIMEOUT);
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### calling findCandidateResources EXIT!!!!");
}

static void getConfiguration(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### getConfiguration ENTRY!!!!");
    if (NULL == g_configurationCollection || NULL == g_configurationCollection.get())
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Note that you first create a group to use this command");
        string logMessage = "FIRST CREATE GROUP <br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
        return;
    }

    ConfigurationName name = "all";
    std::vector< ConfigurationName > configurations;
    configurations.push_back(name);

    try
    {
        g_thingsConfig->getConfigurations(g_configurationCollection, configurations,
                                          &onGetConfigurationsCallback);

        isWaiting = 0;
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception is occured! (%s)", e.what());
    }

    isWaiting = 1;
    dlog_print(DLOG_INFO, LOG_TAG, "#### getConfiguration EXIT!!!!");
}

// Updates the configuration i.e. region value to INDIA
static void updateConfiguration(std::string newRegionValue)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### updateConfiguration ENTRY!!!!");
    dlog_print(DLOG_INFO, LOG_TAG, "#### %s", newRegionValue.c_str());

    if (NULL == g_configurationCollection || NULL == g_configurationCollection.get())
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Note that you first create a group to use this command");
        string logMessage = "FIRST CREATE GROUP <br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
        return;
    }

    OCStackResult result;
    ConfigurationName name = DEFAULT_REGION;
    ConfigurationValue value = newRegionValue;

    std::map< ConfigurationName, ConfigurationValue > configurations;
    configurations.insert(std::make_pair(name, value));

    try
    {
        result = g_thingsConfig->updateConfigurations(g_configurationCollection, configurations,
                 &onUpdateConfigurationsCallback);
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception is occured! (%s)", e.what());
    }

    if (OC_STACK_OK == result)
    {
        string logMessage = "UpdateConfigurations called successfully<br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### updateConfiguration EXIT!!!!");
}

// This method will reset all the configuration attributes to their default values
static void factoryReset(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### factoryReset ENTRY!!!!");
    if (NULL == g_maintenanceCollection || NULL == g_maintenanceCollection.get())
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Note that you first create a group to use this command");
        string logMessage = "FIRST CREATE GROUP <br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
        return;
    }

    OCStackResult result;

    try
    {
        result = g_thingsMnt->factoryReset(g_maintenanceCollection, &onFactoryReset);
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception is occured! (%s)", e.what());
    }

    if (OC_STACK_OK == result)
    {
        string logMessage = "FactoryReset called successfully<br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
    }

    dlog_print(DLOG_INFO, LOG_TAG, "#### factoryReset EXIT!!!!");
}

// Reboots the server
static void reboot(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### reboot ENTRY!!!!");
    if (NULL == g_maintenanceCollection || NULL == g_maintenanceCollection.get())
    {
        dlog_print(DLOG_INFO, LOG_TAG, "Note that you first create a group to use this command");
        string logMessage = "FIRST CREATE GROUP <br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
        return;
    }

    OCStackResult result;

    try
    {
        result = g_thingsMnt->reboot(g_maintenanceCollection, &onReboot);
    }
    catch (std::exception &e)
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Exception is occured! (%s)", e.what());
    }

    if (OC_STACK_OK == result)
    {
        string logMessage = "Reboot called successfully<br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
    }
    dlog_print(DLOG_INFO, LOG_TAG, "#### reboot EXIT!!!!");
}

/* For getting list of all supported configuration Values it will give all
   configuration in JSON format (key-value pair) */
static void getListOfSupportedConfigurationUnits(void *data, Evas_Object *obj, void *event_info)
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### getListOfSupportedConfigurationUnits ENTRY!!!!");
    string listOfSupportedConfigurationUnits =
        g_thingsConfig->getListOfSupportedConfigurationUnits();
    dlog_print(DLOG_INFO, LOG_TAG, "#### List : %s", listOfSupportedConfigurationUnits.c_str());

    string logMessage;
    logMessage = "Supported Configuration List :<br>" + listOfSupportedConfigurationUnits + "<br>";
    logMessage += "----------------------<br>";
    dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
    ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
    dlog_print(DLOG_INFO, LOG_TAG, "#### getListOfSupportedConfigurationUnits EXIT!!!!");
}

// Creates all the resources
static void onStartConfigure()
{
    createResourceCollection(CONFIGURATION_COLLECTION_RESOURCE_URI,
                             CONFIGURATION_COLLECTION_RESOURCE_TYPE,
                             configurationCollectionHandle);
    createResourceCollection(MAINTENANCE_COLLECTION_RESOURCE_URI,
                             MAINTENANCE_COLLECTION_RESOURCE_TYPE,
                             maintenanceCollectionHandle);
    createResourceCollection(FACTORYSET_COLLECTION_RESOURCE_URI,
                             FACTORYSET_COLLECTION_RESOURCE_TYPE,
                             setCollectionHandle);

    g_groupManager = new GroupManager();
    g_thingsConfig = new ThingsConfiguration();
    g_thingsMnt = new ThingsMaintenance();
}

// Deletes all the resources
static void onDestroyConfigure()
{
    dlog_print(DLOG_INFO, LOG_TAG, "#### Destroy sequence called");

    deleteResource(setCollectionHandle, setFoundHandle);

    deleteResource(maintenanceCollectionHandle, maintenanceFoundHandle);

    deleteResource(configurationCollectionHandle, configurationFoundHandle);

    delete g_thingsMnt;
    delete g_thingsConfig;
    delete g_groupManager;

    dlog_print(DLOG_INFO, LOG_TAG, "#### Resources destroyed successfully");
}

static void
popup_cancel_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    region_popup_fields *popup_fields = (region_popup_fields *)data;
    evas_object_del(popup_fields->popup);
    free(popup_fields);
}

static void
popup_set_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    region_popup_fields *popup_fields = (region_popup_fields *)data;
    Evas_Object *entry = popup_fields->entry;
    const char *newRegionValue = elm_entry_entry_get(entry);
    if (NULL == newRegionValue || strlen(newRegionValue) < 1)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Read NULL RegionValue");
        string logMessage = "Region Value should not be NULL<br>";
        logMessage += "----------------------<br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
    }
    else
    {
        std::string regionValue = std::string(newRegionValue);
        updateConfiguration(regionValue);
    }
    evas_object_del(popup_fields->popup);
    free(popup_fields);
}

static void
list_update_region_cb(void *data, Evas_Object *obj, void *event_info)
{
    if (NULL == g_configurationCollection || NULL == g_configurationCollection.get())
    {
        dlog_print(DLOG_ERROR, LOG_TAG, "Note that you first create a group to use this command");
        string logMessage = "FIRST CREATE GROUP <br>";
        dlog_print(DLOG_INFO, LOG_TAG, " %s", logMessage.c_str());
        ecore_main_loop_thread_safe_call_sync((void * ( *)(void *))updateConfigLog, &logMessage);
        return;
    }

    Evas_Object *popup, *btn;
    Evas_Object *nf = (Evas_Object *)data;
    Evas_Object *entry;
    Evas_Object *layout;

    /* popup */
    popup = elm_popup_add(nf);
    elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
    eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_part_text_set(popup, "title,text", "Enter New Region Value");

    layout = elm_layout_add(popup);
    elm_layout_file_set(layout, ELM_DEMO_EDJ, "popup_region_text");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_content_set(popup, layout);

    entry = elm_entry_add(layout);
    elm_entry_single_line_set(entry, EINA_TRUE);
    elm_entry_scrollable_set(entry, EINA_TRUE);
    evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
    eext_entry_selection_back_event_allow_set(entry, EINA_TRUE);
    elm_object_part_text_set(entry, "elm.guide", "region value");
    elm_object_part_content_set(layout, "elm.swallow.content" , entry);

    region_popup_fields *popup_fields;
    popup_fields = (region_popup_fields *)malloc(sizeof(region_popup_fields));
    if (NULL == popup_fields)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "#### Memory allocation failed");
    }
    else
    {
        popup_fields->popup = popup;
        popup_fields->entry = entry;
    }

    /* Cancel button */
    btn = elm_button_add(popup);
    elm_object_style_set(btn, "popup");
    elm_object_text_set(btn, "Cancel");
    elm_object_part_content_set(popup, "button1", btn);
    evas_object_smart_callback_add(btn, "clicked", popup_cancel_clicked_cb, popup_fields);

    /* Set button */
    btn = elm_button_add(popup);
    elm_object_style_set(btn, "popup");
    elm_object_text_set(btn, "Set");
    elm_object_part_content_set(popup, "button2", btn);
    evas_object_smart_callback_add(btn, "clicked", popup_set_clicked_cb, popup_fields);

    evas_object_show(popup);
}

static void
list_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *it = (Elm_Object_Item *)event_info;
    elm_list_item_selected_set(it, EINA_FALSE);
}

void
test_fn(void *data, Evas_Object *obj, void *event_info)
{
    if (NULL != log_entry)
    {
        elm_entry_entry_append(log_entry, "First item selected");
        elm_entry_cursor_end_set(log_entry);
    }
    else
    {
        dlog_print(DLOG_ERROR, "test_fn", "log_entry object is NULL");
    }
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
    onDestroyConfigure();
    resourceTable.clear();
    if (NULL != log_entry)
    {
        evas_object_del(log_entry);
        log_entry = NULL;
    }
    return EINA_TRUE;
}

// Method to be called when configuration API UI button is selected
void
configuration_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *layout;
    Evas_Object *scroller;
    Evas_Object *list;
    Evas_Object *nf = (Evas_Object *)data;
    Elm_Object_Item *nf_it;

    // Scroller
    scroller = elm_scroller_add(nf);
    elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
    elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

    // Layout
    layout = elm_layout_add(nf);
    elm_layout_file_set(layout, ELM_DEMO_EDJ, "configuration_layout");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    elm_object_content_set(scroller, layout);

    // List
    list = elm_list_add(layout);
    elm_list_mode_set(list, ELM_LIST_COMPRESS);
    evas_object_smart_callback_add(list, "selected", list_selected_cb, NULL);
    elm_object_part_content_set(layout, "list", list);
    elm_list_go(list);

    // log_entry - text area for log
    log_entry = elm_entry_add(layout);
    elm_entry_scrollable_set(log_entry, EINA_TRUE);
    elm_entry_editable_set(log_entry, EINA_FALSE);
    elm_object_part_text_set(log_entry, "elm.guide", "logs will be updated here!!!");
    evas_object_size_hint_weight_set(log_entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(log_entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_content_set(layout, "log", log_entry);

    nf_it = elm_naviframe_item_push(nf, "configuration APIs", NULL, NULL, scroller, NULL);
    elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, NULL);

    onStartConfigure();

    // Shows the UI list of group APIs
    elm_list_item_append(list, "Find All Groups", NULL, NULL, findAllGroups, NULL);
    elm_list_item_append(list, "Find All Resources",
                         NULL, NULL, findAllResources, NULL);
    elm_list_item_append(list, "Get a Configuration Resource", NULL, NULL, getConfiguration, NULL);
    elm_list_item_append(list, "Update Attribute (Region)", NULL, NULL,
                         list_update_region_cb, nf);
    elm_list_item_append(list, "Factory Reset", NULL, NULL, factoryReset, NULL);
    elm_list_item_append(list, "Reboot", NULL, NULL, reboot, NULL);
    elm_list_item_append(list, "Get Supported Configuration Units", NULL, NULL,
                         getListOfSupportedConfigurationUnits, NULL);

    elm_list_go(list);
}
